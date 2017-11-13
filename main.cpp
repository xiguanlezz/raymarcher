// External includes
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

// Raytracer
#include "window.h"
#include "clock.h"
#include "shaderprogram.h"
#include "framebuffer.h"
#include "quad.h"
#include "sphere.h"
#include "marchingmesh.h"
#include "boundingcube.h"
#include "colorcube.h"
#include "volume.h"

#define W 1920 / 2
#define H 1080 / 2

int main()
{

	std::cout << "======== Marching Time =========" << std::endl;

	// Define window
	GLFWwindow *window = nullptr;
	Window w = Window(window, W, H);
	w.init();
	Clock clock = Clock(window);

	// Define screen
	GLint locator;
	Framebuffer screenBuffer = Framebuffer(W, H);
	Framebuffer cubeBuffer = Framebuffer(W, H);
	Framebuffer rayEnterBuffer = Framebuffer(W, H);
	Framebuffer rayExitBuffer = Framebuffer(W, H);

	// Define shaders
	ShaderProgram phong_shader("shaders/phong.vert", "", "", "", "shaders/phong.frag");
	ShaderProgram cube_shader("shaders/cube.vert", "", "", "", "shaders/cube.frag");
	ShaderProgram color_position_normalized_shader("shaders/color_position_normalized.vert", "", "", "", "shaders/color_position_normalized.frag");
	ShaderProgram color_position_shader("shaders/color_position.vert", "", "", "", "shaders/color_position.frag");
	ShaderProgram screen_shader("shaders/screen.vert", "", "", "", "shaders/screen.frag");
	ShaderProgram post_shader("shaders/screen.vert", "", "", "", "shaders/post.frag");

	// Controls
	MouseRotator rotator;
	rotator.init(window);

	// Volume data
	Volume volume;

	glfwSetWindowTitle(window, "Loading data...");


	// volume.loadTestData(100, 100, 100);
	// volume.loadDataPVM("data/DTI-B0.pvm");
	volume.loadDataPVM("data/Bruce.pvm"); // 256 * 256 * 156
	// volume.loadDataPVM("data/Bonsai2.pvm"); // 512, 512, 189 99MB 107MB on RAM
	// volume.loadDataPVM("data/CT-Head.pvm");
	// volume.loadDataPVM("data/CT-Chest.pvm"); // 384, 384, 240
	// volume.loadDataPVM("data/Foot.pvm"); // 256, 256, 256
	// volume.loadDataPVM("data/Engine.pvm"); // 256 * 256 * 256
	// volume.loadDataPVM("data/MRI-Woman.pvm"); // 256 * 256 * 109

	float isoValue = 0.1; 
	// Define meshes
	Quad quad = Quad();
	Sphere sphere = Sphere(25, 25, 1.0f);
	BoundingCube boundingCube;
	std::cout << "Marching...\n" << std::endl;
	MarchingMesh mm = MarchingMesh(volume, glm::ivec3(50), &isoValue);
	ColorCube colorCube;

	glfwSetWindowTitle(window, "Marching time");

	do
	{
		rotator.poll(window);
		// // clock.tic();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);


		glFrontFace(GL_CCW); // exit position
		rayExitBuffer.bindBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		color_position_normalized_shader();
		color_position_normalized_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
		colorCube.draw();

		glFrontFace(GL_CW); // enter position
		rayEnterBuffer.bindBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		color_position_shader();
		color_position_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
		mm.draw();

		// Bounding box
		cubeBuffer.bindBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube_shader();
		cube_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
		boundingCube.draw();

		// Ray marcher
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screen_shader();
		glViewport(0, 0, W, H);
		screen_shader.updateCommonUniforms(rotator, W, H, glfwGetTime());

		locator = glGetUniformLocation(screen_shader, "volumeTexture");
		glUniform1i(locator, 0);
		glActiveTexture(GL_TEXTURE0);
		volume.bindTexture();

		locator = glGetUniformLocation(screen_shader, "cubeTexture");
		glUniform1i(locator, 1);
		glActiveTexture(GL_TEXTURE1);
		cubeBuffer.bindTexture();
		
		locator = glGetUniformLocation(screen_shader, "rayExitTexture");
		glUniform1i(locator, 2);
		glActiveTexture(GL_TEXTURE2);
		rayExitBuffer.bindTexture();

		locator = glGetUniformLocation(screen_shader, "rayEnterTexture");
		glUniform1i(locator, 3);
		glActiveTexture(GL_TEXTURE3);
		rayEnterBuffer.bindTexture();

		// locator = glGetUniformLocation(screen_shader, "test");
		// glUniform1i(locator, 4);
		// glActiveTexture(GL_TEXTURE4);
		// volume.InitTextures3D();

		locator = glGetUniformLocation(screen_shader, "volumeResolution");
		glUniform3fv(locator, 1, &volume.getResolution()[0]);
		quad.draw();

		
		
		

		// clock.toc();

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			 glfwWindowShouldClose(window) == 0);

	glDisableVertexAttribArray(0);

	return 0;
}