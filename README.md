# xsdk-base
An SDK compatible with multiple Windows versions of Source Engine.

## Downloading / cloning:

`git clone <repo> --recursive`
If files / submodules appear to be missing after pulling new commits, update with `git submodule update --init --recursive`

## Building

### CMake
1. Open your build directory
```sh
mkdir build
cd build
```
2. Configure a 32-bit or 64-bit build (depends on the game)
```sh
# This will use the default architecture
cmake <xsdk-base-folder> ..
# This will use 32-bit, specifically
cmake <xsdk-base-folder> .. -A Win32
```
3. Build
```sh
cmake --build .
```

### Visual Studio
There are a couple options.
- A: Open the CMake project with VS, or
- B: Perform the CMake steps 1 & 2 to generate the VS project files
    - This should create VS project files by default. If not, you may have to add a commandline argument to set VS as the CMake Generator.
