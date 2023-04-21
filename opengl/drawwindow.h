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

    unsigned int TEXTURE_WIDTH = 512, TEXTURE_HEIGHT = 512;
    unsigned int textureID;
};

#endif // DRAWWINDOW_H
