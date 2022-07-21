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

#ifndef TILEMANCER_SHADER_H
#define TILEMANCER_SHADER_H

#include "tilemancer/gl.h"

class Shader {
 public:
  void load(const char* const name, const char* const vert, const char* const frag);
  GLenum program;
  GLenum vertex_shader;
  GLenum fragment_shader;
};

#endif  // TILEMANCER_SHADER_H
