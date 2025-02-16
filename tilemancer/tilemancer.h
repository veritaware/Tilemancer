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

#ifndef TILEMANCER_TILEMANCER_H
#define TILEMANCER_TILEMANCER_H

void initGL();
void LoadStuff();
void renderGL();
void update();
void tilemancer_close();
void resizeWindow(int w, int h);

// run old sdl2 app
int tilemancer_main();

#endif  // TILEMANCER_TILEMANCER_H
