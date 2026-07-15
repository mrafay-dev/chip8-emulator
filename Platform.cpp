#include "Platform.hpp"
#include "include/glad/glad.h"
#include <SDL2/SDL.h>
#include <iostream>

// Simple vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoord = texCoord;
}
)";

// Simple fragment shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		windowWidth, windowHeight,
		SDL_WINDOW_OPENGL);

	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	// Compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	// Compile fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	// Link shaders
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertexShader);
	glAttachShader(shader_program, fragmentShader);
	glLinkProgram(shader_program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Setup vertex data
	float vertices[] = {
		// pos      // texCoord
		 1.0f,  1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 1.0f,
		-1.0f,  1.0f,  0.0f, 0.0f
	};

	unsigned int indices[] = {0, 1, 3, 1, 2, 3};

	unsigned int EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup texture
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

Platform::~Platform()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(shader_program);
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Platform::Update(void const* buffer, int pitch)
{
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_program);
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SDL_GL_SwapWindow(window);
}

bool Platform::ProcessInput(uint8_t* keys)
{
	bool quit = false;
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE: quit = true; break;
					case SDLK_x: keys[0x0] = 1; break;
					case SDLK_1: keys[0x1] = 1; break;
					case SDLK_2: keys[0x2] = 1; break;
					case SDLK_3: keys[0x3] = 1; break;
					case SDLK_q: keys[0x4] = 1; break;
					case SDLK_w: keys[0x5] = 1; break;
					case SDLK_e: keys[0x6] = 1; break;
					case SDLK_a: keys[0x7] = 1; break;
					case SDLK_s: keys[0x8] = 1; break;
					case SDLK_d: keys[0x9] = 1; break;
					case SDLK_z: keys[0xA] = 1; break;
					case SDLK_c: keys[0xB] = 1; break;
					case SDLK_4: keys[0xC] = 1; break;
					case SDLK_r: keys[0xD] = 1; break;
					case SDLK_f: keys[0xE] = 1; break;
					case SDLK_v: keys[0xF] = 1; break;
				}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
					case SDLK_x: keys[0x0] = 0; break;
					case SDLK_1: keys[0x1] = 0; break;
					case SDLK_2: keys[0x2] = 0; break;
					case SDLK_3: keys[0x3] = 0; break;
					case SDLK_q: keys[0x4] = 0; break;
					case SDLK_w: keys[0x5] = 0; break;
					case SDLK_e: keys[0x6] = 0; break;
					case SDLK_a: keys[0x7] = 0; break;
					case SDLK_s: keys[0x8] = 0; break;
					case SDLK_d: keys[0x9] = 0; break;
					case SDLK_z: keys[0xA] = 0; break;
					case SDLK_c: keys[0xB] = 0; break;
					case SDLK_4: keys[0xC] = 0; break;
					case SDLK_r: keys[0xD] = 0; break;
					case SDLK_f: keys[0xE] = 0; break;
					case SDLK_v: keys[0xF] = 0; break;
				}
				break;
		}
	}

	return quit;
}