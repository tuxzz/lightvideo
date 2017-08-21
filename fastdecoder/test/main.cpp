#include "../../fastdecoder/src/decoder.hpp"
#include "../../fastdecoder/src/error.hpp"
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace LightVideoDecoder;

static void framebufferSizeCallback(GLFWwindow *, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void speedtest(Decoder &dec)
{
  auto start = std::chrono::steady_clock::now();
  dec.seekFrame(0);
  printf("%d\n", dec.currentFrameNumber());
  dec.decodeCurrentFrame();
  while(dec.currentFrameNumber() < dec.frameCount() - 1)
  {
    dec.nextFrame();
    printf("%d\n", dec.currentFrameNumber());
    dec.decodeCurrentFrame();
  }
  double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) / 1e3;
  printf("%lfs for %lfs, ratio %lfx\n", duration, dec.duration(), dec.duration() / duration);
}

static GLfloat vertices[] = {
  -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  1.0f, -1.0f, 0.0f, 1.0f, 1.0f
};

static const char *presentVert =
"#version 400 core\n"
"layout (location = 0) in vec3 inPos;\n"
"layout (location = 1) in vec2 inTexCoord;\n"
"noperspective out vec3 vertPos;\n"
"noperspective out vec2 atexCoord;\n"
"\n"
"void main()\n"
"{\n"
"  vertPos = inPos;\n"
"  atexCoord = inTexCoord;\n"
"  gl_Position = vec4(inPos.x, inPos.y, inPos.z, 1.0);\n"
"}";

static const char *presentFrag =
"#version 400 core\n"
"noperspective in vec3 vertPos;\n"
"noperspective in vec2 atexCoord;\n"
"uniform sampler2D texY, texU, texV;\n"
"out vec4 color;\n"
"\n"
"const mat3 yuvrgbMat = mat3(\n"
"  1.0f, -1.21889418866994e-6f, 1.40199958865734f,\n"
"  1.0f, -0.344135678165337f, -0.714136155581812f,\n"
"  1.0f, 1.77200006607382f, 4.06298062919653e-8f\n"
");\n"
"\n"
"vec3 convertYUVToRGB(vec3 yuv)\n"
"{\n"
"  yuv.g -= 0.5f;\n"
"  yuv.b -= 0.5f;\n"
"  return clamp(yuv * yuvrgbMat, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"  vec2 texCoord = vec2(atexCoord.x + 0.25, atexCoord.y - 0.0);\n"
"  float y = texture(texY, texCoord).r;\n"
"  float u = texture(texU, texCoord).r;\n"
"  float v = texture(texV, texCoord).r;\n"
"  vec3 rgb = convertYUVToRGB(vec3(y, u, v));\n"
"  color = vec4(rgb.r, rgb.g, rgb.b, 1.0f);\n"
"}";

static void play(Decoder &dec)
{
  /* initialize */
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(dec.width(), dec.height(), "LightPlay", nullptr, nullptr);
  if(!window)
  {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    std::abort();
  }
  glfwMakeContextCurrent(window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "Failed to initialize GLAD.\n");
    std::abort();
  }

  glViewport(0, 0, dec.width(), dec.height());
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  
  GLuint vertShader, fragShader;
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertShader, 1, &presentVert, nullptr);
  glCompileShader(vertShader);
  glShaderSource(fragShader, 1, &presentFrag, nullptr);
  glCompileShader(fragShader);
  {
    int success;
    char infoLog[1024];
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(vertShader, 1024, nullptr, infoLog);
      fprintf(stderr, "Failed to compile vertex shader.\n");
      fprintf(stderr, "%s\n", infoLog);
    }
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(fragShader, 1024, nullptr, infoLog);
      fprintf(stderr, "Failed to compile fragment shader.\n");
      fprintf(stderr, "%s\n", infoLog);
    }
  }
  GLuint presentProgram = glCreateProgram();
  glAttachShader(presentProgram, vertShader);
  glAttachShader(presentProgram, fragShader);
  glLinkProgram(presentProgram);
  {
    int success;
    char infoLog[1024];
    glGetProgramiv(presentProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
      glGetProgramInfoLog(presentProgram, 1024, nullptr, infoLog);
      fprintf(stderr, "Failed to link present program.\n");
      fprintf(stderr, "%s\n", infoLog);
    }
  }
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  GLuint vboRect, vaoRect;
  glGenBuffers(1, &vboRect);
  glGenVertexArrays(1, &vaoRect);
  glBindVertexArray(vaoRect);
  glBindBuffer(GL_ARRAY_BUFFER, vboRect);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  GLuint texY, texU, texV;
  glGenTextures(1, &texY);
  glGenTextures(1, &texU);
  glGenTextures(1, &texV);

  glBindTexture(GL_TEXTURE_2D, texY);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, texU);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, texV);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glUseProgram(presentProgram);
  dec.seekFrame(0);
  dec.decodeCurrentFrame();
  auto start = std::chrono::steady_clock::now();
  while(!glfwWindowShouldClose(window))
  {
    double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) / 1e3;
    uint32_t nowFrame = static_cast<uint32_t>(std::fmod(duration, dec.duration()) * static_cast<double>(dec.framerate()));
    while(dec.currentFrameNumber() < nowFrame)
    {
      dec.nextFrame();
      printf("%d\n", dec.currentFrameNumber());
      dec.decodeCurrentFrame();
    }
    if(dec.currentFrameNumber() != nowFrame)
    {
      dec.seekFrame(0);
      dec.decodeCurrentFrame();
    }

    {
      glBindTexture(GL_TEXTURE_2D, texY);
      const auto &channel = dec.getCurrentFrame<uint8_t>(0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, channel.width(), channel.height(), 0, GL_RED, GL_UNSIGNED_BYTE, channel.data());
    }
    {
      glBindTexture(GL_TEXTURE_2D, texU);
      const auto &channel = dec.getCurrentFrame<uint8_t>(1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, channel.width(), channel.height(), 0, GL_RED, GL_UNSIGNED_BYTE, channel.data());
    }
    {
      glBindTexture(GL_TEXTURE_2D, texV);
      const auto &channel = dec.getCurrentFrame<uint8_t>(2);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, channel.width(), channel.height(), 0, GL_RED, GL_UNSIGNED_BYTE, channel.data());
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texY);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texU);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texV);

    glUniform1i(glGetUniformLocation(presentProgram, "texY"), 0);
    glUniform1i(glGetUniformLocation(presentProgram, "texU"), 1);
    glUniform1i(glGetUniformLocation(presentProgram, "texV"), 2);

    glBindVertexArray(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
}

int main()
{
  FILE *f = fopen("D:/codebase/lightvideo/reference/out.rcv", "rb");
  Decoder::SeekFunc seek = [=](int64_t pos) {
    if(fseek(f, static_cast<long>(pos), SEEK_SET))
      throw IOError("Failed to seek.");
  };
  Decoder::PosFunc pos = [=]() -> int64_t {
    int64_t pos = ftell(f);
    if(pos == -1)
      throw IOError("Failed to tell");
    return pos;
  };
  Decoder::ReadFunc read = [=](char *dest, int64_t size) {
    int64_t realSize = fread(dest, 1, size, f);
    if(realSize != size)
      throw IOError("Failed to read.");
  };

  Decoder *dec = new Decoder(read, seek, pos);
  play(*dec);
  delete dec;
  //system("pause");
  return 0;
}