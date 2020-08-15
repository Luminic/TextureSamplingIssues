#include <QApplication>
#include "OpenGLWindow.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  OpenGLWindow window;

  return app.exec();
}
