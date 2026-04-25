# mpk_mix

A C++20 general-purpose utility library.

## Requirements

- CMake 3.20+, Ninja
- GCC 14+ or Clang 19+

## Build

```bash
git submodule update --init --recursive

cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug -GNinja
cmake --build build/Debug --parallel $(nproc)
cd build/Debug && ctest --output-on-failure --parallel $(nproc)
```

## Consuming the library

### As a git submodule

```cmake
# In your 3p/CMakeLists.txt — add your own deps first, then mpk_mix.
# mpk_mix will skip any dep whose CMake target already exists.
add_subdirectory(quill)      # defines quill::quill — mpk_mix will reuse it
add_subdirectory(mpk_mix)    # defines mpk_mix::headers, mpk_mix::value, …
```

### Via Conan

```python
# conanfile.py
def requirements(self):
    self.requires("mpk_mix/0.1.0")
```

```cmake
find_package(mpk_mix REQUIRED)
target_link_libraries(my_target PRIVATE mpk_mix::headers)
```

## CMake targets

| Target | Contents | External deps |
|--------|----------|---------------|
| `mpk_mix::headers` | Meta, strong types, FuncRef, utilities — header-only | none |
| `mpk_mix::value` | Runtime type/value system | `magic_enum` |
| `mpk_mix::log` | Logging wrapper (quill / cerr / none) | `quill` (optional) |
| `mpk_mix::serial` | YAML and JSON helpers | `yaml-cpp`, `nlohmann_json` |
| `mpk_mix::all` | All of the above | all above |

## Namespace

```
mpk::mix          — the vast majority of things
mpk::mix::value   — runtime type/value system
mpk::mix::serial  — YAML and JSON serialization
```

## Naming conventions

- Classes and types: `CamelCase`
- Functions and methods: `snake_case`
- Header files: `snake_case.hpp`
- Private members: `trailing_underscore_`
- Macro prefix: `MPKMIX_`
