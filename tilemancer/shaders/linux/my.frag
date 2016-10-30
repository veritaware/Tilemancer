#version 330

layout(location = 0)
out vec4 fragColor;

layout(location = 1)
out vec4 fragColorN;

layout(location = 2)
out vec4 fragColorMisc;

layout(location = 3)
out vec4 fragColorB;

uniform vec2 flip;
uniform sampler2D tex;
uniform float strength;
uniform float alpha;
uniform float depth;
uniform sampler2D texN;
in vec2 txc;

void main() {
  fragColor = vec4(texture(tex, txc).xyz/texture(tex, txc).w, texture(tex, txc).w*alpha);
}
