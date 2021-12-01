/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/
#include <assert.h>
#include <windows.h>
#include "../ref_gl/gl_local.h"
//#include "glw_win.h"
#include "dx12w_win.h"
#include "../win32/winquake.h"

static qboolean GLimp_SwitchFullscreen(int width, int height);
//qboolean GLimp_InitGL(void);

glwstate_t glw_state;

extern cvar_t* vid_fullscreen;
extern cvar_t* vid_ref;




/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/
