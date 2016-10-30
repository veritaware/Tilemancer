#version 330

in vec4 position;
uniform int frame;
uniform vec2 frameSize;
uniform vec2 off;
uniform vec2 texSize;
uniform mat4 model;
uniform mat4 proj;
in vec2 t;
out vec2 txc;

void main() {
  gl_Position = (proj*model) * position;
  int tx = frame%5;
  int ty = frame/5;
  txc = vec2(((t.x+tx)*frameSize.x+off.x)/texSize.x, ((t.y+ty)*frameSize.y+off.y)/texSize.y);
}
