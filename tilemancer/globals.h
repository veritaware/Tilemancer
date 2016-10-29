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

#ifndef TILEMANCER_GLOBALS_H
#define TILEMANCER_GLOBALS_H

#include "glm/glm.hpp"
#include "tilemancer/color.h"
#include "tilemancer/gl.h"

#include <string>
#include <vector>

extern "C" {
struct lua_State;
};

class Texture;
class Effect;
class File;
class Parameter;
class Socket;

const int Vmajor = 0;
const int Vminor = 2;
const int Vrevision = 1;

extern double dt;
extern double currentTime;
extern double lastTime;
extern double accumulator;
extern unsigned int fps;
extern int fpsTimer;

extern float texSizeX;
extern float texSizeY;
extern int screenW;
extern int screenH;
extern int displayW;
extern int displayH;
extern float camX;
extern float camY;
extern float nodeCSX;
extern float nodeCSY;
extern float nodeCX;
extern float nodeCY;
extern bool camMoving;
extern bool draggingCam;
extern bool draggingCam2;
extern float camOffset;
extern float camOffsetSpeed;
extern float camOffsetMagnitude;
extern bool camPositive;
extern float camRot;
extern int camXFinal;
extern int camYFinal;
extern int zoom;
extern int draggedLayer;
extern bool draggingNode;
extern int draggedNode;
extern bool draggingSocket;
extern int draggedSocket;
extern float screenScale;
extern int logoTimer;
extern int logoTimerMax;
extern std::string toolTip;
extern std::string errorMessage;
extern int errorTimer;
extern std::vector<float> data;
extern std::vector<Color*> palette;
extern Color rampA;
extern Color rampB;
extern int selectedColor;
extern std::vector<Parameter*> colorParams;
extern Parameter* textType;
extern Effect* textTypeFx;
extern int textTypeLayer;
extern Parameter* textTypeTemp;
extern Effect* textTypeFxTemp;
extern int textTypeLayerTemp;
extern int blinkTimer;
extern std::vector<Texture*> texs;
extern std::vector<Effect*> newEffects;
extern int collH;
extern int expH;
extern int currentTexture;
extern Socket* currentSocket;
extern float barX;
extern float barXRight;
extern float barY;
extern float barY2;
extern float barY3;
extern float barY4;
extern float barY5;
extern float layersScroll;
extern float toolsScroll;
extern float palScroll;
extern float texScroll;
extern float newEScroll;
extern float layersScrollH;
extern bool scrollDir;
extern bool scrollSet;
extern float browserScroll;
extern bool draggingParam;
extern bool draggingFX;
extern bool draggingUI;
extern bool draggingSBar;
extern int sBarDrag;
extern float sRatio;
extern int rotTimer;
extern int UIdragType;
extern int UIdragRange;
extern float mouseOX;
extern float mouseOY;
extern float mouseX;
extern float mouseY;
extern bool gameStarted;
extern int view;
extern int doubleClickTimer;
extern int typeTimer;
extern float mouseSX;
extern float mouseSY;
extern float mouseSDX;
extern float mouseSDY;
extern float prevC;
extern float prevCount;
extern Socket* preview;
extern int previewTimer;
extern std::vector<std::string> listUndo;
extern std::vector<std::string> listRedo;
extern std::string currentDir;
extern std::string filenameB;
extern bool browserOpen;
extern int browserMode;
extern std::vector<File*> filenames;
extern std::vector<std::string> fnUndo;
extern std::vector<std::string> fnRedo;
extern std::string lastPalDir;
extern std::string lastPalName;
extern std::string lastSaveDir;
extern std::string lastSaveName;
extern std::string lastTexDir;
extern std::string lastTexName;
extern bool overwrite;
extern int selectedFile;
extern GLuint gridImg;
extern GLuint effectImg;
extern GLuint effectImg2;
extern GLuint effectImg3;
extern GLuint effectImg4;
extern GLuint effectImg5;
extern GLuint effectImg6;
extern GLuint effectImg8;
extern GLuint effectImg10;
extern GLuint effectImg11;
extern GLuint effectImg14;
extern GLuint effectImg15;
extern GLuint effectImg16;
extern GLuint bezierFill;
extern GLuint bezierFillError;
extern GLuint iconImg2;
extern GLuint iconImg3;
extern GLuint iconImg6;
extern GLuint iconImg7;
extern GLuint iconImg8;
extern GLuint iconImg9;
extern GLuint iconImg10;
extern GLuint iconImg11;
extern GLuint iconImg12;
extern GLuint iconImg13;
extern GLuint palImg;
extern GLuint palImgReal;
extern GLuint fontImg;
extern GLuint fontImg2;

extern GLenum my_program;
extern GLenum my_vertex_shader;
extern GLenum my_fragment_shader;

extern GLenum light_program;
extern GLenum light_vertex_shader;
extern GLenum light_fragment_shader;

extern GLenum blur_program;
extern GLenum blur_vertex_shader;
extern GLenum blur_fragment_shader;

extern GLenum transition_program;
extern GLenum transition_vertex_shader;
extern GLenum transition_fragment_shader;

extern GLuint screenFbo;
extern GLuint screenTex;
extern GLuint screenTexN;
extern GLuint screenTexB;
extern GLuint screenTexMisc;

extern GLuint screenFboFinal2;
extern GLuint screenTexFinal2;

extern GLuint vbo;
extern GLuint vboTri;
extern GLuint vao;

extern GLuint digitsImg;
extern GLuint noTex;
extern GLuint logoImage;

extern GLint mp_tex;
extern GLint mp_texN;
extern GLint mp_frameSize;
extern GLint mp_texSize;
extern GLint mp_off;
extern GLint mp_frame;
extern GLint mp_depth;
extern GLint mp_alpha;
extern GLint mp_strength;
extern GLint mp_model;
extern GLint mp_flip;
extern GLint mp_proj;
extern lua_State* L;

extern glm::mat4 model;
extern glm::mat4 proj;

#endif  // TILEMANCER_GLOBALS_H
