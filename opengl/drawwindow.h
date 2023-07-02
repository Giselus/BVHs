#ifndef DRAWWINDOW_H
#define DRAWWINDOW_H

#include "openglwindow.h"

class DrawWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;

    void initialize() override;
    void render() override;
private:
    unsigned int VAO, VBO;

    unsigned int TEXTURE_WIDTH = 1024, TEXTURE_HEIGHT = 1024;
    unsigned int textureID;

    unsigned int triangleBuffer, nodeBuffer, queueBuffer, rayBuffer;
    int counter = 0;
};

#endif // DRAWWINDOW_H
