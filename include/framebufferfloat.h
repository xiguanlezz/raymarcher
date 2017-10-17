#pragma once

#include <iostream>
#include "GL/glew.h"
#include <GLFW/glfw3.h>


class FramebufferFloat
{
public:
	FramebufferFloat();
	FramebufferFloat(const int w, const int h);
	void create(const int &w, const int &h);

	~FramebufferFloat();

	// Generates a texture that is suited for attachments to a framebuffer
	GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);

	void bindBuffer();
	void bindTexture();
    GLuint get();
private:
	GLuint framebuffer;
	GLuint textureColorbuffer;
	void create_framebuffer();
	int width;
	int height;
};