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

#include "tilemancer/color.h"

Color::~Color() {}

Color::Color(int r, int g, int b) {
  this->r = r;
  this->g = g;
  this->b = b;
  this->a = 255;
  disabled = false;
}

bool Color::equals(Color* c) const {
  if (c->r == this->r && c->g == this->g && c->b == this->b) {
    return true;
  } else {
    return false;
  }
}
