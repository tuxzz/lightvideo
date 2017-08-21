#version 400 core
noperspective in vec3 vertPos;
noperspective in vec2 texCoord;
uniform sampler2D texY, texU, texV;
out vec4 color;

const mat3 yuvrgbMat = mat3(
  1.0f, -1.21889418866994e-6f, 1.40199958865734f,
  1.0f, -0.344135678165337f, -0.714136155581812f,
  1.0f, 1.77200006607382f, 4.06298062919653e-8f
);

vec3 convertYUVToRGB(vec3 yuv)
{
  yuv.g -= 0.5f;
  yuv.b -= 0.5f;
  return clamp(yuv * yuvrgbMat, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
}

void main()
{
  float y = texture(texY, texCoord).r;
  float u = texture(texU, texCoord).r;
  float v = texture(texV, texCoord).r;
  vec3 rgb = convertYUVToRGB(vec3(y, u, v));
  color = vec4(rgb.r, rgb.g, rgb.b, 1.0f);
}