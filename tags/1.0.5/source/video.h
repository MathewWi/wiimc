/****************************************************************************
 * WiiMC
 * Tantric 2009
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <ogcsys.h>

#ifdef __cplusplus
extern "C" {
#endif

void InitVideo();
void StopGX();
void ResetVideo_Menu();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF, u8 format);
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);
int DrawMPlayerGui();

extern GXRModeObj *vmode;
extern int screenheight;
extern int screenwidth;
extern u8 * videoScreenshot;
extern u32 FrameTimer;
extern bool drawGui;

#ifdef __cplusplus
}
#endif

#endif
