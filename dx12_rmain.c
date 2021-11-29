// r_main.c
#include "dx12_local.h"

viddef_t	vid;

refimport_t	ri;

// параметры в конфиг файле

cvar_t* r_norefresh;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_speeds;
cvar_t* r_fullbright;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
cvar_t* r_lefthand;

cvar_t* r_lightlevel;	// FIXME: This is a HACK to get the client's light level

cvar_t* vid_fullscreen;
cvar_t* vid_gamma;
cvar_t* vid_ref;

// запись параметров в конфиг файл
void R_Register(void)
{
	r_lefthand = ri.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);
	r_fullbright = ri.Cvar_Get("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get("r_nocull", "0", 0);
	r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);
	r_speeds = ri.Cvar_Get("r_speeds", "0", 0);
	
	r_lightlevel = ri.Cvar_Get("r_lightlevel", "0", 0);
	
	//gl_nosubimage = ri.Cvar_Get("gl_nosubimage", "0", 0);
	//gl_allow_software = ri.Cvar_Get("gl_allow_software", "0", 0);
	
	//gl_particle_min_size = ri.Cvar_Get("gl_particle_min_size", "2", CVAR_ARCHIVE);
	//gl_particle_max_size = ri.Cvar_Get("gl_particle_max_size", "40", CVAR_ARCHIVE);
	//gl_particle_size = ri.Cvar_Get("gl_particle_size", "40", CVAR_ARCHIVE);
	//gl_particle_att_a = ri.Cvar_Get("gl_particle_att_a", "0.01", CVAR_ARCHIVE);
	//gl_particle_att_b = ri.Cvar_Get("gl_particle_att_b", "0.0", CVAR_ARCHIVE);
	//gl_particle_att_c = ri.Cvar_Get("gl_particle_att_c", "0.01", CVAR_ARCHIVE);
	
	//gl_modulate = ri.Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	//gl_log = ri.Cvar_Get("gl_log", "0", 0);
	//gl_bitdepth = ri.Cvar_Get("gl_bitdepth", "0", 0);
	//gl_mode = ri.Cvar_Get("gl_mode", "3", CVAR_ARCHIVE);
	//gl_lightmap = ri.Cvar_Get("gl_lightmap", "0", 0);
	//gl_shadows = ri.Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
	//gl_dynamic = ri.Cvar_Get("gl_dynamic", "1", 0);
	//gl_nobind = ri.Cvar_Get("gl_nobind", "0", 0);
	//gl_round_down = ri.Cvar_Get("gl_round_down", "1", 0);
	//gl_picmip = ri.Cvar_Get("gl_picmip", "0", 0);
	//gl_skymip = ri.Cvar_Get("gl_skymip", "0", 0);
	//gl_showtris = ri.Cvar_Get("gl_showtris", "0", 0);
	//gl_ztrick = ri.Cvar_Get("gl_ztrick", "0", 0);
	//gl_finish = ri.Cvar_Get("gl_finish", "0", CVAR_ARCHIVE);
	//gl_clear = ri.Cvar_Get("gl_clear", "0", 0);
	//gl_cull = ri.Cvar_Get("gl_cull", "1", 0);
	//gl_polyblend = ri.Cvar_Get("gl_polyblend", "1", 0);
	//gl_flashblend = ri.Cvar_Get("gl_flashblend", "0", 0);
	//gl_playermip = ri.Cvar_Get("gl_playermip", "0", 0);
	//gl_monolightmap = ri.Cvar_Get("gl_monolightmap", "0", 0);
	//gl_driver = ri.Cvar_Get("gl_driver", "opengl32", CVAR_ARCHIVE);
	//gl_texturemode = ri.Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	//gl_texturealphamode = ri.Cvar_Get("gl_texturealphamode", "default", CVAR_ARCHIVE);
	//gl_texturesolidmode = ri.Cvar_Get("gl_texturesolidmode", "default", CVAR_ARCHIVE);
	//gl_lockpvs = ri.Cvar_Get("gl_lockpvs", "0", 0);
	
	//gl_vertex_arrays = ri.Cvar_Get("gl_vertex_arrays", "0", CVAR_ARCHIVE);
	
	//gl_ext_swapinterval = ri.Cvar_Get("gl_ext_swapinterval", "1", CVAR_ARCHIVE);
	//gl_ext_palettedtexture = ri.Cvar_Get("gl_ext_palettedtexture", "1", CVAR_ARCHIVE);
	//gl_ext_multitexture = ri.Cvar_Get("gl_ext_multitexture", "1", CVAR_ARCHIVE);
	//gl_ext_pointparameters = ri.Cvar_Get("gl_ext_pointparameters", "1", CVAR_ARCHIVE);
	//gl_ext_compiled_vertex_array = ri.Cvar_Get("gl_ext_compiled_vertex_array", "1", CVAR_ARCHIVE);
	
	//gl_drawbuffer = ri.Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	//gl_swapinterval = ri.Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);
	
	//gl_saturatelighting = ri.Cvar_Get("gl_saturatelighting", "0", 0);
	
	//gl_3dlabs_broken = ri.Cvar_Get("gl_3dlabs_broken", "1", CVAR_ARCHIVE);

	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
	vid_ref = ri.Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);



	//ri.Cmd_AddCommand("imagelist", GL_ImageList_f);
	//ri.Cmd_AddCommand("screenshot", GL_ScreenShot_f);
	//ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	//ri.Cmd_AddCommand("gl_strings", GL_Strings_f);
}

/*
===============
R_Init
===============
*/
int R_Init(void* hinstance, void* hWnd)
{
	/*char renderer_buffer[1000];
	char vendor_buffer[1000];
	int		err;
	int		j;
	extern float r_turbsin[256];

	for (j = 0; j < 256; j++)
	{
		r_turbsin[j] *= 0.5;
	}

	ri.Con_Printf(PRINT_ALL, "ref_dx12 version: "REF_VERSION"\n");*/

	//Draw_GetPalette();

	R_Register();

	// не нужно на винде!!!
	// initialize our QGL dynamic bindings
	/*if (!QGL_Init(gl_driver->string))
	{
		QGL_Shutdown();
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - could not load \"%s\"\n", gl_driver->string);
		return -1;
	}*/

	// инициализация специфичных фич для конкретной ОС
	// а также привязка окна и инстанса
	// initialize OS-specific parts of OpenGL
	if (!GLimp_Init(hinstance, hWnd))
	{
		//QGL_Shutdown();
		return -1;
	}

	// я так понял он тут устанавливает 3 версию opengl
	// set our "safe" modes
	//gl_state.prev_mode = 3;

	// установка полноэкранного режима и размера окна
	// а также контекста
	// create the window and set up the context
	/*if (!R_SetMode())
	{
		QGL_Shutdown();
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_Init() - could not R_SetMode()\n");
		return -1;
	}*/

	// инициализация менюшки
	//ri.Vid_MenuInit();

	/*
	** get our various GL strings
	* временно удалил чтобы не путались
	*/
	

	/*
	** grab extensions
	* временно удалил чтобы не путались
	*/

	// устанавливается дефолтное состояние рендера
	//GL_SetDefaultState();


	// ладно, пусть будет, может быть это важно (нет)
	/*
	** draw our stereo patterns
	*/
#if 0 // commented out until H3D pays us the money they owe us
	GL_DrawStereoPattern();
#endif

	//GL_InitImages();
	//Mod_Init();
	//R_InitParticleTexture();
	//Draw_InitLocal();

	// тут была проверка на ошибки
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown(void)
{
	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("gl_strings");

	//Mod_FreeAll();

	//GL_ShutdownImages();

	/*
	** shut down OS specific OpenGL stuff like contexts, etc.
	*/
	//GLimp_Shutdown();

	/*
	** shutdown our QGL subsystem
	*/
	//QGL_Shutdown();
}

// ==================================================================================================================================
/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginRegistration(char* model)
{
	return;
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RegisterModel

@@@@@@@@@@@@@@@@@@@@@
*/
struct model_s* R_RegisterModel(char* name)
{
	return NULL;
}

/*
===============
R_RegisterSkin
===============
*/
struct image_s* R_RegisterSkin(char* name)
{
	return NULL;
}

/*
=============
Draw_FindPic
=============
*/
image_t* Draw_FindPic(char* name)
{
	return NULL;
}

/*
============
R_SetSky
============
*/
// 3dstudio environment map names
char* suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };
void R_SetSky(char* name, float rotate, vec3_t axis)
{
	return;
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_EndRegistration

@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndRegistration(void)
{
	return;
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame(refdef_t* fd)
{
	return;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize(int* w, int* h, char* pic)
{
	return;
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic(int x, int y, char* pic)
{
	return;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic(int x, int y, int w, int h, char* pic)
{
	return;
}

/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char(int x, int y, int num)
{
	return;
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear(int x, int y, int w, int h, char* pic)
{
	return;
}

/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill(int x, int y, int w, int h, int c)
{
	return;
}

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen(void)
{
	return;
}

/*
=============
Draw_StretchRaw
=============
*/
extern unsigned	r_rawpalette[256];

void Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte* data)
{
	return;
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette(const unsigned char* palette)
{
	return;
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginFrame(float camera_separation)
{
	return;
}

/*
** GLimp_EndFrame
**
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame(void)
{
	return;
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate(qboolean active)
{
	return;
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/

refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t	re;

	ri = rimp;

	re.api_version = API_VERSION;

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;

	re.DrawStretchRaw = Draw_StretchRaw;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	re.CinematicSetPalette = R_SetPalette;
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = GLimp_EndFrame;

	re.AppActivate = GLimp_AppActivate;

	// что это?
	Swap_Init();

	return re;
}

#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error(char* error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	ri.Sys_Error(ERR_FATAL, "%s", text);
}

void Com_Printf(char* fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	ri.Con_Printf(PRINT_ALL, "%s", text);
}

#endif