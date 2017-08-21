#version 400 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;
noperspective out vec3 vertPos;
noperspective out vec2 texCoord;

void main()
{
  vertPos = inPos;
  texCoord = inTexCoord;
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}