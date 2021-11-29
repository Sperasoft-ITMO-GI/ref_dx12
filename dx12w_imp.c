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
qboolean GLimp_InitGL(void);

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
qboolean GLimp_Init(void* hinstance, void* wndproc)
{
#define OSR2_BUILD_NUMBER 1111

	OSVERSIONINFO	vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	glw_state.allowdisplaydepthchange = false;

	/*if (GetVersionEx(&vinfo))
	{
		if (vinfo.dwMajorVersion > 4)
		{
			glw_state.allowdisplaydepthchange = true;
		}
		else if (vinfo.dwMajorVersion == 4)
		{
			if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				glw_state.allowdisplaydepthchange = true;
			}
			else if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				if (LOWORD(vinfo.dwBuildNumber) >= OSR2_BUILD_NUMBER)
				{
					glw_state.allowdisplaydepthchange = true;
				}
			}
		}
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - GetVersionEx failed\n");
		return false;
	}*/

	glw_state.hInstance = (HINSTANCE)hinstance;
	glw_state.wndproc = wndproc;

	return true;
}