#include "../../fastdecoder/src/decoder.hpp"
#include "../../fastdecoder/src/error.hpp"
#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace LightVideoDecoder;
static int vw = 1280;
static int vh = 720;
static void framebufferSizeCallback(GLFWwindow *, int width, int height)
{
  vw = width;
  vh = height;
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
"noperspective out vec2 texCoord;\n"
"\n"
"void main()\n"
"{\n"
"  vertPos = inPos;\n"
"  texCoord = inTexCoord;\n"
"  gl_Position = vec4(inPos.x, inPos.y, inPos.z, 1.0);\n"
"}";

static const char *presentFrag =
"#version 400 core\n"
"noperspective in vec3 vertPos;\n"
"noperspective in vec2 texCoord;\n"
"uniform sampler2D texFS, texHS;\n"
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
"  float y = texture(texFS, texCoord).x;\n"
"  vec2 uv = texture(texHS, texCoord).xy;\n"
"  vec3 rgb = convertYUVToRGB(vec3(y, uv.r, uv.g));\n"
"  color = vec4(rgb.r, rgb.g, rgb.b, 1.0f);\n"
"}";

static void play(GLFWwindow *window, Decoder &dec)
{ 
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
  
  dec.seekFrame(0);
  dec.decodeCurrentFrame();
  auto start = std::chrono::steady_clock::now();
  while(!glfwWindowShouldClose(window))
  {
    double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()) / 1e3;
    uint32_t nowFrame = static_cast<uint32_t>(std::fmod(duration, dec.duration()) * static_cast<double>(dec.framerate()));
    //uint32_t nowFrame = (dec.currentFrameNumber() + 1) % dec.frameCount();
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
    GLuint texFS = dec.getCurrentFrameFS();
    GLuint texHS = dec.getCurrentFrameHS();
    glViewport(0, 0, vw, vh);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texFS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texHS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUseProgram(presentProgram);
    glUniform1i(glGetUniformLocation(presentProgram, "texFS"), 0);
    glUniform1i(glGetUniformLocation(presentProgram, "texHS"), 1);

    glBindVertexArray(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
}

void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
  if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " <<  message << std::endl;

  switch (source)
  {
  case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
  case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
  case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
  } std::cout << std::endl;

  switch (type)
  {
  case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
  case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
  case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
  case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
  case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
  case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
  case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
  } std::cout << std::endl;

  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
  case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
  case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
  case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
  } std::cout << std::endl;
  std::cout << std::endl;
}

int main()
{
  /* initialize opengl */
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "LightPlay", nullptr, nullptr);
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
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glDebugMessageCallback(glDebugOutput, nullptr);
  glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE); 
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
  //speedtest(*dec);
  play(window, *dec);
  delete dec;
  system("pause");
  return 0;
}