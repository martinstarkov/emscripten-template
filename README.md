Minimal Emscripten OpenGL+SDL2+(image/mixer/ttf) CMake Game Template
===
This template repository allows for compiling OpenGL ES 3.0 (WebGL 2.0) projects with SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf using Emscripten and CMake.

---
# Live demo: [click here](https://bicyclemice.itch.io/emscriptendemo)
Demo Controls:
A - move left
D - move right
Click anywhere to enable audio ([why and how to deal with this?](https://github.com/libsdl-org/SDL/issues/6385))
---
Dependencies:
---
* [Emscripten SDK](https://emscripten.org/): Check that Emscripten is added to PATH by running ```emcc --version```
* [CMake](https://cmake.org/): Check that CMake is added to PATH by running ```cmake --version```
* [Ninja](https://ninja-build.org/) or [MinGW](https://www.mingw-w64.org/): Check that Ninja/MinGW is added to PATH by running ```ninja --version``` or ```gcc --version``` respectively
---
Setup & Usage
---
Clone this repository using Git:
```git clone https://github.com/martinstarkov/emscripten-template.git```

Navigate to the scripts directory:
```cd emscripten-template/scripts```

Run any of the following script commands:
| Command    | Description |
| -------- | ------- |
| ```./build-emscripten.sh```  | Generates emscripten build files (index.html, etc) in the build directory |
| ```./run-emscripten.sh``` | Runs the generate build files locally using emrun |
| ```./build-run-emscripten.sh```    | Combines the two above scripts |
| ```./zip-for-itch.sh```    | Zips the generated build files and places the zip in the build directory (helpful for uploading to [itch.io](https://itch.io/) or elsewhere) |
---
Additional Info
---
- To modify the HTML code surrounding the game canvas, simply edit the ```emscripten/shell.html``` file and rerun the build script.
- Resource directory is determined by ```ASSETS_DIRECTORY``` variable in the ```CMakeLists.txt``` file.
- Window title can be modified by changing the first argument of SDL_CreateWindow in ```src/main.cpp```.