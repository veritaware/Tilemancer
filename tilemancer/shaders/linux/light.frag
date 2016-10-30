#version 330

layout(location = 0)
out vec4 fragColor;

uniform vec3 color;
uniform vec3 color2;

in vec2 txc;

void main() {
  fragColor = vec4(color*(1.0-txc.x)+color2*txc.x, 1.0);
}
