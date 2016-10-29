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

#include "tilemancer/globals.h"

double dt = 1.0 / 60.0;
double currentTime;
double lastTime;
double accumulator;
unsigned int fps;
int fpsTimer;

float texSizeX;
float texSizeY;
int screenW;
int screenH;
int displayW;
int displayH;
float camX;
float camY;
float nodeCSX;
float nodeCSY;
float nodeCX;
float nodeCY;
bool camMoving = false;
bool draggingCam = false;
bool draggingCam2 = false;
float camOffset;
float camOffsetSpeed;
float camOffsetMagnitude;
bool camPositive;
float camRot;
int camXFinal;
int camYFinal;
int zoom = 1;
int draggedLayer;
bool draggingNode;
int draggedNode;
bool draggingSocket;
int draggedSocket;
float screenScale = 2.0;
int logoTimer;
int logoTimerMax = 150;
std::string toolTip;
std::string errorMessage;
int errorTimer;
std::vector<float> data;
std::vector<Color*> palette;
Color rampA(254, 237, 186);
Color rampB(54, 42, 96);
int selectedColor = 0;
std::vector<Parameter*> colorParams;
Parameter* textType;
Effect* textTypeFx;
int textTypeLayer;
Parameter* textTypeTemp;
Effect* textTypeFxTemp;
int textTypeLayerTemp;
int blinkTimer;
std::vector<Texture*> texs;
std::vector<Effect*> newEffects;
int collH = 18;
int expH = 92;
int currentTexture;
Socket* currentSocket;
float barX;
float barXRight;
float barY;
float barY2;
float barY3;
float barY4;
float barY5;
float layersScroll;
float toolsScroll;
float palScroll;
float texScroll;
float newEScroll;
float layersScrollH;
bool scrollDir;
bool scrollSet;
float browserScroll;
bool draggingParam;
bool draggingFX;
bool draggingUI;
bool draggingSBar;
int sBarDrag;
float sRatio;
int rotTimer;
int UIdragType;
int UIdragRange = 5;
float mouseOX;
float mouseOY;
float mouseX;
float mouseY;
bool gameStarted;
int view = 0;
int doubleClickTimer;
int typeTimer;
float mouseSX;
float mouseSY;
float mouseSDX;
float mouseSDY;
float prevC;
float prevCount;
Socket* preview;
int previewTimer;
std::vector<std::string> listUndo;
std::vector<std::string> listRedo;
std::string currentDir;
std::string filenameB;
bool browserOpen;
int browserMode;
std::vector<File*> filenames;
std::vector<std::string> fnUndo;
std::vector<std::string> fnRedo;
std::string lastPalDir;
std::string lastPalName;
std::string lastSaveDir;
std::string lastSaveName;
std::string lastTexDir;
std::string lastTexName;
bool overwrite;
int selectedFile;
GLuint gridImg;
GLuint effectImg;
GLuint effectImg2;
GLuint effectImg3;
GLuint effectImg4;
GLuint effectImg5;
GLuint effectImg6;
GLuint effectImg8;
GLuint effectImg10;
GLuint effectImg11;
GLuint effectImg14;
GLuint effectImg15;
GLuint effectImg16;
GLuint bezierFill;
GLuint bezierFillError;
GLuint iconImg2;
GLuint iconImg3;
GLuint iconImg6;
GLuint iconImg7;
GLuint iconImg8;
GLuint iconImg9;
GLuint iconImg10;
GLuint iconImg11;
GLuint iconImg12;
GLuint iconImg13;
GLuint palImg;
GLuint palImgReal;
GLuint fontImg;
GLuint fontImg2;

GLenum my_program;
GLenum my_vertex_shader;
GLenum my_fragment_shader;

GLenum light_program;
GLenum light_vertex_shader;
GLenum light_fragment_shader;

GLenum blur_program;
GLenum blur_vertex_shader;
GLenum blur_fragment_shader;

GLenum transition_program;
GLenum transition_vertex_shader;
GLenum transition_fragment_shader;

GLuint screenFbo;
GLuint screenTex;
GLuint screenTexN;
GLuint screenTexB;
GLuint screenTexMisc;

GLuint screenFboFinal2;
GLuint screenTexFinal2;

GLuint vbo;
GLuint vboTri;
GLuint vao;

GLuint digitsImg;
GLuint noTex;
GLuint logoImage;

GLint mp_tex;
GLint mp_texN;
GLint mp_frameSize;
GLint mp_texSize;
GLint mp_off;
GLint mp_frame;
GLint mp_depth;
GLint mp_alpha;
GLint mp_strength;
GLint mp_model;
GLint mp_flip;
GLint mp_proj;
lua_State* L;

glm::mat4 model = glm::mat4(1.0);
glm::mat4 proj = glm::mat4(1.0);
