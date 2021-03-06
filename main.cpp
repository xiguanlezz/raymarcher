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
#include "gui.h"

#define W 1920 / 2
#define H 1080 / 2

int main(int argc, char *argv[])
{

    std::cout << "======== Marching Time =========" << std::endl;
    // Define window
    GLFWwindow *window = nullptr;
    Window w = Window(window, W, H);
    w.init();
    Clock clock = Clock(window);

    // Define screen
    GLint locator;
    Framebuffer volumeRenderBuffer = Framebuffer(W, H);
    Framebuffer cubeBuffer = Framebuffer(W, H);
    Framebuffer rayEnterBuffer = Framebuffer(W, H);
    Framebuffer rayExitBuffer = Framebuffer(W, H);
    Framebuffer colorPickBuffer = Framebuffer(W, H);

    // Define shaders
    ShaderProgram phong_shader("shaders/phong.vert", "", "", "", "shaders/phong.frag");
    ShaderProgram cube_shader("shaders/cube.vert", "", "", "", "shaders/cube.frag");
    ShaderProgram color_position_normalized_shader("shaders/color_position_normalized.vert", "", "", "", "shaders/color_position_normalized.frag");
    ShaderProgram color_position_shader("shaders/color_position.vert", "", "", "", "shaders/color_position.frag");
    ShaderProgram screen_shader("shaders/screen.vert", "", "", "", "shaders/screen.frag");
    ShaderProgram color_pick_shader("shaders/color_pick.vert", "", "", "", "shaders/color_pick.frag");
    ShaderProgram final_shader("shaders/final.vert", "", "", "", "shaders/final.frag");

    // Controls
    MouseRotator rotator;
    rotator.init(window);

    // Volume data
    Volume volume;

    glfwSetWindowTitle(window, "Loading data...");
    if (argc > 1)
    {
        volume.loadDataPVM(argv[1]);
    }
    else
    {
        // volume.loadTestData(100, 100, 100);
        // volume.loadDataPVM("data/DTI-B0.pvm");
        // volume.loadDataPVM("data/Bruce.pvm"); // 256 * 256 * 156
        // volume.loadDataPVM("data/Bonsai2.pvm"); // 512, 512, 189 99MB 107MB on RAM
        // volume.loadDataPVM("data/CT-Head.pvm");
        volume.loadDataPVM("data/CT-Chest.pvm"); // 384, 384, 240
        // volume.loadDataPVM("data/Foot.pvm"); // 256, 256, 256
        // volume.loadDataPVM("data/Engine.pvm"); // 256 * 256 * 256
        // volume.loadDataPVM("data/MRI-Woman.pvm"); // 256 * 256 * 109
        // volume.loadDataPVM("data/CT-Knee.pvm");
        // volume.loadDataPVM("data/VisMale.pvm");
        // volume.loadDataPVM("data/Sheep.pvm");
        // volume.loadDataPVM("data/Teddy.pvm");
        // volume.loadDataPVM("data/Pig.pvm");
    }

    float dimx = volume.getResolution().x;
    float dimy = volume.getResolution().y;
    float dimz = volume.getResolution().z;
    float spacingx = volume.getSpacing().x;
    float spacingy = volume.getSpacing().y;
    float spacingz = volume.getSpacing().z;
    float yRelativex = (dimy / dimx) * (spacingy / spacingx);
    float zRelativex = (dimz / dimx) * (spacingz / spacingx);
    glm::vec2 yzRelativex = glm::vec2(yRelativex, zRelativex);
     if (argc > 2)
    {
        yzRelativex = glm::vec2(1.0);
    }

    float isoValue = 0.1;

    // Define meshes
    Quad quad = Quad();
    Sphere sphere = Sphere(25, 25, 1.0f);
    BoundingCube boundingCube;
    MarchingMesh mm = MarchingMesh(volume, glm::ivec3(30), &isoValue);

    ColorCube colorCube;

    // GUI
    GUI gui = GUI(W, H);

    glfwSetWindowTitle(window, "Marching time");
    glfwSetTime(0.0);
    int frame = 0;
    do
    {

        frame++;

        if (gui.getCursorPosTF().y > 1.0 && !gui.isDraggedFun())
        {
            rotator.poll(window);
        }
        else
        {
            if (!gui.isActive())
            {
                rotator.poll(window);
            }
        }

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glFrontFace(GL_CCW);
        rayExitBuffer.bindBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        color_position_normalized_shader();
        color_position_normalized_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
        locator = glGetUniformLocation(color_position_normalized_shader, "yzRelativex");
        glUniform2fv(locator, 1, &yzRelativex[0]);
        colorCube.draw();

        glFrontFace(GL_CW);
        rayEnterBuffer.bindBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        color_position_shader();
        color_position_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
        locator = glGetUniformLocation(color_position_shader, "yzRelativex");
        glUniform2fv(locator, 1, &yzRelativex[0]);
        mm.draw();

        // Bounding box
        cubeBuffer.bindBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube_shader();
        cube_shader.updateCommonUniforms(rotator, W, H, clock.getTime());
        locator = glGetUniformLocation(cube_shader, "yzRelativex");
        glUniform2fv(locator, 1, &yzRelativex[0]);
        locator = glGetUniformLocation(cube_shader, "cursorPos");
        glUniform2fv(locator, 1, &gui.getCursorPos()[0]);
        boundingCube.draw();

        // Ray marcher
        volumeRenderBuffer.bindBuffer();
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

        locator = glGetUniformLocation(screen_shader, "controlPointValues");
        glUniform1i(locator, 4);
        glActiveTexture(GL_TEXTURE4);
        gui.bindControlPointValueTexture();

        locator = glGetUniformLocation(screen_shader, "controlPointPositions");
        glUniform1i(locator, 5);
        glActiveTexture(GL_TEXTURE5);
        gui.bindControlPointPositionTexture();

        // locator = glGetUniformLocation(screen_shader, "test");
        // glUniform1i(locator, 4);
        // glActiveTexture(GL_TEXTURE4);
        // volume.InitTextures3D();

        locator = glGetUniformLocation(screen_shader, "volumeResolution");
        glUniform3fv(locator, 1, &volume.getResolution()[0]);
        locator = glGetUniformLocation(screen_shader, "renderOption");
        glProgramUniform1f(screen_shader, locator, gui.getRenderOption());
        locator = glGetUniformLocation(screen_shader, "isInteracting");
        glProgramUniform1f(screen_shader, locator, rotator.getIsInteracting());
        locator = glGetUniformLocation(screen_shader, "opacityFactor");
        glProgramUniform1f(screen_shader, locator, gui.getOpacityFactor());
        locator = glGetUniformLocation(screen_shader, "numberOfControlPoints");
        glProgramUniform1f(screen_shader, locator, gui.getNumberOfControlPoints());
        locator = glGetUniformLocation(screen_shader, "numberOfActiveControlPoints");
        glProgramUniform1f(screen_shader, locator, gui.getNumberOfActiveControlPoints());
        locator = glGetUniformLocation(screen_shader, "volumeSlicing");
        glUniform3fv(locator, 1, &gui.getVolumeSlicing()[0]);
        quad.draw();

        // FINAL COMPOSITING
        colorPickBuffer.bindBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        color_pick_shader();
        quad.draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        final_shader();
        glViewport(0, 0, W, H);
        final_shader.updateCommonUniforms(rotator, W, H, glfwGetTime());

        locator = glGetUniformLocation(final_shader, "volumeRender");
        glUniform1i(locator, 0);
        glActiveTexture(GL_TEXTURE0);
        volumeRenderBuffer.bindTexture();

        locator = glGetUniformLocation(final_shader, "controlPointValues");
        glUniform1i(locator, 1);
        glActiveTexture(GL_TEXTURE1);
        gui.bindControlPointValueTexture();

        locator = glGetUniformLocation(final_shader, "controlPointPositions");
        glUniform1i(locator, 2);
        glActiveTexture(GL_TEXTURE2);
        gui.bindControlPointPositionTexture();

        locator = glGetUniformLocation(final_shader, "colorPicker");
        glUniform1i(locator, 3);
        glActiveTexture(GL_TEXTURE3);
        colorPickBuffer.bindTexture();

        locator = glGetUniformLocation(final_shader, "numberOfControlPoints");
        glProgramUniform1f(final_shader, locator, gui.getNumberOfControlPoints());
        locator = glGetUniformLocation(final_shader, "numberOfActiveControlPoints");
        glProgramUniform1f(final_shader, locator, gui.getNumberOfActiveControlPoints());
        locator = glGetUniformLocation(final_shader, "hoveredControlPoint");
        glProgramUniform1f(final_shader, locator, gui.getHoveredControlPoint());
        locator = glGetUniformLocation(final_shader, "selectedControlPoint");
        glProgramUniform1f(final_shader, locator, gui.getSelectedControlPoint());
        locator = glGetUniformLocation(final_shader, "guiActive");
        glProgramUniform1f(final_shader, locator, gui.isActive());
        locator = glGetUniformLocation(final_shader, "guiColorPickActive");
        glProgramUniform1f(final_shader, locator, gui.isColorPickActive());

        quad.draw();

        gui.update(window, colorPickBuffer);

        glfwSwapBuffers(window);
        glfwPollEvents();

        char buf[32];
        float fps = frame / glfwGetTime();
        sprintf(buf,"%.0000f", fps);
        const char *test = "2";
        glfwSetWindowTitle(window, buf);

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);

    glDisableVertexAttribArray(0);

    return 0;
}