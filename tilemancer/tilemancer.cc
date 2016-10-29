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

#include "os.h"

#include <dirent.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "lua.hpp"

#include "tilemancer/file.h"
#include "tilemancer/bpoint.h"
#include "tilemancer/bezier.h"
#include "tilemancer/color.h"
#include "tilemancer/cpoint.h"
#include "tilemancer/text.h"
#include "tilemancer/effect.h"
#include "tilemancer/palette.h"
#include "tilemancer/render.h"
#include "tilemancer/math.h"
#include "tilemancer/undoredo.h"
#include "tilemancer/saveload.h"
#include "tilemancer/globals.h"
#include "tilemancer/drag.h"
#include "tilemancer/socket.h"
#include "tilemancer/texture.h"
#include "tilemancer/parameter.h"

using namespace std;

SDL_Window* window = NULL;
SDL_GLContext gContext = NULL;
SDL_Renderer* gRenderer = NULL;

class Layer;

#ifdef _WIN32
GLuint loadTexture(string path) {
  GLuint tex;
  unzFile data = unzOpen("data.lpf");
  unz_file_info info;
  Uint8* buffer;
  SDL_RWops* rw = NULL;
  SDL_Surface* ls = NULL;

  unzLocateFile(data, path.c_str(), NULL);
  unzOpenCurrentFile(data);
  unzGetCurrentFileInfo(data, &info, NULL, 0, NULL, 0, NULL, 0);
  buffer = (Uint8*)malloc(info.uncompressed_size);
  unzReadCurrentFile(data, buffer, info.uncompressed_size);
  rw = SDL_RWFromConstMem(buffer, info.uncompressed_size);
  ls = IMG_LoadPNG_RW(rw);
  free(buffer);

  if (ls != NULL) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    int mode = GL_RGB;
    int mode2 = GL_BGR;
    if (ls->format->BytesPerPixel == 4) {
      mode = GL_RGBA;
      mode2 = GL_BGRA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, mode, ls->w, ls->h, 0, mode,
                 GL_UNSIGNED_BYTE, ls->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // SDL_FreeSurface(ls);
  } else {
    // cout << "Failed to load image: "+path << endl;
  }
  return tex;
}

GLuint loadTexture2(string path) {
  GLuint tex;
  SDL_Surface* ls = IMG_Load(path.c_str());
  if (ls != NULL) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    int mode = GL_RGB;
    int mode2 = GL_BGR;
    if (ls->format->BytesPerPixel == 4) {
      mode = GL_RGBA;
      mode2 = GL_BGRA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, mode, ls->w, ls->h, 0, mode,
                 GL_UNSIGNED_BYTE, ls->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // SDL_FreeSurface(ls);
  } else {
    // cout << "Failed to load image: "+path << endl;
  }
  return tex;
}
#elif defined(__APPLE__) || defined(__linux__)
GLuint loadTexture(string path) {
  GLuint tex;
  SDL_Surface* ls = IMG_Load(path.c_str());
  if (ls != NULL) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    int mode = GL_RGB;
    int mode2 = GL_BGR;
    if (ls->format->BytesPerPixel == 4) {
      mode = GL_RGBA;
      mode2 = GL_BGRA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, mode, ls->w, ls->h, 0, mode2,
                 GL_UNSIGNED_BYTE, ls->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // SDL_FreeSurface(ls);
  } else {
    cout << "Failed to load image: " + path << endl;
  }
  return tex;
}

GLuint loadTexture2(string path) {
  cout << "Loading texture2 " << path << endl;
  return 0;
}
#else
#endif

void exportTexSingle(string dir) {
  SDL_Surface* surface =
      SDL_CreateRGBSurface(SDL_SWSURFACE, texSizeX, texSizeY, 32, 0x000000FF,
                           0x0000FF00, 0x00FF0000, 0xFF000000);
  glBindTexture(GL_TEXTURE_2D, currentSocket->texture);
  GLfloat* pixels = new GLfloat[int(texSizeX * texSizeY * 4)];
  if (OS & Windows) {
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
  } else if (OS & Unix) {
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);
    for (int i = 0; i < texSizeX * texSizeY * 4; i++) {
      Uint8* p =
          (Uint8*)surface->pixels + i * surface->format->BytesPerPixel / 4;
      int val = pixels[i];
      if (val > 255) {
        val = 255;
      } else if (val < 0) {
        val = 0;
      }
      p[0] = (Uint8)val;
    }
  }
  IMG_SavePNG(surface, dir.c_str());
  SDL_FreeSurface(surface);
}

void exportPalette(string dir) {
  SDL_Surface* surface = SDL_CreateRGBSurface(
      SDL_SWSURFACE, 16, 16, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
  glBindTexture(GL_TEXTURE_2D, palImgReal);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
  IMG_SavePNG(surface, dir.c_str());
  SDL_FreeSurface(surface);
}

void initLua() {
  L = luaL_newstate();
  luaL_openlibs(L);
}

static string executable_path() {
  char cwd[1024];
  uint32_t size = sizeof(cwd);
#if defined(__WIN32)
  GetModuleFileName(NULL, cwd, size);
#elif defined(__APPLE__)
  _NSGetExecutablePath(cwd, &size);
#else
  ssize_t readSize = readlink("/proc/self/exe", cwd, size);
  if (readSize <= 0 || readSize == size) {
    printf("Could not determine executable path!\n");
    exit(1);
  }
  cwd[readSize] = 0;
#endif
  return string(cwd);
}

#ifdef _WIN32
void importFxs() {
  string cwd2 = executable_path();
  cwd2.erase(cwd2.rfind('\\'));
  if (cwd2.size() < 1) {
    cwd2 = "\\Nodes";
  } else {
    cwd2 += "\\Nodes";
  }
  DIR* dirr;
  struct dirent* ent;
  vector<File*> temp;
  if ((dirr = opendir(cwd2.c_str())) != NULL) {
    while ((ent = readdir(dirr)) != NULL) {
      if (ent->d_name[0] != '.') {
        string fn = ent->d_name;
        if (fn.substr(fn.find_last_of(".") + 1) == "lua") {
          string fullfn = cwd2 + "\\" + fn;
          lua_settop(L, 0);
          lua_pushnil(L);
          lua_setglobal(L, "init");
          lua_settop(L, 0);
          lua_pushnil(L);
          lua_setglobal(L, "apply");
          int s = luaL_dofile(L, fullfn.c_str());
          if (s == 0) {
            // cout << "Loaded" << endl;
            // checking if everything works
            bool OK = true;
            lua_getglobal(L, "init");
            lua_getglobal(L, "apply");
            if (!lua_isfunction(L, -2)) {
              OK = false;
            }
            if (!lua_isfunction(L, -1)) {
              OK = false;
            }
            if (OK) {
              // cout << "Success" << endl;
              newEffects.push_back(new Effect(fullfn, fn));
            }
          }
        }
      }
    }
    closedir(dirr);
  } else {
  }
}

void importPresets() {
  string cwd2 = executable_path();
  cwd2.erase(cwd2.rfind('\\'));
  if (cwd2.size() < 1) {
    cwd2 = "\\Presets";
  } else {
    cwd2 += "\\Presets";
  }
  DIR* dirr;
  struct dirent* ent;
  vector<File*> temp;
  if ((dirr = opendir(cwd2.c_str())) != NULL) {
    while ((ent = readdir(dirr)) != NULL) {
      if (ent->d_name[0] != '.') {
        string fn = ent->d_name;
        if (fn.substr(fn.find_last_of(".") + 1) == "lua") {
          string fullfn = cwd2 + "\\" + fn;
          lua_settop(L, 0);
          lua_pushnil(L);
          lua_setglobal(L, "init");
          lua_settop(L, 0);
          lua_pushnil(L);
          lua_setglobal(L, "apply");
          int s = luaL_dofile(L, fullfn.c_str());
          if (s == 0) {
            // cout << "Loaded" << endl;
            // checking if everything works
            bool OK = true;
            lua_getglobal(L, "init");
            lua_getglobal(L, "apply");
            if (!lua_isfunction(L, -2)) {
              OK = false;
            }
            if (!lua_isfunction(L, -1)) {
              OK = false;
            }
            if (OK) {
              // cout << "Success" << endl;
              newEffects.push_back(new Effect(fullfn, fn, true));
            }
          }
        }
      }
    }
    closedir(dirr);
  } else {
  }
}
#elif defined(__APPLE__) || defined(__linux__)
void importFxs() {
  string cwd2 = executable_path();
  cwd2.erase(cwd2.rfind('/'));
#if defined(__APPLE__)
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
#endif
  if (cwd2.size() < 1) {
    cwd2 = "/Nodes";
  } else {
    cwd2 += "/Nodes";
  }

  int num_entries;
  struct dirent** entries = NULL;
  num_entries = scandir(cwd2.c_str(), &entries, NULL, NULL);

  for (int i = 0; i < num_entries; i++) {
    if (entries[i]->d_name[0] != '.') {
      string fn = entries[i]->d_name;
      if (fn.substr(fn.find_last_of(".") + 1) == "lua") {
        string fullfn = cwd2 + "/" + fn;
        lua_settop(L, 0);
        lua_pushnil(L);
        lua_setglobal(L, "init");
        lua_settop(L, 0);
        lua_pushnil(L);
        lua_setglobal(L, "apply");
        int s = luaL_dofile(L, fullfn.c_str());
        if (s == 0) {
          // cout << "Loaded" << endl;
          // checking if everything works
          bool OK = true;
          lua_getglobal(L, "init");
          lua_getglobal(L, "apply");
          if (!lua_isfunction(L, -2)) {
            OK = false;
          }
          if (!lua_isfunction(L, -1)) {
            OK = false;
          }
          if (OK) {
            // cout << "Success" << endl;
            newEffects.push_back(new Effect(fullfn, fn));
          }
        }
      }
    }
  }
}
void importPresets() {
  string cwd2 = executable_path();
  cwd2.erase(cwd2.rfind('/'));
#if defined(__APPLE__)
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
#endif
  if (cwd2.size() < 1) {
    cwd2 = "/Presets";
  } else {
    cwd2 += "/Presets";
  }

  int num_entries;
  struct dirent** entries = NULL;
  num_entries = scandir(cwd2.c_str(), &entries, NULL, NULL);

  for (int i = 0; i < num_entries; i++) {
    if (entries[i]->d_name[0] != '.') {
      string fn = entries[i]->d_name;
      if (fn.substr(fn.find_last_of(".") + 1) == "lua") {
        string fullfn = cwd2 + "/" + fn;
        lua_settop(L, 0);
        lua_pushnil(L);
        lua_setglobal(L, "init");
        lua_settop(L, 0);
        lua_pushnil(L);
        lua_setglobal(L, "apply");
        int s = luaL_dofile(L, fullfn.c_str());
        if (s == 0) {
          // cout << "Loaded" << endl;
          // checking if everything works
          bool OK = true;
          lua_getglobal(L, "init");
          lua_getglobal(L, "apply");
          if (!lua_isfunction(L, -2)) {
            OK = false;
          }
          if (!lua_isfunction(L, -1)) {
            OK = false;
          }
          if (OK) {
            // cout << "Success" << endl;
            newEffects.push_back(new Effect(fullfn, fn, true));
          }
        }
      }
    }
  }
}
#else
#endif

void getHome();

void loadGen() {
  texSizeX = 32;
  texSizeY = texSizeX;

  nodeCX = 0;
  nodeCY = 0;

  gridImg = loadTexture("resources/grid.png");
  effectImg = loadTexture("resources/effect0.png");
  effectImg2 = loadTexture("resources/effect1.png");
  effectImg3 = loadTexture("resources/effect2.png");
  effectImg4 = loadTexture("resources/effect3.png");
  effectImg5 = loadTexture("resources/effect4.png");
  effectImg6 = loadTexture("resources/effect5.png");
  effectImg8 = loadTexture("resources/effect7.png");
  effectImg10 = loadTexture("resources/effect9.png");
  effectImg11 = loadTexture("resources/effect10.png");
  effectImg14 = loadTexture("resources/effect13.png");
  effectImg15 = loadTexture("resources/effect14.png");
  effectImg16 = loadTexture("resources/effect15.png");
  bezierFill = loadTexture("resources/bezierFill.png");
  bezierFillError = loadTexture("resources/bezierFillError.png");
  iconImg2 = loadTexture("resources/icon2.png");
  iconImg3 = loadTexture("resources/icon3.png");
  iconImg6 = loadTexture("resources/icon6.png");
  iconImg7 = loadTexture("resources/icon7.png");
  iconImg8 = loadTexture("resources/icon8.png");
  iconImg9 = loadTexture("resources/icon9.png");
  iconImg10 = loadTexture("resources/icon10.png");
  iconImg11 = loadTexture("resources/icon11.png");
  iconImg12 = loadTexture("resources/icon12.png");
  iconImg13 = loadTexture("resources/icon13.png");
  palImg = loadTexture("resources/pal.png");
  fontImg = loadTexture("resources/font.png");
  fontImg2 = loadTexture("resources/font2.png");

  glGenTextures(1, &screenTexFinal2);
  glBindTexture(GL_TEXTURE_2D, screenTexFinal2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSizeX, texSizeY, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenFramebuffers(1, &screenFboFinal2);
  glBindFramebuffer(GL_FRAMEBUFFER, screenFboFinal2);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screenTexFinal2, 0);
  GLenum d2[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, d2);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  barY = screenH - 150;
  barX = 128 + 7;
  barXRight = barX;
  barY3 = expH;

  getHome();

  currentTexture = 0;

  Texture* t = new Texture();
  texs.push_back(t);

  initLua();

  // IMPORT FXS
  importFxs();
  importPresets();

  t->genTex();

  int space = 4;
  int s = 36;
  int s2 = (barXRight - 7 - space * 4) - s;
  int paramSize = s2 / 2.0;
  int paramH = s / 3.0;
  int title = collH - 4;
  int offset = (paramH - 7) / 4.0;
  colorParams.push_back(
      new Parameter(4, "c", space, space, s, s, 0, 0, 1, "Color"));

  colorParams.push_back(new Parameter(1, "R", space + s + space + offset,
                                      space + offset, paramSize - offset * 2,
                                      paramH, 0, 0, 255, "Red component"));
  colorParams.push_back(new Parameter(
      1, "G", space + s + space + offset, space + s / 2.0 - (paramH) / 2.0,
      paramSize - offset * 2, paramH, 0, 0, 255, "Green component"));
  colorParams.push_back(new Parameter(
      1, "B", space + s + space + offset, space + s - (paramH)-offset,
      paramSize - offset * 2, paramH, 0, 0, 255, "Blue component"));
  colorParams.push_back(new Parameter(
      1, "H", space + s + space * 2 + offset + paramSize, space + offset,
      paramSize - offset * 2, paramH, 0, 0, 360, "Hue component"));
  colorParams.push_back(
      new Parameter(1, "S", space + s + space * 2 + offset + paramSize,
                    space + s / 2.0 - (paramH) / 2.0, paramSize - offset * 2,
                    paramH, 0, 0, 100, "Saturation component"));
  colorParams.push_back(
      new Parameter(1, "V", space + s + space * 2 + offset + paramSize,
                    space + s - (paramH)-offset, paramSize - offset * 2, paramH,
                    0, 0, 100, "Value component"));
  barY2 = barY3 + 2 + space * 3 + s + title + 12;
  int c = 8;
  int w = barXRight - 6 + space - offset * 2;
  float size = w / float(c);
  colorParams.push_back(new Parameter(3, "Indexed", space, space * 2 + s,
                                      barXRight - 7 - space * 2, title, 0, 0, 1,
                                      "Indexed mode"));
  colorParams.push_back(new Parameter(
      1, "Size", space, space * 2 + s + 3 + 9 + 2 + 14 + 4,
      barXRight - 7 - space * 2, title, texSizeX, 1, 64, "Tile size"));
  barY4 = barY3 + 2 + 12 + space * 2 + s + 3 + 9 + 2 + 14 + 4 + title + space;
  barY5 = barY4 + 3 + 3 + 8 + 4 + 96;

  loadPalette();
}

void initGL() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  proj = glm::ortho(0.0, (double)screenW * screenScale,
                    (double)screenH * screenScale, 0.0, -1.0, 1.0);
}

void browserAction(string dir, string subDir, string parent) {
  bool exists = false;
  for (int i = 0; i < filenames.size(); i++) {
    if (!strcmp(filenames.at(i)->name.c_str(), subDir.c_str())) {
      exists = true;
    }
  }
  if (browserMode == 0) {
    if (exists) {
      if (OS & Windows) {
        palImg = loadTexture2(dir);
      } else if (OS & Unix) {
        palImg = loadTexture(dir);
      }
      loadPalette();
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    }
  } else if (browserMode == 1) {
    if (!exists || overwrite) {
      exportPalette(dir);
      lastPalName = subDir;
      lastPalDir = parent;
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    } else {
      overwrite = true;
    }
  } else if (browserMode == 2) {
    if (!exists || overwrite) {
      exportTexSingle(dir);
      currentSocket->lastTexName = subDir;
      currentSocket->lastTexDir = parent;
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    } else {
      overwrite = true;
    }
  } else if (browserMode == 3) {
    if (!exists || overwrite) {
      // exportTexMulti(dir);
      lastTexName = subDir;
      lastTexDir = parent;
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    } else {
      overwrite = true;
    }
  } else if (browserMode == 4) {
    if (exists) {
      listUndo.clear();
      listRedo.clear();
      loadFile(dir);
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    }
  } else if (browserMode == 5) {
    if (!exists || overwrite) {
      lastSaveName = subDir;
      lastSaveDir = parent;
      saveFile(dir);
      browserOpen = false;
      fnUndo.clear();
      fnRedo.clear();
    } else {
      overwrite = true;
    }
  }
}

#ifdef _WIN32
void openBrowser(string dir, int type, int mode) {
  overwrite = false;
  browserMode = mode;
  browserScroll = 0;
  if (type == 1) {  // undo
    fnUndo.pop_back();
    fnRedo.push_back(currentDir);
  } else if (type == 2) {  // redo
    fnRedo.pop_back();
    fnUndo.push_back(currentDir);
  } else {
    fnUndo.push_back(currentDir);
    fnRedo.clear();
  }
  selectedFile = -1;
  currentDir = dir;
  filenameB = "";
  if (browserMode == 1 && lastPalDir.size() > 0 && !browserOpen) {
    currentDir = lastPalDir;
    filenameB = lastPalName;
  }
  Texture* t = texs.at(currentTexture);
  if (browserMode == 2 && currentSocket->lastTexDir.size() > 0 &&
      !browserOpen) {
    currentDir = currentSocket->lastTexDir;
    filenameB = currentSocket->lastTexName;
  }
  if (browserMode == 3 && lastTexDir.size() > 0 && !browserOpen) {
    currentDir = lastTexDir;
    filenameB = lastTexName;
  }
  if (browserMode == 5 && lastSaveDir.size() > 0 && !browserOpen) {
    currentDir = lastSaveDir;
    filenameB = lastSaveName;
  }
  for (int toDel = 0; toDel < filenames.size(); toDel++) {
    delete filenames.at(toDel);
  }
  filenames.clear();

  DIR* dirr;
  struct dirent* ent;
  vector<File*> temp;
  if ((dirr = opendir(currentDir.c_str())) != NULL) {
    while ((ent = readdir(dirr)) != NULL) {
      if (ent->d_name[0] != '.') {
        string fullDir = currentDir;
        fullDir = fullDir.append("\\");
        fullDir = fullDir.append(string(ent->d_name));
        struct stat path_stat;
        stat(fullDir.c_str(), &path_stat);
        File* a = new File(string(ent->d_name), !S_ISREG(path_stat.st_mode));
        if (a->folder) {
          filenames.push_back(a);
        } else {
          bool compatible = true;
          // SDL_RWops *rwop = SDL_RWFromFile(fullDir.c_str(), "rb");
          /*compatible = compatible | IMG_isBMP(rwop);
           compatible = compatible | IMG_isCUR(rwop);
           compatible = compatible | IMG_isGIF(rwop);
           compatible = compatible | IMG_isICO(rwop);
           compatible = compatible | IMG_isJPG(rwop);
           compatible = compatible | IMG_isLBM(rwop);
           compatible = compatible | IMG_isPCX(rwop);
           compatible = compatible | IMG_isPNG(rwop);
           compatible = compatible | IMG_isPNM(rwop);
           compatible = compatible | IMG_isTIF(rwop);
           compatible = compatible | IMG_isWEBP(rwop);
           compatible = compatible | IMG_isXCF(rwop);
           compatible = compatible | IMG_isXPM(rwop);
           compatible = compatible | IMG_isXV(rwop);*/
          if (compatible) {
            temp.push_back(a);
          }
        }
      }
    }
    closedir(dirr);
  } else {
  }
  filenames.insert(filenames.end(), temp.begin(), temp.end());
  browserOpen = true;
}

void setIcon() {
  unzFile data = unzOpen("data.lpf");
  unz_file_info info;
  Uint8* buffer;
  SDL_RWops* rw = NULL;
  SDL_Surface* ls = NULL;

  unzLocateFile(data, "icon.ico", NULL);
  unzOpenCurrentFile(data);
  unzGetCurrentFileInfo(data, &info, NULL, 0, NULL, 0, NULL, 0);
  buffer = (Uint8*)malloc(info.uncompressed_size);
  unzReadCurrentFile(data, buffer, info.uncompressed_size);
  rw = SDL_RWFromConstMem(buffer, info.uncompressed_size);
  ls = IMG_LoadICO_RW(rw);
  free(buffer);

  SDL_SetWindowIcon(window, ls);
}

void initGlew() {
  glewExperimental = GL_TRUE;
  glewInit();
}

void getHome() {
  const char* homeDir;
  homeDir = getenv("HOMEPATH");
  currentDir = homeDir;
}

void warpMouse(int x, int y) { SDL_WarpMouseInWindow(window, x, y); }
#elif defined(__APPLE__) || defined(__linux__)
void openBrowser(string dir, int type, int mode) {
  overwrite = false;
  browserMode = mode;
  browserScroll = 0;
  if (type == 1) {  // undo
    fnUndo.pop_back();
    fnRedo.push_back(currentDir);
  } else if (type == 2) {  // redo
    fnRedo.pop_back();
    fnUndo.push_back(currentDir);
  } else {
    fnUndo.push_back(currentDir);
    fnRedo.clear();
  }
  selectedFile = -1;
  currentDir = dir;
  filenameB = "";
  if (browserMode == 1 && lastPalDir.size() > 0 && !browserOpen) {
    currentDir = lastPalDir;
    filenameB = lastPalName;
  }
  Texture* t = texs.at(currentTexture);
  if (browserMode == 2 && currentSocket->lastTexDir.size() > 0 &&
      !browserOpen) {
    currentDir = currentSocket->lastTexDir;
    filenameB = currentSocket->lastTexName;
  }
  if (browserMode == 3 && lastTexDir.size() > 0 && !browserOpen) {
    currentDir = lastTexDir;
    filenameB = lastTexName;
  }
  if (browserMode == 5 && lastSaveDir.size() > 0 && !browserOpen) {
    currentDir = lastSaveDir;
    filenameB = lastSaveName;
  }
  for (int toDel = 0; toDel < filenames.size(); toDel++) {
    delete filenames.at(toDel);
  }
  filenames.clear();
  int num_entries;
  struct dirent** entries = NULL;
  num_entries = scandir(currentDir.c_str(), &entries, NULL, NULL);
  vector<File*> temp;
  for (int i = 0; i < num_entries; i++) {
    if (entries[i]->d_name[0] != '.') {
      string fullDir = currentDir;
      fullDir = fullDir.append("/");
      fullDir = fullDir.append(string(entries[i]->d_name));
      struct stat path_stat;
      stat(fullDir.c_str(), &path_stat);
      File* a =
          new File(string(entries[i]->d_name), !S_ISREG(path_stat.st_mode));
      if (a->folder) {
        filenames.push_back(a);
      } else {
        temp.push_back(a);
      }
    }
  }
  filenames.insert(filenames.end(), temp.begin(), temp.end());
  for (int i = 0; i < num_entries; i++) {
    free(entries[i]);
  }
  free(entries);
  browserOpen = true;
}

void getHome() {
  char* homeDir;
  homeDir = getenv("HOME");
  if (!homeDir) {
    struct passwd* pwd = getpwuid(getuid());
    if (pwd) {
      homeDir = pwd->pw_dir;
    }
  }
  currentDir = homeDir;
}

void warpMouse(int x, int y) { SDL_WarpMouseGlobal(x, y); }
#else
#endif

void LoadStuff() {
  srand(time(NULL));

  light_program = glCreateProgram();
  light_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  light_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Load Shader Sources
  glShaderSource(light_vertex_shader, 1, &light_vertex_shader_source, NULL);
  glShaderSource(light_fragment_shader, 1, &light_fragment_shader_source, NULL);

  // Compile The Shaders
  glCompileShader(light_vertex_shader);
  glCompileShader(light_fragment_shader);

  // Attach The Shader Objects To The Program Object
  glAttachShader(light_program, light_vertex_shader);
  glAttachShader(light_program, light_fragment_shader);

  // Link The Program Object
  glLinkProgram(light_program);

  // Use The Program Object Instead Of Fixed Function OpenGL
  glUseProgram(light_program);

  blur_program = glCreateProgram();
  blur_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  blur_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Load Shader Sources
  glShaderSource(blur_vertex_shader, 1, &blur_vertex_shader_source, NULL);
  glShaderSource(blur_fragment_shader, 1, &blur_fragment_shader_source, NULL);

  // Compile The Shaders
  glCompileShader(blur_vertex_shader);
  glCompileShader(blur_fragment_shader);

  // Attach The Shader Objects To The Program Object
  glAttachShader(blur_program, blur_vertex_shader);
  glAttachShader(blur_program, blur_fragment_shader);

  // Link The Program Object
  glLinkProgram(blur_program);

  // Use The Program Object Instead Of Fixed Function OpenGL
  glUseProgram(blur_program);

  transition_program = glCreateProgram();
  transition_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  transition_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Load Shader Sources
  glShaderSource(transition_vertex_shader, 1, &transition_vertex_shader_source,
                 NULL);
  glShaderSource(transition_fragment_shader, 1,
                 &transition_fragment_shader_source, NULL);

  // Compile The Shaders
  glCompileShader(transition_vertex_shader);
  glCompileShader(transition_fragment_shader);

  // Attach The Shader Objects To The Program Object
  glAttachShader(transition_program, transition_vertex_shader);
  glAttachShader(transition_program, transition_fragment_shader);

  // Link The Program Object
  glLinkProgram(transition_program);

  // Use The Program Object Instead Of Fixed Function OpenGL
  glUseProgram(transition_program);

  my_program = glCreateProgram();
  my_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  my_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Load Shader Sources
  glShaderSource(my_vertex_shader, 1, &my_vertex_shader_source, NULL);
  glShaderSource(my_fragment_shader, 1, &my_fragment_shader_source, NULL);

  // Compile The Shaders
  glCompileShader(my_vertex_shader);
  glCompileShader(my_fragment_shader);

  // Attach The Shader Objects To The Program Object
  glAttachShader(my_program, my_vertex_shader);
  glAttachShader(my_program, my_fragment_shader);

  // Link The Program Object
  glLinkProgram(my_program);

  // Use The Program Object Instead Of Fixed Function OpenGL
  glUseProgram(my_program);

  mp_tex = glGetUniformLocation(my_program, "tex");
  mp_texN = glGetUniformLocation(my_program, "texN");
  mp_frameSize = glGetUniformLocation(my_program, "frameSize");
  mp_texSize = glGetUniformLocation(my_program, "texSize");
  mp_off = glGetUniformLocation(my_program, "off");
  mp_frame = glGetUniformLocation(my_program, "frame");
  mp_depth = glGetUniformLocation(my_program, "depth");
  mp_alpha = glGetUniformLocation(my_program, "alpha");
  mp_strength = glGetUniformLocation(my_program, "strength");
  mp_model = glGetUniformLocation(my_program, "model");
  mp_flip = glGetUniformLocation(my_program, "flip");
  mp_proj = glGetUniformLocation(my_program, "proj");

  float texRect[] = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0,
                     1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                     1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0};
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texRect), texRect, GL_STATIC_DRAW);
  // glVertexAttribPointer(shaderPos, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(glGetAttribLocation(my_program, "position"));

  float texTri[] = {-float(0.5 * sqrt(3) / 3.0),
                    -0.5,
                    0.0,
                    0.0,
                    float(0.5 * sqrt(3)) - float(0.5 * sqrt(3) / 3.0),
                    0.0,
                    1.0,
                    1.0,
                    -float(0.5 * sqrt(3) / 3.0),
                    0.5,
                    0.0,
                    1.0};
  glGenBuffers(1, &vboTri);
  glBindBuffer(GL_ARRAY_BUFFER, vboTri);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texTri), texTri, GL_STATIC_DRAW);

  glGenTextures(1, &screenTex);
  glBindTexture(GL_TEXTURE_2D, screenTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexN);
  glBindTexture(GL_TEXTURE_2D, screenTexN);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexMisc);
  glBindTexture(GL_TEXTURE_2D, screenTexMisc);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexB);
  glBindTexture(GL_TEXTURE_2D, screenTexB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenFramebuffers(1, &screenFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screenTex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         screenTexN, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         screenTexMisc, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
                         screenTexB, 0);
  GLenum d[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, d);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  logoImage = loadTexture("resources/logo.png");
  camOffset = 1.0;
  camOffsetMagnitude = 1.2;

  loadGen();

  logoTimer = logoTimerMax;

#ifdef __APPLE__
  mainBundle = CFBundleGetMainBundle();
#endif
}

int textW(string text, int x, int y, GLuint tex, bool alignRight) {
  int kerning[] = {2,  2, 2, 2, 3, 3, 2, 2, 6, 3, 3, 3, 1, 3, 2, 2, 2, 2, 3,
                   2,  3, 2, 1, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 5, 4, 3, 5,
                   -1, 2, 2, 2, 2, 2, 3, 3, 2, 2, 1, 3, 2, 2, 2, 2, 6,

                   6,  5, 3, 3, 2, 2, 6, 5, 5, 3, 3, 6, 4, 6, 2, 6, 6, 3, 4,
                   3,  2, 1, 5, 4, 5, 5, 4, 4, 6, 4, 4};
  int kerningD[] = {3, 5, 3, 3, 3, 3, 3, 4, 3, 3, 2, 4};
  int w = -1;
  for (int i = 0; i < text.size(); i++) {
    if (text[i] >= 33 && text[i] <= 47) {
      w += 9 - kerning[55 + text[i] - 33];
    } else if (text[i] >= 48 && text[i] <= 57) {
      w += 9 - kerningD[text[i] - 48];
    } else if (text[i] >= 58 && text[i] <= 64) {
      w += 9 - kerning[70 + text[i] - 58];
    } else if (text[i] >= 91 && text[i] <= 95) {
      w += 9 - kerning[77 + text[i] - 91];
    } else if (text[i] >= 123 && text[i] <= 126) {
      w += 9 - kerning[82 + text[i] - 123];
    } else if (text[i] == 32) {
      w += 9 - kerning[52];
    } else if ((text[i] >= 65 && text[i] <= 90) ||
               (text[i] >= 97 && text[i] <= 122)) {
      int f = text[i] - 'A';
      if (f >= 32) {
        f -= 6;
      }
      w += 9 - kerning[f];
    } else {
      w += 9 - kerning[0];
    }
  }
  int wd = w;
  return wd;
}

int textH(string text, int x, int y, GLuint tex, bool alignRight) {
  int h = 9;
  for (int i = 0; i < text.size(); i++) {
    if (text[i] == '\n') {
      h += 9;
    }
  }
  return h;
}

void adjustBrowserScroll() {
  int title = collH - 4;
  int bSpace = 5;
  float tS = title / 2 - 4;
  int fh = tS;
  int fw = -(screenW - barXRight - barX - bSpace * 2);
  int fwNext = 0;
  for (int i = 0; i < filenames.size(); i++) {
    int n = textW(filenames.at(i)->name, barX + 1 + bSpace + fw + tS + 8,
                  title * 2 + bSpace * 3 + fh, fontImg, 0);
    if (n > fwNext) {
      fwNext = n;
    }
    fh += title;
    if (fh + title * 2 + title + bSpace * 3 >
            title + bSpace * 2 + barY - 1 - bSpace * 3 - title - 6 &&
        i != filenames.size() - 1) {
      fh = tS;
      fw += fwNext + tS * 2 + 8 * 3;
      fwNext = 0;
    }
  }
  fw += fwNext + tS * 2 + 8 * 3;
  if (browserScroll > fw) {
    browserScroll = fw;
  }
  if (browserScroll < 0) {
    browserScroll = 0;
  }
}

void adjustToolsScroll() {
  int maxH = -(barY) + barY5;
  if (toolsScroll > maxH + 1) {
    toolsScroll = maxH + 1;
  }
  if (toolsScroll < 0) {
    toolsScroll = 0;
  }
  int maxHPal = 0;
  int space = 1;
  float offset = 0;
  int w = barXRight - 10 + space - offset * 2 - 7 - 7;
  float size = 12;
  int c = w / size;
  for (int p = 0; p < palette.size(); p++) {
    int y = p / c;
    if (y * (size) + size > maxHPal) {
      maxHPal = y * (size) + size;
    }
  }
  maxHPal -= barY3 - 2 - 5 - 8 - 5;
  if (palScroll > maxHPal) {
    palScroll = maxHPal;
  }
  if (palScroll < 0) {
    palScroll = 0;
  }
  int maxHTex = 0;
  int wT = barXRight - 2 - 7;
  float sizeT = texSizeX + 1;
  int cT = wT / sizeT;
  for (int p = 0; p < texs.size(); p++) {
    int x = p % cT;
    int y = p / cT;
    if (y * (sizeT) + sizeT + 1 > maxHTex) {
      maxHTex = y * (sizeT) + sizeT + 1;
    }
  }
  maxHTex -= screenH - barY - 5 - 8 - 5 - 8 - (14 - 8) / 2 - 5;
  if (texScroll > maxHTex) {
    texScroll = maxHTex;
  }
  if (texScroll < 0) {
    texScroll = 0;
  }
}

void adjustLayersScroll() {
  int hTop = -(barY);
  int count = newEffects.size();
  hTop += collH * count;
  if (newEScroll > hTop + 1) {
    newEScroll = hTop + 1;
  }
  if (newEScroll < 0) {
    newEScroll = 0;
  }
}

void adjustWidths() {
  int space = 4;
  int s = 36;
  int s2 = (barXRight - 7 - space * 4) - s;
  int paramSize = s2 / 2.0;
  int paramH = s / 3.0;
  int offset = (paramH - 7) / 4.0;
  colorParams.at(1)->w = paramSize - offset * 2;
  colorParams.at(2)->w = paramSize - offset * 2;
  colorParams.at(3)->w = paramSize - offset * 2;
  colorParams.at(4)->x = space + s + space * 2 + offset + paramSize;
  colorParams.at(5)->x = space + s + space * 2 + offset + paramSize;
  colorParams.at(6)->x = space + s + space * 2 + offset + paramSize;
  colorParams.at(4)->w = paramSize - offset * 2;
  colorParams.at(5)->w = paramSize - offset * 2;
  colorParams.at(6)->w = paramSize - offset * 2;
  colorParams.at(7)->w = barXRight - 7 - space * 2;
  colorParams.at(8)->w = barXRight - 7 - space * 2;
}

void resizeWindow(int w, int h) {
  float by = screenH - barY;
  screenW = w / float(screenScale);
  screenH = h / float(screenScale);
  barY = screenH - by;
  glViewport(0, 0, screenW * screenScale, screenH * screenScale);
  proj = glm::ortho(0.0, (double)screenW * screenScale,
                    (double)screenH * screenScale, 0.0, -1.0, 1.0);
  glGenTextures(1, &screenTex);
  glBindTexture(GL_TEXTURE_2D, screenTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexN);
  glBindTexture(GL_TEXTURE_2D, screenTexN);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexMisc);
  glBindTexture(GL_TEXTURE_2D, screenTexMisc);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenTextures(1, &screenTexB);
  glBindTexture(GL_TEXTURE_2D, screenTexB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA,
               GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenFramebuffers(1, &screenFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screenTex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         screenTexN, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         screenTexMisc, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
                         screenTexB, 0);
  GLenum d[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, d);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (barX < 128 + 7) {
    barX = 128 + 7;
  } else if (barX > screenW / 3) {
    barX = screenW / 3;
  }
  barY = screenH;
  if (barXRight < 128 + 7) {
    barXRight = 128 + 7;
  } else if (barXRight > screenW / 3) {
    barXRight = screenW / 3;
  }

  adjustWidths();
  adjustBrowserScroll();
  adjustLayersScroll();
  adjustToolsScroll();
}

void update() {
  previewTimer++;
  blinkTimer++;
  if (blinkTimer > 64) {
    blinkTimer = 0;
  }
  if (errorTimer > 0) {
    errorTimer--;
  }
  if (draggingCam || draggingCam2) {
    float d = 4.0;
    float dx = (mouseX - mouseOX) / d;
    float dy = (mouseY - mouseOY) / d;
    mouseOX += dx;
    mouseOY += dy;
    nodeCSX = dx;
    nodeCSY = dy;
  }
  float cams = 1.5;
  nodeCX += nodeCSX;
  nodeCY += nodeCSY;
  nodeCSX /= cams;
  nodeCSY /= cams;
  if (logoTimer > 0) {
    logoTimer--;
  }
  rotTimer += 15;
  if (rotTimer >= 360) {
    rotTimer -= 360;
  }
  if (doubleClickTimer < 100) {
    doubleClickTimer++;
  }
  if (typeTimer < 100) {
    typeTimer++;
    if (typeTimer == 20) {
      textType = textTypeTemp;
      textTypeFx = textTypeFxTemp;
      textTypeLayer = textTypeLayerTemp;
      if (textType != NULL) {
        textType->typing = "";
      }
    }
  }
  Texture* t = texs.at(currentTexture);
  if (!t->done) {
    t->doneTimer++;
  } else {
    t->doneTimer = 0;
  }
  if (draggingNode) {
    float d = 4.0;
    float dx = (mouseX - mouseOX) / d;
    float dy = (mouseY - mouseOY) / d;
    mouseOX += dx;
    mouseOY += dy;
    t->fxs.at(draggedNode)->sx = dx;
    t->fxs.at(draggedNode)->sy = dy;
    t->fxs.at(draggedNode)->r = dx;
  }
  for (int i = 0; i < t->fxs.size(); i++) {
    Effect* fx = t->fxs.at(i);
    float d = 1.5;
    fx->x += fx->sx;
    fx->y += fx->sy;
    fx->sx /= d;
    fx->sy /= d;

    fx->r /= d;

    if (!fx->done) {
      fx->doneTimer++;
    } else {
      fx->doneTimer = 0;
    }
    if (fx->loaded) {  // gentex2
      fx->side.join();
      if (fx->abort && fx->deleted) {
        texs.at(currentTexture)
            ->fxs.erase(std::remove(texs.at(currentTexture)->fxs.begin(),
                                    texs.at(currentTexture)->fxs.end(), fx),
                        texs.at(currentTexture)->fxs.end());
        delete fx;
      } else {
        fx->loading = false;
        fx->loaded = false;
        if (fx->abort) {
          fx->abort = false;
        } else {
          fx->done = true;
          bool done = true;
          for (int i2 = 0; i2 < t->fxs.size(); i2++) {
            if (!t->fxs.at(i2)->done) {
              done = false;
            }
          }
          if (!done) {
            t->genTex(false);
          }
        }
        for (int out = 0; out < fx->outputs.size(); out++) {
          Socket* output = fx->outputs.at(out);
          glGenTextures(1, &output->texture);
          glBindTexture(GL_TEXTURE_2D, output->texture);
          if (OS & Windows) {
            vector<GLubyte> bv = output->texData.toByteArray();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSizeX, texSizeY, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, &bv[0]);
          } else if (OS & Unix) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texSizeX, texSizeY, 0,
                         GL_RGBA, GL_FLOAT, output->texData.ptr());
          }
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
      }
    }
    for (int in = 0; in < fx->inputs.size(); in++) {
      Socket* input = fx->inputs.at(in);
      if (input->s != NULL) {
        float xa = input->s->parent->x + input->s->parent->w;
        float ya = input->s->parent->y + input->s->y + 4;
        xa -= input->s->parent->x + input->s->parent->w / 2.0;
        ya -= input->s->parent->y + input->s->parent->h / 2.0;
        float a = glm::radians(-input->s->parent->r);
        float xb = ya * sin(a) + xa * cos(a);
        float yb = ya * cos(a) - xa * sin(a);
        xb += input->s->parent->x + input->s->parent->w / 2.0;
        yb += input->s->parent->y + input->s->parent->h / 2.0;
        input->b->P0.x = xb + int(nodeCX);
        input->b->P0.y = yb + int(nodeCY);
        xa = input->parent->x;
        ya = input->parent->y + input->y + 4;
        xa -= input->parent->x + input->parent->w / 2.0;
        ya -= input->parent->y + input->parent->h / 2.0;
        a = glm::radians(-input->parent->r);
        xb = ya * sin(a) + xa * cos(a);
        yb = ya * cos(a) - xa * sin(a);
        xb += input->parent->x + input->parent->w / 2.0;
        yb += input->parent->y + input->parent->h / 2.0;
        input->b->P3.x = xb + int(nodeCX);
        input->b->P3.y = yb + int(nodeCY);
        float dx = input->b->P3.x - input->b->P0.x;
        float offset = abs(dx) / 2.0;
        input->b->P1.x = input->b->P0.x + offset;
        input->b->P1.y = input->b->P0.y;
        input->b->P2.x = input->b->P3.x - offset;
        input->b->P2.y = input->b->P3.y;
        input->b->update();
      }
    }
  }
  if (t->abort) {  // gentex3
    bool stillLoading = false;
    for (int in = 0; in < t->fxs.size(); in++) {
      if (t->fxs.at(in)->loading) {
        stillLoading = true;
      }
    }
    if (!stillLoading) {
      for (int in = 0; in < t->fxs.size(); in++) {
        if (t->fxs.at(in)->undone) {
          t->fxs.at(in)->clearTree();
        }
      }
      t->genTex(false);
      t->abort = false;
    }
  }
  if (draggingSocket) {
    float d = 4.0;
    float dx = (mouseX - mouseOX) / d;
    float dy = (mouseY - mouseOY) / d;
    mouseOX += dx;
    mouseOY += dy;
    Effect* fx = t->fxs.at(draggedNode);
    Socket* output = fx->outputs.at(draggedSocket);
    float xa = output->parent->x + output->parent->w;
    float ya = output->parent->y + output->y + 4;
    xa -= output->parent->x + output->parent->w / 2.0;
    ya -= output->parent->y + output->parent->h / 2.0;
    float a = glm::radians(-output->parent->r);
    float xb = ya * sin(a) + xa * cos(a);
    float yb = ya * cos(a) - xa * sin(a);
    xb += output->parent->x + output->parent->w / 2.0;
    yb += output->parent->y + output->parent->h / 2.0;
    output->b->P0.x = xb + int(nodeCX);
    output->b->P0.y = yb + int(nodeCY);
    output->snapped = false;
    output->px += dx;
    output->py += dy;
    output->b->P3.x = output->px;
    output->b->P3.y = output->py;
    float expand = 2;
    for (int i = 0; i < t->fxs.size(); i++) {
      Effect* fx2 = t->fxs.at(i);
      if (i != draggedNode) {
        for (int in = 0; in < fx2->inputs.size(); in++) {
          if (mouseX > fx2->x + nodeCX - 4 - expand &&
              mouseX < fx2->x + nodeCX + 4 + expand &&
              mouseY > fx2->y + nodeCY + fx2->inputs.at(in)->y - expand &&
              mouseY < fx2->y + nodeCY + fx2->inputs.at(in)->y + 8 +
                           expand /* && fx2->inputs.at(in)->s == NULL*/) {
            output->b->P3.x = fx2->x + nodeCX;
            output->b->P3.y = fx2->y + nodeCY + fx2->inputs.at(in)->y + 4;
            output->snapped = true;
            output->s = fx2->inputs.at(in);
          }
        }
      }
    }
    float ddx = output->b->P3.x - output->b->P0.x;
    float offset = abs(ddx) / 2.0;
    output->b->P1.x = output->b->P0.x + offset;
    output->b->P1.y = output->b->P0.y;
    output->b->P2.x = output->b->P3.x - offset;
    output->b->P2.y = output->b->P3.y;
    output->b->update();
  }

  if (draggingSBar) {
    float d = 4.0;
    float dx = (mouseX - mouseOX) / d;
    float dy = (mouseY - mouseOY) / d;
    mouseOX += dx;
    mouseOY += dy;
    if (sBarDrag == 0) {
      layersScroll += dy * sRatio;
      adjustLayersScroll();
    } else if (sBarDrag == 1) {
      layersScrollH += dx * sRatio;
      adjustLayersScroll();
    } else if (sBarDrag == 2) {
      newEScroll += dy * sRatio;
      adjustLayersScroll();
    } else if (sBarDrag == 3) {
      palScroll += dy * sRatio;
      adjustToolsScroll();
    } else if (sBarDrag == 4) {
      toolsScroll += dy * sRatio;
      adjustToolsScroll();
    } else if (sBarDrag == 5) {
      texScroll += dy * sRatio;
      adjustToolsScroll();
    } else if (sBarDrag == 6) {
      browserScroll += dx * sRatio;
      adjustBrowserScroll();
    }
  }

  if (draggingUI) {
    if (UIdragType == 0) {
      float d = 4.0;
      float dx = (mouseX - mouseOX) / d;
      float dy = (mouseY - mouseOY) / d;
      mouseOX += dx;
      mouseOY += dy;
      barX += dx;
      if (barX < 128 + 7) {
        barX = 128 + 7;
      } else if (barX > screenW / 3) {
        barX = screenW / 3;
      }
      adjustWidths();
    } else if (UIdragType == 1) {
      float d = 4.0;
      float dx = (mouseX - mouseOX) / d;
      float dy = (mouseY - mouseOY) / d;
      mouseOX += dx;
      mouseOY += dy;
      barY += dy;
      barY = screenH;
    } else if (UIdragType == 2) {
      float d = 4.0;
      float dx = (mouseX - mouseOX) / d;
      float dy = (mouseY - mouseOY) / d;
      mouseOX += dx;
      mouseOY += dy;
      barXRight -= dx;
      if (barXRight < 128 + 7) {
        barXRight = 128 + 7;
      } else if (barXRight > screenW / 3) {
        barXRight = screenW / 3;
      }
      adjustWidths();
    }
    adjustBrowserScroll();
    adjustLayersScroll();
    adjustToolsScroll();
  }
  camX = 0;
  camY = 0;
  camRot += 1;
  float tr = 0.005;
  if (camOffset > 1.0) {
    camOffsetSpeed -= (camOffset - 1.0) / 1.5;
    if (camOffsetSpeed < tr && !camPositive) {
      camOffsetSpeed = 0.0;
      camOffset = 1.0;
    }
    camPositive = true;
  } else if (camOffset < 1.0) {
    camOffsetSpeed += (1.0 - camOffset) / 1.5;
    if (camOffsetSpeed > -tr && camPositive) {
      camOffsetSpeed = 0.0;
      camOffset = 1.0;
    }
    camPositive = false;
  }
  camOffset += camOffsetSpeed;
  camOffsetSpeed /= camOffsetMagnitude;
  int maxOffset = 16 * 6.0;
  camXFinal =
      camX + (sin(camRot * M_PI / 180.0) * (camOffset - 1.0) * maxOffset);
  camYFinal =
      camY + (cos(camRot * M_PI / 180.0) * (camOffset - 1.0) * maxOffset);
}

void renderScreen() {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, screenTex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, screenTexN);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, screenTexMisc);
  glm::mat4 currentMatrix = model;
  currentMatrix =
      glm::translate(currentMatrix, glm::vec3(0, screenH * screenScale, 0.0));
  currentMatrix = glm::scale(currentMatrix, glm::vec3(screenW, -screenH, 1.0));
  glUseProgram(light_program);
  glUniform1i(glGetUniformLocation(light_program, "tex"), 0);
  glUniform1i(glGetUniformLocation(light_program, "texN"), 1);
  glUniform1i(glGetUniformLocation(light_program, "texMisc"), 2);
  glUniform2f(glGetUniformLocation(light_program, "frameSize"), 1.0, 1.0);
  glUniform2f(glGetUniformLocation(light_program, "resolution"), (float)screenW,
              (float)screenH);
  glUniform1f(glGetUniformLocation(light_program, "timex"), 0);
  glUniform1f(glGetUniformLocation(light_program, "timey"), 0);
  glUniform1f(glGetUniformLocation(light_program, "tileSize"), 16);
  glUniform2f(glGetUniformLocation(light_program, "texSize"), 1.0, 1.0);
  glUniform1i(glGetUniformLocation(light_program, "frame"), 0);
  glUniformMatrix4fv(glGetUniformLocation(light_program, "model"), 1, GL_FALSE,
                     glm::value_ptr(currentMatrix));
  glUniformMatrix4fv(glGetUniformLocation(light_program, "proj"), 1, GL_FALSE,
                     glm::value_ptr(proj));
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(glGetAttribLocation(light_program, "position"), 2,
                        GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glEnableVertexAttribArray(glGetAttribLocation(light_program, "position"));
  glVertexAttribPointer(glGetAttribLocation(light_program, "t"), 2, GL_FLOAT,
                        GL_FALSE, 4 * sizeof(float),
                        (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(glGetAttribLocation(light_program, "t"));
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glUniform3f(glGetUniformLocation(light_program, "lightPos"), 0, 0, 2.0);
  glUniform1f(glGetUniformLocation(light_program, "lightRadius"), 0.0);
  glUniform4f(glGetUniformLocation(light_program, "lightColor"), 0, 0, 0, 1.0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(0);
}

void renderGL() {
  int a;
  a = barX;
  int barX = a;
  a = barY;
  int barY = a;
  a = barXRight;
  int barXRight = a;
  a = barY2;
  int barY2 = a;
  a = barY3;
  int barY3 = a;
  a = barY4;
  int barY4 = a;
  a = barY5;
  int barY5 = a;
  float sDepth = 0.2;
  glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
  glClear(GL_COLOR_BUFFER_BIT);

  model =
      glm::translate(glm::mat4(1.0), glm::vec3(-camXFinal, -camYFinal, 0.0));
  model =
      glm::translate(model, glm::vec3(0, screenH * (screenScale - 1.0), 0.0));

  renderUI(0, 0, screenW, screenH, effectImg);
  int x = int(nodeCX) % 16;
  int y = int(nodeCY) % 16;
  renderSprite(0, x - 16, y - 16, screenW + 32, screenH + 32, gridImg,
               screenW + 32, screenH + 32, 0, 0, 0, 1.0, 0, 0, 0, 0, 0);

  Texture* t = texs.at(currentTexture);

  int h = -layersScroll;
  int maxW = -layersScrollH;

  int scrollW = 6;

  // file browser
  if (browserOpen) {
    renderUI(barX + 1, 0, screenW - barX - barXRight - 2, barY - 1, effectImg5);
    int title = collH - 4;
    int bSpace = 5;
    float tS = title / 2 - 4;
    renderUI(barX + 1 + bSpace * 4 + 8 * 3, bSpace,
             screenW - barX - barXRight - 2 - bSpace * 6 - 8 * 4, title,
             effectImg6);
    int ln = textW(currentDir, barX + 1 + bSpace * 4 + 8 * 3 + tS, bSpace + tS,
                   fontImg, 0);
    if (ln > screenW - barX - barXRight - 2 - bSpace * 6 - 8 * 4 - tS * 2) {
      renderText(currentDir, screenW - barXRight - 1 - bSpace * 2 - 8 - tS,
                 bSpace + tS, fontImg, 1);
    } else {
      renderText(currentDir, barX + 1 + bSpace * 4 + 8 * 3 + tS, bSpace + tS,
                 fontImg, 0);
    }
    renderUI(barX + 1 + bSpace, title + bSpace * 2,
             screenW - barX - barXRight - 2 - bSpace * 3 - 8, title,
             effectImg6);
    if (overwrite) {
      renderText(filenameB, barX + 1 + bSpace + tS, bSpace * 2 + tS + title,
                 fontImg2, 0);
    } else {
      renderText(filenameB, barX + 1 + bSpace + tS, bSpace * 2 + tS + title,
                 fontImg, 0);
    }
    renderUI(barX + 1 + bSpace, title * 2 + bSpace * 3,
             screenW - barX - barXRight - 2 - bSpace * 2,
             barY - 1 - bSpace * 4 - title * 2, effectImg6);
    int fh = tS;
    int fw = tS - browserScroll;
    int fwNext = 0;
    for (int i = 0; i < filenames.size(); i++) {
      int n = textW(filenames.at(i)->name, barX + 1 + bSpace + fw + tS + 8,
                    title * 2 + bSpace * 3 + fh, fontImg, 0);
      if (barX + 1 + bSpace + fw - tS + n + 8 * 2 + tS > barX &&
          barX + 1 + bSpace + fw - tS < screenW - barXRight) {
        if (i == selectedFile) {
          renderUI(barX + 1 + bSpace + fw - tS,
                   title * 2 + bSpace * 3 + fh - tS, n + 8 * 2 + tS, title,
                   effectImg5);
        }
        renderIcon(barX + 1 + bSpace + fw, title * 2 + bSpace * 3 + fh, 8, 8,
                   iconImg9, filenames.at(i)->folder);
        renderText(filenames.at(i)->name, barX + 1 + bSpace + fw + tS + 8,
                   title * 2 + bSpace * 3 + fh, fontImg, 0);
      }

      if (n > fwNext) {
        fwNext = n;
      }
      fh += title;
      if (fh + title * 2 + title + bSpace * 3 >
              title + bSpace * 2 + barY - 1 - bSpace * 3 - title - 6 &&
          i != filenames.size() - 1) {
        fh = tS;
        fw += fwNext + tS * 2 + 8 * 3;
        fwNext = 0;
      }
    }
    fw += fwNext + tS * 2 + 8 * 3;
    int scrollH = fw + browserScroll;
    if (scrollH < screenW - barX - barXRight - bSpace * 2) {
      scrollH = screenW - barX - barXRight - bSpace * 2;
    }
    renderUI(barX + bSpace + 1 + int(int(browserScroll) *
                                     (screenW - barX - barXRight - bSpace * 2) /
                                     float(scrollH)),
             barY - bSpace - scrollW - 1,
             int((screenW - barX - barXRight - bSpace * 2) *
                 (screenW - barX - barXRight - bSpace * 2) / float(scrollH)),
             scrollW, effectImg14);

    renderUI(barX, 0, bSpace, barY - 1, effectImg);
    renderUI(screenW - barXRight - bSpace, 0, bSpace, barY - 1, effectImg);
    renderUI(barX + 1, 0, bSpace, barY - 1, effectImg5);
    renderUI(barX + 1, 0, bSpace * 4 + 8 * 3, title + bSpace, effectImg5);
    renderUI(screenW - barXRight - bSpace - 1, 0, bSpace, barY - 1, effectImg5);
    renderUI(screenW - barXRight - bSpace * 2 - 1 - 8, 0, bSpace * 2 + 8,
             title * 2 + bSpace * 2, effectImg5);
    bool leftActive = (fnUndo.size() > 1);
    bool rightActive = (fnRedo.size() > 0);
    bool upActive = (currentDir.size() != 1);
    renderIcon(barX + 1 + bSpace, bSpace + tS, 8, 8, iconImg10, 0 + leftActive);
    renderIcon(barX + 1 + bSpace + 8 + bSpace, bSpace + tS, 8, 8, iconImg10,
               2 + rightActive);
    renderIcon(barX + 1 + bSpace + 8 * 2 + bSpace * 2, bSpace + tS, 8, 8,
               iconImg10, 4 + upActive);
    renderIcon(screenW - barXRight - 8 - bSpace - 1, bSpace + tS, 8, 8,
               iconImg10, 6);
    int impexp = 0;
    if (browserMode == 1 || browserMode == 2) {
      impexp = 1;
    } else if (browserMode == 3) {
      impexp = 2;
    } else if (browserMode == 4) {
      impexp = 4;
    } else if (browserMode == 5) {
      impexp = 3;
    }
    renderIcon(screenW - barXRight - 8 - bSpace - 1, bSpace * 2 + title + tS, 8,
               8, iconImg7, impexp);
  } else {
    // render nodes
    model =
        glm::translate(glm::mat4(1.0), glm::vec3(-camXFinal, -camYFinal, 0.0));
    model =
        glm::translate(model, glm::vec3(0, screenH * (screenScale - 1.0), 0.0));
    if (draggingSocket) {
      Effect* fx = t->fxs.at(draggedNode);
      Socket* out = fx->outputs.at(draggedSocket);
      renderBezier(out->b, bezierFill);
    }
    for (int i = 0; i < t->fxs.size(); i++) {
      Effect* fx = t->fxs.at(i);
      for (int in = 0; in < fx->inputs.size(); in++) {
        if (fx->inputs.at(in)->s != NULL) {
          if (fx->inputs.at(in)->infloop) {
            renderBezier(fx->inputs.at(in)->b, bezierFillError);
          } else {
            renderBezier(fx->inputs.at(in)->b, bezierFill);
          }
        }
      }
    }
    for (int i = 0; i < t->fxs.size(); i++) {
      Effect* fx = t->fxs.at(i);
      if (fx->x + fx->w + 4 + int(nodeCX) > barX &&
          fx->x - 4 + int(nodeCX) < screenW - barXRight &&
          fx->y + fx->h + 4 + int(nodeCY) > 0 &&
          fx->y - 4 + int(nodeCY) < screenH) {
        model = glm::translate(
            model, glm::vec3(fx->x + int(nodeCX) + fx->w / 2.0,
                             fx->y + int(nodeCY) + fx->h / 2.0, 0.0));
        model = glm::rotate(model, glm::radians(fx->r), glm::vec3(0, 0, 1));
        model = glm::translate(
            model, glm::vec3(-(fx->x + int(nodeCX) + fx->w / 2.0),
                             -(fx->y + int(nodeCY) + fx->h / 2.0), 0.0));

        renderUI(fx->x + int(nodeCX), fx->y + int(nodeCY), fx->w, fx->h,
                 effectImg4);
        renderUI(fx->x + int(nodeCX), fx->y + int(nodeCY), fx->w, 9 + 8,
                 effectImg16);
        renderText(fx->name, fx->x + int(nodeCX) + 4, fx->y + int(nodeCY) + 4,
                   fontImg, false);
        renderIcon(fx->x + fx->w + int(nodeCX) - 8 - 4, fx->y + int(nodeCY) + 4,
                   8, 8, iconImg3, 1);
        renderIcon(fx->x + fx->w + int(nodeCX) - 8 - 4 - 8 - 4,
                   fx->y + int(nodeCY) + 4, 8, 8, iconImg3, 2);
        for (int p = 0; p < fx->texts.size(); p++) {
          renderText(fx->texts.at(p)->name, fx->x + int(nodeCX) + 7,
                     fx->y + fx->texts.at(p)->y + int(nodeCY), fontImg, false);
        }
        for (int p = 0; p < fx->params.size(); p++) {
          fx->params.at(p)->render(fx->x + int(nodeCX), fx->y + int(nodeCY));
        }
        for (int out = 0; out < fx->outputs.size(); out++) {
          int size = 8;
          int size2 = 64;
          renderUI(fx->x + fx->w / 2.0 + int(nodeCX) - int(size2 / 2.0),
                   fx->y + int(nodeCY) + fx->outputs.at(out)->y + size / 2.0 -
                       size2 / 2.0,
                   size2, size2, effectImg);
          if (fx->doneTimer < 3) {
            renderSprite(
                0, int(fx->x + fx->w / 2.0 + int(nodeCX) - int(texSizeX / 2.0)),
                int(fx->y + int(nodeCY) + fx->outputs.at(out)->y + size / 2.0 -
                    texSizeY / 2.0),
                texSizeX, texSizeY, fx->outputs.at(out)->texture, texSizeX,
                texSizeY, 0, 0, 0, 1, 0, 0, 0, 0, 0);
          } else {
            renderRotIcon(int(fx->x + fx->w / 2.0 + int(nodeCX) - 4),
                          int(fx->y + int(nodeCY) + fx->outputs.at(out)->y +
                              size / 2.0 - 4),
                          8, 8, iconImg8, 0);
          }
          renderIcon(fx->x + fx->w + int(nodeCX) - size / 2.0,
                     fx->y + int(nodeCY) + fx->outputs.at(out)->y, size, size,
                     iconImg6, 1);
        }
        for (int in = 0; in < fx->inputs.size(); in++) {
          int size = 8;
          renderIcon(fx->x + int(nodeCX) - size / 2.0,
                     fx->y + int(nodeCY) + fx->inputs.at(in)->y, size, size,
                     iconImg6, 1);
        }

        model = glm::translate(glm::mat4(1.0),
                               glm::vec3(-camXFinal, -camYFinal, 0.0));
        model = glm::translate(
            model, glm::vec3(0, screenH * (screenScale - 1.0), 0.0));
      }
    }
  }

  renderUI(0, 0, barX, barY, effectImg8);

  if (view == 0) {
    int newEScrolli = newEScroll;
    for (int i = 0; i < newEffects.size(); i++) {
      Effect* fx = newEffects.at(i);
      if (draggingFX && draggedLayer == -1 && i == newEffects.size() - 1) {
      } else {
        renderUI(0, collH * i - newEScrolli, barX - scrollW - 1, collH,
                 effectImg2);
        renderText(fx->name, 5, 5 + collH * i - newEScrolli, fontImg, 0, -1, -1,
                   -1, barY - 1);
        if (fx->isPreset) {
          renderIcon(barX - scrollW - 1 - 8 - 4 - 2,
                     5 + collH * i - newEScrolli, 8, 8, iconImg10, 7);
        }
      }
    }
    int count = newEffects.size();
    if (draggingFX && draggedLayer == -1) {
      count--;
    }
    int scrollH = collH * count;
    if (scrollH < barY) {
      scrollH = barY;
    }
    renderUI(barX - scrollW, 0, scrollW, barY - 1, effectImg8);
    renderUI(barX - scrollW - 1, 0, scrollW, barY + 1, effectImg15);
    renderUI(barX - scrollW, int(int(newEScroll) * (barY - 2) / float(scrollH)),
             scrollW, int((barY) * (barY - 2) / float(scrollH)) + 1,
             effectImg14);
  }

  // right bar background (including lines)
  int toolsScrolli = toolsScroll;
  int palScrolli = palScroll;
  renderUI(screenW - int(barXRight), -toolsScrolli, int(barXRight), barY2,
           effectImg3);
  renderUI(screenW - int(barXRight), barY2 - toolsScrolli, int(barXRight),
           3 + 8 + 3 + (14 * 1) + 4, effectImg3);
  renderUI(screenW - int(barXRight),
           barY2 - toolsScrolli + 3 + 8 + 3 + (14 * 1) + 4, int(barXRight),
           screenH - (barY2 - toolsScrolli + 3 + 8 + 3 + (14 * 1) + 4) + 2,
           effectImg3);

  renderText("Palette", screenW - barXRight + 4, 3 - toolsScrolli, fontImg, 0);
  renderUI(screenW - barXRight + 4, 2 + 12 - toolsScrolli, barXRight - 8 - 7,
           barY3, effectImg);
  renderUI(screenW - barXRight + 5, 3 + 12 - toolsScrolli, barXRight - 10 - 7,
           barY3 - 2 - 5 - 8 - 5, effectImg5);
  int space = 1;
  float offset = 0;
  int w = barXRight - 10 + space - offset * 2 - 7 - 7;
  float size = 12;
  int c = w / size;
  int leftspace = w - size * c;
  leftspace = 0;
  int maxHPal = 0;
  for (int p = 0; p < palette.size(); p++) {
    int x = p % c;
    int y = p / c;
    int offset2 = int(p == selectedColor) * 2;
    renderUI(leftspace + offset - 1 + x * (size) + screenW - barXRight + 5,
             offset - 1 + 3 + y * (size) + 12 - toolsScrolli - palScrolli,
             int((x + 1) * (size)) - int(x * (size)) - space + 2,
             int((y + 1) * (size)) - int(y * (size)) - space + 2, effectImg11,
             -1, -1, 3 + 12 - toolsScrolli,
             barY3 - 2 - 5 - 8 - 5 + 3 + 12 - toolsScrolli);
    renderColor(
        leftspace + offset + offset2 + x * (size) + screenW - barXRight + 5,
        offset + offset2 + 3 + y * (size) + 12 - toolsScrolli - palScrolli,
        size - space - offset2 * 2, size - space - offset2 * 2, palette.at(p),
        -1, -1, 3 + 12 - toolsScrolli,
        barY3 - 2 - 5 - 8 - 5 + 3 + 12 - toolsScrolli);
    if (y * (size) + size > maxHPal) {
      maxHPal = y * (size) + size;
    }
  }

  int scrollH = maxHPal;
  if (scrollH < barY3 - 5 - 8 - 5) {
    scrollH = barY3 - 5 - 8 - 5;
  }
  renderUI(screenW - 4 - 8 - scrollW, 2 + 12 - toolsScrolli + 1, scrollW,
           barY3 - 5 - 8 - 5 - 2, effectImg5);
  renderUI(screenW - 4 - 8 - scrollW - 1, 2 + 12 - toolsScrolli + 1, scrollW,
           barY3 - 5 - 8 - 5 + 2, effectImg10);
  renderUI(
      screenW - 4 - 8 - scrollW,
      2 + 12 - toolsScrolli +
          int(int(palScroll) * (barY3 - 5 - 8 - 5) / float(scrollH)) + 1,
      scrollW,
      int((barY3 - 5 - 8 - 5) * (barY3 - 5 - 8 - 5 - 3) / float(scrollH)) + 1,
      effectImg14);

  renderIcon(screenW - 5 - 8 - 5 - 7, 1 + 12 - toolsScrolli + barY3 - 8 - 5, 8,
             8, iconImg7, 1);
  renderIcon(screenW - 5 - 8 - 5 - 5 - 8 - 7,
             1 + 12 - toolsScrolli + barY3 - 8 - 5, 8, 8, iconImg7, 0);
  renderIcon(screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 7,
             1 + 12 - toolsScrolli + barY3 - 8 - 5, 8, 8, iconImg3, 1);
  renderIcon(screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 7,
             1 + 12 - toolsScrolli + barY3 - 8 - 5, 8, 8, iconImg3, 2);
  renderIcon(screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 5 - 8 - 7,
             1 + 12 - toolsScrolli + barY3 - 8 - 5, 8, 8, iconImg3, 0);
  Parameter* red = colorParams.at(1);
  Parameter* blue = colorParams.at(3);
  Parameter* hue = colorParams.at(4);
  int ss = 36;
  int paramH = ss / 3.0;
  int off = (paramH - 7) / 4.0;
  renderUI(screenW - barXRight + red->x - off,
           barY3 + 2 + red->y - off + 12 - toolsScrolli, red->w + off * 2,
           (blue->h + blue->y) - red->y + off * 2, effectImg6);
  renderUI(screenW - barXRight + hue->x - off,
           barY3 + 2 + red->y - off + 12 - toolsScrolli, red->w + off * 2,
           (blue->h + blue->y) - red->y + off * 2, effectImg6);
  renderText("Tile Properties", screenW - barXRight + 4,
             barY2 + 3 - toolsScrolli, fontImg, 0);

  renderText("Node Preview", screenW - barXRight + 4,
             barY2 + 3 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli, fontImg, 0);
  renderUI(screenW - barXRight + 4,
           barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 2 + 12,
           barXRight - 8 - 7, 96, effectImg);
  renderUI(screenW - barXRight + 5,
           barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 3 + 12,
           barXRight - 10 - 7, 96 - 2, effectImg5);

  if (preview != NULL && !browserOpen) {
    int tileW = texSizeX;
    int tileH = texSizeY;
    while (tileW < barXRight - 10 - 7) {
      tileW += texSizeX;
    }
    while (tileH < 96 - 2) {
      tileH += texSizeY;
    }
    float d = 0.3;
    int xOff = -previewTimer * d;
    while (xOff < -tileW) {
      xOff += tileW;
    }
    int yOff = -previewTimer / 2 * d;
    while (yOff < -tileH) {
      yOff += tileH;
    }
    if (preview->parent->doneTimer < 3) {
      renderSprite(
          0, screenW - barXRight + 5 + xOff,
          barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 3 + 12 + yOff,
          tileW * 2, tileH * 2, preview->texture, tileW * 2, tileH * 2, 0, 0, 0,
          1, 0, 0, 0, 0, 0, 0, 0, screenW - barXRight + 5,
          screenW - barXRight + 5 + barXRight - 10 - 7,
          barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 3 + 12,
          barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 3 + 12 + 96 - 2);
    } else {
      renderRotIcon(screenW - barXRight + 5 + (barXRight - 8 - 7) / 2 - 4,
                    barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 3 + 12 +
                        (96 - 2) / 2 - 4,
                    8, 8, iconImg8, 0);
    }
  } else {
    string txt = "Hover on a node";
    int txtW = textW(txt, 0, 0, fontImg, 0);
    renderText(
        txt, screenW - barXRight + 4 + (barXRight - 8 - 7 - txtW) / 2,
        barY2 + 3 + 8 + 3 + (14 * 1) + 4 - toolsScrolli + 2 + 12 + 48 - 4,
        fontImg, 0);
  }

  for (int p = 0; p < colorParams.size(); p++) {
    colorParams.at(p)->render(screenW - barXRight,
                              barY3 + 2 + 12 - toolsScrolli);
  }

  scrollH = barY5;
  if (scrollH < barY) {
    scrollH = barY;
  }
  renderUI(screenW - scrollW, 0, scrollW, barY - 1, effectImg8);
  renderUI(screenW - scrollW - 1, 0, scrollW, barY + 1, effectImg15);
  renderUI(screenW - scrollW,
           int(int(toolsScroll) * (barY - 2) / float(scrollH)), scrollW,
           int((barY) * (barY - 2) / float(scrollH)) + 1, effectImg14);

  if (!browserOpen) {
    int d3 = barX - (screenW - 5 - 8 - barXRight);
    if (d3 > 0 && d3 < 8) {
      renderIcon(barX, 5, screenW - 5 - barXRight - barX, 8, iconImg9, 2, d3);
      renderIcon(barX, 5 + 8 + 5, screenW - 5 - barXRight - barX, 8, iconImg9,
                 3, d3);
    } else if (d3 >= 8) {
    } else {
      renderIcon(screenW - 5 - 8 - barXRight, 5, 8, 8, iconImg9, 2);
      renderIcon(screenW - 5 - 8 - barXRight, 5 + 8 + 5, 8, 8, iconImg9, 3);
    }
    // renderIcon(screenW-5-8-barXRight, 5, 8, 8, iconImg9, 2);
    int d2 = barX - (screenW - 5 - 8 - 5 - 8 - barXRight);
    if (d2 > 0 && d2 < 8) {
      renderIcon(barX, 5, screenW - 5 - 8 - 5 - barXRight - barX, 8, iconImg9,
                 1, d2);
    } else if (d2 >= 8) {
    } else {
      renderIcon(screenW - 5 - 8 - 5 - 8 - barXRight, 5, 8, 8, iconImg9, 1);
    }
    int d1 = barX - (screenW - 5 - 8 - 5 - 8 - 5 - 8 - barXRight);
    if (d1 > 0 && d1 < 8) {
      renderIcon(barX, 5, screenW - 5 - 8 - 5 - 8 - 5 - barXRight - barX, 8,
                 iconImg9, 0, d1);
    } else if (d1 >= 8) {
    } else {
      renderIcon(screenW - 5 - 8 - 5 - 8 - 5 - 8 - barXRight, 5, 8, 8, iconImg9,
                 0);
    }
    renderText(toolTip, barX + 4, 4, fontImg, false, -1, screenW - barXRight,
               -1, -1);
    int errorH = textH(errorMessage, 0, 0, fontImg2, false);
    float errorA = fmin(1.0, errorTimer / 30.0);
    renderText(errorMessage, barX + 4, screenH - errorH - 4, fontImg2, false,
               -1, screenW - barXRight, -1, -1, errorA);
    if (logoTimer > 0) {
      float alpha = fmin(1.0, logoTimer / float(logoTimerMax - 120));
      renderSprite(0,
                   int((screenW - barX - barXRight) / 2.0 + barX - 128 / 2.0),
                   int(barY / 2.0 - 79 / 2.0), 128, 79, logoImage, 128, 79, 0,
                   0, 0, alpha, 0, sDepth, 0.0, false, false, 0, 0, barX,
                   screenW - barXRight, -1, barY);
    }
  }

  int cursorType = 0;
  bool noChange = false;
  for (int i = 0; i < newEffects.size(); i++) {
    if (mouseX > 2 && mouseX <= barX - 2 &&
        mouseY > 2 + (collH - 2) * i - newEScroll &&
        mouseY <= 2 + (collH - 2) * i - newEScroll + collH - 4 &&
        mouseY < barY) {
      noChange = true;
    }
  }
  if (mouseX > 0 && mouseX < screenW && mouseY > barY && mouseY < screenH) {
    Texture* t = texs.at(currentTexture);
    int h = -layersScroll;
  }
  if (mouseX > barX && mouseX < barX + UIdragRange && mouseY < barY &&
      !noChange) {
    cursorType = 1;
  }
  if (mouseX > screenW - barXRight - UIdragRange &&
      mouseX < screenW - barXRight && mouseY < barY && !noChange) {
    cursorType = 1;
  }
  if (draggingUI && UIdragType == 0) {
    cursorType = 1;
  }
  if (draggingUI && UIdragType == 2) {
    cursorType = 1;
  }
  if (cursorType == 1) {
    renderIcon(mouseX - 4, mouseY - 1, 8, 8, iconImg11, cursorType);
  } else if (cursorType == 2) {
    renderIcon(mouseX - 1, mouseY - 4, 8, 8, iconImg11, cursorType);
  } else {
    renderIcon(mouseX, mouseY, 8, 8, iconImg11, cursorType);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  model = glm::mat4(1.0);
  renderSprite(0, 0, screenH * screenScale, screenW * screenScale,
               -screenH * screenScale, screenTex, screenW, screenH, 0, 0, 0, 1,
               0, sDepth, 0.0, false, false);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  model = glm::mat4(1.0);

  bool renderfps = false;

  if (renderfps) {
    int numDigits = 1;
    if (fps > 0) {
      numDigits = log10(fps) + 1;
    }
    int f = fps;
    int kerning[] = {6, 4, 6, 6, 6, 6, 6, 5, 6, 6};
    float width = 0;
    for (int d = 0; d < numDigits; d++) {
      width += kerning[f % 10] * 16 / 16.0;
      f /= 10;
    }
    f = fps;
    int pos = 0;
    for (int d = 0; d < numDigits; d++) {
      pos -= kerning[f % 10] * 16 / 16.0;
      renderSprite(f % 10, (width + pos) * screenScale, 0, 16 * screenScale,
                   16 * screenScale, digitsImg, 16, 16, 0, 0, 0, 1, 0, 0, 0.0,
                   false, false);
      f /= 10;
    }
  }
}

void genGrad(Color c1, Color c2) {
  int shades = colorParams.at(9)->value;
  int middleS = shades / 2.0;
  Color ahsv = RGBtoHSV(c1.r, c1.g, c1.b);
  Color bhsv = RGBtoHSV(c2.r, c2.g, c2.b);
  int AH = ahsv.r;
  int BH = bhsv.r;
  if (BH <= AH - 180) {
    BH += 360;
  }
  int Hshift = (BH - AH) / float(middleS);
  int Sshift = (bhsv.g - ahsv.g) / float(middleS);
  int Vshift = (bhsv.b - ahsv.b) / float(middleS);
  for (int i = 1; i < middleS; i++) {
    float CH = i * Hshift / float(BH - AH);
    // CH = sqrt(CH);
    CH *= (BH - AH);
    int currentH = ahsv.r + CH;
    while (currentH > 360) {
      currentH -= 360;
    }
    while (currentH < 0) {
      currentH += 360;
    }
    float CS = i * Sshift / float(bhsv.g - ahsv.g);
    CS = CS * CS * CS;
    CS *= (bhsv.g - ahsv.g);
    int currentS = ahsv.g + CS;
    int currentV = ahsv.b + i * Vshift;
    Color finalhsv = Color(currentH, currentS, currentV);
    Color finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
    Color* final = new Color(finalrgb.r, finalrgb.g, finalrgb.b);
    palette.push_back(final);
  }
  Color* cf = new Color(c2.r, c2.g, c2.b);
  palette.push_back(cf);
}

float curve(float x) { return x * sqrt(x); }

void baseToBright(Color base, Color target, int s, int s2) {
  int middleS = s2;
  Color ahsv = RGBtoHSV(target.r, target.g, target.b);
  Color bhsv = RGBtoHSV(base.r, base.g, base.b);
  int AH = ahsv.r;
  int BH = bhsv.r;
  if (AH <= BH - 180) {
    AH += 360;
  }
  vector<Color*> palN;
  int Hshift = (BH - AH) * (colorParams.at(8)->value / 100.0) / float(middleS);
  int Sshift = (ahsv.g - bhsv.g) / float(middleS);
  int Vshift = (ahsv.b - bhsv.b) / float(middleS);
  Color prevColor = Color(base.r, base.g, base.b);

  double lB = getLuminance(base.r, base.g, base.b);
  double lT = getLuminance(target.r, target.g, target.b);
  double offset = 0.05;

  double a = (lT + offset) / (lB + offset);
  double r = pow(a, 1.0 / float(middleS));

  for (int i = 0; i < middleS; i++) {
    float index = (i + 1) / float(middleS);
    float curv = index;
    curv = curve(curv);
    index *= (middleS);
    curv *= (middleS);
    int currentH = bhsv.r - curv * Hshift;
    int currentS = bhsv.g + index * Sshift;
    int currentV = bhsv.b + index * Vshift;
    while (currentH > 360) {
      currentH -= 360;
    }
    while (currentH < 0) {
      currentH += 360;
    }
    currentS = max(0, currentS);
    currentS = min(100, currentS);
    currentV = max(0, currentV);
    currentV = min(100, currentV);
    Color finalhsv = Color(currentH, currentS, currentV);
    Color finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
    double contrast = getContrast(prevColor, finalrgb);

    int V = 100;
    finalhsv = Color(currentH, currentS, V);
    finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
    contrast = getContrast(prevColor, finalrgb);
    while (contrast > r) {
      V--;
      finalhsv = Color(currentH, currentS, V);
      finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
      contrast = getContrast(prevColor, finalrgb);
    }

    Color* final = new Color(finalrgb.r, finalrgb.g, finalrgb.b);
    palN.push_back(final);
    prevColor = Color(finalrgb.r, finalrgb.g, finalrgb.b);
    // prevContrast = contrast;
  }
  for (int i = palN.size() - 1; i >= 0; i--) {
    palette.push_back(palN.at(i));
  }
}

void baseToDark(Color base, Color target, int s, int s2) {
  int shades = s - 1;
  int middleS = s2;
  Color ahsv = RGBtoHSV(target.r, target.g, target.b);
  Color bhsv = RGBtoHSV(base.r, base.g, base.b);
  int AH = ahsv.r;
  int BH = bhsv.r;
  if (BH <= AH - 180) {
    BH += 360;
  }
  int Hshift =
      (AH - BH) * (colorParams.at(8)->value / 100.0) / float(shades - middleS);
  int Sshift = (ahsv.g - bhsv.g) / float(shades - middleS);
  int Vshift = (ahsv.b - bhsv.b) / float(shades - middleS);
  Color prevColor = Color(base.r, base.g, base.b);

  double lB = getLuminance(base.r, base.g, base.b);
  double lT = getLuminance(target.r, target.g, target.b);
  double offset = 0.05;

  double a = (lB + offset) / (lT + offset);
  double r = pow(a, 1.0 / float(shades - middleS));

  for (int i = middleS + 1; i < shades + 1; i++) {
    float index = (i - middleS) / float(shades - (middleS));
    float curv = index;
    curv = curve(curv);
    index *= (shades - (middleS));
    curv *= (shades - (middleS));
    int currentH = bhsv.r + curv * Hshift;
    int currentS = bhsv.g + index * Sshift;
    int currentV = bhsv.b + index * Vshift;
    while (currentH > 360) {
      currentH -= 360;
    }
    while (currentH < 0) {
      currentH += 360;
    }
    currentS = max(0, currentS);
    currentS = min(100, currentS);
    currentV = max(0, currentV);
    currentV = min(100, currentV);
    Color finalhsv = Color(currentH, currentS, currentV);
    Color finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
    double contrast = getContrast(prevColor, finalrgb);

    int V = 0;
    finalhsv = Color(currentH, currentS, V);
    finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
    contrast = getContrast(prevColor, finalrgb);
    while (contrast > r) {
      V++;
      finalhsv = Color(currentH, currentS, V);
      finalrgb = HSVtoRGB(finalhsv.r, finalhsv.g, finalhsv.b);
      contrast = getContrast(prevColor, finalrgb);
    }

    Color* final = new Color(finalrgb.r, finalrgb.g, finalrgb.b);
    palette.push_back(final);
    prevColor = Color(finalrgb.r, finalrgb.g, finalrgb.b);
  }
}

void genShades(Color c, Color b, Color d, int shades, int shadesM) {
  baseToBright(c, b, shades, shadesM);
  Color* a = new Color(c.r, c.g, c.b);
  palette.push_back(a);
  baseToDark(c, d, shades, shadesM);
}

void generatePalette() {
  for (int toDel = 0; toDel < palette.size(); toDel++) {
    delete palette.at(toDel);
  }
  palette.clear();

  int globalSat = (rand() % 72) + 18;
  globalSat = colorParams.at(8)->value;
  Color bright = Color(255, 243, 232);
  Color dark = Color(31, 25, 46);

  int count = 5;

  int angleInit = rand() % 360;
  int angleOffset = 360 / count;
  int maxVar = angleOffset / 2.0;
  angleOffset += (rand() % (maxVar * 2)) - maxVar;

  for (int i = 0; i < count + 1; i++) {
    bool invalid = true;
    Color nextrgb = Color(0, 0, 0);
    while (invalid) {
      invalid = false;
      int nextAngle = angleInit + angleOffset * i;
      while (nextAngle >= 360) {
        nextAngle -= 360;
      }
      while (nextAngle < 0) {
        nextAngle += 360;
      }
      Color next = Color(nextAngle, (rand() % 3) - 1 + globalSat, 65);
      if (i == count) {
        next.g = 10;
      }

      double lB = getLuminance(bright.r, bright.g, bright.b);
      double lT = getLuminance(dark.r, dark.g, dark.b);
      double offset = 0.05;

      double aa = (lB + offset) / (lT + offset);
      double r = pow(aa, 1.0 / 2);

      double l = (lT + offset) * r - offset;

      nextrgb = HSVtoRGB(next.r, next.g, 100);
      double luminance = getLuminance(nextrgb.r, nextrgb.g, nextrgb.b);

      int V = 100;
      while (luminance > l) {
        V--;
        nextrgb = HSVtoRGB(next.r, next.g, V);
        luminance = getLuminance(nextrgb.r, nextrgb.g, nextrgb.b);
      }
    }

    genShades(nextrgb, bright, dark, colorParams.at(9)->value,
              (colorParams.at(9)->value) / 2.0);
  }

  float treshold = 40;
  for (int i = 0; i < count + 1; i++) {
    int ramp = i;
    for (int s = 1; s < 2; s++) {
      int side = s;
      int offset = 1;
      int offset2 = offset;
      if (side) {
        offset2 = colorParams.at(9)->value - 1 - offset;
      }
      int final = ramp * colorParams.at(9)->value + offset2;
      Color* finalC = palette.at(final);
      Color* color2 = NULL;
      float dist = 10000;
      for (int i2 = 0; i2 < count + 1; i2++) {
        if (i2 != i) {
          int ramp2 = i2;
          int final2 = ramp2 * colorParams.at(9)->value + offset2;
          Color* finalC2 = palette.at(final2);
          if (finalC2->r != -1 && finalC->r != -1) {
            float dr = finalC2->r - finalC->r;
            float dg = finalC2->g - finalC->g;
            float db = finalC2->b - finalC->b;
            float dt = sqrt(dr * dr + dg * dg + db * db);
            if (dt < treshold) {
              if (dt < dist) {
                dist = dt;
                color2 = finalC2;
              }
            }
          }
        }
      }
      if (color2 != NULL) {
        if (side) {
          dark = Color(color2->r, color2->g, color2->b);
        } else {
          bright = Color(color2->r, color2->g, color2->b);
        }
        Color* next = palette.at(ramp * colorParams.at(9)->value +
                                 colorParams.at(9)->value / 2.0);
        genShades(*next, bright, dark, colorParams.at(9)->value - 1,
                  (colorParams.at(9)->value) / 2.0 - (!side));
        for (int f = 0; f < colorParams.at(9)->value; f++) {
          Color* rampC = palette.at(ramp * colorParams.at(9)->value + f);
          rampC->r = -1;
          rampC->g = -1;
          rampC->b = -1;
        }
      }
    }
  }

  float removeTresh = 15;
  for (int i = 0; i < palette.size(); i++) {
    Color* finalC = palette.at(i);
    for (int j = 0; j < palette.size(); j++) {
      if (i != j) {
        Color* finalC2 = palette.at(j);
        float dr = finalC2->r - finalC->r;
        float dg = finalC2->g - finalC->g;
        float db = finalC2->b - finalC->b;
        float dt = sqrt(dr * dr + dg * dg + db * db);
        if (dt < removeTresh) {
          finalC2->r = -1;
          finalC2->g = -1;
          finalC2->b = -1;
        }
      }
    }
  }

  for (int i = 0; i < palette.size(); i++) {
    Color* finalC = palette.at(i);
    if (finalC->r == -1) {
      delete finalC;
      palette.erase(std::remove(palette.begin(), palette.end(), finalC),
                    palette.end());
      i--;
    }
  }
  paletteChanged();
}

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16*)p = pixel;
      break;

    case 3:
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32*)p = pixel;
      break;
  }
}

int tilemancer_main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  } else {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Rect r;
    SDL_GetDisplayBounds(0, &r);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    displayW = current.w;
    displayH = current.h;
    screenW = displayW / 2.4;
    screenH = displayH / 2.4;

    window = SDL_CreateWindow(
        "Tilemancer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenW * screenScale, screenH * screenScale,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_SetWindowMinimumSize(window, (128 + 7) * 3 * screenScale,
                             256 * screenScale);

    SDL_ShowCursor(SDL_FALSE);

    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
      gContext = SDL_GL_CreateContext(window);
      cout << "Created OpenGL " << glGetString(GL_VERSION) << " context"
           << endl;
      initGL();
      cout << "Loading stuff" << endl;
      LoadStuff();
      cout << "Done loading stuff" << endl;
    }
  }
  bool quit = false;
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      if (e.type == SDL_WINDOWEVENT) {
        if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          resizeWindow(e.window.data1, e.window.data2);
        }
      }
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (textType != NULL) {
          textType->value = atoi(textType->typing.c_str());
          if (textType->value > textType->value3 && textType->value3 != -1) {
            textType->value = textType->value3;
          }
          if (textType->value < textType->value2 && textType->value2 != -1) {
            textType->value = textType->value2;
          }
          updateDrag(textTypeLayer, textTypeFx, textType);
        }
        textType = NULL;
        typeTimer = 0;
        if (e.button.button == SDL_BUTTON_LEFT) {
          int x = e.button.x / screenScale;
          int y = e.button.y / screenScale;
          if (view == 0) {
            if (browserOpen) {
              int title = collH - 4;
              int bSpace = 5;
              float tS = title / 2 - 4;
              if (x > barX + bSpace && x < screenW - barXRight - bSpace) {
                int fh = tS;
                int fw = tS - browserScroll;
                int fwNext = 0;
                for (int i = 0; i < filenames.size(); i++) {
                  int n = textW(filenames.at(i)->name,
                                barX + 1 + bSpace + fw + tS + 8,
                                title * 2 + bSpace * 3 + fh, fontImg, 0);
                  if (x > barX + 1 + bSpace + fw - tS &&
                      x < barX + 1 + bSpace + fw - tS + n + 8 * 2 + tS &&
                      y > title * 2 + bSpace * 3 + fh - tS &&
                      y < title * 2 + bSpace * 3 + fh - tS + title) {
                    if (doubleClickTimer <= 20 && selectedFile == i) {
                      string fullDir = currentDir;
                      if (fullDir.size() != 1) {
                        if (OS & Windows) {
                          fullDir = fullDir.append("\\");
                        } else if (OS & Unix) {
                          fullDir = fullDir.append("/");
                        }
                      }
                      fullDir = fullDir.append(string(filenames.at(i)->name));
                      if (filenames.at(i)->folder) {
                        openBrowser(fullDir, 0, browserMode);
                      } else {
                        browserAction(fullDir, filenames.at(i)->name,
                                      currentDir);
                      }
                    } else {
                      if (selectedFile != i) {
                        overwrite = false;
                      }
                      selectedFile = i;
                      filenameB = filenames.at(i)->name;
                    }
                    doubleClickTimer = 0;
                  }
                  if (n > fwNext) {
                    fwNext = n;
                  }
                  fh += title;
                  if (fh + title * 2 + title + bSpace * 3 >
                          title + bSpace * 2 + barY - 1 - bSpace * 3 - title -
                              6 &&
                      i != filenames.size() - 1) {
                    fh = tS;
                    fw += fwNext + tS * 2 + 8 * 3;
                    fwNext = 0;
                  }
                }
                fw += fwNext + tS * 2 + 8 * 3;
                int scrollH = fw + browserScroll;
                if (scrollH < screenW - barX - barXRight - bSpace * 2) {
                  scrollH = screenW - barX - barXRight - bSpace * 2;
                }
                if (x > barX + bSpace + 1 +
                            int(int(browserScroll) *
                                (screenW - barX - barXRight - bSpace * 2) /
                                float(scrollH)) &&
                    x < barX + bSpace + 1 +
                            int(int(browserScroll) *
                                (screenW - barX - barXRight - bSpace * 2) /
                                float(scrollH)) +
                            int((screenW - barX - barXRight - bSpace * 2) *
                                (screenW - barX - barXRight - bSpace * 2) /
                                float(scrollH)) &&
                    y > barY - bSpace - 6 - 1 && y < barY - bSpace - 1) {
                  draggingSBar = true;
                  mouseOX = x;
                  mouseOY = y;
                  mouseX = x;
                  mouseY = y;
                  sBarDrag = 6;
                  sRatio =
                      scrollH / float(screenW - barX - barXRight - bSpace * 2);
                }
              }
              if (x > barX + 1 + bSpace && x < barX + 1 + bSpace + 8 &&
                  y > bSpace + tS && y < bSpace + tS + 8) {
                // left
                if (fnUndo.size() > 1) {
                  openBrowser(fnUndo.back(), 1, browserMode);
                }
              }
              if (x > barX + 1 + bSpace + 8 + bSpace &&
                  x < barX + 1 + bSpace + 8 + 8 + bSpace && y > bSpace + tS &&
                  y < bSpace + tS + 8) {
                // right
                if (fnRedo.size() > 0) {
                  openBrowser(fnRedo.back(), 2, browserMode);
                }
              }
              if (x > barX + 1 + bSpace + 8 * 2 + bSpace * 2 &&
                  x < barX + 1 + bSpace + 8 + 8 * 2 + bSpace * 2 &&
                  y > bSpace + tS && y < bSpace + tS + 8) {
                // up
                string newDir = currentDir;
                if (OS & Windows) {
                  newDir.erase(newDir.rfind('\\'));
                } else if (OS & Unix) {
                  newDir.erase(newDir.rfind('/'));
                }
                if (newDir.size() < 1) {
                  if (OS & Windows) {
                    newDir = "\\";
                  } else if (OS & Unix) {
                    newDir = "/";
                  }
                }
                openBrowser(newDir, 0, browserMode);
              }
              if (x > screenW - barXRight - 8 - bSpace - 1 &&
                  x < screenW - barXRight - bSpace - 1 && y > bSpace + tS &&
                  y < bSpace + tS + 8) {
                browserOpen = false;
                fnUndo.clear();
                fnRedo.clear();
              }
              if (x > screenW - barXRight - 8 - bSpace - 1 &&
                  x < screenW - barXRight - bSpace - 1 &&
                  y > bSpace * 2 + title + tS &&
                  y < bSpace * 2 + title + tS + 8) {
                // action
                string fullDir = currentDir;
                if (fullDir.size() != 1) {
                  if (OS & Windows) {
                    fullDir = fullDir.append("\\");
                  } else if (OS & Unix) {
                    fullDir = fullDir.append("/");
                  }
                }
                fullDir = fullDir.append(filenameB);
                bool folder = false;
                for (int i = 0; i < filenames.size(); i++) {
                  if (!strcmp(filenames.at(i)->name.c_str(),
                              filenameB.c_str()) &&
                      filenames.at(i)->folder) {
                    openBrowser(fullDir, 0, browserMode);
                    folder = true;
                  }
                }
                if (!folder) {
                  browserAction(fullDir, filenameB, currentDir);
                }
              }
            } else {
              if (x > barX && x < screenW - barXRight) {
                if (camMoving) {
                  draggingCam = true;
                  mouseOX = x;
                  mouseOY = y;
                  mouseX = x;
                  mouseY = y;
                } else {
                  for (int n = texs.at(currentTexture)->fxs.size() - 1; n >= 0;
                       n--) {
                    Effect* fx = texs.at(currentTexture)->fxs.at(n);
                    bool breaking = false;
                    bool noMove = false;
                    for (int in = 0; in < fx->inputs.size();
                         in++) {  // next step
                      if (x > fx->x + nodeCX - 4 && x < fx->x + nodeCX + 4 &&
                          y > fx->y + nodeCY + fx->inputs.at(in)->y &&
                          y < fx->y + nodeCY + fx->inputs.at(in)->y + 8) {
                        if (fx->inputs.at(in)->s != NULL) {
                          draggingSocket = true;
                          draggingNode = false;
                          saveUndo();
                          fx->inputs.at(in)->s->px = x;
                          fx->inputs.at(in)->s->py = y;
                          for (int j = 0;
                               j < texs.at(currentTexture)->fxs.size(); j++) {
                            if (texs.at(currentTexture)->fxs.at(j) ==
                                fx->inputs.at(in)->s->parent) {
                              draggedNode = j;
                              for (int out = 0; out < texs.at(currentTexture)
                                                          ->fxs.at(j)
                                                          ->outputs.size();
                                   out++) {
                                if (texs.at(currentTexture)
                                        ->fxs.at(j)
                                        ->outputs.at(out) ==
                                    fx->inputs.at(in)->s) {
                                  draggedSocket = out;
                                }
                              }
                            }
                          }
                          mouseOX = x;
                          mouseOY = y;
                          mouseX = x;
                          mouseY = y;
                          fx->undone = true;
                          fx->inputs.at(in)->s = NULL;
                        }
                        noMove = true;
                        breaking = true;
                      }
                    }
                    for (int out = 0; out < fx->outputs.size(); out++) {
                      if (x > fx->x + nodeCX + fx->w - 4 &&
                          x < fx->x + nodeCX + fx->w + 4 &&
                          y > fx->y + nodeCY + fx->outputs.at(out)->y &&
                          y < fx->y + nodeCY + fx->outputs.at(out)->y + 8) {
                        draggingSocket = true;
                        saveUndo();
                        fx->outputs.at(out)->px = x;
                        fx->outputs.at(out)->py = y;
                        draggedNode = n;
                        draggedSocket = out;
                        mouseOX = x;
                        mouseOY = y;
                        mouseX = x;
                        mouseY = y;
                        noMove = true;
                        breaking = true;
                      }
                    }
                    if (x > fx->x + nodeCX && x < fx->x + fx->w + nodeCX &&
                        y > fx->y + nodeCY && y < fx->y + fx->h + nodeCY) {
                      if (x > fx->x + fx->w + nodeCX - 8 - 4 &&
                          x < fx->x + fx->w + nodeCX - 4 &&
                          y > fx->y + nodeCY + 4 &&
                          y < fx->y + nodeCY + 4 + 8) {
                        noMove = true;
                        // delete Node
                        toolTip = "";
                        saveUndo();
                        for (int n2 = texs.at(currentTexture)->fxs.size() - 1;
                             n2 >= 0; n2--) {
                          Effect* fx2 = texs.at(currentTexture)->fxs.at(n2);
                          for (int in = 0; in < fx2->inputs.size(); in++) {
                            if (fx2->inputs.at(in)->s != NULL &&
                                fx2->inputs.at(in)->s->parent == fx) {
                              fx2->inputs.at(in)->s = NULL;
                              fx2->undone = true;
                              texs.at(currentTexture)->genTex();
                            }
                          }
                        }
                        if (fx->loading) {
                          fx->abort = true;
                          fx->deleted = true;
                        } else {
                          texs.at(currentTexture)
                              ->fxs.erase(
                                  std::remove(
                                      texs.at(currentTexture)->fxs.begin(),
                                      texs.at(currentTexture)->fxs.end(), fx),
                                  texs.at(currentTexture)->fxs.end());
                          delete fx;
                        }
                      }
                      if (x > fx->x + fx->w + nodeCX - 8 - 4 - 8 - 4 &&
                          x < fx->x + fx->w + nodeCX - 4 - 8 - 4 &&
                          y > fx->y + nodeCY + 4 &&
                          y < fx->y + nodeCY + 4 + 8) {
                        noMove = true;
                        // duplicate Node
                        saveUndo();
                        Effect* d = new Effect(fx->luafn, fx->fxname);
                        d->x = fx->x + 8;
                        d->y = fx->y + 8;
                        for (int pa = 0; pa < d->params.size(); pa++) {
                          d->params.at(pa)->value = fx->params.at(pa)->value;
                          d->params.at(pa)->points.clear();
                          for (int po = 0;
                               po < fx->params.at(pa)->points.size(); po++) {
                            CPoint* cp = new CPoint();
                            cp->a = fx->params.at(pa)->points.at(po)->a;
                            cp->r = fx->params.at(pa)->points.at(po)->r;
                            cp->g = fx->params.at(pa)->points.at(po)->g;
                            cp->b = fx->params.at(pa)->points.at(po)->b;
                            d->params.at(pa)->points.push_back(cp);
                          }
                        }
                        texs.at(currentTexture)->fxs.push_back(d);
                        texs.at(currentTexture)->genTex();
                      }
                      for (int out = 0; out < fx->outputs.size(); out++) {
                        if (x > int(fx->x + fx->w / 2.0 + nodeCX -
                                    int(texSizeX / 2.0)) &&
                            x < int(fx->x + fx->w / 2.0 + nodeCX +
                                    int(texSizeX / 2.0)) &&
                            y > int(fx->y + nodeCY + fx->outputs.at(out)->y +
                                    4 - texSizeY / 2.0) &&
                            y < int(fx->y + nodeCY + fx->outputs.at(out)->y +
                                    4 + texSizeY / 2.0)) {
                          // export tex
                          if (fx->doneTimer < 3) {
                            if (!browserOpen) {
                              currentSocket = fx->outputs.at(out);
                              openBrowser(currentDir, 0, 2);
                            }
                          }
                          noMove = true;
                        }
                      }
                      for (int p = 0; p < fx->params.size(); p++) {
                        Parameter* pa = fx->params.at(p);
                        if (x > fx->x + nodeCX + pa->x &&
                            x < fx->x + nodeCX + pa->x + pa->w &&
                            y > fx->y + nodeCY + pa->y &&
                            y < fx->y + nodeCY + pa->y + pa->h) {
                          pa->mouseDown(x, y, fx->x + nodeCX, fx->y + nodeCY, 0,
                                        fx);
                          noMove = true;
                        }
                      }
                      if (!noMove) {
                        draggingNode = true;
                        draggedNode = n;
                        mouseOX = x;
                        mouseOY = y;
                        mouseX = x;
                        mouseY = y;
                      }
                      breaking = true;
                    }
                    if (breaking) {
                      break;
                    }
                  }
                }
              }
              if (x > barX) {
                if (x > screenW - barXRight - 5 - 8 &&
                    x < screenW - barXRight - 5 && y > 5 && y < 5 + 8) {
                  // save file
                  if (!browserOpen) {
                    openBrowser(currentDir, 0, 5);
                  }
                }
                if (x > screenW - barXRight - 5 - 8 - 5 - 8 &&
                    x < screenW - barXRight - 5 - 5 - 8 && y > 5 && y < 5 + 8) {
                  // load file
                  if (!browserOpen) {
                    openBrowser(currentDir, 0, 4);
                  }
                }
                if (x > screenW - barXRight - 5 - 8 - 5 - 8 - 5 - 8 &&
                    x < screenW - barXRight - 5 - 5 - 8 - 5 - 8 && y > 5 &&
                    y < 5 + 8) {
                  // new file
                  newFile();
                }
              }
            }
            if (y > barY) {
              int spaceT = 1;
              int wT = barXRight - 2 - 7;
              float sizeT = texSizeX + 1;
              int cT = wT / sizeT;
              int leftspaceT = wT - sizeT * cT;
              int maxHTex = 0;
              leftspaceT = 0;
              for (int p = 0; p < texs.size(); p++) {
                int xt = p % cT;
                int yt = p / cT;
                if (yt * (sizeT) + sizeT > maxHTex) {
                  maxHTex = yt * (sizeT) + sizeT;
                }
                if (y > barY && y < screenH - 5 - 8 - 5 - (collH - 4) - 5) {
                  if (x > screenW - barXRight + 1 + xt * (sizeT) &&
                      x < screenW - barXRight + 1 + xt * (sizeT) + sizeT &&
                      y > barY + 1 + yt * (sizeT)-texScroll &&
                      y < barY + 1 + yt * (sizeT) + sizeT) {
                    currentTexture = p;
                  }
                }
              }
              int scrollW = 6;
              int scrollH = maxHTex;
              if (scrollH < screenH - barY - 5 - 8 - 5 - 8 - (14 - 8) / 2 - 5) {
                scrollH = screenH - barY - 5 - 8 - 5 - 8 - (14 - 8) / 2 - 5;
              }
              if (x > screenW - scrollW && x < screenW &&
                  y > barY + int(int(texScroll) * (screenH - barY - 5 - 8 - 5 -
                                                   8 - (14 - 8) / 2 - 5) /
                                 float(scrollH)) &&
                  y < barY + int(int(texScroll) * (screenH - barY - 5 - 8 - 5 -
                                                   8 - (14 - 8) / 2 - 5) /
                                 float(scrollH)) +
                          int((screenH - barY - 5 - 8 - 5 - 8 - (14 - 8) / 2 -
                               5) *
                              (screenH - barY - 5 - 8 - 5 - 8 - (14 - 8) / 2 -
                               5) /
                              float(scrollH))) {
                draggingSBar = true;
                mouseOX = x;
                mouseOY = y;
                mouseX = x;
                mouseY = y;
                sBarDrag = 5;
                sRatio = scrollH / float(screenH - barY - 5 - 8 - 5 - 8 -
                                         (14 - 8) / 2 - 5);
              }
            }
            if (y < barY) {
              for (int i = 0; i < newEffects.size(); i++) {
                Effect* fx = newEffects.at(i);
                if (x > 0 && x <= barX - 2 - 7 && y > collH * i - newEScroll &&
                    y <= collH * i - newEScroll + collH && y < barY) {
                  // new node
                  saveUndo();
                  if (fx->isPreset) {
                    fx->presetError = false;
                    fx->presetFxs.clear();
                    luaL_dofile(fx->L, fx->luafn.c_str());
                    lua_settop(fx->L, 0);
                    lua_getglobal(fx->L, "apply");
                    lua_pcall(fx->L, 0, 0, 0);
                    if (!fx->presetError) {
                      for (int i = 0; i < fx->presetFxs.size(); i++) {
                        texs.at(currentTexture)
                            ->fxs.push_back(fx->presetFxs.at(i));
                      }
                    }
                  } else {
                    texs.at(currentTexture)
                        ->fxs.push_back(new Effect(fx->luafn, fx->fxname));
                  }
                  texs.at(currentTexture)->genTex();
                }
              }
              int toolsScrolli = toolsScroll;
              int space = 1;
              float offset = 0;
              int w = barXRight - 10 + space - offset * 2 - 7 - 7;
              float size = 12;
              int c = w / size;
              int leftspace = w - size * c;
              leftspace = 0;
              int maxHPal = 0;
              for (int p = 0; p < palette.size(); p++) {
                int Cx = p % c;
                int Cy = p / c;
                int colorX =
                    leftspace + offset + Cx * (size) + screenW - barXRight + 5;
                int colorY =
                    offset + 3 + Cy * (size) + 12 - toolsScrolli - palScroll;
                if (Cy * (size) + size > maxHPal) {
                  maxHPal = Cy * (size) + size;
                }
                if (y > 3 + 12 - toolsScroll &&
                    y < 3 + 12 - toolsScroll + barY3 - 2 - 5 - 8 - 5) {
                  if (x > colorX && x < colorX + size && y > colorY &&
                      y < colorY + size) {
                    selectedColor = p;
                    Color* rgb = palette.at(selectedColor);
                    Color hsv = RGBtoHSV(rgb->r, rgb->g, rgb->b);
                    colorParams.at(0)->value = hsv.r;
                    colorParams.at(0)->value2 = hsv.g / 100.0;
                    colorParams.at(0)->value3 = hsv.b / 100.0;
                    colorParams.at(1)->value = rgb->r;
                    colorParams.at(2)->value = rgb->g;
                    colorParams.at(3)->value = rgb->b;
                    colorParams.at(4)->value = hsv.r;
                    colorParams.at(5)->value = hsv.g;
                    colorParams.at(6)->value = hsv.b;
                  }
                }
              }
              int scrollW = 6;
              int scrollH = maxHPal;
              if (scrollH < barY3 - 5 - 8 - 5) {
                scrollH = barY3 - 5 - 8 - 5;
              }
              if (x > screenW - 4 - 8 - scrollW && x < screenW - 4 - 8 &&
                  y > 2 + 12 - toolsScrolli +
                          int(int(palScroll) * (barY3 - 5 - 8 - 5) /
                              float(scrollH)) +
                          1 &&
                  y < 2 + 12 - toolsScrolli +
                          int(int(palScroll) * (barY3 - 5 - 8 - 5) /
                              float(scrollH)) +
                          1 + int((barY3 - 5 - 8 - 5) *
                                  (barY3 - 5 - 8 - 5 - 3) / float(scrollH)) +
                          1) {
                draggingSBar = true;
                mouseOX = x;
                mouseOY = y;
                mouseX = x;
                mouseY = y;
                sBarDrag = 3;
                sRatio = scrollH / float(barY3 - 5 - 8 - 5 - 3);
              }

              scrollH = barY5;
              if (scrollH < barY) {
                scrollH = barY;
              }
              if (x > screenW - scrollW && x < screenW &&
                  y > int(int(toolsScroll) * (barY - 2) / float(scrollH)) &&
                  y < int(int(toolsScroll) * (barY - 2) / float(scrollH)) +
                          int((barY) * (barY - 2) / float(scrollH)) + 1) {
                draggingSBar = true;
                mouseOX = x;
                mouseOY = y;
                mouseX = x;
                mouseY = y;
                sBarDrag = 4;
                sRatio = scrollH / float(barY);
              }
              if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
                  x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
                  y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
                  y < 1 + 12 - toolsScrolli + barY3 - 5) {
                saveUndo();
                Color* c = new Color(0, 0, 0);
                palette.push_back(c);
                selectedColor = palette.size() - 1;
                Color* rgb = palette.at(selectedColor);
                Color hsv = RGBtoHSV(rgb->r, rgb->g, rgb->b);
                colorParams.at(0)->value = hsv.r;
                colorParams.at(0)->value2 = hsv.g / 100.0;
                colorParams.at(0)->value3 = hsv.b / 100.0;
                colorParams.at(1)->value = rgb->r;
                colorParams.at(2)->value = rgb->g;
                colorParams.at(3)->value = rgb->b;
                colorParams.at(4)->value = hsv.r;
                colorParams.at(5)->value = hsv.g;
                colorParams.at(6)->value = hsv.b;

                for (int i = 0; i < texs.size(); i++) {
                  Texture* t = texs.at(i);
                  bool updateTex = false;
                  // UPDATE COLORS
                }

                paletteChanged();
              }
              if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
                  x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
                  y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
                  y < 1 + 12 - toolsScrolli + barY3 - 5) {
                saveUndo();
                Color* rgb = palette.at(selectedColor);
                Color* c = new Color(rgb->r, rgb->g, rgb->b);
                palette.insert(palette.begin() + selectedColor, c);
                selectedColor++;
                Color hsv = RGBtoHSV(rgb->r, rgb->g, rgb->b);
                colorParams.at(0)->value = hsv.r;
                colorParams.at(0)->value2 = hsv.g / 100.0;
                colorParams.at(0)->value3 = hsv.b / 100.0;
                colorParams.at(1)->value = rgb->r;
                colorParams.at(2)->value = rgb->g;
                colorParams.at(3)->value = rgb->b;
                colorParams.at(4)->value = hsv.r;
                colorParams.at(5)->value = hsv.g;
                colorParams.at(6)->value = hsv.b;

                for (int i = 0; i < texs.size(); i++) {
                  Texture* t = texs.at(i);
                  bool updateTex = false;
                  // UPDATECOLORS
                }

                paletteChanged();
              }
              if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 7 &&
                  x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 7 &&
                  y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
                  y < 1 + 12 - toolsScrolli + barY3 - 5) {
                saveUndo();
                Color* c = palette.at(selectedColor);
                if (palette.size() > 1) {
                  delete c;
                  palette.erase(std::remove(palette.begin(), palette.end(), c),
                                palette.end());
                  if (selectedColor >= palette.size()) {
                    selectedColor = palette.size() - 1;
                  }
                } else {
                  c->r = 0;
                  c->g = 0;
                  c->b = 0;
                }
                Color* rgb = palette.at(selectedColor);
                Color hsv = RGBtoHSV(rgb->r, rgb->g, rgb->b);
                colorParams.at(0)->value = hsv.r;
                colorParams.at(0)->value2 = hsv.g / 100.0;
                colorParams.at(0)->value3 = hsv.b / 100.0;
                colorParams.at(1)->value = rgb->r;
                colorParams.at(2)->value = rgb->g;
                colorParams.at(3)->value = rgb->b;
                colorParams.at(4)->value = hsv.r;
                colorParams.at(5)->value = hsv.g;
                colorParams.at(6)->value = hsv.b;

                for (int i = 0; i < texs.size(); i++) {
                  Texture* t = texs.at(i);
                  bool updateTex = false;
                  // UPDATECOLORS
                }

                paletteChanged();
              }
              if (x > screenW - 5 - 8 - 5 - 5 - 8 - 7 &&
                  x < screenW - 5 - 5 - 5 - 8 - 7 &&
                  y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
                  y < 1 + 12 - toolsScrolli + barY3 - 5) {
                // import
                if (!browserOpen) {
                  openBrowser(currentDir, 0, 0);
                }
              }
              if (x > screenW - 5 - 8 - 5 - 7 && x < screenW - 5 - 5 - 7 &&
                  y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
                  y < 1 + 12 - toolsScrolli + barY3 - 5) {
                // export
                if (!browserOpen) {
                  openBrowser(currentDir, 0, 1);
                }
              }
              for (int i = 0; i < colorParams.size(); i++) {
                colorParams.at(i)->mouseDown(x, y, screenW - barXRight,
                                             barY3 + 2 + 12 - toolsScrolli, -1,
                                             NULL);
              }
            }
            int scrollW = 6;
            int count = newEffects.size();
            if (draggingFX && draggedLayer == -1) {
              count--;
            }
            int scrollH = collH * count;
            if (scrollH < barY) {
              scrollH = barY;
            }
            if (x > barX - scrollW && x < barX &&
                y > int(int(newEScroll) * (barY - 2) / float(scrollH)) &&
                y < int(int(newEScroll) * (barY - 2) / float(scrollH)) +
                        int((barY) * (barY - 2) / float(scrollH)) + 1) {
              draggingSBar = true;
              mouseOX = x;
              mouseOY = y;
              mouseX = x;
              mouseY = y;
              sBarDrag = 2;
              sRatio = scrollH / float(barY);
            }
            if (x > 0 && x < screenW && y > barY && y < screenH) {
              Texture* t = texs.at(currentTexture);
              int h = -layersScroll;
              int iconsize = 8;
              int iconoffset = 5;
              int maxW = -layersScrollH;
              int scrollW = 6;
              int scrollH = h + layersScroll + collH;
              if (scrollH < screenH - (barY)) {
                scrollH = screenH - (barY);
              }
              int scrollH2 = maxW + layersScrollH;
              if (scrollH2 < screenW - barX - barXRight - 2 - scrollW) {
                scrollH2 = screenW - barX - barXRight - 2 - scrollW;
              }
              if (x > screenW - barXRight - scrollW - 1 &&
                  x < screenW - barXRight - scrollW - 1 + scrollW &&
                  y > barY + int(int(layersScroll) *
                                 (screenH - (barY)-scrollW) / float(scrollH)) &&
                  y < barY + int(int(layersScroll) *
                                 (screenH - (barY)-scrollW) / float(scrollH)) +
                          int((screenH - (barY)-scrollW) * (screenH - (barY)) /
                              float(scrollH)) +
                          1) {
                draggingSBar = true;
                mouseOX = x;
                mouseOY = y;
                mouseX = x;
                mouseY = y;
                sBarDrag = 0;
                sRatio = scrollH / float(screenH - (barY)-scrollW);
              }
              if (x > barX + 1 +
                          int(int(layersScrollH) *
                              (screenW - barX - barXRight - 2 - scrollW) /
                              float(scrollH2)) &&
                  x < barX + 1 +
                          int(int(layersScrollH) *
                              (screenW - barX - barXRight - 2 - scrollW) /
                              float(scrollH2)) +
                          int((screenW - barX - barXRight - 2 - scrollW) *
                              (screenW - barX - barXRight - 2 - scrollW) /
                              float(scrollH2)) &&
                  y > screenH - scrollW && y < screenH) {
                draggingSBar = true;
                mouseOX = x;
                mouseOY = y;
                mouseX = x;
                mouseY = y;
                sBarDrag = 1;
                sRatio =
                    scrollH2 / float(screenW - barX - barXRight - 2 - scrollW);
              }
              if (y > barY + h + iconoffset &&
                  y <= barY + h + iconoffset + iconsize && x > iconoffset &&
                  x <= iconoffset + iconsize) {
              }
            }
            if (x > barX && x < barX + UIdragRange && y < barY && !draggingFX &&
                !draggingSBar) {
              draggingUI = true;
              draggingSocket = false;
              draggingNode = false;
              mouseOX = x;
              mouseOY = y;
              mouseX = x;
              mouseY = y;
              UIdragType = 0;
            }
            if (x > screenW - barXRight - UIdragRange &&
                x < screenW - barXRight && y < barY && !draggingFX &&
                !draggingSBar) {
              draggingUI = true;
              draggingSocket = false;
              draggingNode = false;
              mouseOX = x;
              mouseOY = y;
              mouseX = x;
              mouseY = y;
              UIdragType = 2;
            }
          }
        }
        if (e.button.button == SDL_BUTTON_RIGHT) {
        }
        if (e.button.button == SDL_BUTTON_MIDDLE) {
          int x = e.button.x / screenScale;
          int y = e.button.y / screenScale;
          if (x > barX && x < screenW - barXRight) {
            draggingCam2 = true;
            mouseOX = x;
            mouseOY = y;
            mouseX = x;
            mouseY = y;
          }
        }
      }
      if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_MIDDLE) {
          draggingCam2 = false;
        }
        if (e.button.button == SDL_BUTTON_LEFT) {
          int x = e.button.x / screenScale;
          int y = e.button.y / screenScale;
          textTypeTemp = NULL;
          textTypeFxTemp = NULL;
          textTypeLayerTemp = -2;
          Texture* t = texs.at(currentTexture);
          if (view == 0) {
            for (int n = texs.at(currentTexture)->fxs.size() - 1; n >= 0; n--) {
              Effect* fx = texs.at(currentTexture)->fxs.at(n);
              for (int p = 0; p < fx->params.size(); p++) {
                Parameter* pa = fx->params.at(p);
                pa->mouseUp(x, y, fx->x + nodeCX, fx->y + nodeCY, 0, fx);
              }
            }
            if (draggingSocket) {
              Effect* fx = t->fxs.at(draggedNode);
              Socket* s = fx->outputs.at(draggedSocket);
              if (s->snapped) {
                s->s->s = s;
                s->s->parent->undone = true;
              }
              s->s = NULL;
              draggingSocket = false;
              t->genTex();
            }
            draggingCam = false;
            draggingNode = false;
            draggingUI = false;
            draggingSBar = false;
            int toolsScrolli = toolsScroll;
            for (int i = 0; i < colorParams.size(); i++) {
              colorParams.at(i)->mouseUp(x, y, screenW - barXRight,
                                         barY3 + 2 + 12 - toolsScrolli, -1,
                                         NULL);
            }
            draggingFX = false;
            int h = -layersScroll;
          }
        }
      }
      if (e.type == SDL_MOUSEMOTION) {
        int x = e.motion.x / screenScale;
        int y = e.motion.y / screenScale;
        mouseX = x;
        mouseY = y;
        textTypeTemp = NULL;
        textTypeFxTemp = NULL;
        textTypeLayerTemp = -2;
        preview = NULL;
        bool ttSet = false;
        for (int i = 0; i < newEffects.size(); i++) {
          Effect* fx = newEffects.at(i);
          int newEScrolli = newEScroll;
          if (x > 0 && x <= barX - 2 - 7 && y > collH * i - newEScroll &&
              y <= collH * i - newEScroll + collH && y < barY) {
            toolTip = newEffects.at(i)->desc + "\n\n\"";
            if (newEffects.at(i)->isPreset) {
              toolTip += "Presets/" + fx->fxname + "\"";
            } else {
              toolTip += "Nodes/" + fx->fxname + "\"";
            }
            ttSet = true;
          }
        }
        if (x > 0 && x < screenW && y > barY && y < screenH) {
          Texture* t = texs.at(currentTexture);
          int h = -layersScroll;
          int iconsize = 8;
          int iconoffset = 5;
          if (y > barY + h + iconoffset &&
              y <= barY + h + iconoffset + iconsize && x > iconoffset &&
              x <= iconoffset + iconsize) {
            toolTip = "New layer";
            ttSet = true;
          }
        }
        if (!browserOpen) {
          if (x > barX) {
            if (x > screenW - barXRight - 5 - 8 &&
                x < screenW - barXRight - 5 && y > 5 + 8 + 5 &&
                y < 5 + 8 + 5 + 8) {
              // info
              toolTip = "Tilemancer " + to_string(Vmajor) + "." +
                        to_string(Vminor) + "." + to_string(Vrevision);
              toolTip += "\nDeveloped by @ledgamedev";
              ttSet = true;
            }
            if (x > screenW - barXRight - 5 - 8 &&
                x < screenW - barXRight - 5 && y > 5 && y < 5 + 8) {
              // save file
              toolTip = "Save file";
              ttSet = true;
            }
            if (x > screenW - barXRight - 5 - 8 - 5 - 8 &&
                x < screenW - barXRight - 5 - 5 - 8 && y > 5 && y < 5 + 8) {
              // load file
              toolTip = "Load file";
              ttSet = true;
            }
            if (x > screenW - barXRight - 5 - 8 - 5 - 8 - 5 - 8 &&
                x < screenW - barXRight - 5 - 5 - 8 - 5 - 8 && y > 5 &&
                y < 5 + 8) {
              // new file
              toolTip = "New file";
              ttSet = true;
            }
          }
        }
        if (x > barX && x < screenW - barXRight) {
          for (int n = texs.at(currentTexture)->fxs.size() - 1; n >= 0; n--) {
            Effect* fx = texs.at(currentTexture)->fxs.at(n);
            bool breaking = false;
            if (x > fx->x + nodeCX && x < fx->x + fx->w + nodeCX &&
                y > fx->y + nodeCY && y < fx->y + fx->h + nodeCY) {
              for (int p = 0; p < fx->params.size(); p++) {
                Parameter* pa = fx->params.at(p);
                if (x > fx->x + nodeCX + pa->x &&
                    x < fx->x + nodeCX + pa->x + pa->w &&
                    y > fx->y + nodeCY + pa->y &&
                    y < fx->y + nodeCY + pa->y + pa->h) {
                  toolTip = pa->tt;
                  ttSet = true;
                }
              }
              if (x > fx->x + fx->w + nodeCX - 8 - 4 &&
                  x < fx->x + fx->w + nodeCX - 4 && y > fx->y + nodeCY + 4 &&
                  y < fx->y + nodeCY + 4 + 8) {
                toolTip = "Delete node";
                ttSet = true;
              }
              if (x > fx->x + fx->w + nodeCX - 8 - 4 - 8 - 4 &&
                  x < fx->x + fx->w + nodeCX - 4 - 8 - 4 &&
                  y > fx->y + nodeCY + 4 && y < fx->y + nodeCY + 4 + 8) {
                toolTip = "Duplicate node";
                ttSet = true;
              }
              for (int out = 0; out < fx->outputs.size(); out++) {
                if (x > int(fx->x + fx->w / 2.0 + nodeCX -
                            int(texSizeX / 2.0)) &&
                    x < int(fx->x + fx->w / 2.0 + nodeCX +
                            int(texSizeX / 2.0)) &&
                    y > int(fx->y + nodeCY + fx->outputs.at(out)->y + 4 -
                            texSizeY / 2.0) &&
                    y < int(fx->y + nodeCY + fx->outputs.at(out)->y + 4 +
                            texSizeY / 2.0)) {
                  toolTip = "Export tile";
                  ttSet = true;
                  preview = fx->outputs.at(out);
                }
              }
              breaking = true;
            }
            for (int in = 0; in < fx->inputs.size(); in++) {
              if (x > fx->x + nodeCX - 4 && x < fx->x + nodeCX + 4 &&
                  y > fx->y + nodeCY + fx->inputs.at(in)->y &&
                  y < fx->y + nodeCY + fx->inputs.at(in)->y + 8) {
                breaking = true;
              }
            }
            for (int out = 0; out < fx->outputs.size(); out++) {
              if (x > fx->x + nodeCX + fx->w - 4 &&
                  x < fx->x + nodeCX + fx->w + 4 &&
                  y > fx->y + nodeCY + fx->outputs.at(out)->y &&
                  y < fx->y + nodeCY + fx->outputs.at(out)->y + 8) {
                breaking = true;
              }
            }
            if (breaking) {
              break;
            }
          }
        }
        if (y < barY) {
          int toolsScrolli = toolsScroll;
          if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
              x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
              y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
              y < 1 + 12 - toolsScrolli + barY3 - 5) {
            toolTip = "New color";
            ttSet = true;
          }
          if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
              x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 5 - 8 - 7 &&
              y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
              y < 1 + 12 - toolsScrolli + barY3 - 5) {
            toolTip = "Duplicate color";
            ttSet = true;
          }
          if (x > screenW - 5 - 8 - 5 - 5 - 8 - 5 - 8 - 7 &&
              x < screenW - 5 - 5 - 5 - 8 - 5 - 8 - 7 &&
              y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
              y < 1 + 12 - toolsScrolli + barY3 - 5) {
            toolTip = "Delete color";
            ttSet = true;
          }
          if (x > screenW - 5 - 8 - 5 - 5 - 8 - 7 &&
              x < screenW - 5 - 5 - 5 - 8 - 7 &&
              y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
              y < 1 + 12 - toolsScrolli + barY3 - 5) {
            // import
            toolTip = "Load palette";
            ttSet = true;
          }
          if (x > screenW - 5 - 8 - 5 - 7 && x < screenW - 5 - 5 - 7 &&
              y > 1 + 12 - toolsScrolli + barY3 - 8 - 5 &&
              y < 1 + 12 - toolsScrolli + barY3 - 5) {
            // export
            toolTip = "Save palette";
            ttSet = true;
          }
          /*renderColor(screenW-barXRight+5+offsetA,
           barY2+9+3+3+offsetA-toolsScrolli, size-offsetA*2, size-offsetA*2,
           &rampA);
           renderColor(screenW-(size-offsetB)-5,
           barY2+9+3+3+offsetB-toolsScrolli, size-offsetB*2, size-offsetB*2,
           &rampB);*/
          for (int i = 0; i < colorParams.size(); i++) {
            Parameter* pr = colorParams.at(i);
            if (x - (screenW - barXRight) > pr->x &&
                x - (screenW - barXRight) <= pr->x + pr->w &&
                y - (barY3 + 2 + 12 - toolsScrolli) > pr->y &&
                y - (barY3 + 2 + 12 - toolsScrolli) <= pr->y + pr->h) {
              toolTip = pr->tt;
              ttSet = true;
            }
          }
        }
        if (!ttSet) {
          toolTip = "";
        }

        int ox = 0;
        int oy = 0;
        if (draggingParam) {
          int mx = 0;
          int my = 0;
          SDL_GetGlobalMouseState(&mx, &my);
          int sx = 0;
          int sy = 0;
          int sw = 0;
          int sh = 0;
          SDL_GetWindowPosition(window, &sx, &sy);
          if (OS & Windows) {
            mx += sx;
            my += sy;
          }
          SDL_GetWindowSize(window, &sw, &sh);
          while (mx < sx) {
            mx += sw;
            ox += sw / screenScale;
          }
          while (mx > sx + sw) {
            mx -= sw;
            ox -= sw / screenScale;
          }
          while (my < sy) {
            my += sh;
            oy += sh / screenScale;
          }
          while (my > sy + sh) {
            my -= sh;
            oy -= sh / screenScale;
          }
          if (OS & Windows) {
            warpMouse(mx - sx, my - sy);
          } else if (OS & Unix) {
            warpMouse(mx, my);
          }
          x += ox;
          y += oy;
        }

        for (int n = texs.at(currentTexture)->fxs.size() - 1; n >= 0; n--) {
          Effect* fx = texs.at(currentTexture)->fxs.at(n);
          for (int p = 0; p < fx->params.size(); p++) {
            Parameter* pa = fx->params.at(p);
            pa->oldmX += ox;
            pa->oldmY += oy;
            pa->mouseMove(x, y, fx->x + nodeCX, fx->y + nodeCY, 0, fx);
          }
        }

        if (view == 0) {
          int toolsScrolli = toolsScroll;
          for (int i = 0; i < colorParams.size(); i++) {
            colorParams.at(i)->oldmX += ox;
            colorParams.at(i)->oldmY += oy;
            colorParams.at(i)->mouseMove(x, y, screenW - barXRight,
                                         barY3 + 2 + 12 - toolsScrolli, -1,
                                         NULL);
          }
        }
      }
      if (e.type == SDL_FINGERDOWN) {
        mouseSX = -1;
        mouseSY = -1;
        scrollSet = false;
      }
      if (e.type == SDL_MULTIGESTURE) {
        mouseSDX = e.tfinger.x * screenH;
        mouseSDY = e.tfinger.y * screenH;
        if (!scrollSet && mouseSX != -1 && mouseSY != -1) {
          float distX = mouseSDX - mouseSX;
          float distY = mouseSDY - mouseSY;
          float dist = sqrt(distX * distX + distY * distY);
          // if(dist > 2) {
          if (abs(mouseSDY - mouseSY) >= abs(mouseSDX - mouseSX)) {
            scrollDir = true;
            scrollSet = true;
          } else {
            scrollDir = false;
            scrollSet = true;
          }
          //}
        }
        if (view == 0) {
          if (scrollSet) {
            if (!browserOpen) {
              if (mouseSX != -1 && mouseSY != -1 && mouseX > barX &&
                  mouseX < screenW - barXRight) {
                nodeCX += mouseSDX - mouseSX;
                nodeCY += mouseSDY - mouseSY;
              }
            }
            bool adjust = false;
            if (mouseSX != -1 && mouseSY != -1 && mouseY > barY &&
                mouseX < screenW - barXRight && scrollDir) {
              layersScroll -= mouseSDY - mouseSY;
              adjust = true;
            }
            if (mouseSX != -1 && mouseSY != -1 && mouseY > barY &&
                mouseX > barX && mouseX < screenW - barXRight && !scrollDir) {
              layersScrollH -= mouseSDX - mouseSX;
              adjust = true;
            }
            if (mouseSX != -1 && mouseSY != -1 && mouseY < barY &&
                mouseX < barX) {
              newEScroll -= mouseSDY - mouseSY;
              adjust = true;
            }
            if (adjust) {
              adjustLayersScroll();
            }

            bool adjust2 = false;
            bool scrollingPal = false;
            if (mouseSX != -1 && mouseSY != -1 &&
                mouseX > screenW - barXRight + 5 && mouseX < screenW - 5 &&
                mouseY < barY && mouseY > 3 + 12 - toolsScroll &&
                mouseY < 3 + 12 - toolsScroll + barY3 - 2 - 5 - 8 - 5) {
              palScroll -= mouseSDY - mouseSY;
              adjust2 = true;
              scrollingPal = true;
            }
            if (mouseSX != -1 && mouseSY != -1 &&
                mouseX > screenW - barXRight && mouseY < barY &&
                !scrollingPal) {
              toolsScroll -= mouseSDY - mouseSY;
              adjust2 = true;
            }
            if (mouseSX != -1 && mouseSY != -1 &&
                mouseX > screenW - barXRight && mouseY > barY &&
                mouseY < screenH - 5 - 8 - 5 - (collH - 4) - 5) {
              texScroll -= mouseSDY - mouseSY;
              adjust2 = true;
            }
            if (adjust2) {
              adjustToolsScroll();
            }

            bool adjust3 = false;
            int title = collH - 4;
            int bSpace = 5;
            if (mouseSX != -1 && mouseSY != -1 && mouseX > barX + bSpace &&
                mouseX < screenW - barXRight - bSpace &&
                mouseY > bSpace * 2 + title && mouseY < barY - bSpace &&
                browserOpen) {
              browserScroll -= mouseSDX - mouseSX;
              adjust3 = true;
            }
            if (adjust3) {
              adjustBrowserScroll();
            }
          }
        }
        mouseSX = e.tfinger.x * screenH;
        mouseSY = e.tfinger.y * screenH;
      }
      if (e.type == SDL_MOUSEWHEEL) {
        if (!browserOpen) {
          if (e.wheel.y < 0) {
            if (mouseX > barX && mouseX < screenW - barXRight && mouseY > 0 &&
                mouseY < barY) {
              if (zoom < 2) {
                zoom++;
              }
            }
          } else if (e.wheel.y > 0) {
            if (mouseX > barX && mouseX < screenW - barXRight && mouseY > 0 &&
                mouseY < barY) {
              if (zoom > 1) {
                zoom--;
              }
            }
          }
        }
      }
      if (e.type == SDL_TEXTINPUT) {
        if (browserOpen) {
          filenameB.append(e.text.text);
          overwrite = false;
        }
        if (textType != NULL && (textType->ID == 1 || textType->ID == 2)) {
          if (textType->typing.length() < 10) {
            textType->typing.append(e.text.text);
          }
        }
      }
      if (e.type == SDL_KEYUP) {
        if (e.key.keysym.sym == SDLK_LSHIFT) {
          camMoving = false;
        }
      }
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_LSHIFT) {
          camMoving = true;
        }
        if (e.key.keysym.sym == SDLK_BACKSPACE) {
          if (filenameB.size() > 0) {
            filenameB.pop_back();
          }
          overwrite = false;
          if (textType != NULL && (textType->ID == 1 || textType->ID == 2)) {
            if (textType->typing.size() > 0) {
              textType->typing.pop_back();
            }
          }
        }
        if (e.key.keysym.sym == SDLK_z &&
            SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
          undo();
        }
        if (e.key.keysym.sym == SDLK_y &&
            SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
          redo();
        }
        if (e.key.keysym.sym == SDLK_n &&
            SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
          newFile();
        }
        if (e.key.keysym.sym == SDLK_s &&
            SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
          if (!browserOpen) {
            openBrowser(currentDir, 0, 5);
          }
        }
        if (e.key.keysym.sym == SDLK_o &&
            SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
          if (!browserOpen) {
            openBrowser(currentDir, 0, 4);
          }
        }
        /*if((e.key.keysym.unicode >= 33 && e.key.keysym.unicode <= 95) ||
        e.key.keysym.unicode >= 97 && e.key.keysym.unicode <= 126) ||
        e.key.keysym.unicode == 32) {
        }*/
        if (e.key.keysym.sym == SDLK_RETURN) {
          if (browserOpen) {
            string fullDir = currentDir;
            if (fullDir.size() != 1) {
              fullDir = fullDir.append("/");
            }
            fullDir = fullDir.append(filenameB);
            bool folder = false;
            for (int i = 0; i < filenames.size(); i++) {
              if (!strcmp(filenames.at(i)->name.c_str(), filenameB.c_str()) &&
                  filenames.at(i)->folder) {
                openBrowser(fullDir, 0, browserMode);
                folder = true;
              }
            }
            if (!folder) {
              browserAction(fullDir, filenameB, currentDir);
            }
          }
          if (textType != NULL) {
            textType->value = atoi(textType->typing.c_str());
            if (textType->value > textType->value3 && textType->value3 != -1) {
              textType->value = textType->value3;
            }
            if (textType->value < textType->value2 && textType->value2 != -1) {
              textType->value = textType->value2;
            }
            updateDrag(textTypeLayer, textTypeFx, textType);
          }
          textType = NULL;
        }
        if (e.key.keysym.sym == SDLK_LEFT) {
          if (SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
            // swap with left side
          }
          currentTexture--;
          if (currentTexture < 0) {
            currentTexture = 0;
          }
        }
        if (e.key.keysym.sym == SDLK_RIGHT) {
          if (SDL_GetModState() & (KMOD_GUI | KMOD_CTRL)) {
            // swap with right side
          }
          currentTexture++;
          if (currentTexture >= texs.size()) {
            currentTexture = texs.size() - 1;
          }
        }
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          if (browserOpen) {
            browserOpen = false;
            fnUndo.clear();
            fnRedo.clear();
          }
        }
      }
      if (e.type == SDL_KEYUP) {
      }
    }
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    double newTime = 0;
    if (gameStarted) {
      newTime = SDL_GetTicks();
    } else {
      newTime = SDL_GetTicks();
      currentTime = SDL_GetTicks();
    }
    gameStarted = true;
    double frameTime = (newTime - currentTime) / 1000.0;
    currentTime = newTime;
    accumulator += frameTime;
    if (fpsTimer == 0) {
      if (frameTime != 0) {
        fps = 1.0 / frameTime;
      } else {
        fps = 0;
      }
      fpsTimer = 0;
    } else {
      fpsTimer--;
    }
    while (accumulator >= dt) {
      update();
      accumulator -= dt;
    }
    renderGL();
    SDL_GL_SwapWindow(window);

    double newTime2 = 0;
    if (gameStarted) {
      newTime2 = SDL_GetTicks();
    } else {
      newTime2 = SDL_GetTicks();
      lastTime = SDL_GetTicks();
    }
    double frameTime2 = (newTime2 - lastTime) / 1000.0;
    double delayTime = dt - frameTime2;
    if (delayTime > 0) {
      SDL_Delay(delayTime * 1000);
    }
    lastTime = SDL_GetTicks();
  }
  lua_close(L);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
