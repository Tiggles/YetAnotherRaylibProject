# YetAnotherRaylibProject
 
### Features!

- Place buildings with the click of a mouse! (Left mouse button)
- Zoom in and out using your mouse wheel! (Breaks mouse position! Can invert rendering!)
- Use WASD to scroll around!
- Reset scrolling AND zoom all at once, using the "R" key!
- Quit pressing the "Escape" key!
- Experience dynamic allocation of new wrappers when placing too many houses!
- Experience some of the houses not being correctly rendered if you move too far away from your starting point!

### Building with cmake

This project has an CMakeLists.txt setup for generating a plethora of build scripts for build systems
such as GNU Makefile, Ninja, and NMake (Visual Studio).

Generating the build script with cmake can be done in various ways, but doing the following will
generate an build script with the default build systems of your platform:
```
## Assuming you are in project root
$ cmake -S . -B build
```
This will create an directory `build` with the build scripts. You can start the build process with
cmake:
```
$ cmake --build ./build/
```
This will run the relevant build script in `build`.

The current `CMakeLists.txt` file is tested on both Windows (with the `VCPKG 2020.11` package manager
to find the raylib dependency) and Debian.
