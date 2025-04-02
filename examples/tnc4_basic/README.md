# Mobilinkd TNC4 Instructions

The `libaprsroute` library can be used on the Mobilinkd TNC4 project.

**NOTE**: while I do have a Mobilinkd TNC4, I have not tested this on a real device, use at your own risk. These changes are development changes, they do not add any new functionality. I provide no warranty for this code. Use at your own risk.

Product names mentioned in this document may be trademarks or registered trademarks of their respective owners.

The `example.patch` file contains code that was automatically generated. This code is Copyright © 2017 Rob Riggs <rob@mobilinkd.com>. The patch file is provided as-is for convenience purposes only. No copyright claims are made on the patch contents.

## Build Instructions

I use a local Ubuntu WSL environment in Windows, for building the TNC4 project.

### Dependencies

Install all the dependencies by running the `install_dependencies.sh` script. 

Run with sudo or root access.

This will install an ARM GCC toolchain, CMake, git, Boost and Blaze.

### Clone and build the TNC4 repository

Clone and cd into the repository:

``` bash
git clone https://github.com/mobilinkd/tnc4-firmware.git
cd tnc4-firmware
```

Configure CMake, as per the official instructions:

``` bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -S. -Bbuild/Debug -G Ninja
```

Build the repository:

``` bash
cmake --build build/Debug --target tnc4-firmware --
```

At this point you should be able to build the repository successfully.

### Add a dependency to the `libaprsroute` library

Add this line to the end of the CMakeLists.txt script in the root of the project:

``` cmake
file(DOWNLOAD https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)
```

Re-run the CMake configure:

``` bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -S. -Bbuild/Debug -G Ninja
```

This will fetch the library, and copy it to the external directory in the root of the project.

### Using the library

Open `Core\TNC\Digipeater.hpp`.

Include the `libaprsroute` header:

``` cpp
#include "../../external/aprsroute.hpp"
```

Call the `try_route_packet` function using this small example.

I've done this in the `rewrite_frame` funtion.

``` cpp
aprs::router::router_settings settings { "DIGI", {}, { "WIDE1" }, aprs::router::routing_option::none, true };
aprs::router::routing_result result;        
aprs::router::try_route_packet("N0CALL>APRS,WIDE1-1,WIDE2-2:data", settings, result);
```

Rebuild the project:

``` bash
cmake --build build/Debug --target tnc4-firmware --
```

### Patch

The example above can be applied onto the repo using a git patch: `example.patch`.

This patch was created on commit: 97784af8221be75a660c531aa2b8a08d878e0cd0

The patch can be applied using:

``` bash
git apply example.patch
```

## License

All the files, code, documents, materials etc. in the `tnc4_basic` directory are in the public domain. See `LICENSE` for more information.
