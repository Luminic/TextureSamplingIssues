#include "OpenGLWindow.hpp"

#include <QDebug>
#include <QOpenGLDebugLogger>

uint32_t round_up_to_pow_2(uint32_t x) {
    /*
    In C++20 we can use:
        #include <bit>
        std::bit_ceil(x)
    */
    // Current implementation from https://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

OpenGLWindow::OpenGLWindow() {
    resize(800,600);
    show();
}

void OpenGLWindow::initializeGL() {
    initializeOpenGLFunctions();

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
    if (!logger->initialize()) {
        qWarning("QOpenGLDebugLogger failed to initialize.");
    }
    if (!ctx->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
        qWarning("KHR Debug extension unavailable.");
    }

    connect(logger, &QOpenGLDebugLogger::messageLogged, this,
        [](const QOpenGLDebugMessage& message){
            if (message.severity() == QOpenGLDebugMessage::HighSeverity) {
                qCritical(message.message().toLatin1().constData());
            }
            else {
                qWarning(message.message().toLatin1().constData());
            }
        }
    );
    logger->startLogging();

    glEnable(GL_DEBUG_OUTPUT);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    // Create the frame shader
    float frame_vertices[] = {
        // Top left triangle
        -1.0f,  1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
        // Bottom left triangle
        -1.0f, -1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f
    };

    glGenVertexArrays(1, &frame_vao);
    glBindVertexArray(frame_vao);

    glGenBuffers(1, &frame_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, frame_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frame_vertices), frame_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ShaderStage shaders[] = {
        ShaderStage{GL_VERTEX_SHADER, "shaders/framebuffer_vs.glsl"},
        ShaderStage{GL_FRAGMENT_SHADER, "shaders/framebuffer_fs.glsl"}
    };

    frame_shader.load_shaders(shaders, 2);
    frame_shader.validate();
    glUseProgram(frame_shader.get_id());

    // Create the frame texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &screen_texture_id);
    glBindTexture(GL_TEXTURE_2D, screen_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Create the compute shader
    ShaderStage comp_shader = ShaderStage{GL_COMPUTE_SHADER, "shaders/compute.glsl"};
    compute_shader.load_shaders(&comp_shader, 1);
    glGetProgramiv(compute_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    compute_shader.validate();
    glUseProgram(compute_shader.get_id());

    // Create the textures
    unsigned char red_pixels[4] = {
        255, 0, 0, 255
    };
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_0);
    glBindTexture(GL_TEXTURE_2D, texture_0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, red_pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned char blue_pixels[4] = {
        0, 0, 255, 255
    };
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &texture_1);
    glBindTexture(GL_TEXTURE_2D, texture_1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue_pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}


void OpenGLWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw to screen_texture
    glUseProgram(compute_shader.get_id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_0);
    compute_shader.set_int("textures[0]", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_1);
    compute_shader.set_int("textures[1]", 1);

    glBindImageTexture(0, screen_texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    unsigned int worksize_x = round_up_to_pow_2(width());
    unsigned int worksize_y = round_up_to_pow_2(height());
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Draw screen_texture to the screen
    glUseProgram(frame_shader.get_id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_texture_id);
    frame_shader.set_int("render", 0);

    glBindVertexArray(frame_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void OpenGLWindow::resizeGL(int w, int h) {
    glUseProgram(frame_shader.get_id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}