#pragma once

#include <cstdint>
#include <SDL2/SDL.h>
#include "include/glad/glad.h"

class Platform
{
public:
	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
	~Platform();

	void Update(void const* buffer, int pitch);
	bool ProcessInput(uint8_t* keys);

private:
	SDL_Window* window{nullptr};
	SDL_GLContext gl_context{nullptr};
	GLuint framebuffer_texture{0};
	GLuint VAO{0};
	GLuint VBO{0};
	GLuint shader_program{0};
};