# Meson Cheat Sheet

## Configure & Build

```bash
# Configure (creates build/)
meson setup build

# Configure with options
meson setup build -Dtests=true -Ddemos=true -Dbuildtype=debug

# Build
meson compile -C build -j16

# Reconfigure (change options without wiping build/)
meson configure build -Dtests=true

# Wipe and reconfigure
meson setup build --wipe

# Install
meson install -C build
meson install -C build --destdir /tmp/staging
```

## Testing

```bash
meson test -C build                     # run all tests
meson test -C build --print-errorlogs   # show output on failure
meson test -C build -v                  # verbose
```

## Build Options

```bash
meson configure build                   # show all options
meson configure build -Dfoo=bar         # change option
```

### ICL-specific options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `demos` | bool | false | Build demo programs |
| `apps` | bool | false | Build applications |
| `examples` | bool | false | Build examples |
| `tests` | bool | false | Build test suite |
| `native` | bool | false | -march=native |
| `qt` | feature | auto | Qt6 GUI |
| `opencv` | feature | auto | OpenCV |
| `opencl` | feature | auto | OpenCL acceleration |
| `cycles` | feature | auto | Blender Cycles |
| `bullet` | feature | auto | Bullet physics |

Feature values: `auto` (detect), `enabled` (require), `disabled` (skip)

```bash
meson setup build -Dqt=disabled -Dopencl=disabled
```

## Build Types

```bash
meson setup build -Dbuildtype=debug     # -O0 -g
meson setup build -Dbuildtype=release   # -O3
meson setup build -Dbuildtype=debugoptimized  # -O2 -g
```

## Introspection

```bash
meson introspect build --targets        # list all targets
meson introspect build --dependencies   # list found deps
meson introspect build --buildoptions   # all options as JSON
```

## Cross-compilation

```bash
meson setup build --cross-file cross_linux_arm64.txt
```

## Meson.build Syntax Quick Ref

```meson
# Variables
x = 'hello'
nums = [1, 2, 3]
d = {'key': 'value'}

# Conditionals
if foo.found()
  # ...
elif bar
  # ...
endif

# Loops
foreach name, deps : _demos
  executable(name, name + '.cpp', dependencies: deps)
endforeach

# Functions
dep = dependency('libfoo', required: false)
lib = shared_library('name', sources, dependencies: [dep])
exe = executable('name', 'main.cpp', dependencies: [dep])

# Dependency declaration (for downstream)
my_dep = declare_dependency(
  link_with: lib,
  include_directories: inc,
)

# Pkg-config generation
pkg = import('pkgconfig')
pkg.generate(lib, name: 'mylib')

# Qt MOC
qt6 = import('qt6')
moc = qt6.compile_moc(headers: files('Widget.h'), dependencies: qt_dep)

# Generated files
conf = configuration_data()
conf.set('VERSION', '1.0')
configure_file(output: 'config.h', configuration: conf)

# Custom command
custom_target('name',
  input: 'input.cl',
  output: 'output.h',
  command: [prog, '@INPUT@', '@OUTPUT@'],
)
```

## Key Differences from CMake

| CMake | Meson |
|-------|-------|
| `cmake -B build` | `meson setup build` |
| `cmake --build build` | `meson compile -C build` |
| `ctest` | `meson test -C build` |
| `option(FOO "" ON)` | `option('foo', type: 'boolean', value: true)` |
| `find_package(Foo)` | `dependency('foo')` |
| `target_link_libraries()` | `dependencies: [foo_dep]` |
| `add_subdirectory()` | `subdir()` |
| `file(GLOB ...)` | Not supported (explicit file lists) |
| `CMAKE_BUILD_TYPE` | `buildtype` option |
| `install(TARGETS ...)` | `install: true` on target |
