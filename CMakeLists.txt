cmake_minimum_required(VERSION 3.21)

project(EmscriptenDemo)

add_executable(main src/main.cpp)

set(ASSETS_DIRECTORY "../assets")
set(SHELL_HTML_FILE "../emscripten/shell.html")

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html") 
    set(ECXXFLAGS "-O3 -std=c++17 -Werror --use-port=sdl2 --use-port=sdl2_image:formats=bmp,png,xpm,jpg --use-port=sdl2_mixer --use-port=sdl2_ttf")
    set_target_properties(main PROPERTIES LINK_FLAGS "${ECXXFLAGS} --shell-file ${SHELL_HTML_FILE} --preload-file ${ASSETS_DIRECTORY} -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s NO_EXIT_RUNTIME=1 -s AGGRESSIVE_VARIABLE_ELIMINATION=1")
    set_target_properties(main PROPERTIES COMPILE_FLAGS "${ECXXFLAGS}")
    set_target_properties(main PROPERTIES OUTPUT_NAME "index")
endif()
