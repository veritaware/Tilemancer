/*

  Tilemancer
  Copyright (C) 2016  Lucca Pedersoli Junior

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "tilemancer/shader.h"

#include <string>
#include <iostream>
#include <vector>

namespace {
  bool GetShaderCompileStatus(GLuint object) {
    int r = GL_TRUE;
    glGetShaderiv(object, GL_COMPILE_STATUS, &r);
    return r == GL_TRUE;
  }

bool GetShaderLinkStatus(GLuint object) {
  int r = GL_TRUE;
  glGetShaderiv(object, GL_LINK_STATUS, &r);
  return r == GL_TRUE;
}

std::string GetShaderLog(GLuint shader) {
  int length = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  if( length <= 0) return "";
  const int max_length = length + 1;
  std::vector<char> str(max_length, 0);
  glGetShaderInfoLog(shader, max_length, &length, &str[0]);
  return &str[0];
}

std::string GetProgramLog(GLuint shader) {
  int length = 0;
  glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
  if( length <= 0) return "";
  const int max_length = length + 1;
  std::vector<char> str(max_length, 0);
  glGetProgramInfoLog(shader, max_length, &length, &str[0]);
  return &str[0];
}

}

void Shader::load(const char* const name, const char* const vert, const char* const frag) {
  program = glCreateProgram();
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Load Shader Sources
  glShaderSource(vertex_shader, 1, &vert, NULL);
  glShaderSource(fragment_shader, 1, &frag, NULL);

  // Compile The Shaders
  glCompileShader(vertex_shader);
  const bool vertex_compile = GetShaderCompileStatus(vertex_shader);

  glCompileShader(fragment_shader);
  const bool fragment_compile = GetShaderCompileStatus(fragment_shader);

  // Attach The Shader Objects To The Program Object
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  // Link The Program Object
  glLinkProgram(program);

  const bool program_link = GetShaderLinkStatus(program);

  if( vertex_compile == false ) {
    std::cerr << "Vertex compile errors("<<name<<"):\n" << GetShaderLog(vertex_shader) << "\n";
  }
  if( fragment_compile == false ) {
    std::cerr << "Fragment compile errors("<<name<<"):\n" << GetShaderLog(fragment_shader) << "\n";
  }
  if( program_link == false ) {
    std::cerr << "Link errors("<<name<<"):\n" << GetProgramLog(program) << "\n";
  }

  // Use The Program Object Instead Of Fixed Function OpenGL
  glUseProgram(program);
}
