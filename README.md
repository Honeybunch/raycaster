# raycaster
A simple 2.5 game with 0 dependencies

## Building
Building this project should work fairly straightforwardly with CMake.

On Windows:
 - VS2019 should work fine
 - Ninja with LLVM for Windows is preferred 
  
`cmake -E LDFLAGS=\"-fuse-ld=lld\" cmake -S . -B build_dir -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RC_COMPILER=llvm-rc -DCMAKE_NO_SYSTEM_FROM_IMPORTED=TRUE`

On Linux:
  - Ninja is preferred
  - Clang is preferred
  - Make or Other Generators should work
  - GCC should work

On MacOS:
  - Ninja is preferred
  - Clang required
  - Xcode should work

## Contributing
Please make sure to format on save with the provided clang-format file.
Coding style standards are just LLVM