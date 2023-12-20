//
// Created by alexs on 12/19/2023.
//

#include "fl_game.hpp"
#include "../counter.hpp"
#include "QGuiApplication"
#include "QQuickWindow"
#include <QOpenGLTexture>
#include <iostream>

static const GLchar *fragmentShader = "#version 330 core\n"
                                      "out vec4 FragColor;\n"

                                      "uniform vec3 color;\n"

                                      "void main()\n"
                                      "{\n"
                                      " FragColor = vec4(color.rgb, 1.0f);\n"
                                      "}\0";

// compiler combines adjacent strings apparently
static const GLchar *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

static std::array<double, 16> calculateTexVertexArray(int x, int y, int w,
                                                      int h, QRectF bounds) {
  std::array<double, 16> result = {0};

  auto topLeftX = x;
  auto bottomLeftX = x;
  auto topRightX = x + w;
  auto bottomRightX = x + w;

  auto topLeftY = y;
  auto bottomLeftY = y + h;
  auto topRightY = y;
  auto bottomRightY = y + h;

  result[0] = float(topLeftX) / bounds.width() * 2 - 1;
  result[1] = (float(topLeftY) / bounds.height() * 2 - 1) * -1;
  result[2] = 0;
  result[3] = 0;

  result[4] = float(bottomLeftX) / bounds.width() * 2 - 1;
  result[5] = (float(bottomLeftY) / bounds.height() * 2 - 1) * -1;
  result[6] = 0;
  result[7] = 1.0;

  result[8] = float(topRightX) / bounds.width() * 2 - 1;
  result[9] = (float(topRightY) / bounds.height() * 2 - 1) * -1;
  result[10] = 1.0f;
  result[11] = 0;

  result[12] = float(bottomRightX) / bounds.width() * 2 - 1;
  result[13] = (float(bottomRightY) / bounds.height() * 2 - 1) * -1;
  result[14] = 1.0f;
  result[15] = 1.0f;

  return result;
}

FLGame::FLGame(QQuickItem *parent) : QQuickPaintedItem(parent) {}
void FLGame::paint(QPainter *painter) {

  printf("thread in paint: %d\n", QThread::currentThreadId());

  painter->setPen(Qt::black);
  painter->setFont(QFont("Arial", 72));
  painter->drawText(100, 100,
                    QString::fromStdString(std::to_string(getCounterValue())));
  //  QImage image((uchar *)m_data, m_width, m_height, m_pitch,
  //               QImage::Format_RGB16);
  //  painter->drawImage(boundingRect().toRect(), image);
}

void FLGame::setRefreshRate(qreal refreshRate) {
  printf("yeah called set refresh rate!: %f\n", refreshRate);
  double value = framerate / refreshRate;
  double closest = 0.0;
  double minDifference = std::abs(1.0 - value);

  // Iterate through possible denominators (1 to some maximum value)
  for (int d = 1; d <= 1000; ++d) { // Adjust the maximum value as needed
    double n = std::round(value * d);
    double diff = std::abs(value - n / d);

    if (diff < minDifference) {
      closest = n / d;
      minDifference = diff;
    }
  }

  // Update the numerator and denominator with the closest fraction
  printf("got this: %f\n", closest * refreshRate);
}

void FLGame::runOneFrame() {
  printf("thread in run: %d\n", QThread::currentThreadId());
  if (!openGlInitialized) {
    //    auto thing = QOpenGLContext::currentContext();
    //    if (thing) {
    //      printf("there is a context\n");
    //    } else {
    //      printf("NO CONTEXT\n");
    //    }
    //
    //    tex = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    //    tex->create();
    //    tex->bind();
    //
    //    auto err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    tex->release();
    //
    //    glGenBuffers(1, &texVbo);
    //    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    actualVao = new QOpenGLVertexArrayObject();
    //    actualVao->bind();
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glEnableVertexAttribArray(0);
    //    glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * 4, nullptr);
    //
    //    glEnableVertexAttribArray(1);
    //    glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * 4,
    //                          (void *)(2 * sizeof(float)));
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    actualVao->release();
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    auto frag = glCreateShader(GL_FRAGMENT_SHADER);
    //    glShaderSource(frag, 1, &fragmentShader, nullptr);
    //    glCompileShader(frag);
    //
    //    GLint status;
    //    glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
    //    if (status == GL_FALSE) {
    //      printf("Oh no, error: %d :(\n", status);
    //    }
    //
    //    auto vert = glCreateShader(GL_VERTEX_SHADER);
    //    glShaderSource(vert, 1, &vertexShaderSource, nullptr);
    //    glCompileShader(vert);
    //
    //    glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
    //    if (status == GL_FALSE) {
    //      printf("Oh no, error: %d :(\n", status);
    //      GLint logLength;
    //      glGetShaderiv(vert, GL_INFO_LOG_LENGTH, &logLength);
    //
    //      char *str = static_cast<char *>(malloc(logLength + 1));
    //      memset(str, 'a', logLength);
    //      str[logLength] = '\0';
    //      glGetShaderInfoLog(vert, logLength, nullptr, str);
    //
    //      printf("error: %s\n", str);
    //    }
    //
    //    program = glCreateProgram();
    //    glAttachShader(program, frag);
    //    glAttachShader(program, vert);
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glLinkProgram(program);
    //    //  glDetachShader(program, vert);
    //    //  glDetachShader(program, frag);
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glDeleteShader(vert);
    //    glDeleteShader(frag);
    //
    //    err = glGetError();
    //    if (err != GL_NO_ERROR) {
    //      printf("GL ERROR: %d\n", err);
    //    }
    //
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //    openGlInitialized = true;
  }

  if (running) {
    lastTick = thisTick;
    thisTick = SDL_GetPerformanceCounter();

    auto deltaTime =
        ((thisTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency());

    //    printf("delta time: %f\n", deltaTime);
    //    if (dumb == 0) {
    core->run(deltaTime);
    if (deltaTime > 20) {
      printf("delta time: %f\n", deltaTime);
    }
    //      dumb++;
    //    } else {
    //      dumb = 0;
    //    }
    auto rect = this->boundingRect();
    QQuickPaintedItem::update(rect.toRect());
  }
}
void FLGame::init() {

  QObject::connect(window(), &QQuickWindow::beforeRendering, this,
                   &FLGame::runOneFrame, Qt::DirectConnection);
  auto screen = QGuiApplication::primaryScreen();
  QObject::connect(screen, &QScreen::refreshRateChanged, this,
                   &FLGame::setRefreshRate);
  QObject::connect(
      screen, &QScreen::refreshRateChanged, [&](qreal refreshRate) {
        printf("got a new refresh rate in the lambda: %f\n", refreshRate);
      });
}
void FLGame::receive(const void *data, unsigned int width, unsigned int height,
                     size_t pitch) {
  m_data = const_cast<void *>(data);
  m_width = width;
  m_height = height;
  m_pitch = pitch;
}
