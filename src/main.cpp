#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>
#include "SDL_opengles2.h"

#else

#if TARGET_OS_MAC == 1

#include <OpenGL/OpenGL.h>

#if ESSENTIAL_GL_PRACTICES_SUPPORT_GL3
#include <OpenGL/gl3.h>
#else
#include <OpenGL/gl.h>
#endif //! ESSENTIAL_GL_PRACTICES_SUPPORT_GL3

#else

#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#endif

#endif

#include <stdio.h>
#include <string>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <cmath>

struct _TTF_Font;
using TTF_Font = _TTF_Font;

GLuint programObject;
int width = 960;
int height = 540;

SDL_Window *window = nullptr;
SDL_GLContext context;
SDL_Surface* surface = nullptr;
Mix_Chunk* sound = nullptr;
TTF_Font* font = nullptr;

bool quit = false;

float dt = 10.0f;
float currentTime = SDL_GetTicks();
float accumulator = 0.0f;

float pos_x = 0.0f;
float pos_y = 0.0f;
float speed_x = 0.001f;
float speed_y = 0.001f;
float velocity_x = 0.0f;
float velocity_y = speed_y;

GLuint LoadShader(GLenum type, const char *shaderSrc) {
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	if (shader == 0)
		return 0;

	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

int Init() {
    surface = IMG_Load("assets/texture.png");

    if (surface == nullptr) {
        std::cout << "Could not load surface" << std::endl;
        std::cout << IMG_GetError() << std::endl;
    }

	sound = Mix_LoadWAV("assets/sound.wav");

    if (sound == nullptr) {
        std::cout << "Could not load sound" << std::endl;
        std::cout << Mix_GetError() << std::endl;
    }

	font = TTF_OpenFontIndex("assets/DroidSans.ttf", 32, 0);

	if (font == nullptr) {
        std::cout << "Could not load font" << std::endl;
        std::cout << TTF_GetError() << std::endl;
	}

	const std::string vShaderStr = R"(
		attribute vec4 vPosition;
		void main() {
		   gl_Position = vPosition;
		}
	)";

	const std::string fShaderStr = R"(
		void main() { 
		  gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		}
	)";

	GLuint vertexShader;
	GLuint fragmentShader;
	GLint linked;

	vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr.c_str());
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr.c_str());

	programObject = glCreateProgram();
	if (programObject == 0)
		return 0;

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	glBindAttribLocation(programObject, 0, "vPosition");
	glLinkProgram(programObject);
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

	if (!linked) {
		GLint infoLen = 0;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));
			glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
			printf("Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(programObject);
		return GL_FALSE;
	}

	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	return GL_TRUE;
}

void Render() {
	GLfloat vVertices[] = {
		0.0f + pos_x,  0.5f + pos_y, 0.0f,
	   -0.5f + pos_x, -0.5f + pos_y, 0.0f,
		0.5f + pos_x, -0.5f + pos_y, 0.0f
	};

	GLuint vertexPosObject;
	glGenBuffers(1, &vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glBufferData(GL_ARRAY_BUFFER, 9 * 4, vVertices, GL_STATIC_DRAW);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	SDL_GL_SwapWindow(window);
}

void Input() {
	static SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            std::cout << "Window was resized: (" << w << ", " << h << ")" << std::endl;
        }
        if (event.type == SDL_KEYDOWN) {
            std::cout << "Key Down" << std::endl;
        }
    }
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_A]) {
        velocity_x = -speed_x;
    } else if (currentKeyStates[SDL_SCANCODE_D]) {
        velocity_x = speed_x;
    } else {
        velocity_x = 0;
    }
}

void Update(float dt) {
    // Keep rectangle within bounds.
    if (pos_y <= -1.0f) {
        velocity_y = speed_y;
		Mix_PlayChannel(0, sound, 0);
    } else if (pos_y >= 1.0f) {
        velocity_y = -speed_y;
		Mix_PlayChannel(0, sound, 0);
    }
    if (pos_x + velocity_x * dt <= -1.0f || pos_x + velocity_x * dt >= 1.0f) {
        velocity_x = 0;
    }

    // Update rectangle position.
    pos_x += velocity_x * dt;
    pos_y += velocity_y * dt;
}

void MainLoop() {
    float newTime = SDL_GetTicks();
    float frameTime = newTime - currentTime;

    if (frameTime > 250) {
        // std::cout << "Lag encountered" << std::endl;
        frameTime = 250;
    }

	Input();

	currentTime = newTime;
    accumulator += frameTime;

    while (accumulator >= dt) {
        Update(dt);
        accumulator -= dt;
    };

	Render();
#ifdef __EMSCRIPTEN__
	if (quit) emscripten_cancel_main_loop();
#endif
}

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	printf("Initialized SDL2\n");

	if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
		printf("Unable to initialize SDL_image: %s\n", IMG_GetError());
		return 1;
	}
	printf("Initialized SDL2_image\n");

	int mixer_flags{ MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS |
					 MIX_INIT_WAVPACK /* | MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MID*/ };

	if (Mix_Init(mixer_flags) == 0) {
		printf("Unable to initialize SDL_mixer: %s\n", Mix_GetError());
		return 1;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		printf("Unable to open mixer audio: %s\n", Mix_GetError());
		return 1;
	}
	printf("Initialized SDL2_mixer\n");

	if (TTF_Init() == -1) {
		printf("Unable to initialize SDL_ttf: %s\n", TTF_GetError());
		return 1;
	}
	printf("Initialized SDL2_ttf\n");

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!\n");
	}

	window = SDL_CreateWindow("Window Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);

	if (!window) {
		printf("Unable to create window: %s\n", SDL_GetError());
		return 1;
	}

#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

	context = SDL_GL_CreateContext(window);

#ifndef __EMSCRIPTEN__
	// Load OpenGL functions using GLEW/SDL_GL_GetProcAddress/etc.
#endif

	if (SDL_GL_SetSwapInterval(1) < 0) {
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}

	Init();

	std::cout << glGetString(GL_VERSION) << std::endl;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoop, 0, 1);
#else
	while (!quit) { MainLoop(); }
#endif
	Mix_FreeChunk(sound);
	TTF_CloseFont(font);
	SDL_FreeSurface(surface);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    window = nullptr;
    surface = nullptr;
    font = nullptr;
    sound = nullptr;
	Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

	return 0;
}