/** @file
 * @brief Value — implementation.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/value/value.hpp"

#include <sstream>


using namespace std::string_view_literals;

namespace mpk::mix::value {

namespace {

class ScalarFormatter final
{
public:
    template <typename T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
    auto operator()(mpk::mix::Type_Tag<T> tag, const Value& v) const
        -> std::string
    {
        if constexpr (sizeof(T) == 1)
            return mpk::mix::format("{}", v.convert_to<int>());

        return mpk::mix::format("{}", v.as(tag));
    }

    auto operator()(mpk::mix::Type_Tag<bool> tag, const Value& v) const
        -> std::string
    {
        auto b = v.as(tag);
        return b ? "true" : "false";
    }

    auto operator()(mpk::mix::Type_Tag<std::byte> tag, const Value& v) const
        -> std::string
    {
        auto b = static_cast<uint8_t>(v.as(tag));
        constexpr auto hex_chars = "0123456789abcdef";
        auto result = std::string("xx");
        result[0] = hex_chars[b>>4];
        result[1] = hex_chars[b&0xf];
        return result;
    }
};

class ValueFormatter final
{
public:
    explicit ValueFormatter(const Value& value) :
        result_{ value.type() ? visit(value.type(), *this, value) : "<empty>" }
    {}

    auto operator()(const ArrayT& t, const Value& value) const
        -> std::string
    { return format_seq( t, value, value.size(), '[', ']' ); }

    auto operator()(const CustomT&, const Value&) const
        -> std::string
    { return "custom"; }

    auto operator()(const EnumT&, const Value& value) const
        -> std::string
    { return value.get(ValuePath{ "name"sv }).convert_to<std::string>(); }

    auto operator()(const PathT&, const Value& value) const
        -> std::string
    { return mpk::mix::format("{}", value.as<ValuePath>()); }

    auto operator()(const ScalarT& t, const Value& value) const
        -> std::string
    { return t.visit(ScalarFormatter{}, value); }

    auto operator()(const SetT&, const Value& value) const
        -> std::string
    {
        auto keys = value.keys();
        std::ostringstream s;
        s << '{';
        auto delim = ""sv;
        for (const auto& k : keys)
        {
            s << delim << std::string{ ValueFormatter{k} };
            delim = ", "sv;
        }
        s << '}';
        return s.str();
    }

    auto operator()(const StringT&, const Value& value) const
        -> std::string
    { return value.convert_to<std::string>(); }

    auto operator()(const StrongT&, const Value& value) const
        -> std::string
    {
        // Format as weakly-typed value - TODO better
        return ValueFormatter{ value.get( ValuePath{ "v"sv } ) };
    }

    auto operator()(const StructT& t, const Value& value) const
        -> std::string
    {
        auto field_names = t.field_names();
        auto tuple = t.tuple();
        auto n = tuple.element_count();
        std::ostringstream s;
        s << '{';
        auto delim = "";
        for (uint8_t i=0; i<n; ++i, delim=",")
        {
            auto field_name = field_names[i];
            auto field = value.get(ValuePath{field_name});
            s << delim << field_name
              << '=' << std::string{ ValueFormatter{ field } };
        }
        s << '}';
        return s.str();
    }

    auto operator()(const TupleT& t, const Value& value) const
        -> std::string
    { return format_seq( t, value, t.element_count(), '{', '}' ); }

    auto operator()(const VectorT& t, const Value& value) const
        -> std::string
    { return format_seq( t, value, value.size(), '[', ']' ); }

    operator std::string() && noexcept
    { return result_; }

private:
    template <typename SecT>
    auto format_seq(const SecT&,
                    const Value& value,
                    size_t size,
                    char open_brace,
                    char close_brace) const
        -> std::string
    {
        std::ostringstream s;
        s << open_brace;
        auto delim = "";
        for (size_t i=0; i<size; ++i, delim=",")
        {
            auto element = value.get(ValuePath{i});
            s << delim << std::string{ ValueFormatter{ element } };
        }
        s << close_brace;
        return s.str();
    }

    std::string result_;
};

} // anonymous namespace


Value::Value() noexcept = default;

Value::Value(mpk::mix::Type_Tag<std::string> tag, std::string_view value) :
    Value(tag, std::string(value))
{}

Value::Value(const Value&) = default;
Value::Value(Value&&) = default;

Value::Value(Value&) = default;

auto Value::operator=(const Value&) -> Value& = default;
auto Value::operator=(Value&&) -> Value& = default;


auto Value::type() const noexcept
    -> const Type*
{ return type_; }

auto Value::data() noexcept
    -> std::any&
{ return data_; }

auto Value::data() const noexcept
    -> const std::any&
{ return data_; }


auto Value::path_item_keys() const
    -> std::vector<ValuePathItem>
{ return type_->value_component_access()->path_intem_keys(data_); }

auto Value::get(ValuePathView path) const
    -> Value
{
    auto [t, d] = type_->value_component_access()->get(path, data_);
    return { t, std::move(d) };
}

auto Value::set(ValuePathView path, const Value& v)
    -> void
{ type_->value_component_access()->set(path, data_, v.data_); }

auto Value::set_default(ValuePathView path)
    -> void
{
    auto [t, _] = type_->value_component_access()->get(path, data_);
    auto d = t->value_component_access()->make_data();
    type_->value_component_access()->set(path, data_, d);
}

auto Value::set_default()
    -> void
{ set_default({}); }

auto Value::size(ValuePathView path) const
    -> size_t
{ return type_->value_component_access()->size(path, data_); }

auto Value::size() const
    -> size_t
{ return type_->value_component_access()->size({}, data_); }

auto Value::resize(ValuePathView path, size_t size)
    -> void
{ return type_->value_component_access()->resize(path, data_, size); }

auto Value::resize(size_t size)
    -> void
{ return resize({}, size); }

auto Value::keys() const -> std::vector<Value>
{
    auto [key_type, any_keys] = type_->value_component_access()->keys(data_);
    auto result = std::vector<Value>{};
    result.reserve(any_keys.size());
    std::ranges::transform(
        any_keys,
        back_inserter(result),
        [&](const std::any& k) { return Value{key_type, k}; });
    return result;
}

auto Value::contains(const Value& key) const -> bool
{ return type_->value_component_access()->contains(data_, key.data_); }

auto Value::insert(ValuePathView path, const Value& key) -> void
{ type_->value_component_access()->insert(path, data_, key.data_); }

auto Value::insert(const Value& key) -> void
{ type_->value_component_access()->insert({}, data_, key.data_); }

auto Value::remove(ValuePathView path, const Value& key) -> void
{ type_->value_component_access()->remove(path, data_, key.data_); }

auto Value::remove(const Value& key) -> void
{ type_->value_component_access()->remove({}, data_, key.data_); }

auto Value::operator==(const Value& that) const
    -> bool
{
    if (type_ != that.type_)
        return false;

    if (!type_)
        return true;

    return type_->value_component_access()->equal(data_, that.data_);
}

auto Value::make(const Type* type)
    -> Value
{ return { type, type->value_component_access()->make_data() }; }

Value::Value(const Type* type, std::any data) :
    type_{ type },
    data_{ std::move(data) }
{}

auto operator<<(std::ostream& s, const Value& v)
    -> std::ostream&
{
    s << std::string{ ValueFormatter{ v } };
    return s;
}

} // namespace mpk::mix::value
