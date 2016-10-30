#version 330

layout(location = 0)

out vec4 fragColor;
uniform float rot;
uniform vec2 texSize;
uniform sampler2D tex;
uniform sampler2D texP;
in vec2 txc;

void main() {
  vec3 off = vec3(-1.0/texSize.x, 0.0, 1.0/texSize.x);

  vec2 size = vec2(2.0, 0.0);
  float s11 = texture(tex, txc).x;
  float s01 = texture(tex, txc+off.xy).x;
  float s21 = texture(tex, txc+off.zy).x;
  float s10 = texture(tex, txc+off.yx).x;
  float s12 = texture(tex, txc+off.yz).x;
  vec3 va = normalize(vec3(size.xy,s21-s01));
  vec3 vb = normalize(vec3(size.yx,s12-s10));
  vec3 bump = vec3(cross(va,vb));
  vec3 light = vec3(sin(rot), -cos(rot), 0.3);
  float d = max(0.0, dot(normalize(light), bump));
  vec2 ptxc = vec2(d, 0.0);
  vec3 colorFinal = vec3(texture(texP, ptxc));
  fragColor = vec4(colorFinal, 1.0);
}