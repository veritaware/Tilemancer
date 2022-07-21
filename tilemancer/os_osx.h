#ifndef TILEMANCER_OS_OSX_H
#define TILEMANCER_OS_OSX_H

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <mach-o/dyld.h>
#include <pwd.h>
#include <future>
#include "CoreFoundation/CFBundle.h"

extern const char* light_vertex_shader_source;
extern const char* light_fragment_shader_source;

extern const char* blur_vertex_shader_source;
extern const char* blur_fragment_shader_source;

extern const char* transition_vertex_shader_source;
extern const char* transition_fragment_shader_source;

extern const char* my_vertex_shader_source;
extern const char* my_fragment_shader_source;
extern CFBundleRef mainBundle;

extern int OS;

#endif  // TILEMANCER_OS_OSX_H
