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

#ifndef TILEMANCER_OS_H
#define TILEMANCER_OS_H

#include <string>
#include <vector>

#ifdef _WIN32
#define TILEMANCER_OS_WINDOWS
#include "os_windows.h"
#elif defined(__APPLE__)
#define TILEMANCER_OS_OSX
#define TILEMANCER_OS_UNIX
#include "os_osx.h"
#elif defined(__linux__)
#define TILEMANCER_OS_LINUX
#define TILEMANCER_OS_UNIX
#else
#error "Unsupported OS"
#endif

extern const char OS_SEPARATOR_CHAR;
extern const std::string OS_SEPARATOR_STRING;

const std::string executable_path();

std::string GetFolder(const std::string& folder);

std::vector<std::string> FilesInFolder(const std::string& folder,
                                       const char* ext);

#endif  // TILEMANCER_OS_H