#ifndef OPENGL_WINDOW_HPP
#define OPENGL_WINDOW_HPP

#include <QOpenGLWindow>
#include <QOpenGLFunctions_4_5_Core>

#include "Shader.hpp"

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    OpenGLWindow();

    unsigned int texture_0;
    unsigned int texture_1;

    Shader compute_shader;
    int work_group_size[3];
    unsigned int screen_texture_id;

    Shader frame_shader;
    unsigned int frame_vao;
    unsigned int frame_vbo;

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
};

#endif 