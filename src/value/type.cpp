/** @file
 * @brief Type — runtime type descriptor implementation.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/value/type.hpp"

#include "magic_enum/magic_enum.hpp"

#include <algorithm>
#include <cstring>
#include <set>


using namespace std::string_view_literals;

namespace mpk::mix::value {

namespace {

auto interned_types()
    -> std::set<Type>&
{
    static auto result = std::set<Type>{};
    return result;
}

[[maybe_unused]]
auto type_storage_size(const Type* t)
    -> size_t
{ return static_cast<uint8_t>(t->storage()[0]); }

[[maybe_unused]]
auto agg_type(const Type* t)
    -> AggregateType
{ return static_cast<AggregateType>(t->storage()[1]); }

template <typename T>
auto get_pointer(const std::byte* data)
{
    T* result;
    memcpy(&result, data, sizeof(T*));
    return result;
}

auto print_type(std::ostream& s, const Type* t)
    -> void;

struct AggregatePrinter final
{
    std::ostream& s;

    AggregatePrinter(std::ostream& s) :
        s{s}
    {}

    auto operator()(const ArrayT& t)
        -> void
    {
        s << "Array<" << static_cast<size_t>(t.element_count()) << ">[";
        print_type(s, t.element_type());
        s << ']';
    }

    auto operator()(const CustomT& t)
        -> void
    {
        s << "Custom<" << t.name()
          << ": " << static_cast<uint32_t>(t.id()) << '>';
    }

    auto operator()(const EnumT& t) const
        -> void
    {
        s << "Enum<" << t.name()
          << ": " << static_cast<uint32_t>(t.id()) << '>';
    }

    auto operator()(const PathT&)
        -> void
    { s << "Path"; }

    auto operator()(const ScalarT& t)
        -> void
    { s << t.name(); }

    auto operator()(const SetT& t)
        -> void
    {
        s << t.name() << '{';
        print_type(s, t.key_type());
        s << '}';
    }

    auto operator()(const StringT& t)
        -> void
    { s << t.name(); }

    auto operator()(const StrongT& t)
        -> void
    {
        s << "Strong{";
        print_type(s, t.weak_type());
        s << '}';
    }

    auto operator()(const StructT& t)
        -> void
    {
        auto names = t.field_names();
        auto types = t.tuple().element_types();
        auto size = names.size();
        assert(types.size() == size);
        s << "Struct{";
        auto delim = ""sv;
        for (size_t i=0; i<size; ++i)
        {
            s << delim << names[i] << ": ";
            print_type(s, types[i]);
            delim = ", "sv;
        }
        s << '}';
    }

    auto operator()(const TupleT& t)
        -> void
    {
        auto types = t.element_types();
        s << "Tuple{";
        auto delim = ""sv;
        for (const auto* type : types)
        {
            s << delim;
            print_type(s, type);
            delim = ", "sv;
        }
        s << '}';

    }

    auto operator()(const VectorT& t)
        -> void
    {
        s << "Vector[";
        print_type(s, t.element_type());
        s << ']';
    }
};

auto print_type(std::ostream& s, const Type* t)
    -> void
{
    auto printer = AggregatePrinter{s};
    visit(t, printer);
}

constexpr auto align_index(size_t alignment, size_t index)
    -> size_t
{ return (index + (alignment-1)) & ~(alignment-1); }

constexpr auto ptr_align_index(size_t index)
    -> size_t
{ return align_index(sizeof(void*), index); }

} // anonymous namespace


Type::Type(std::initializer_list<ByteInitializer> init,
           std::initializer_list<const Type*> bases,
           const std::string_view* names)
{
    auto total_bases_size =
        bases.size() * sizeof(const Type*);

    uint32_t names_size = names ? sizeof(names) : 0;

    auto ptrs_index = ptr_align_index(1 + init.size());

    auto size = ptrs_index + total_bases_size + names_size;

    assert(size <= max_size);
    storage_[0] = ByteInitializer{ static_cast<uint8_t>(size) };
    auto* d = storage_.data() + 1;
    std::copy(init.begin(), init.end(), d);
    d += init.size();

    std::fill(d, storage_.data() + ptrs_index, std::byte{});
    d = storage_.data() + ptrs_index;

    auto copy_ptr = [&](const void* ptr)
    {
        auto ptr_bytes = reinterpret_cast<const std::byte*>(&ptr);
        std::copy(ptr_bytes, ptr_bytes+sizeof(void*), d);
        d += sizeof(void*);
    };

    for (const auto* base : bases)
        copy_ptr(base);

    if (names)
        copy_ptr(names);

    assert(d - storage_.data() == static_cast<ptrdiff_t>(size));
    std::fill(d, storage_.data() + storage_.size(), std::byte{});
}

auto Type::intern(
    ValueComponentsAccessFactoryFunc value_component_access_factory,
    std::initializer_list<ByteInitializer> init,
    std::initializer_list<const Type*> bases,
    const std::string_view* names)
        -> const Type*
{
    auto& types = interned_types();
    auto [it, inserted] = types.emplace(
        mpk::mix::Impl, init, bases, names);

    const auto* result = &*it;

    if (inserted)
        result->value_component_access_ = value_component_access_factory();

    return result;
}

// ---

ArrayT::ArrayT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Array); }

auto ArrayT::type() const noexcept
    -> const Type*
{ return type_; }

auto ArrayT::element_count() const noexcept
    -> uint8_t
{ return static_cast<uint8_t>(type_->storage()[2]); }

auto ArrayT::element_type() const noexcept
    -> const Type*
{
    return get_pointer<const Type>(
        type_->storage().data() + ptr_align_index(3));
}


CustomT::CustomT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Custom); }

auto CustomT::type() const noexcept
    -> const Type*
{ return type_; }

auto CustomT::name() const noexcept
    -> std::string_view
{
    return *get_pointer<const std::string_view>(
        type_->storage().data() + ptr_align_index(3));
}

auto CustomT::id() const noexcept
    -> uint8_t
{ return static_cast<uint8_t>(type_->storage()[2]); }


EnumT::EnumT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Enum); }

auto EnumT::type() const noexcept
    -> const Type*
{ return type_; }

auto EnumT::name() const noexcept
    -> std::string_view
{
    return *get_pointer<const std::string_view>(
        type_->storage().data() + ptr_align_index(3));
}

auto EnumT::id() const noexcept
    -> uint8_t
{ return static_cast<uint8_t>(type_->storage()[2]); }


PathT::PathT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Path); }

auto PathT::type() const noexcept
    -> const Type*
{ return type_; }


ScalarT::ScalarT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Scalar); }

auto ScalarT::type() const noexcept
    -> const Type*
{ return type_; }

auto ScalarT::id() const noexcept
    -> ScalarTypeId
{ return static_cast<ScalarTypeId>(type_->storage()[2]); }

auto ScalarT::name() const noexcept
    -> std::string_view
{ return magic_enum::enum_name(id()); }


StringT::StringT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::String); }

auto StringT::type() const noexcept
    -> const Type*
{ return type_; }

auto StringT::id() const noexcept
    -> StringTypeId
{ return static_cast<StringTypeId>(type_->storage()[2]); }

auto StringT::name() const noexcept -> std::string_view
{ return magic_enum::enum_name(id()); }


SetT::SetT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Set); }

auto SetT::type() const noexcept
    -> const Type*
{ return type_; }

auto SetT::id() const noexcept
    -> SetTypeId
{ return static_cast<SetTypeId>(type_->storage()[2]); }

auto SetT::name() const noexcept -> std::string_view
{ return magic_enum::enum_name(id()); }

auto SetT::key_type() const noexcept
    -> const Type*
{
    return get_pointer<const Type>(
        type_->storage().data() + ptr_align_index(3));
}


StrongT::StrongT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Strong); }

auto StrongT::type() const noexcept
    -> const Type*
{ return type_; }

auto StrongT::weak_type() const noexcept
    -> const Type*
{
    return get_pointer<const Type>(
        type_->storage().data() + ptr_align_index(2));
}


StructT::StructT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Struct); }

auto StructT::type() const noexcept
    -> const Type*
{ return type_; }

auto StructT::tuple_type() const noexcept
    -> const Type*
{
    return get_pointer<const Type>(
        type_->storage().data() + ptr_align_index(3));
}

auto StructT::field_names() const noexcept
    -> std::span<const std::string_view>
{
    auto* names = get_pointer<const std::string_view>(
        type_->storage().data() + ptr_align_index(3) + sizeof(void*));
    return { names, tuple().element_count() };
}

auto StructT::tuple() const noexcept
    -> TupleT
{ return { tuple_type() }; }


TupleT::TupleT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Tuple); }

auto TupleT::type() const noexcept
    -> const Type*
{ return type_; }

auto TupleT::element_count() const noexcept
    -> uint8_t
{ return static_cast<uint8_t>(type_->storage()[2]); }

auto TupleT::element_types() const noexcept
    -> std::span<const Type* const>
{
    const Type* const* types = reinterpret_cast<const Type* const*>(
        type_->storage().data() + ptr_align_index(3));
    return { types, element_count() };
}


VectorT::VectorT(const Type* type) noexcept :
    type_{ type }
{ assert(agg_type(type_) == AggregateType::Vector); }

auto VectorT::type() const noexcept
    -> const Type*
{ return type_; }

auto VectorT::element_type() const noexcept
    -> const Type*
{
    return get_pointer<const Type>(
        type_->storage().data() + ptr_align_index(2));
}

// ---

auto operator<<(std::ostream& s, const Type* t)
    -> std::ostream&
{
    s << "Type{";
    print_type(s, t);
    s << "}";
    return s;
}

} // namespace mpk::mix::value
