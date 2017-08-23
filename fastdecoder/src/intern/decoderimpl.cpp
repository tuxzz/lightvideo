#include "decoderimpl_p.hpp"
#include <atomic>
#include <mutex>

namespace LightVideoDecoder
{
  static std::mutex lock;
  static std::atomic_int refCount = 0;
  GLuint vboRect, vaoRect;
  GLuint progNMS;

  static const GLfloat rectangle[] = {
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f
  };

  static const char *vertShaderSrc = 
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

  static const char *fragNMSSrc =
    "#version 400 core\n"
    "noperspective in vec3 vertPos;\n"
    "noperspective in vec2 texCoord;\n"
    "\n"
    "uniform sampler2D curr, ref;\n"
    "layout (location = 0) out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "  vec4 uc = texture(curr, texCoord) * 255.0f;\n"
    "  vec4 rc = texture(ref, texCoord) * 255.0f;\n"
    "  color = mod(uc + rc, 256.0f) / 255.0f;\n"
    "}";

  void initializeDecoder()
  {
    std::unique_lock<std::mutex> locker(lock);
    if(refCount++ == 0)
    {
      GLuint vertShader, fragNMS;
      vertShader = glCreateShader(GL_VERTEX_SHADER);
      fragNMS = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(vertShader, 1, &vertShaderSrc, nullptr);
      glCompileShader(vertShader);
      glShaderSource(fragNMS, 1, &fragNMSSrc, nullptr);
      glCompileShader(fragNMS);

      // chkerr
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
        glGetShaderiv(fragNMS, GL_COMPILE_STATUS, &success);
        if(!success)
        {
          glGetShaderInfoLog(fragNMS, 1024, nullptr, infoLog);
          fprintf(stderr, "Failed to compile fragPrev shader.\n");
          fprintf(stderr, "%s\n", infoLog);
        }
      } // chkerr
      progNMS = glCreateProgram();
      glAttachShader(progNMS, vertShader);
      glAttachShader(progNMS, fragNMS);
      glLinkProgram(progNMS);

      glDeleteShader(vertShader);
      glDeleteShader(fragNMS);
      
      glGenBuffers(1, &vboRect);
      glGenVertexArrays(1, &vaoRect);
      glBindVertexArray(vaoRect);
      glBindBuffer(GL_ARRAY_BUFFER, vboRect);
      glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
      glEnableVertexAttribArray(1);
    } // if
  }

  void destroyDecoder()
  {
    std::unique_lock<std::mutex> locker(lock);
    if(--refCount == 0)
    {
      glDeleteVertexArrays(1, &vaoRect);
      glDeleteBuffers(1, &vboRect);
      glDeleteProgram(progNMS);
    }
  }

  void drawNMS()
  {
    glUseProgram(progNMS);
    glUniform1i(glGetUniformLocation(progNMS, "curr"), 0);
    glUniform1i(glGetUniformLocation(progNMS, "ref"), 1);
    
    glBindVertexArray(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
} // namespace LightVideoDecoder
