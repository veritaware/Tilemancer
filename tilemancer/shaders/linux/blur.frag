#version 330
#define M_PI 3.1415926535897932384626433832795

layout(location = 0)
out vec4 fragColor;

in vec2 txc;
uniform sampler2D tex;
uniform vec2 texSize;
uniform float v;
uniform bool tri;
uniform bool full;

void main() {
  float a = 0.0;
  vec2 dist = vec2(0.5, 0.5)-txc;
  a = ((length(dist)>0.5) ? 0.0 : 1.0);
  a = ((length(dist)<0.4) ? 0.0 : a);
  a = (tri ? 1.0 : a);
  float H = atan(dist.y, dist.x)/M_PI*180.0+180.0;
  H = (H==360 ? 0.0 : H);
  H = (tri ? v : H);
  float S = (tri ? txc.x/txc.y : 1.0);
  float V = (tri ? txc.y : 1.0);
  float C = V*S;
  float X = C*(1.0-abs(mod(H/60.0, 2)-1.0));
  float m = V-C;
  vec3 rgb = vec3(0.0, 0.0, 0.0);
  if(0 <= H && H < 60) {
    rgb = vec3(C, X, 0.0);
  }
  else if(60 <= H && H < 120) {
    rgb = vec3(X, C, 0.0);
  }
  else if(120 <= H && H < 180) {
    rgb = vec3(0.0, C, X);
  }
  else if(180 <= H && H < 240) {
    rgb = vec3(0.0, X, C);
  }
  else if(240 <= H && H < 300) {
    rgb = vec3(X, 0.0, C);
  }
  else if(300 <= H && H < 360) {
    rgb = vec3(C, 0.0, X);
  }
  rgb = rgb+m;
  float d = 10.0;
  vec3 finalColor = vec3(0.0, 0.0, 0.0);
  for(int x = 0; x < texSize.x; x++) {
    for(int y = 0; y < texSize.y; y++) {
      vec3 color = vec3(texture(tex, vec2(x/texSize.x, y/texSize.y)));
      vec3 dist = rgb-color;
      float d2 = length(dist);
      if(d2 < d) {
        d = d2;
        finalColor = color;
      }
    }
  }
  if(full) {
    finalColor = rgb;
  }
  fragColor = vec4(finalColor, a);
}
