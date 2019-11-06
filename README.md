# raycaster
A simple 2.5 game with 0 dependencies

## Setup
You will need:
 - CMake
 - A compiler
 - A build tool

### Linux
Keeping dependencies to a minimum is a priority for this project but to build on Linux you will make sure you have some XCB extension libraries installed.
 - xcb-shm
 - xcb-image

On Ubuntu these can be installed with
 - `sudo apt install libxcb-shm0-dev`
 - `sudo apt install libxcb-image0-dev`

Check with your distro to see how to get these xcb extension libs installed.
Currently CMake will only check for XCB/X11 base libs so it's possible to hit a compiler error.

## Building
Building this project should work fairly straightforwardly with CMake.

On Windows:
 - VS2019 should work fine
 - Ninja with LLVM for Windows is preferred 

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