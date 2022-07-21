#ifndef TILEMANCER_OS_WINDOWS_H
#define TILEMANCER_OS_WINDOWS_H

#define GLEW_STATIC
#include <GL\glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <Shlobj.h>
#include <gl\glu.h>
#include <windows.h>
#include "mingw.mutex.h"
#include "mingw.thread.h"
#include "minizip/unzip.h"
#define SDL_GetGlobalMouseState SDL_GetMouseState
#define glGenFramebuffers glGenFramebuffersEXT
#define glBindFramebuffer glBindFramebufferEXT
#define glFramebufferTexture2D glFramebufferTexture2DEXT
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#define GL_COLOR_ATTACHMENT1 GL_COLOR_ATTACHMENT1_EXT
#define GL_COLOR_ATTACHMENT2 GL_COLOR_ATTACHMENT2_EXT
#define GL_COLOR_ATTACHMENT3 GL_COLOR_ATTACHMENT3_EXT

extern const char* light_vertex_shader_source;
extern const char* light_fragment_shader_source;

extern const char* blur_vertex_shader_source;
extern const char* blur_fragment_shader_source;

extern const char* transition_vertex_shader_source;
extern const char* transition_fragment_shader_source;

extern const char* my_vertex_shader_source;
extern const char* my_fragment_shader_source;

extern int OS;

#endif  // TILEMANCER_OS_WINDOWS_H
