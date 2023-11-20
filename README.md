# Sparkly FX
A video recording tool for Team Fortress 2, developed using xsdk-base.

Features:
- Record compressed image formats (QOI, PNG)
- Output multiple streams/layers
- Highly customizeable streams (Multiple mattes, visibility filters, color adjustment, unlimited FOV)

Please read [known-bugs.md](known-bugs.md). This software is in early development.

You can chat with fellow TF2 video editors (including me) in the Castellum Discord server:

[![Discord Shield](https://discordapp.com/api/guilds/405422047221645312/widget.png?style=shield)](https://discord.gg/f5QumD4hE4)

Below are the original README contents for xsdk-base.
When configuring CMake, the `-A Win32` flag is required for Team Fortress 2.

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
2. Configure a 32-bit or 64-bit build (depending on the game)
```sh
# This will use the default architecture
cmake <xsdk-base-folder> -D XSDK_BUILD_EXAMPLES=1
# This will use 32-bit, specifically
cmake <xsdk-base-folder> -D XSDK_BUILD_EXAMPLES=1 -A Win32
```
3. Build a debug or release binary
```sh
# This builds the default config (Debug)
cmake --build .
# This builds a release binary with debug info. This improves the symbols in stack traces.
cmake --build . --config RelWithDebInfo
```

### Visual Studio
There are a couple options.
- A: Open the CMake project with VS, or
- B: Perform the CMake steps 1 & 2 to generate the VS project files
    - This should use Visual Studio's CMake Generator by default. If not, you will have to add a commandline argument to set VS as the CMake Generator.
