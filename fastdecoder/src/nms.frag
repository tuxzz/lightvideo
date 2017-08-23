#version 400 core
noperspective in vec3 vertPos;
noperspective in vec2 texCoord;

uniform sampler2D curr, ref;
layout (location = 0) out vec4 color;

void main()
{
  vec4 uc = texture(curr, texCoord) * 255.0f;
  vec4 rc = texture(ref, texCoord) * 255.0f;
  color = mod(uc + rc, 256.0f) / 255.0f;
}