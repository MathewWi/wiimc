/****************************************************************************
 * WiiMC
 * Tantric 2009-2010
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>

#include "libwiigui/gui.h"
#include "menu.h"
#include "wiimc.h"
#include "settings.h"
#include "fileop.h"
#include "input.h"
#include "networkop.h"
#include "musicplaylist.h"
#include "filebrowser.h"
#include "utils/gettext.h"
#include "utils/http.h"
#include "filelist.h"

#define THREAD_SLEEP 200
#define GSTACK (256*1024)
static u8 guistack[GSTACK] ATTRIBUTE_ALIGN (32);

extern char streamtitle[128]; // ICY data (http.c)
extern char streamurl[128]; // ICY data (http.c)

// frequently used objects

static GuiImageData * bg = NULL;
static GuiImageData * pointer[4] = { NULL, NULL, NULL, NULL };
static GuiImageData throbber(throbber_png);
static GuiImageData progressLeft(progressbar_left_png);
static GuiImageData progressMid(progressbar_mid_png);
static GuiImageData progressRight(progressbar_right_png);
static GuiImageData progressEmpty(progressbar_empty_png);
static GuiImageData progressShortEmpty(progressbar_short_empty_png);
static GuiImageData progressLongEmpty(progressbar_long_empty_png);
static GuiImageData progressLine(progressbar_line_png);
static GuiImageData volumeTop(volume_top_png);
static GuiImageData volumeMid(volume_mid_png);
static GuiImageData volumeBottom(volume_bottom_png);
static GuiImageData volumeEmpty(volume_empty_png);
static GuiImageData volumeLine(volume_line_png);
static GuiImage * disabled;
static GuiTrigger * trigA = NULL;
static GuiTrigger * trigB = NULL;
static GuiTrigger * trigLeft = NULL;
static GuiTrigger * trigRight = NULL;
static GuiTrigger * trigUp = NULL;
static GuiTrigger * trigDown = NULL;

static GuiImage * videoImg = NULL;
static GuiButton * videosBtn = NULL;
static GuiButton * musicBtn = NULL;
static GuiButton * picturesBtn = NULL;
static GuiButton * dvdBtn = NULL;
static GuiButton * onlineBtn = NULL;
static GuiButton * settingsBtn = NULL;

static GuiImage * videosBtnImg = NULL;
static GuiImage * videosBtnOnImg = NULL;
static GuiImage * musicBtnImg = NULL;
static GuiImage * musicBtnOnImg = NULL;
static GuiImage * picturesBtnImg = NULL;
static GuiImage * picturesBtnOnImg = NULL;
static GuiImage * dvdBtnImg = NULL;
static GuiImage * dvdBtnOnImg = NULL;
static GuiImage * onlineBtnImg = NULL;
static GuiImage * onlineBtnOnImg = NULL;
static GuiImage * settingsBtnImg = NULL;
static GuiImage * settingsBtnOnImg = NULL;

static GuiButton * logoBtn = NULL;
static GuiWindow * mainWindow = NULL;
static GuiText * settingText = NULL;
static GuiText * settingText2 = NULL;
static GuiText * nowPlaying = NULL;

// actionbar

static GuiText * statusText = NULL;

static GuiWindow * videobar = NULL;
static GuiWindow * audiobar = NULL;
static GuiWindow * audiobar2 = NULL;
static GuiWindow * picturebar = NULL;

static GuiImageData * actionbarLeft = NULL;
static GuiImageData * actionbarMid = NULL;
static GuiImageData * actionbarRight = NULL;
static GuiImageData * actionbarBackward = NULL;
static GuiImageData * actionbarPause = NULL;
static GuiImageData * actionbarPlay = NULL;
static GuiImageData * actionbarForward = NULL;
static GuiImageData * actionbarSingle = NULL;
static GuiImageData * actionbarContinuous = NULL;
static GuiImageData * actionbarShuffle = NULL;
static GuiImageData * actionbarLoop = NULL;
static GuiImageData * actionbarPlaylist = NULL;
static GuiImageData * actionbarClose = NULL;
static GuiImageData * actionbarVolume = NULL;

static GuiImage * videobarLeftImg = NULL;
static GuiImage * videobarMidImg = NULL;
static GuiImage * videobarRightImg = NULL;
static GuiImage * videobarProgressImg = NULL;
static GuiImage * videobarProgressLeftImg = NULL;
static GuiImage * videobarProgressMidImg = NULL;
static GuiImage * videobarProgressLineImg = NULL;
static GuiImage * videobarProgressRightImg = NULL;
static GuiImage * videobarVolumeImg = NULL;
static GuiImage * videobarVolumeLevelImg = NULL;
static GuiImage * videobarVolumeLevelTopImg = NULL;
static GuiImage * videobarVolumeLevelMidImg = NULL;
static GuiImage * videobarVolumeLevelLineImg = NULL;
static GuiImage * videobarVolumeLevelBottomImg = NULL;
static GuiImage * videobarBackwardImg = NULL;
static GuiImage * videobarPauseImg = NULL;
static GuiImage * videobarForwardImg = NULL;

static GuiTooltip * videobarVolumeTip = NULL;
static GuiTooltip * videobarBackwardTip = NULL;
static GuiTooltip * videobarPauseTip = NULL;
static GuiTooltip * videobarForwardTip = NULL;

static GuiButton * videobarProgressBtn = NULL;
static GuiButton * videobarVolumeBtn = NULL;
static GuiButton * videobarVolumeLevelBtn = NULL;
static GuiButton * videobarBackwardBtn = NULL;
static GuiButton * videobarPauseBtn = NULL;
static GuiButton * videobarForwardBtn = NULL;

static GuiText * videobarTime = NULL;

static GuiImage * audiobarLeftImg = NULL;
static GuiImage * audiobarMidImg = NULL;
static GuiImage * audiobarRightImg = NULL;
static GuiImage * audiobarProgressImg = NULL;
static GuiImage * audiobarProgressLeftImg = NULL;
static GuiImage * audiobarProgressMidImg = NULL;
static GuiImage * audiobarProgressLineImg = NULL;
static GuiImage * audiobarProgressRightImg = NULL;
static GuiImage * audiobarPlaylistImg = NULL;
static GuiImage * audiobarBackwardImg = NULL;
static GuiImage * audiobarPauseImg = NULL;
static GuiImage * audiobarForwardImg = NULL;
static GuiImage * audiobarModeImg = NULL;

static GuiTooltip * audiobarPlaylistTip = NULL;
static GuiTooltip * audiobarBackwardTip = NULL;
static GuiTooltip * audiobarPauseTip = NULL;
static GuiTooltip * audiobarForwardTip = NULL;
static GuiTooltip * audiobarModeTip = NULL;

static GuiButton * audiobarProgressBtn = NULL;
static GuiButton * audiobarPlaylistBtn = NULL;
static GuiButton * audiobarBackwardBtn = NULL;
static GuiButton * audiobarPauseBtn = NULL;
static GuiButton * audiobarForwardBtn = NULL;
static GuiButton * audiobarModeBtn = NULL;

static GuiText * audiobarNowPlaying[4] = { NULL, NULL, NULL, NULL };
static GuiButton * audiobarNowPlayingBtn = NULL;
bool nowPlayingSet = false;

static GuiImage * picturebarLeftImg = NULL;
static GuiImage * picturebarMidImg = NULL;
static GuiImage * picturebarRightImg = NULL;
static GuiImage * picturebarPreviousImg = NULL;
static GuiImage * picturebarNextImg = NULL;
static GuiImage * picturebarSlideshowImg = NULL;
static GuiImage * picturebarCloseImg = NULL;

static GuiTooltip * picturebarPreviousTip = NULL;
static GuiTooltip * picturebarNextTip = NULL;
static GuiTooltip * picturebarSlideshowTip = NULL;
static GuiTooltip * picturebarCloseTip = NULL;

static GuiButton * picturebarPreviousBtn = NULL;
static GuiButton * picturebarNextBtn = NULL;
static GuiButton * picturebarSlideshowBtn = NULL;
static GuiButton * picturebarCloseBtn = NULL;

int menuCurrent = MENU_BROWSE_VIDEOS;
static int menuPrevious = MENU_BROWSE_VIDEOS;
static int menuUndo = MENU_BROWSE_VIDEOS;
static int netEditIndex = 0; // current index of FTP/SMB share being edited

// threads
static lwp_t guithread = LWP_THREAD_NULL;
static lwp_t progressthread = LWP_THREAD_NULL;
static lwp_t creditsthread = LWP_THREAD_NULL;
static lwp_t updatethread = LWP_THREAD_NULL;
static lwp_t picturethread = LWP_THREAD_NULL;

static int progressThreadHalt = 0;
static int creditsThreadHalt = 0;
static int updateThreadHalt = 0;
static int pictureThreadHalt = 0;

static int guiHalt = 0;
static bool guiShutdown = true;
static int showProgress = 0;

static char progressTitle[100];
static char progressMsg[200];
static int progressDone = 0;
static int progressTotal = 0;

bool menuMode = 0; // 0 - normal GUI, 1 - GUI for MPlayer

static void UpdateMenuImages(int oldBtn, int newBtn)
{	
	if(oldBtn > MENU_SETTINGS)
		oldBtn = MENU_SETTINGS;
	if(newBtn > MENU_SETTINGS)
		newBtn = MENU_SETTINGS;

	if(oldBtn == newBtn)
		return; // do nothing

	switch(oldBtn)
	{
		case MENU_BROWSE_VIDEOS:
			videosBtn->SetImage(videosBtnImg); break;
		case MENU_BROWSE_MUSIC:
			musicBtn->SetImage(musicBtnImg); break;
		case MENU_BROWSE_PICTURES:
			picturesBtn->SetImage(picturesBtnImg); break;
		case MENU_DVD:
			dvdBtn->SetImage(dvdBtnImg); break;
		case MENU_BROWSE_ONLINEMEDIA:
			onlineBtn->SetImage(onlineBtnImg); break;
		case MENU_SETTINGS:
			settingsBtn->SetImage(settingsBtnImg); break;
	}

	switch(newBtn)
	{
		case MENU_BROWSE_VIDEOS:
			videosBtn->SetImage(videosBtnOnImg); break;
		case MENU_BROWSE_MUSIC:
			musicBtn->SetImage(musicBtnOnImg); break;
		case MENU_BROWSE_PICTURES:
			picturesBtn->SetImage(picturesBtnOnImg); break;
		case MENU_DVD:
			dvdBtn->SetImage(dvdBtnOnImg); break;
		case MENU_BROWSE_ONLINEMEDIA:
			onlineBtn->SetImage(onlineBtnOnImg); break;
		case MENU_SETTINGS:
			settingsBtn->SetImage(settingsBtnOnImg); break;
	}
}

static void ChangeMenuNoHistory(int menu)
{
	if(menu == menuCurrent)
		return;

	UpdateMenuImages(menuCurrent, menu);
	menuCurrent = menu;
}

static void ChangeMenu(int menu)
{
	if(menu == menuCurrent)
		return;

	menuUndo = menuPrevious;
	menuPrevious = menuCurrent;
	menuCurrent = menu;
	UpdateMenuImages(menuPrevious, menuCurrent);
}

void UndoChangeMenu()
{
	UpdateMenuImages(menuCurrent, menuPrevious);
	menuCurrent = menuPrevious;
	menuPrevious = menuUndo;
}

static void ChangeMenu(void * ptr, int menu)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		ChangeMenu(menu);
		b->ResetState();
	}
}

static void ChangeMenuVideos(void * ptr) { ChangeMenu(ptr, MENU_BROWSE_VIDEOS); }
static void ChangeMenuMusic(void * ptr) { ChangeMenu(ptr, MENU_BROWSE_MUSIC); }
static void ChangeMenuPictures(void * ptr) { ChangeMenu(ptr, MENU_BROWSE_PICTURES); }
static void ChangeMenuDVD(void * ptr) { ChangeMenu(ptr, MENU_DVD); }
static void ChangeMenuOnline(void * ptr) { ChangeMenu(ptr, MENU_BROWSE_ONLINEMEDIA); }
static void ChangeMenuSettings(void * ptr) { ChangeMenu(ptr, MENU_SETTINGS); }

/****************************************************************************
 * UpdateThread
 *
 * Prompts for confirmation, and downloads/installs updates
 ***************************************************************************/
static void * UpdateThread (void *arg)
{
	while(1)
	{
		LWP_SuspendThread(updatethread);

		if(updateThreadHalt == 2)
			return NULL;

		bool installUpdate = WindowPrompt(
			"Update Available",
			"An update is available!",
			"Update now",
			"Update later");
		if(installUpdate)
			if(DownloadUpdate())
				ExitRequested = 1;
	}
	return NULL;
}

static void ResumeUpdateThread()
{
	if(updatethread == LWP_THREAD_NULL || guiShutdown)
		return;

	updateThreadHalt = 0;
	LWP_ResumeThread(updatethread);
}

static bool videoPaused;

extern "C" void DoMPlayerGuiDraw()
{
	if(menuMode != 1)
		return;

	mainWindow->Draw();
	mainWindow->DrawTooltip();

	int i = 3;
	do
	{
		if(userInput[i].wpad->ir.valid)
			Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
				96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
		DoRumble(i);
		--i;
	} while(i>=0);

	mainWindow->Update(&userInput[3]);
	mainWindow->Update(&userInput[2]);
	mainWindow->Update(&userInput[1]);
	mainWindow->Update(&userInput[0]);

	if(mainWindow->IsVisible() && wiiInDVDMenu())
	{
		mainWindow->SetVisible(false);
		mainWindow->SetState(STATE_DISABLED);
	}
	else if(!mainWindow->IsVisible() && !wiiInDVDMenu())
	{
		mainWindow->SetVisible(true);
		mainWindow->SetState(STATE_DEFAULT);
		HideVolumeLevelBar();
		statusText->SetVisible(false);
	}

	if(videoPaused != wiiIsPaused())
	{
		videoPaused = !videoPaused;
		if(videoPaused)
		{
			videobarPauseImg->SetImage(actionbarPlay);
			videobarPauseTip->SetText("Play");
		}
		else
		{
			videobarPauseImg->SetImage(actionbarPause);
			videobarPauseTip->SetText("Pause");
		}
	}
}

/****************************************************************************
 * GuiThread
 *
 * Primary GUI thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
static void *GuiThread (void *arg)
{
	int i;

	while(1)
	{
		if(guiHalt == 1)
			LWP_SuspendThread(guithread);

		UpdatePads();

		mainWindow->Draw();

		if (mainWindow->GetState() != STATE_DISABLED)
			mainWindow->DrawTooltip();

		i = 3;
		do
		{
			if(userInput[i].wpad->ir.valid)
				Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
					96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255, GX_TF_RGBA8);
			DoRumble(i);
			--i;
		} while(i>=0);

		mainWindow->Update(&userInput[3]);
		mainWindow->Update(&userInput[2]);
		mainWindow->Update(&userInput[1]);
		mainWindow->Update(&userInput[0]);

		for(i=3; i >= 0; i--)
		{
			if(userInput[i].wpad->btns_d & (WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_X))
			{
				int newMenu = menuCurrent + 1;
				if(newMenu > MENU_SETTINGS)
					newMenu = MENU_BROWSE_VIDEOS;
				ChangeMenu(newMenu);
			}
			else if(userInput[i].wpad->btns_d & (WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_Y))
			{
				int newMenu = menuCurrent - 1;
				if(newMenu < MENU_BROWSE_VIDEOS)
					newMenu = MENU_SETTINGS;
				ChangeMenu(newMenu);
			}
		}

		Menu_Render();

		if(updateFound)
		{
			updateFound = false;
			ResumeUpdateThread();
		}

		if((userInput[0].wpad->btns_d & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) && 
			controlledbygui == 1 && !inNetworkInit)
		{
			ExitRequested = 1; // exit program
		}

		if(ExitRequested || ShutdownRequested)
		{
			for(i = 0; i <= 255; i += 15)
			{
				mainWindow->Draw();
				Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, i},1);
				Menu_Render();
			}
			ExitApp();
		}
		usleep(THREAD_SLEEP);
	}
	return NULL;
}

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
static void ResumeGui()
{
	if(guithread == LWP_THREAD_NULL || guiShutdown)
		return;

	guiHalt = 0;
	LWP_ResumeThread (guithread);
}

/****************************************************************************
 * SuspendGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
static void SuspendGui()
{
	if(guithread == LWP_THREAD_NULL)
		return;

	guiHalt = 1;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

extern "C" void ShutdownGui()
{
	if(menuMode == 1) // prevent MPlayer from shutting down OSD
		return;

	guiShutdown = true;
	CancelAction();
	SuspendGui();
}

static void ResetText()
{
	LoadLanguage();

	videobar->ResetText();
	audiobar->ResetText();
	picturebar->ResetText();

	if(mainWindow)
		mainWindow->ResetText();
}

static int currentFont = FONT_STANDARD;
static u8 *ext_font_ttf = NULL;

void ChangeLanguage()
{
	if(WiiSettings.language == LANG_JAPANESE ||
		WiiSettings.language == LANG_SIMP_CHINESE ||
		WiiSettings.language == LANG_TRAD_CHINESE ||
		WiiSettings.language == LANG_KOREAN)
	{
		char filepath[MAXPATHLEN];
		char httppath[MAXPATHLEN];
		char httpRoot[] = "http://wiimc.googlecode.com/svn/trunk/fonts";
		int newFont;

		switch(WiiSettings.language)
		{
			case LANG_SIMP_CHINESE:			
				if(currentFont == FONT_SIMP_CHINESE) return;
				sprintf(filepath, "%s/zh_cn.ttf", appPath);
				sprintf(httppath, "%s/zh_cn.ttf", httpRoot);
				newFont = FONT_SIMP_CHINESE;
				break;
			case LANG_TRAD_CHINESE:
				if(currentFont == FONT_TRAD_CHINESE) return;
				sprintf(filepath, "%s/zh_cn.ttf", appPath);
				sprintf(httppath, "%s/zh_cn.ttf", httpRoot);
				newFont = FONT_TRAD_CHINESE;
				break;
			case LANG_KOREAN:
				if(currentFont == FONT_KOREAN) return;
				sprintf(filepath, "%s/ko.ttf", appPath);
				sprintf(httppath, "%s/ko.ttf", httpRoot);
				newFont = FONT_KOREAN;
				break;
			case LANG_JAPANESE:
				if(currentFont == FONT_JAPANESE) return;
				sprintf(filepath, "%s/jp.ttf", appPath);
				sprintf(httppath, "%s/jp.ttf", httpRoot);
				newFont = FONT_JAPANESE;
				break;
		}

		// try to load font
restart:
		FILE * file = fopen (filepath, "rb");

		if(file)
		{
			fseeko(file,0,SEEK_END);
			int loadSize = ftello(file);
			fseeko(file,0,SEEK_SET);
			if(ext_font_ttf)
			{
				SuspendGui();
				free(ext_font_ttf);
			}
			ext_font_ttf = (u8 *)memalign(32, loadSize);
			fread (ext_font_ttf, 1, loadSize, file);
			fclose(file);

			if(ext_font_ttf)
			{
				SuspendGui();
				DeinitFreeType();
				InitFreeType(ext_font_ttf, loadSize);
				currentFont = newFont;
				ResetText();
				ResumeGui();
				return;
			}
			else
			{
				ErrorPrompt("Could not change language. Not enough memory!");
			}
		}
		else
		{
			bool installFont = WindowPrompt(
				"Font Required",
				gettext("A new font is required to display this language."),
				"Download font",
				"Cancel");

			if(installFont)
			{
				FILE * hfile = fopen (filepath, "wb");

				if (hfile > 0)
				{
					http_request(httppath, hfile, NULL, 1024*1024*2, NOTSILENT);
					fclose (hfile);
					goto restart;
				}
				else
				{
					ErrorPrompt("Unable to save font file!");
				}
			}
		}
		WiiSettings.language = LANG_ENGLISH;
	}

	if(currentFont != FONT_STANDARD)
	{
		SuspendGui();
		DeinitFreeType();
		if(ext_font_ttf) free(ext_font_ttf);
		ext_font_ttf = NULL;
		InitFreeType((u8*)font_ttf, font_ttf_size);
		currentFont = FONT_STANDARD;
		ResetText();
		ResumeGui();
	}
	else
	{
		ResetText();
	}
}

void DisableMainWindow()
{
	SuspendGui();
	if(!mainWindow->Find(disabled))
		mainWindow->Append(disabled);

	mainWindow->SetState(STATE_DISABLED);
	ResumeGui();
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int
WindowPrompt(const char *title, wchar_t *msg, const char *btn1Label, const char *btn2Label)
{
	int choice = -1;

	GuiWindow promptWindow(556,244);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetAlpha(220);

	GuiText titleTxt(title, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,18);
	GuiText msgTxt(NULL, 20, (GXColor){255, 255, 255, 255});
	msgTxt.SetWText(msg);
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-20);
	msgTxt.SetWrap(true, 430);

	GuiText btn1Txt(btn1Label, 20, (GXColor){255, 255, 255, 255});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if(btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(20, -40);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -40);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetImageOver(&btn1ImgOver);
	btn1.SetTrigger(trigA);
	btn1.SetSelectable(false);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 20, (GXColor){255, 255, 255, 255});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -40);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetTrigger(trigA);
	btn2.SetSelectable(false);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	if(btn2Label)
		promptWindow.Append(&btn2);

	promptWindow.SetEffect(EFFECT_FADE, 50);
	CancelAction();

	bool isDisabled = mainWindow->Find(disabled);

	SuspendGui();
	mainWindow->SetState(STATE_DISABLED);
	if(!isDisabled)
		mainWindow->Append(disabled);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	while(choice == -1)
	{
		usleep(THREAD_SLEEP);

		if(btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if(btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	promptWindow.SetEffect(EFFECT_FADE, -50);
	while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
	SuspendGui();
	mainWindow->Remove(&promptWindow);
	if(!isDisabled)
		mainWindow->Remove(disabled);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return choice;
}

int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
	wchar_t *tempmsg = charToWideChar(gettext(msg));
	int res = WindowPrompt(title, tempmsg, btn1Label, btn2Label);
	delete[] tempmsg;
	return res;
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
static int progsleep = 0;

static void
ProgressWindow(char *title, char *msg)
{
	GuiWindow promptWindow(556,244);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetAlpha(220);

	GuiImage progressEmptyImg(&progressEmpty);
	progressEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressEmptyImg.SetPosition(128, 40);
	
	GuiImage progressLeftImg(&progressLeft);
	progressLeftImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressLeftImg.SetPosition(128, 40);
	progressLeftImg.SetVisible(false);
	
	GuiImage progressMidImg(&progressMid);
	progressMidImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressMidImg.SetPosition(136, 40);
	progressMidImg.SetTile(0);

	GuiImage progressLineImg(&progressLine);
	progressLineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressLineImg.SetPosition(136, 40);
	progressLineImg.SetVisible(false);

	GuiImage progressRightImg(&progressRight);
	progressRightImg.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	progressRightImg.SetPosition(-128, 40);
	progressRightImg.SetVisible(false);

	GuiImage throbberImg(&throbber);
	throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg.SetPosition(0, 40);

	GuiText titleTxt(title, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,18);
	GuiText msgTxt(msg, 20, (GXColor){255, 255, 255, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,80);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	if(showProgress == 1)
	{
		promptWindow.Append(&progressEmptyImg);
		promptWindow.Append(&progressLeftImg);
		promptWindow.Append(&progressMidImg);
		promptWindow.Append(&progressLineImg);
		promptWindow.Append(&progressRightImg);
	}
	else
	{
		promptWindow.Append(&throbberImg);
	}
	
	// wait to see if progress flag changes soon
	progsleep = 400000;

	while(progsleep > 0)
	{
		if(!showProgress)
			break;
		usleep(THREAD_SLEEP);
		progsleep -= THREAD_SLEEP;
	}

	if(!showProgress || progressThreadHalt > 0)
		return;

	SuspendGui();
	int oldState = mainWindow->GetState();
	bool isDisabled = mainWindow->Find(disabled);
	mainWindow->SetState(STATE_DISABLED);
	if(!isDisabled)
		mainWindow->Append(disabled);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	float angle = 0;
	u32 count = 0;
	float done = 0;
	int tile = 0;

	while(showProgress && progressThreadHalt == 0)
	{
		progsleep = 20000;

		while(progsleep > 0)
		{
			if(!showProgress)
				break;
			usleep(THREAD_SLEEP);
			progsleep -= THREAD_SLEEP;
		}

		if(showProgress == 1)
		{
			done = progressDone/(float)progressTotal;

			if(done > 0.02)
			{
				progressLeftImg.SetVisible(true);
				tile = 73*(done-0.02);
				if(tile > 71) tile = 71;
				progressMidImg.SetTile(tile);
				progressLineImg.SetPosition(136 + tile*4, 40);
				progressLineImg.SetVisible(true);
			}
			
			if(tile == 71)
			{
				progressLineImg.SetVisible(false);
				progressRightImg.SetVisible(true);
			}
		}
		else if(showProgress == 2)
		{
			if(count % 5 == 0)
			{
				angle+=45.0f;
				if(angle >= 360.0f)
					angle = 0;
				throbberImg.SetAngle(angle);
			}
			++count;
		}
	}

	SuspendGui();
	mainWindow->Remove(&promptWindow);
	if(!isDisabled)
		mainWindow->Remove(disabled);
	mainWindow->SetState(oldState);
	ResumeGui();
}

static void * ProgressThread (void *arg)
{
	while(1)
	{
		if(progressThreadHalt == 1)
			LWP_SuspendThread (progressthread);
		if(progressThreadHalt == 2)
			return NULL;

		ProgressWindow(progressTitle, progressMsg);
		usleep(THREAD_SLEEP);
	}
	return NULL;
}

/****************************************************************************
 * CancelAction
 *
 * Signals the GUI progress window thread to halt, and waits for it to
 * finish. Prevents multiple progress window events from interfering /
 * overriding each other.
 ***************************************************************************/
void
CancelAction()
{
	progressThreadHalt = 1;
	showProgress = 0;

	if(progressthread == LWP_THREAD_NULL)
		return;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(progressthread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar. Also resumes the progress window thread if it is suspended.
 ***************************************************************************/
extern "C" {
void
ShowProgress (const char *msg, int done, int total)
{
	if(progressthread == LWP_THREAD_NULL || guiShutdown)
		return;

	if(!mainWindow || ExitRequested || ShutdownRequested)
		return;

	if(total <= 0 || done < 0) // invalid values
		return;

	if(done > total) // this shouldn't happen
		done = total;

	if(showProgress != 1)
		CancelAction(); // wait for previous progress window to finish

	snprintf(progressMsg, 200, "%s", gettext(msg));
	sprintf(progressTitle, "Please Wait");
	progressThreadHalt = 0;
	showProgress = 1;
	progressTotal = total;
	progressDone = done;

	LWP_ResumeThread (progressthread);
}
}
/****************************************************************************
 * ShowAction
 *
 * Shows that an action is underway. Also resumes the progress window thread
 * if it is suspended.
 ***************************************************************************/
void
ShowAction (const char *msg)
{
	if(!mainWindow || ExitRequested || ShutdownRequested)
		return;

	if(progressthread == LWP_THREAD_NULL || guiShutdown)
		return;

	if(showProgress != 2)
		CancelAction(); // wait for previous progress window to finish

	snprintf(progressMsg, 200, "%s", gettext(msg));
	sprintf(progressTitle, "Please Wait");
	progressThreadHalt = 0;
	showProgress = 2;
	progressDone = 0;
	progressTotal = 0;

	LWP_ResumeThread (progressthread);
}

void ErrorPrompt(const char *msg)
{
	WindowPrompt("Error", gettext(msg), "OK", NULL);
}

int ErrorPromptRetry(const char *msg)
{
	return WindowPrompt("Error", gettext(msg), "Retry", "Cancel");
}

void ErrorPrompt(wchar_t *msg)
{
	WindowPrompt("Error", msg, "OK", NULL);
}

int ErrorPromptRetry(wchar_t *msg)
{
	return WindowPrompt("Error", msg, "Retry", "Cancel");
}

void InfoPrompt(const char *msg)
{
	WindowPrompt("Information", gettext(msg), "OK", NULL);
}

void InfoPrompt(const char *title, const char *msg)
{
	WindowPrompt(title, gettext(msg), "OK", NULL);
}

void InfoPrompt(wchar_t *msg)
{
	WindowPrompt("Information", msg, "OK", NULL);
}

void InfoPrompt(const char *title, wchar_t *msg)
{
	WindowPrompt(title, msg, "OK", NULL);
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
void OnScreenKeyboard(char * var, u32 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText okBtnTxt("OK", 20, (GXColor){255, 255, 255, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(trigA);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 20, (GXColor){255, 255, 255, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	SuspendGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(disabled);
	mainWindow->Append(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen+1, "%s", keyboard.kbtextstr);
	}

	SuspendGui();
	mainWindow->Remove(&keyboard);
	mainWindow->Remove(disabled);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

/****************************************************************************
 * SettingWindow
 *
 * Opens a new window, with the specified window element appended. Allows
 * for a customizable prompted setting.
 ***************************************************************************/
static int
SettingWindow(const char * title, GuiWindow * w)
{
	int save = -1;

	GuiWindow promptWindow(556,244);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetAlpha(220);

	GuiText titleTxt(title, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,18);

	GuiText okBtnTxt("OK", 20, (GXColor){255, 255, 255, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(20, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(trigA);
	okBtn.SetSelectable(false);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 20, (GXColor){255, 255, 255, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-20, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetSelectable(false);
	cancelBtn.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&okBtn);
	promptWindow.Append(&cancelBtn);

	SuspendGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(disabled);
	mainWindow->Append(&promptWindow);
	mainWindow->Append(w);
	mainWindow->ChangeFocus(w);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}
	SuspendGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->Remove(w);
	mainWindow->Remove(disabled);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

/****************************************************************************
 * CreditsWindow
 * Display credits, legal copyright and licence
 *
 * THIS MUST NOT BE REMOVED OR DISABLED IN ANY DERIVATIVE WORK
 ***************************************************************************/

static void CreditsWindow()
{
	int i = 0;
	int y = 76;

	GuiWindow * oldWindow = mainWindow;
	GuiWindow creditsWindow(screenwidth, screenheight);

	GuiImage bgImg(bg);
	creditsWindow.Append(&bgImg);
	
	GuiImageData logo(logo_png);
	GuiImage logoImg(&logo);
	logoImg.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	logoImg.SetPosition(0, 30);
	
	creditsWindow.Append(&logoImg);

	GuiWindow alignWindow(0, screenheight);
	alignWindow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	creditsWindow.Append(&alignWindow);

	int numEntries = 15;
	GuiText * txt[numEntries];

	char iosVersion[10];
	sprintf(iosVersion, "IOS: %d", IOS_GetVersion());

	wchar_t appVersion[20];
	swprintf(appVersion, 20, L"%s %s", gettext("Version"), APPVERSION);

	txt[i] = new GuiText(iosVersion, 16, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition((screenwidth/2)-30,30); i++;
	txt[i] = new GuiText(NULL, 16, (GXColor){255, 255, 255, 255});
	txt[i]->SetWText(appVersion);
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition((screenwidth/2)-30,56); i++;

	txt[i] = new GuiText("www.wiimc.org", 16, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	txt[i]->SetPosition(0,y); i++; y+=50;

	txt[i] = new GuiText("Coding & menu design", 20, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition(-15,y); i++;
	txt[i] = new GuiText("Tantric", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=26;

	txt[i] = new GuiText("Coding", 20, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition(-15,y); i++;
	txt[i] = new GuiText("rodries", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=26;
	
	txt[i] = new GuiText("Menu artwork", 20, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition(-15,y); i++;
	txt[i] = new GuiText("Sam Mularczyk", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=26;
	
	txt[i] = new GuiText("Logo", 20, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	txt[i]->SetPosition(-15,y); i++;
	txt[i] = new GuiText("Psyche & drmr", 20, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=50;

	txt[i] = new GuiText("Thanks to", 20, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	txt[i]->SetPosition(0,y); i++; y+=36;
	
	GuiImageData foundmy(foundmy_png);
	GuiImage foundmyImg(&foundmy);
	foundmyImg.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	foundmyImg.SetPosition(-15, y);
	alignWindow.Append(&foundmyImg);

	txt[i] = new GuiText("MPlayer Team", 16, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=20;

	txt[i] = new GuiText("shagkur & wintermute (libogc / devkitPPC)", 16, (GXColor){255, 255, 255, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	txt[i]->SetPosition(15,y); i++; y+=44;

	txt[i] = new GuiText("This software is open source and may be copied, distributed, or modified under the terms of the GNU General Public License (GPL) Version 2.", 14, (GXColor){160, 160, 160, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	txt[i]->SetPosition(0,y);
	txt[i]->SetWrap(true, 500);

	for(i=0; i < numEntries; i++)
		alignWindow.Append(txt[i]);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);
	
	creditsWindow.Append(&backBtn);

	SuspendGui();
	mainWindow = &creditsWindow;
	ResumeGui();
	
	while(creditsThreadHalt == 0)
	{
		usleep(THREAD_SLEEP);
		
		if(backBtn.GetState() == STATE_CLICKED)
			break;
	}

	SuspendGui();
	mainWindow = oldWindow;
	ResumeGui();
	
	for(i=0; i < numEntries; i++)
		delete txt[i];
}

static void *CreditsThread(void *arg)
{
	while(1)
	{
		LWP_SuspendThread (creditsthread);

		if(creditsThreadHalt == 2)
			break;

		CreditsWindow();
		usleep(THREAD_SLEEP);
	}
	return NULL;
}

static void ResumeCreditsThread()
{
	if(creditsthread == LWP_THREAD_NULL || guiShutdown)
		return;

	creditsThreadHalt = 0;
	LWP_ResumeThread(creditsthread);
}

static void DisplayCredits(void * ptr)
{
	if(logoBtn->GetState() != STATE_CLICKED)
		return;

	logoBtn->ResetState();
	ResumeCreditsThread();
}

static void UpdateAudiobarModeBtn()
{
	switch(WiiSettings.playOrder)
	{
		case PLAY_SINGLE:
			audiobarModeImg->SetImage(actionbarSingle);
			audiobarModeTip->SetText("Single Play");
			break;
		case PLAY_CONTINUOUS:
			audiobarModeImg->SetImage(actionbarContinuous);
			audiobarModeTip->SetText("Continuous Play");
			break;
		case PLAY_SHUFFLE:
			audiobarModeImg->SetImage(actionbarShuffle);
			audiobarModeTip->SetText("Shuffle");
			break;
		case PLAY_LOOP:
			audiobarModeImg->SetImage(actionbarLoop);
			audiobarModeTip->SetText("Loop");
			break;
	}
}

void RemoveVideoImg()
{
	if(!videoImg)
		return;

	SuspendGui();
	mainWindow->Remove(videoImg);
	ResumeGui();
	delete videoImg;
	videoImg = NULL;
	free(videoScreenshot);
	videoScreenshot = NULL;
}

/****************************************************************************
 * MenuBrowse
 ***************************************************************************/

static int LoadNewFile()
{
	RemoveVideoImg();

	LoadMPlayerFile();

	// wait until MPlayer is ready to take control (or return control)
	while(!guiShutdown && controlledbygui != 1)
		usleep(THREAD_SLEEP);

	if(guiShutdown)
	{
		playlistIndex = -1;
		return 1; // playing a video
	}

	// failed or we are playing audio
	ResumeDeviceThread();

	if(!wiiAudioOnly())
	{
		playlistIndex = -1;
		ErrorPrompt("Error loading file!");
		return 0;
	}

	// we are playing audio
	wiiSetVolume(WiiSettings.volume);

	if(wiiIsPaused())
		wiiPause(); // unpause playback

	// update the audio bar
	if(wiiGetTimeLength() > 0)
	{
		audiobarProgressBtn->SetVisible(true);
		audiobarProgressBtn->SetState(STATE_DEFAULT);
		audiobarProgressMidImg->SetVisible(true);
	}
	else // this is a stream - hide progress bar
	{
		audiobarProgressBtn->SetVisible(false);
		audiobarProgressBtn->SetState(STATE_DISABLED);
		audiobarProgressLeftImg->SetVisible(false);
		audiobarProgressMidImg->SetVisible(false);
		audiobarProgressLineImg->SetVisible(false);
		audiobarProgressRightImg->SetVisible(false);
	}
	return 2; // playing audio
}

static void MenuBrowse(int menu)
{
	ShutoffRumble();
	ResetBrowser();

	switch(menu)
	{
		case MENU_BROWSE_VIDEOS:
			browser.dir = &WiiSettings.videosFolder[0]; break;
		case MENU_BROWSE_MUSIC:
			browser.dir = &WiiSettings.musicFolder[0]; break;
		case MENU_BROWSE_ONLINEMEDIA:
			browser.dir = &WiiSettings.onlinemediaFolder[0]; break;
		default:
			return;
	}

	GuiTrigger trigPlus;
	trigPlus.SetButtonOnlyTrigger(-1, WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS, PAD_BUTTON_X);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiButton upOneLevelBtn(0,0);
	upOneLevelBtn.SetTrigger(trigB);
	upOneLevelBtn.SetSelectable(false);
	
	GuiText backBtnTxt("Resume", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	backBtnTxt.SetPosition(-74, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	backBtnArrow.SetPosition(-54, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);

	int paused = wiiIsPaused(); // for audiobar Play/Pause button

	if(paused)
	{
		audiobarPauseImg->SetImage(actionbarPlay);
		audiobarPauseTip->SetText("Play");
	}
	else
	{
		audiobarPauseImg->SetImage(actionbarPause);
		audiobarPauseTip->SetText("Pause");
	}

	int pagesize = 11;

	if(videoImg && menu != MENU_BROWSE_MUSIC)
		pagesize = 10;
	else if(menu == MENU_BROWSE_MUSIC || (menu == MENU_BROWSE_ONLINEMEDIA && wiiAudioOnly()))
		pagesize = 8;

	GuiFileBrowser fileBrowser(screenwidth, pagesize);
	fileBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	fileBrowser.SetPosition(0, 90);

	GuiButton playlistAddBtn(0, 0);
	playlistAddBtn.SetTrigger(&trigPlus);
	playlistAddBtn.SetSelectable(false);

	if(menu == MENU_BROWSE_ONLINEMEDIA && onlinemediaSize == 0)
	{
		ErrorPrompt("Online media file not found.");
		UndoChangeMenu();
		goto done;
	}

	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&upOneLevelBtn);

	if(videoScreenshot && menu != MENU_BROWSE_MUSIC) // a video is loaded
	{
		if(!nowPlaying)
		{
			nowPlaying = new GuiText(loadedFileDisplay, 18, (GXColor){255, 255, 255, 255});
			nowPlaying->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
			nowPlaying->SetPosition(30, 10);
			nowPlaying->SetMaxWidth(screenwidth-180);
		}

		backBtn.SetLabel(nowPlaying, 1);
		mainWindow->Append(&backBtn);
	}
	ResumeGui();

	// populate initial directory listing
	if(strncmp(browser.dir, "http:", 5) == 0)
	{
		mainWindow->Append(disabled);
		mainWindow->SetState(STATE_DISABLED);
		ShowAction("Loading...");
	}

	BrowserChangeFolder(false);

	if(mainWindow->Find(disabled))
	{
		CancelAction();
		mainWindow->Remove(disabled);
		mainWindow->SetState(STATE_DEFAULT);
	}

	if(menu == MENU_BROWSE_MUSIC || menu == MENU_BROWSE_ONLINEMEDIA)
	{
		SuspendGui();
		audiobar->SetState(STATE_DEFAULT);

		if(menu == MENU_BROWSE_MUSIC) // add playlist functionality
		{
			audiobarPlaylistBtn->SetVisible(true);
			audiobarModeBtn->SetVisible(true);
			audiobarBackwardBtn->SetVisible(true);
			audiobarForwardBtn->SetVisible(true);
			mainWindow->Append(&playlistAddBtn);
			UpdateAudiobarModeBtn();
			mainWindow->Append(audiobar);
		}
		else // hide playlist functionality for online media area
		{
			audiobarPlaylistBtn->SetVisible(false);
			audiobarModeBtn->SetVisible(false);
			audiobarBackwardBtn->SetVisible(false);
			audiobarForwardBtn->SetVisible(false);
			audiobarPlaylistBtn->SetState(STATE_DISABLED);
			audiobarModeBtn->SetState(STATE_DISABLED);
			audiobarBackwardBtn->SetState(STATE_DISABLED);
			audiobarForwardBtn->SetState(STATE_DISABLED);

			if(wiiAudioOnly())
				mainWindow->Append(audiobar);
		}
		
		if(wiiGetTimeLength() == 0)
		{
			audiobarProgressBtn->SetVisible(false);
			audiobarProgressBtn->SetState(STATE_DISABLED);
			audiobarProgressLeftImg->SetVisible(false);
			audiobarProgressMidImg->SetVisible(false);
		}
		ResumeGui();
	}

	while(menuCurrent == menu && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		if(findLoadedFile == 2)
		{
			findLoadedFile = 0;
			fileBrowser.TriggerUpdate();
		}

		// devices were inserted or removed - update the filebrowser!
		if(devicesChanged)
		{
			devicesChanged = false;

			// video is no longer loaded - remove back button
			if(pagesize == 10 && !videoImg)
			{
				pagesize = 11;
				SuspendGui();
				fileBrowser.ChangeSize(pagesize);
				mainWindow->Remove(&backBtn);
				ResumeGui();
			}

			if(BrowserChangeFolder(false))
			{
				fileBrowser.ResetState();
				fileBrowser.fileList[0]->SetState(STATE_SELECTED);
				fileBrowser.TriggerUpdate();
			}
			else
			{
				goto done;
			}
		}

		// up one level
		if(upOneLevelBtn.GetState() == STATE_CLICKED)
		{
			upOneLevelBtn.ResetState();

			if(browser.dir[0] != 0)
			{
				browser.selIndex = 0;
	
				if(!BrowserChangeFolder())
					goto done;
	
				fileBrowser.ResetState();
				fileBrowser.fileList[0]->SetState(STATE_SELECTED);
				fileBrowser.TriggerUpdate();
			}
		}

		// update file browser based on arrow buttons
		// request guiShutdown if A button pressed on a file
		for(int i=0; i<pagesize; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();

				// check corresponding browser entry
				if(browserList[browser.selIndex].type == TYPE_FOLDER)
				{
					if(BrowserChangeFolder())
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();
						continue;
					}
					else
					{
						goto done;
					}
				}

				// this is a file
				char *ext = GetExt(browserList[browser.selIndex].filename);
				int numItems = 0;

				if(!IsAllowedExt(ext)) // unrecognized audio or video extension
				{
					// parse as a playlist
					if(strncmp(browserList[browser.selIndex].filename, "http:", 5) == 0 &&
						browserList[browser.selIndex].type != TYPE_SEARCH)
					{
						mainWindow->Append(disabled);
						mainWindow->SetState(STATE_DISABLED);
						ShowAction("Loading...");
					}

					numItems = BrowserChangeFolder();

					if(numItems > 1)
					{
						ext = GetExt(browserList[1].filename);
						if(numItems == 2 && !IsPlaylistExt(ext)) // let's load this one file
						{
							sprintf(loadedFile, browserList[1].filename);
							snprintf(loadedFileDisplay, 128, "%s", browserList[1].displayname);
							// go up one level
							browser.selIndex = 0;
							BrowserChangeFolder();
						}
						else
						{
							CancelAction();
							mainWindow->Remove(disabled);
							mainWindow->SetState(STATE_DEFAULT);

							fileBrowser.ResetState();
							fileBrowser.fileList[0]->SetState(STATE_SELECTED);
							fileBrowser.TriggerUpdate();
							continue;
						}
					}
					else if(browserList[browser.selIndex].type != TYPE_FILE)
					{
						CancelAction();
						mainWindow->Remove(disabled);
						mainWindow->SetState(STATE_DEFAULT);
						continue;
					}
				}

				if(numItems == 0)
				{
					GetFullPath(browser.selIndex, loadedFile);
					snprintf(loadedFileDisplay, 128, "%s", browserList[browser.selIndex].displayname);
				}

				playlistIndex = MusicPlaylistFindIndex(loadedFile);

				if(!mainWindow->Find(disabled))
					mainWindow->Append(disabled);
				mainWindow->SetState(STATE_DISABLED);
				ShowAction("Loading...");

				int res = LoadNewFile();
				CancelAction();

				if(res == 1) // loaded a video file
					goto done;

				mainWindow->Remove(disabled);
				mainWindow->SetState(STATE_DEFAULT);

				if(res == 2) // loaded an audio-only file
				{
					FindFile();

					// re-adjust for audio bar, if necessary
					if(pagesize != 8)
					{
						pagesize = 8;
						SuspendGui();
						fileBrowser.ChangeSize(pagesize);
						mainWindow->Remove(&backBtn);
						mainWindow->Append(audiobar);
						ResumeGui();
						break;
					}
				}
			}
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			backBtn.ResetState();
			ResumeMPlayerFile(); // go back to MPlayer
			
			// wait until MPlayer is ready to take control (or return control)
			while(!guiShutdown && controlledbygui != 1)
				usleep(THREAD_SLEEP);
		}

		if(playlistAddBtn.GetState() == STATE_CLICKED)
		{
			playlistAddBtn.ResetState();
			int addIndex = browser.selIndex;

			if(addIndex > 0)
			{
				if(browserList[addIndex].icon == ICON_FILE_CHECKED || 
					browserList[addIndex].icon == ICON_FOLDER_CHECKED)
					MusicPlaylistDequeue(addIndex);
				else
					MusicPlaylistEnqueue(addIndex);

				fileBrowser.TriggerUpdate();
			}
		}

		if(!mainWindow->Find(audiobar))
			continue; // updating audio bar elements is not required

		if(audiobarPauseBtn->GetState() == STATE_CLICKED)
		{
			audiobarPauseBtn->ResetState();

			if(wiiAudioOnly())
			{
				wiiPause();
			}
			else if(playlistSize > 0)
			{
				// start playlist
				if(!mainWindow->Find(disabled))
					mainWindow->Append(disabled);
				mainWindow->SetState(STATE_DISABLED);
				ShowAction("Loading...");

				FindNextFile(false);
				LoadNewFile();

				CancelAction();
				mainWindow->Remove(disabled);
				mainWindow->SetState(STATE_DEFAULT);
			}
		}

		if(paused != wiiIsPaused())
		{
			paused = !paused;
			if(paused)
			{
				audiobarPauseImg->SetImage(actionbarPlay);
				audiobarPauseTip->SetText("Play");
			}
			else
			{
				audiobarPauseImg->SetImage(actionbarPause);
				audiobarPauseTip->SetText("Pause");
			}
		}

		if(wiiAudioOnly() || (playlistSize > 0 && menu == MENU_BROWSE_MUSIC))
		{
			if(audiobarPauseBtn->GetAlpha() == 128 || audiobarPauseBtn->GetState() == STATE_DISABLED)
			{
				audiobarPauseBtn->SetState(STATE_DEFAULT);
				audiobarPauseBtn->SetAlpha(255);
			}
		}
		else
		{		
			if(audiobarPauseBtn->GetAlpha() == 255 || audiobarPauseBtn->GetState() != STATE_DISABLED)
			{
				audiobarPauseBtn->SetState(STATE_DISABLED);
				audiobarPauseBtn->SetAlpha(128);
			}
		}
		
		if(wiiAudioOnly())
		{
			if(wiiGetTimeLength() > 0)
			{
				if(audiobarProgressBtn->GetAlpha() == 128)
				{
					audiobarProgressBtn->SetState(STATE_DEFAULT);
					audiobarProgressBtn->SetAlpha(255);
				}
			}
			else
			{
				if(audiobarProgressBtn->GetState() != STATE_DISABLED)
				{
					audiobarProgressBtn->SetState(STATE_DISABLED);
					audiobarProgressBtn->SetAlpha(128);
					audiobarProgressLeftImg->SetVisible(false);
					audiobarProgressMidImg->SetTile(0);
					audiobarProgressLineImg->SetVisible(false);
					audiobarProgressRightImg->SetVisible(false);
				}
			}
		}
		else
		{
			if(audiobarProgressBtn->GetAlpha() == 255 || audiobarProgressBtn->GetState() != STATE_DISABLED)
			{
				audiobarProgressBtn->SetState(STATE_DISABLED);
				audiobarProgressBtn->SetAlpha(128);
				audiobarProgressLeftImg->SetVisible(false);
				audiobarProgressMidImg->SetTile(0);
				audiobarProgressLineImg->SetVisible(false);
				audiobarProgressRightImg->SetVisible(false);
			}
		}

		if(menu == MENU_BROWSE_ONLINEMEDIA)
			continue;

		if(audiobarPlaylistBtn->GetState() == STATE_CLICKED)
		{
			audiobarPlaylistBtn->ResetState();
			MusicPlaylistLoad();
		}

		if(audiobarBackwardBtn->GetState() == STATE_CLICKED)
		{
			audiobarBackwardBtn->ResetState();
			wiiSeekPos(0);
		}

		if(audiobarForwardBtn->GetState() == STATE_CLICKED)
		{
			audiobarForwardBtn->ResetState();

			if(playlistSize > 0)
			{
				if(wiiAudioOnly())
				{
					StopMPlayerFile(); // end this song
					while(controlledbygui != 1) // wait for song to end
						usleep(THREAD_SLEEP);
					FindNextFile(true); // find next song
				}
				else
				{
					// start playlist
					if(!mainWindow->Find(disabled))
						mainWindow->Append(disabled);
					mainWindow->SetState(STATE_DISABLED);
					ShowAction("Loading...");

					FindNextFile(false);
					LoadNewFile();

					CancelAction();
					mainWindow->Remove(disabled);
					mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}

		if(audiobarModeBtn->GetState() == STATE_CLICKED)
		{
			audiobarModeBtn->ResetState();
			WiiSettings.playOrder++;
			if(WiiSettings.playOrder > PLAY_LOOP)
				WiiSettings.playOrder = 0;

			UpdateAudiobarModeBtn();
		}

		if(playlistSize > 0)
		{
			if(playlistSize == 1)
			{
				if(audiobarForwardBtn->GetAlpha() == 255 || audiobarForwardBtn->GetState() != STATE_DISABLED)
				{
					audiobarForwardBtn->SetState(STATE_DISABLED);
					audiobarForwardBtn->SetAlpha(128);
				}
			}
			else if(audiobarForwardBtn->GetAlpha() == 128)
			{
				audiobarForwardBtn->SetState(STATE_DEFAULT);
				audiobarForwardBtn->SetAlpha(255);
			}
		}
		else
		{
			if(audiobarForwardBtn->GetAlpha() == 255 || audiobarForwardBtn->GetState() != STATE_DISABLED)
			{
				audiobarForwardBtn->SetState(STATE_DISABLED);
				audiobarForwardBtn->SetAlpha(128);
			}
		}

		if(wiiAudioOnly())
		{
			audiobarNowPlayingBtn->SetState(STATE_DEFAULT);

			if(wiiGetTimeLength() > 0)
			{
				if(audiobarBackwardBtn->GetAlpha() == 128)
				{
					audiobarBackwardBtn->SetState(STATE_DEFAULT);
					audiobarBackwardBtn->SetAlpha(255);
				}
			}
			else
			{
				if(audiobarBackwardBtn->GetState() != STATE_DISABLED)
				{
					audiobarBackwardBtn->SetState(STATE_DISABLED);
					audiobarBackwardBtn->SetAlpha(128);
				}
			}
		}
		else
		{
			if(audiobarBackwardBtn->GetState() != STATE_DISABLED)
			{
				audiobarBackwardBtn->SetState(STATE_DISABLED);
				audiobarBackwardBtn->SetAlpha(128);
			}

			if(audiobarNowPlayingBtn->IsVisible())
				audiobarNowPlayingBtn->SetVisible(false);
		}
	}
done:
	SuspendParseThread(); // halt parsing
	SuspendGui();
	mainWindow->Remove(&fileBrowser);
	mainWindow->Remove(&upOneLevelBtn);

	if(videoScreenshot)
		mainWindow->Remove(&backBtn);

	if(menu == MENU_BROWSE_MUSIC || menu == MENU_BROWSE_ONLINEMEDIA)
		mainWindow->Remove(audiobar);

	if(menu == MENU_BROWSE_MUSIC) // remove playlist functionality
		mainWindow->Remove(&playlistAddBtn);
}

// Picture Viewer
u8* picBuffer = NULL;
static int loadPictures = 0; // reload pictures

#define NUM_PICTURES 		5 // 1 image with a buffer of +/- 2 on each side

typedef struct
{
	GuiImageData *image;
	int index;
} picData;

static GuiImage *pictureImg = NULL;
static GuiButton *pictureBtn = NULL;
static picData pictures[NUM_PICTURES];
static int pictureIndexLoaded = -1;
static int pictureIndexLoading = -1;
static int pictureLoaded = -1;

static int closePictureViewer = 1; // 0 = picture viewer is open
static bool setPicture = false;
static int slideshow = 0; // slideshow mode
static u64 slideprev, slidenow; // slideshow timer

static void AllocPicBuffer()
{
	if(picBuffer)
		return;

	u32 level;
	_CPU_ISR_Disable(level);
	picBuffer = (u8*)((u32)SYS_GetArena2Hi()-MAX_PICTURE_SIZE);
	SYS_SetArena2Hi(picBuffer);
	_CPU_ISR_Restore(level);
}

static void FreePicBuffer()
{
	if(!picBuffer)
		return;
	
	u32 level;
	_CPU_ISR_Disable(level);
	SYS_SetArena2Hi(picBuffer+MAX_PICTURE_SIZE);
	_CPU_ISR_Restore(level);
	picBuffer = NULL;
}

static int FoundPicture(int p)
{
	if(p <= 0)
		return -1;

	for(int i=0; i < NUM_PICTURES; i++)
		if(pictures[i].index == p)
			return i;
	return -1;
}

static void SetPicture(int picIndex, int browserIndex)
{
	if(picIndex >= 0)
	{
		pictureLoaded = picIndex;
		pictureIndexLoaded = browserIndex;

		SuspendGui();
		pictureImg->SetImage(pictures[picIndex].image);
		pictureImg->SetScale(screenwidth-410, screenheight-100);
		pictureBtn->SetSize(pictures[picIndex].image->GetWidth()*pictureImg->GetScale(), pictures[picIndex].image->GetHeight()*pictureImg->GetScale());
		pictureBtn->SetState(STATE_DEFAULT);
		pictureImg->SetVisible(true);
		pictureBtn->SetVisible(true);
		ResumeGui();
	}
	else
	{
		SuspendGui();
		pictureBtn->SetVisible(false);
		pictureBtn->SetState(STATE_DISABLED);
		pictureImg->SetVisible(false);
		pictureImg->SetImage(NULL);
		ResumeGui();
		pictureLoaded = -1;
		pictureIndexLoaded = -1;
	}
}

static void CleanupPictures(int selIndex)
{
	// free any unused picture data
	for(int i=0; i < NUM_PICTURES; i++)
	{
		if(pictures[i].image == NULL || i == pictureLoaded)
			continue;

		if(selIndex == -1 || pictures[i].index < (selIndex-(NUM_PICTURES-1)/2) || pictures[i].index > (selIndex+(NUM_PICTURES-1)/2))
		{
			delete pictures[i].image;
			pictures[i].image = NULL;
			pictures[i].index = -1;
		}
	}
}

static void *PictureThread (void *arg)
{
	int selIndex;
	int i,next;
	char filepath[1024];

	pictureLoaded = -1;
	pictureIndexLoaded = -1;
	pictureIndexLoading = -1;
	AllocPicBuffer();

	while(1)
	{
restart:
		if(pictureThreadHalt == 1)
		{
			pictureLoaded = -1;
			pictureIndexLoaded = -1;
			pictureIndexLoading = -1;
			LWP_SuspendThread(picturethread);
		}
		if(pictureThreadHalt == 2)
			break;

		if(loadPictures)
		{
			loadPictures = 0;
			selIndex = browser.selIndex;
			CleanupPictures(selIndex);

			// load missing pictures - starting with selected index
			if(selIndex > 0 
				&& !browserList[selIndex].type == TYPE_FOLDER
				&& pictureIndexLoaded != selIndex
				&& browserList[selIndex].length < MAX_PICTURE_SIZE)
			{
				int found = FoundPicture(selIndex);
				if(found < 0)
				{
					sprintf(filepath, "%s%s", browser.dir, browserList[selIndex].filename);
					pictureIndexLoading = selIndex;
					int size = LoadFile((char *)picBuffer, filepath, SILENT);

					if(size == 0)
						goto restart;

					// find first empty slot
					for(i=0; i < NUM_PICTURES; i++)
						if(pictures[i].index == -1)
							break;

					if(i >= NUM_PICTURES) // no empty slot found!
						goto restart;

					pictures[i].image = new GuiImageData(picBuffer, size, GX_TF_RGBA8);

					if(pictures[i].image->GetImage() != NULL)
					{
						pictures[i].index = selIndex;
						found = i;
					}
					else
					{
						delete pictures[i].image;
						pictures[i].image = NULL;
					}
				}

				pictureIndexLoading = -1;
				setPicture = true; // trigger picture to be reloaded

				if(found < 0)
					goto restart;
			}

			// fill up image buffer slots
			next = selIndex-(NUM_PICTURES-1)/2;

			if(next <= 0)
				next = 1;

			for(i=0; i < NUM_PICTURES; i++)
			{
				if(pictures[i].index > 0)
					continue;

				while(next < browser.numEntries && 
					(browserList[next].type == TYPE_FOLDER
					|| next == selIndex 
					|| browserList[next].length > MAX_PICTURE_SIZE
					|| FoundPicture(next) >= 0))
					next++;

				if(next >= browser.numEntries || next > (selIndex+(NUM_PICTURES-1)/2))
					break;

				sprintf(filepath, "%s%s", browser.dir, browserList[next].filename);
				pictureIndexLoading = next;
				int size = LoadFile((char *)picBuffer, filepath, SILENT);

				if(size == 0)
					goto restart;

				pictures[i].image = new GuiImageData(picBuffer, size, GX_TF_RGBA8);

				if(pictures[i].image->GetImage() != NULL)
				{
					pictures[i].index = next;

					if(browser.selIndex == next)
						setPicture = true; // trigger picture to be reloaded
				}
				else
				{
					delete pictures[i].image;
					pictures[i].image = NULL;
				}

				pictureIndexLoading = -1;

				next++;
			}
		}
		usleep(THREAD_SLEEP);
	}
	SetPicture(-1, -1); // set picture to blank
	CleanupPictures(-1);
	FreePicBuffer();
	return NULL;
}

/****************************************************************************
 * ResumePictureThread
 *
 * Signals the picture thread to start, and resumes the thread.
 ***************************************************************************/
static void ResumePictureThread()
{
	if(picturethread == LWP_THREAD_NULL || guiShutdown)
		return;

	pictureThreadHalt = 0;
	LWP_ResumeThread(picturethread);
}

/****************************************************************************
 * SuspendPictureThread
 *
 * Signals the picture thread to stop.
 ***************************************************************************/
void SuspendPictureThread()
{
	if(picturethread == LWP_THREAD_NULL)
		return;

	pictureThreadHalt = 1;
	CancelFileOp();

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(picturethread))
		usleep(THREAD_SLEEP);

	SetPicture(-1, -1); // set picture to blank
	CleanupPictures(-1);
}

static void ChangePicture(int dir)
{
	int newIndex = browser.selIndex;

	while(1)
	{
		usleep(THREAD_SLEEP);
		newIndex += dir;

		if(newIndex >= browser.numEntries)
			newIndex = 1;
		else if(newIndex < 1)
			newIndex = browser.numEntries-1;

		if(browserList[newIndex].type == TYPE_FOLDER)
			continue;

		if(newIndex == browser.selIndex)
			return; // we have wrapped around to the same image - do nothing

		if(browserList[newIndex].length <= MAX_PICTURE_SIZE)
			break; // found a picture we can display
	}
	browser.selIndex = newIndex;
	loadPictures = 1;
}

static void ToggleSlideshow()
{
	if(slideshow == 0)
		slideprev = gettime(); // setup timer

	slideshow ^= 1;
	
	if(slideshow)
	{
		picturebarSlideshowTip->SetText("Stop Slideshow");
		picturebarSlideshowImg->SetImage(actionbarPause);
	}
	else
	{
		picturebarSlideshowTip->SetText("Start Slideshow");
		picturebarSlideshowImg->SetImage(actionbarPlay);
	}
}

static void PictureViewer()
{
	int currentIndex = -1;
	closePictureViewer = 0;
	if(slideshow != 0)
		ToggleSlideshow();
	GuiWindow * oldWindow = mainWindow;
	GuiImage * pictureFullImg = new GuiImage;
	pictureFullImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiWindow * w = new GuiWindow(screenwidth, screenheight);
	w->Append(pictureFullImg);
	w->Append(picturebar);

	SuspendGui();
	mainWindow = w;
	ResumeGui();

	int irCount = 0;
	bool irLast = false;

	while(closePictureViewer == 0 && !guiShutdown)
	{
		if(browser.selIndex != currentIndex)
		{
			currentIndex = browser.selIndex;
			loadPictures = 1; // trigger picture thread

			// search through already loaded pictures for this picture
			int found = FoundPicture(browser.selIndex);
			if(found >= 0)
				setPicture = true;
			else if(!browserList[browser.selIndex].type == TYPE_FOLDER
				&& pictureIndexLoading != browser.selIndex)
				CancelFileOp();

			if(!slideshow && !setPicture)
			{
				while(!setPicture) // wait for picture to load
				{
					ShowProgress ("Loading...", loadOffset, loadSize);
					usleep(THREAD_SLEEP);
				}
				CancelAction();
			}
		}

		if(setPicture)
		{
			setPicture = false;

			int found = FoundPicture(browser.selIndex);
			if(found >= 0)
			{
				SuspendGui();
				pictureFullImg->SetImage(pictures[found].image);
				pictureFullImg->SetScale(screenwidth, screenheight);
				ResumeGui();
			}
		}

		if(slideshow) // slideshow mode - change every X seconds
		{
			slidenow = gettime();
			if(slidenow > slideprev && 
				diff_usec(slideprev, slidenow) > (u32)(1000*1000*WiiSettings.slideshowDelay))
			{
				ChangePicture(1); // change to next picture
				slideprev = slidenow; // reset timer
			}
		}

		bool ir = false;

		for(int i=0; i<4; i++)
		{
			if(userInput[i].wpad->ir.valid)
			{
				ir = true;
				break;
			}
		}

		if(ir != irLast)
			irCount++;
		else if(irCount > 0)
			irCount--;

		usleep(THREAD_SLEEP);

		if(irCount < 10) // only change state if we've had 10 consecutive reports
			continue;

		irCount = 0;
		irLast = ir;

		if(!picturebar->IsVisible() && ir)
			picturebar->SetVisible(true);
		else if(picturebar->IsVisible() && !ir)
			picturebar->SetVisible(false);
	}

	SuspendGui();
	mainWindow = oldWindow;
	ResumeGui();
	delete w;
	delete pictureFullImg;
}

static void MenuBrowsePictures()
{
	int currentIndex = -1;
	ShutoffRumble();
	ResetBrowser();

	if(SYS_GetArena2Size() < 14680064) // 14 MB
	{
		ResumeGui();
		bool closeMPlayer = WindowPrompt(
			"Out of Memory",
			"WiiMC does not have enough free memory to load the picture viewer.",
			"Close MPlayer",
			"Cancel");

		if(!closeMPlayer)
		{
			UndoChangeMenu(); // go back to last menu
			return;
		}
		StopMPlayerFile();
		RemoveVideoImg();
	}

	browser.dir = &WiiSettings.picturesFolder[0];

	int pagesize = 11;
	float done;
	int tile = 0;

	GuiFileBrowser fileBrowser(388, pagesize);
	fileBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	fileBrowser.SetPosition(0, 90);
	fileBrowser.SetRightCutoff();

	GuiButton upOneLevelBtn(0,0);
	upOneLevelBtn.SetTrigger(trigB);
	upOneLevelBtn.SetSelectable(false);

	GuiImage progressEmptyImg(&progressShortEmpty);
	progressEmptyImg.SetPosition(0, 0);
	progressEmptyImg.SetVisible(false);
	
	GuiImage progressLeftImg(&progressLeft);
	progressLeftImg.SetPosition(0, 0);
	progressLeftImg.SetVisible(false);
	
	GuiImage progressMidImg(&progressMid);
	progressMidImg.SetPosition(8, 0);
	progressMidImg.SetTile(0);

	GuiImage progressLineImg(&progressLine);
	progressLineImg.SetVisible(false);

	GuiImage progressRightImg(&progressRight);
	progressRightImg.SetPosition(0, 0);
	progressRightImg.SetVisible(false);

	SuspendGui();
	mainWindow->Append(&fileBrowser);
	mainWindow->Append(&upOneLevelBtn);
	ResumeGui();

	// populate initial directory listing
	BrowserChangeFolder(false, true);
	
	GuiWindow progressWindow(240, 16);
	progressWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	progressWindow.Append(&progressEmptyImg);
	progressWindow.Append(&progressLeftImg);
	progressWindow.Append(&progressMidImg);
	progressWindow.Append(&progressLineImg);
	progressWindow.Append(&progressRightImg);
	
	GuiWindow pictureWindow(screenwidth-370, 390);
	pictureWindow.SetPosition(0, 90);
	pictureWindow.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	pictureWindow.Append(&progressWindow);
	pictureWindow.Append(pictureBtn);

	SetPicture(-1, -1);
	SuspendGui();
	mainWindow->Append(&pictureWindow);
	ResumeGui();

	// start picture thread
	loadPictures = 1; // trigger picture thread
	ResumePictureThread();

	while(menuCurrent == MENU_BROWSE_PICTURES && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		// devices were inserted or removed - update the filebrowser!
		if(devicesChanged)
		{
			devicesChanged = false;

			if(BrowserChangeFolder(true, true))
			{
				fileBrowser.ResetState();
				fileBrowser.fileList[0]->SetState(STATE_SELECTED);
				fileBrowser.TriggerUpdate();
			}
			else
			{
				goto done;
			}
		}

		// up one level
		if(upOneLevelBtn.GetState() == STATE_CLICKED)
		{
			upOneLevelBtn.ResetState();
			browser.selIndex = 0;

			if(!BrowserChangeFolder())
				goto done;

			fileBrowser.ResetState();
			fileBrowser.fileList[0]->SetState(STATE_SELECTED);
			fileBrowser.TriggerUpdate();
		}
		
		// update progress bar
		if(pictureIndexLoading == browser.selIndex && !browserList[browser.selIndex].type == TYPE_FOLDER && loadSize > 0 && !pictureImg->IsVisible())
		{
			done = loadOffset/(float)loadSize;

			if(done > 0.02)
			{
				progressLeftImg.SetVisible(true);
				tile = 60*(done-0.02);
				if(tile > 58) tile = 58;
				progressMidImg.SetTile(tile);
				progressLineImg.SetPosition(8 + tile*4, 0);
				progressLineImg.SetVisible(true);
			}

			if(done < 0.98)
				progressRightImg.SetVisible(false);

			if(tile == 58)
			{
				progressLineImg.SetVisible(false);
				progressRightImg.SetVisible(true);
			}
			progressEmptyImg.SetVisible(true);
		}
		else
		{
			progressEmptyImg.SetVisible(false);
			progressLeftImg.SetVisible(false);
			progressMidImg.SetTile(0);
			progressLineImg.SetVisible(false);
			progressRightImg.SetVisible(false);
		}

		// update displayed picture
		if(browser.selIndex != currentIndex || setPicture)
		{
			currentIndex = browser.selIndex;
			setPicture = false;

			if(browserList[browser.selIndex].type == TYPE_FOLDER)
			{
				SetPicture(-1, -1); // set picture to blank
			}
			else
			{
				// search through already loaded pictures for this picture
				int found = FoundPicture(browser.selIndex);
				if(found >= 0)
				{
					SetPicture(found, browser.selIndex);
				}
				else
				{
					SetPicture(-1, -1); // set picture to blank
					if(!browserList[browser.selIndex].type == TYPE_FOLDER && 
						pictureIndexLoading != browser.selIndex)
					{
						CancelFileOp();
					}
				}
				loadPictures = 1; // trigger picture thread
			}
		}

		// update file browser based on arrow buttons
		for(int i=0; i < pagesize; i++)
		{
			if(fileBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				fileBrowser.fileList[i]->ResetState();

				if(browserList[browser.selIndex].type == TYPE_FOLDER)
				{
					SuspendPictureThread();

					if(BrowserChangeFolder(true, true))
					{
						fileBrowser.ResetState();
						fileBrowser.fileList[0]->SetState(STATE_SELECTED);
						fileBrowser.TriggerUpdate();
						loadPictures = 1; // trigger picture thread
						ResumePictureThread();
					}
					else
					{
						goto done;
					}
				}
				else
				{
					PictureViewer();
				}
			}
		}
		if(pictureBtn->GetState() == STATE_CLICKED)
		{
			pictureBtn->ResetState();
			PictureViewer();
		}
	}
done:
	SuspendPictureThread(); // halt picture thread
	SuspendParseThread(); // halt parsing
	SuspendGui();
	mainWindow->Remove(&pictureWindow);
	mainWindow->Remove(&fileBrowser);
	mainWindow->Remove(&upOneLevelBtn);
}

static void MenuDVD()
{
	ResumeGui();

	if(!ChangeInterface(DEVICE_DVD, -1, NOTSILENT))
	{
		UndoChangeMenu(); // go back to last menu
		return;
	}

	if(!wiiPlayingDVD())
	{
		if(WiiSettings.dvdMenu)
			sprintf(loadedFile, "dvdnav://");
		else
			sprintf(loadedFile, "dvd://");
		sprintf(loadedFileDisplay, "DVD");
		mainWindow->SetState(STATE_DISABLED);
		mainWindow->Append(disabled);
		ShowAction("Loading...");
		LoadMPlayerFile();
	
		// wait until MPlayer is ready to take or return control
		while(!guiShutdown && controlledbygui != 1)
			usleep(THREAD_SLEEP);

		CancelAction();
		SuspendGui();

		if(!guiShutdown) // load failed
		{
			UndoChangeMenu(); // go back to last menu
			mainWindow->Remove(disabled);
			mainWindow->SetState(STATE_DEFAULT);
			ErrorPrompt("Unrecognized DVD format!");
		}
	}
	else
	{
		GuiImageData btnBottom(button_bottom_png);
		GuiImageData btnBottomOver(button_bottom_over_png);
		GuiImageData arrowRight(arrow_right_small_png);

		GuiText backBtnTxt("Resume", 18, (GXColor){255, 255, 255, 255});
		backBtnTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
		backBtnTxt.SetPosition(-74, 10);
		GuiImage backBtnImg(&btnBottom);
		GuiImage backBtnImgOver(&btnBottomOver);
		GuiImage backBtnArrow(&arrowRight);
		backBtnArrow.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
		backBtnArrow.SetPosition(-54, 11);
		GuiButton backBtn(screenwidth, btnBottom.GetHeight());
		backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		backBtn.SetPosition(0, 0);
		backBtn.SetLabel(&backBtnTxt);
		backBtn.SetImage(&backBtnImg);
		backBtn.SetImageOver(&backBtnImgOver);
		backBtn.SetIcon(&backBtnArrow);
		backBtn.SetTrigger(trigA);

		SuspendGui();
		mainWindow->Append(&backBtn);
		ResumeGui();

		while(menuCurrent == MENU_DVD && !guiShutdown)
		{
			if(backBtn.GetState() == STATE_CLICKED)
			{
				backBtn.ResetState();
				ResumeMPlayerFile(); // go back to MPlayer

				// wait until MPlayer is ready to take control (or return control)
				while(!guiShutdown && controlledbygui != 1)
					usleep(THREAD_SLEEP);

				break;
			}
			usleep(THREAD_SLEEP);
		}
		CancelAction();
		SuspendGui();
		mainWindow->Remove(&backBtn);
	}
}

static void MenuSettingsGlobal()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	
	sprintf(options.name[i++], "File Extensions");
	sprintf(options.name[i++], "Language");
	sprintf(options.name[i++], "Volume");
	sprintf(options.name[i++], "Exit Action");
	sprintf(options.name[i++], "Wiimote Rumble");

	options.length = i;
		
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Global", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_GLOBAL && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				WiiSettings.hideExtensions ^= 1;
				break;
			case 1:
				WiiSettings.language++;

				if(WiiSettings.language >= LANG_LENGTH)
					WiiSettings.language = 0;
				break;
			case 2:
				WiiSettings.volume += 10;
				if(WiiSettings.volume > 100)
					WiiSettings.volume = 0;
				wiiSetVolume(WiiSettings.volume);
				break;
			case 3:
				WiiSettings.exitAction++;
				if(WiiSettings.exitAction > EXIT_LOADER)
					WiiSettings.exitAction = 0;
				break;
			case 4:
				WiiSettings.rumble ^= 1;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			sprintf(options.value[0], "%s", WiiSettings.hideExtensions ? "Hide" : "Show");

			switch(WiiSettings.language)
			{
				case LANG_JAPANESE:				sprintf(options.value[1], "Japanese"); break;
				case LANG_ENGLISH:				sprintf(options.value[1], "English"); break;
				case LANG_GERMAN:				sprintf(options.value[1], "German"); break;
				case LANG_FRENCH:				sprintf(options.value[1], "French"); break;
				case LANG_SPANISH:				sprintf(options.value[1], "Spanish"); break;
				case LANG_ITALIAN:				sprintf(options.value[1], "Italian"); break;
				case LANG_DUTCH:				sprintf(options.value[1], "Dutch"); break;
				case LANG_SIMP_CHINESE:			sprintf(options.value[1], "Chinese (Simplified)"); break;
				case LANG_TRAD_CHINESE:			sprintf(options.value[1], "Chinese (Traditional)"); break;
				case LANG_KOREAN:				sprintf(options.value[1], "Korean"); break;
				case LANG_ROMANIAN:				sprintf(options.value[1], "Romanian"); break;
				case LANG_ESTONIAN:				sprintf(options.value[1], "Estonian"); break;
				case LANG_BRAZILIAN_PORTUGUESE: sprintf(options.value[1], "Brazilian Portuguese"); break;
				case LANG_HUNGARIAN:			sprintf(options.value[1], "Hungarian"); break;
				case LANG_POLISH:				sprintf(options.value[1], "Polish"); break;
			}
			
			sprintf (options.value[2], "%d%%", WiiSettings.volume);

			switch(WiiSettings.exitAction)
			{
				case EXIT_AUTO:		sprintf(options.value[3], "Auto"); break;
				case EXIT_WIIMENU:	sprintf(options.value[3], "Return to Wii Menu"); break;
				case EXIT_POWEROFF:	sprintf(options.value[3], "Power Off Wii"); break;
				case EXIT_LOADER:	sprintf(options.value[3], "Return to Loader"); break;
			}

			sprintf(options.value[4], "%s", WiiSettings.rumble ? "On" : "Off");

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	ChangeLanguage();
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void ScreenZoomWindowUpdate(void * ptr, float h, float v)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		WiiSettings.videoZoomHor += h;
		WiiSettings.videoZoomVert += v;

		char zoom[10];
		sprintf(zoom, "%.2f%%", WiiSettings.videoZoomHor*100);
		settingText->SetText(zoom);
		sprintf(zoom, "%.2f%%", WiiSettings.videoZoomVert*100);
		settingText2->SetText(zoom);
		b->ResetState();
	}
}

static void ScreenZoomWindowLeftClick(void * ptr) { ScreenZoomWindowUpdate(ptr, -0.01, 0); }
static void ScreenZoomWindowRightClick(void * ptr) { ScreenZoomWindowUpdate(ptr, +0.01, 0); }
static void ScreenZoomWindowUpClick(void * ptr) { ScreenZoomWindowUpdate(ptr, 0, +0.01); }
static void ScreenZoomWindowDownClick(void * ptr) { ScreenZoomWindowUpdate(ptr, 0, -0.01); }

static void ScreenZoomWindow()
{
	GuiWindow * w = new GuiWindow(200,200);
	w->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	GuiImageData arrowLeft(arrow_left_png);
	GuiImage arrowLeftImg(&arrowLeft);
	GuiImageData arrowLeftOver(arrow_left_over_png);
	GuiImage arrowLeftOverImg(&arrowLeftOver);
	GuiButton arrowLeftBtn(arrowLeft.GetWidth(), arrowLeft.GetHeight());
	arrowLeftBtn.SetImage(&arrowLeftImg);
	arrowLeftBtn.SetImageOver(&arrowLeftOverImg);
	arrowLeftBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowLeftBtn.SetPosition(46, 0);
	arrowLeftBtn.SetTrigger(0, trigA);
	arrowLeftBtn.SetTrigger(1, trigLeft);
	arrowLeftBtn.SetSelectable(false);
	arrowLeftBtn.SetUpdateCallback(ScreenZoomWindowLeftClick);

	GuiImageData arrowRight(arrow_right_png);
	GuiImage arrowRightImg(&arrowRight);
	GuiImageData arrowRightOver(arrow_right_over_png);
	GuiImage arrowRightOverImg(&arrowRightOver);
	GuiButton arrowRightBtn(arrowRight.GetWidth(), arrowRight.GetHeight());
	arrowRightBtn.SetImage(&arrowRightImg);
	arrowRightBtn.SetImageOver(&arrowRightOverImg);
	arrowRightBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowRightBtn.SetPosition(136, 0);
	arrowRightBtn.SetTrigger(0, trigA);
	arrowRightBtn.SetTrigger(1, trigRight);
	arrowRightBtn.SetSelectable(false);
	arrowRightBtn.SetUpdateCallback(ScreenZoomWindowRightClick);

	GuiImageData arrowUp(arrow_up_png);
	GuiImage arrowUpImg(&arrowUp);
	GuiImageData arrowUpOver(arrow_up_over_png);
	GuiImage arrowUpOverImg(&arrowUpOver);
	GuiButton arrowUpBtn(arrowUp.GetWidth(), arrowUp.GetHeight());
	arrowUpBtn.SetImage(&arrowUpImg);
	arrowUpBtn.SetImageOver(&arrowUpOverImg);
	arrowUpBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowUpBtn.SetPosition(-76, -27);
	arrowUpBtn.SetTrigger(0, trigA);
	arrowUpBtn.SetTrigger(1, trigUp);
	arrowUpBtn.SetSelectable(false);
	arrowUpBtn.SetUpdateCallback(ScreenZoomWindowUpClick);

	GuiImageData arrowDown(arrow_down_png);
	GuiImage arrowDownImg(&arrowDown);
	GuiImageData arrowDownOver(arrow_down_over_png);
	GuiImage arrowDownOverImg(&arrowDownOver);
	GuiButton arrowDownBtn(arrowDown.GetWidth(), arrowDown.GetHeight());
	arrowDownBtn.SetImage(&arrowDownImg);
	arrowDownBtn.SetImageOver(&arrowDownOverImg);
	arrowDownBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowDownBtn.SetPosition(-76, 27);
	arrowDownBtn.SetTrigger(0, trigA);
	arrowDownBtn.SetTrigger(1, trigDown);
	arrowDownBtn.SetSelectable(false);
	arrowDownBtn.SetUpdateCallback(ScreenZoomWindowDownClick);

	GuiImageData screenPosition(screen_position_png);
	GuiImage screenPositionImg(&screenPosition);
	screenPositionImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	screenPositionImg.SetPosition(0, 0);

	settingText = new GuiText(NULL, 20, (GXColor){255, 255, 255, 255});
	settingText2 = new GuiText(NULL, 20, (GXColor){255, 255, 255, 255});
	char zoom[10];
	sprintf(zoom, "%.2f%%", WiiSettings.videoZoomHor*100);
	settingText->SetText(zoom);
	settingText->SetPosition(90, 0);
	sprintf(zoom, "%.2f%%", WiiSettings.videoZoomVert*100);
	settingText2->SetText(zoom);
	settingText2->SetPosition(-76, 0);

	float currentZoomHor = WiiSettings.videoZoomHor;
	float currentZoomVert = WiiSettings.videoZoomVert;

	w->Append(&arrowLeftBtn);
	w->Append(&arrowRightBtn);
	w->Append(&arrowUpBtn);
	w->Append(&arrowDownBtn);
	w->Append(&screenPositionImg);
	w->Append(settingText);
	w->Append(settingText2);

	if(!SettingWindow("Screen Zoom",w))
	{
		// undo changes
		WiiSettings.videoZoomHor = currentZoomHor;
		WiiSettings.videoZoomVert = currentZoomVert;
	}

	delete(w);
	delete(settingText);
	delete(settingText2);
}

static void ScreenPositionWindowUpdate(void * ptr, int x, int y)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		WiiSettings.videoXshift += x;
		WiiSettings.videoYshift += y;

		char shift[10];
		sprintf(shift, "%i, %i", WiiSettings.videoXshift, WiiSettings.videoYshift);
		settingText->SetText(shift);
		b->ResetState();
	}
}

static void ScreenPositionWindowLeftClick(void * ptr) { ScreenPositionWindowUpdate(ptr, -1, 0); }
static void ScreenPositionWindowRightClick(void * ptr) { ScreenPositionWindowUpdate(ptr, +1, 0); }
static void ScreenPositionWindowUpClick(void * ptr) { ScreenPositionWindowUpdate(ptr, 0, -1); }
static void ScreenPositionWindowDownClick(void * ptr) { ScreenPositionWindowUpdate(ptr, 0, +1); }

static void ScreenPositionWindow()
{
	GuiWindow * w = new GuiWindow(120,120);
	w->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	w->SetPosition(0, 0);

	GuiImageData arrowLeft(arrow_left_png);
	GuiImage arrowLeftImg(&arrowLeft);
	GuiImageData arrowLeftOver(arrow_left_over_png);
	GuiImage arrowLeftOverImg(&arrowLeftOver);
	GuiButton arrowLeftBtn(arrowLeft.GetWidth(), arrowLeft.GetHeight());
	arrowLeftBtn.SetImage(&arrowLeftImg);
	arrowLeftBtn.SetImageOver(&arrowLeftOverImg);
	arrowLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	arrowLeftBtn.SetTrigger(0, trigA);
	arrowLeftBtn.SetTrigger(1, trigLeft);
	arrowLeftBtn.SetSelectable(false);
	arrowLeftBtn.SetUpdateCallback(ScreenPositionWindowLeftClick);

	GuiImageData arrowRight(arrow_right_png);
	GuiImage arrowRightImg(&arrowRight);
	GuiImageData arrowRightOver(arrow_right_over_png);
	GuiImage arrowRightOverImg(&arrowRightOver);
	GuiButton arrowRightBtn(arrowRight.GetWidth(), arrowRight.GetHeight());
	arrowRightBtn.SetImage(&arrowRightImg);
	arrowRightBtn.SetImageOver(&arrowRightOverImg);
	arrowRightBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	arrowRightBtn.SetTrigger(0, trigA);
	arrowRightBtn.SetTrigger(1, trigRight);
	arrowRightBtn.SetSelectable(false);
	arrowRightBtn.SetUpdateCallback(ScreenPositionWindowRightClick);

	GuiImageData arrowUp(arrow_up_png);
	GuiImage arrowUpImg(&arrowUp);
	GuiImageData arrowUpOver(arrow_up_over_png);
	GuiImage arrowUpOverImg(&arrowUpOver);
	GuiButton arrowUpBtn(arrowUp.GetWidth(), arrowUp.GetHeight());
	arrowUpBtn.SetImage(&arrowUpImg);
	arrowUpBtn.SetImageOver(&arrowUpOverImg);
	arrowUpBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	arrowUpBtn.SetTrigger(0, trigA);
	arrowUpBtn.SetTrigger(1, trigUp);
	arrowUpBtn.SetSelectable(false);
	arrowUpBtn.SetUpdateCallback(ScreenPositionWindowUpClick);

	GuiImageData arrowDown(arrow_down_png);
	GuiImage arrowDownImg(&arrowDown);
	GuiImageData arrowDownOver(arrow_down_over_png);
	GuiImage arrowDownOverImg(&arrowDownOver);
	GuiButton arrowDownBtn(arrowDown.GetWidth(), arrowDown.GetHeight());
	arrowDownBtn.SetImage(&arrowDownImg);
	arrowDownBtn.SetImageOver(&arrowDownOverImg);
	arrowDownBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	arrowDownBtn.SetTrigger(0, trigA);
	arrowDownBtn.SetTrigger(1, trigDown);
	arrowDownBtn.SetSelectable(false);
	arrowDownBtn.SetUpdateCallback(ScreenPositionWindowDownClick);

	GuiImageData screenPosition(screen_position_png);
	GuiImage screenPositionImg(&screenPosition);
	screenPositionImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	settingText = new GuiText(NULL, 20, (GXColor){0, 0, 0, 255});
	char shift[10];
	sprintf(shift, "%i, %i", WiiSettings.videoXshift, WiiSettings.videoYshift);
	settingText->SetText(shift);

	int currentX = WiiSettings.videoXshift;
	int currentY = WiiSettings.videoYshift;

	w->Append(&arrowLeftBtn);
	w->Append(&arrowRightBtn);
	w->Append(&arrowUpBtn);
	w->Append(&arrowDownBtn);
	w->Append(&screenPositionImg);
	w->Append(settingText);

	if(!SettingWindow("Screen Position",w))
	{
		WiiSettings.videoXshift = currentX; // undo changes
		WiiSettings.videoYshift = currentY;
	}

	delete(w);
	delete(settingText);
}

static void MenuSettingsVideos()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	
	sprintf(options.name[i++], "Screen Zoom");
	sprintf(options.name[i++], "Screen Position");
	sprintf(options.name[i++], "Frame Dropping");
	sprintf(options.name[i++], "Aspect Ratio");
	sprintf(options.name[i++], "Cache Fill");
	sprintf(options.name[i++], "Audio Delay");
	sprintf(options.name[i++], "Auto-Resume");
	sprintf(options.name[i++], "Rewind / Fast Forward");
	sprintf(options.name[i++], "Videos Files Folder");

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Videos", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_VIDEOS && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				ScreenZoomWindow();
				break;
			case 1:
				ScreenPositionWindow();
				break;
			case 2:
				WiiSettings.frameDropping++;
				if (WiiSettings.frameDropping > 2)
					WiiSettings.frameDropping = 0;
				break;
			case 3:
				if(WiiSettings.aspectRatio > 2.34)
					WiiSettings.aspectRatio = -2;
				else if(WiiSettings.aspectRatio > 1.84)
					WiiSettings.aspectRatio = 2.35;
				else if(WiiSettings.aspectRatio > 1.77)
					WiiSettings.aspectRatio = 1.85;
				else if(WiiSettings.aspectRatio > 1.32)
					WiiSettings.aspectRatio = 1.7778;
				else if(WiiSettings.aspectRatio > -2.01)
					WiiSettings.aspectRatio = 1.3333;
				else
					WiiSettings.aspectRatio = -2;
				break;
			case 4:
				WiiSettings.cacheFill += 10;
				if (WiiSettings.cacheFill > 100)
					WiiSettings.cacheFill = 10;
				break;
			case 5:
				WiiSettings.audioDelay += 0.1;
				if (WiiSettings.audioDelay > 2)
					WiiSettings.audioDelay = -2;
				break;
			case 6:
				WiiSettings.autoResume ^= 1;
				break;
			case 7:
				WiiSettings.seekTime += 30;
				if(WiiSettings.seekTime > 600)
					WiiSettings.seekTime = 30;
				break;
			case 8:
				OnScreenKeyboard(WiiSettings.videosFolder, MAXPATHLEN);
				CleanupPath(WiiSettings.videosFolder);
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			sprintf (options.value[0], "%.2f%%, %.2f%%", WiiSettings.videoZoomHor*100, WiiSettings.videoZoomVert*100);
			sprintf (options.value[1], "%d, %d", WiiSettings.videoXshift, WiiSettings.videoYshift);

			switch(WiiSettings.frameDropping)
			{
				case FRAMEDROPPING_AUTO:
					sprintf (options.value[2], "Auto"); break;
				case FRAMEDROPPING_ALWAYS:
					sprintf (options.value[2], "Always"); break;
				case FRAMEDROPPING_DISABLED:
					sprintf (options.value[2], "Disabled"); break;
			}

			if(WiiSettings.aspectRatio > 2.34)
				sprintf (options.value[3], "2.35:1");
			else if(WiiSettings.aspectRatio > 1.84)
				sprintf (options.value[3], "1.85:1");
			else if(WiiSettings.aspectRatio > 1.77)
				sprintf (options.value[3], "16:9");
			else if(WiiSettings.aspectRatio > 1.32)
				sprintf (options.value[3], "4:3");
			else
				sprintf (options.value[3], "Auto");

			sprintf (options.value[4], "%d%%", WiiSettings.cacheFill);
			sprintf (options.value[5], "%.1f sec", WiiSettings.audioDelay);
			sprintf (options.value[6], "%s", WiiSettings.autoResume ? "On" : "Off");
			sprintf (options.value[7], "%d sec", WiiSettings.seekTime);
			snprintf(options.value[8], 40, "%s", WiiSettings.videosFolder);

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsMusic()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Play Order");
	sprintf(options.name[i++], "Music Files Folder");

	options.length = i;
		
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Music", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_MUSIC && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				WiiSettings.playOrder++;
				if(WiiSettings.playOrder > PLAY_LOOP)
					WiiSettings.playOrder = 0;
				break;
			case 1:
				OnScreenKeyboard(WiiSettings.musicFolder, MAXPATHLEN);
				CleanupPath(WiiSettings.musicFolder);
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			

			switch(WiiSettings.playOrder)
			{
				case PLAY_SINGLE:		sprintf(options.value[0], "Single"); break;
				case PLAY_CONTINUOUS:	sprintf(options.value[0], "Continuous"); break;
				case PLAY_SHUFFLE:		sprintf(options.value[0], "Shuffle"); break;
				case PLAY_LOOP:			sprintf(options.value[0], "Loop"); break;
			}
			snprintf(options.value[1], 40, "%s", WiiSettings.musicFolder);

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsPictures()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Pictures Files Folder");
	sprintf(options.name[i++], "Slideshow Delay");

	options.length = i;
		
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Pictures", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_PICTURES && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				OnScreenKeyboard(WiiSettings.picturesFolder, MAXPATHLEN);
				CleanupPath(WiiSettings.picturesFolder);
				break;
			case 1:
				WiiSettings.slideshowDelay++;
				if(WiiSettings.slideshowDelay > 10)
					WiiSettings.slideshowDelay = 1;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			snprintf(options.value[0], 40, "%s", WiiSettings.picturesFolder);
			sprintf(options.value[1], "%d sec", WiiSettings.slideshowDelay);
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsDVD()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "DVD Menu");

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - DVD", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_DVD && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				WiiSettings.dvdMenu ^= 1;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			
			sprintf(options.value[0], "%s", WiiSettings.dvdMenu ? "Show" : "Skip to Main Title");

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsOnlineMedia()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Online Media Folder");

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Online Media", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_ONLINEMEDIA && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				OnScreenKeyboard(WiiSettings.onlinemediaFolder, MAXPATHLEN);
				if(!IsOnlineMediaPath(WiiSettings.onlinemediaFolder))
					CleanupPath(WiiSettings.onlinemediaFolder);
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			snprintf(options.value[0], 40, "%s", WiiSettings.onlinemediaFolder);
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsNetwork()
{
	int ret;
	int i = 0;
	OptionList options;
	
	// find all currently set SMB/FTP entries
	int map[MAX_OPTIONS];
	
	for(int j=0; j < 5; j++)
	{
		if(WiiSettings.smbConf[j].share[0] != 0)
		{
			options.icon[i] = ICON_SMB;
			options.value[i][0] = 0;
			map[i] = j;

			if(WiiSettings.smbConf[j].displayname[0] != 0)
				sprintf(options.name[i], "%s", WiiSettings.smbConf[j].displayname);
			else
				sprintf(options.name[i], "%s", WiiSettings.smbConf[j].share);
			i++;
		}
	}
	for(int j=0; j < 5; j++)
	{
		if(WiiSettings.ftpConf[j].ip[0] != 0)
		{
			options.icon[i] = ICON_FTP;
			options.value[i][0] = 0;
			map[i] = j + 5;

			if(WiiSettings.ftpConf[j].displayname[0] != 0)
				sprintf(options.name[i], "%s", WiiSettings.ftpConf[j].displayname);
			else
				sprintf(options.name[i], "%s@%s/%s", WiiSettings.ftpConf[j].user, WiiSettings.ftpConf[j].ip, WiiSettings.ftpConf[j].folder);
			i++;
		}
	}

	options.length = i;

	GuiText titleTxt("Settings - Network", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData iconSMB(icon_smb_png);
	GuiImageData iconFTP(icon_ftp_png);
	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiText addsmbBtnTxt("Add SMB Share", 20, (GXColor){255, 255, 255, 255});
	addsmbBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiImage addsmbBtnImg(&btnOutline);
	GuiImage addsmbBtnImgOver(&btnOutlineOver);
	GuiButton addsmbBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	addsmbBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	addsmbBtn.SetPosition(-130, -90);
	addsmbBtn.SetLabel(&addsmbBtnTxt);
	addsmbBtn.SetImage(&addsmbBtnImg);
	addsmbBtn.SetImageOver(&addsmbBtnImgOver);
	addsmbBtn.SetTrigger(trigA);
	addsmbBtn.SetEffectGrow();
	
	GuiText addftpBtnTxt("Add FTP Site", 20, (GXColor){255, 255, 255, 255});
	addftpBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiImage addftpBtnImg(&btnOutline);
	GuiImage addftpBtnImgOver(&btnOutlineOver);
	GuiButton addftpBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	addftpBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	addftpBtn.SetPosition(130, -90);
	addftpBtn.SetLabel(&addftpBtnTxt);
	addftpBtn.SetImage(&addftpBtnImg);
	addftpBtn.SetImageOver(&addftpBtnImgOver);
	addftpBtn.SetTrigger(trigA);
	addftpBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(screenwidth, 6, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol1Position(30);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&addsmbBtn);
	w.Append(&addftpBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_NETWORK && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		if((ret >= 0 && map[ret] < 5) || addsmbBtn.GetState() == STATE_CLICKED)
		{
			if(ret >= 0) netEditIndex = map[ret];
			else netEditIndex = -1;
			ChangeMenuNoHistory(MENU_SETTINGS_NETWORK_SMB);
		}
		else if((ret >= 0 && map[ret] >= 5) || addftpBtn.GetState() == STATE_CLICKED)
		{
			if(ret >= 0) netEditIndex = map[ret] - 5;
			else netEditIndex = -1;
			ChangeMenuNoHistory(MENU_SETTINGS_NETWORK_FTP);
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void MenuSettingsNetworkSMB()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	wchar_t titleStr[100];
	char shareName[100];

	sprintf(options.name[i++], "Display Name");
	sprintf(options.name[i++], "Share IP");
	sprintf(options.name[i++], "Share Name");
	sprintf(options.name[i++], "Username");
	sprintf(options.name[i++], "Password");

	options.length = i;
	
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}
	
	if(netEditIndex < 0)
		sprintf(shareName, "New Share");
	else if(WiiSettings.smbConf[netEditIndex].displayname[0] != 0)
		sprintf(shareName, "%s", WiiSettings.smbConf[netEditIndex].displayname);
	else
		sprintf(shareName, "%s", WiiSettings.smbConf[netEditIndex].share);

	swprintf(titleStr, 100, L"%s - %s", gettext("Settings - Network"), gettext(shareName));

	GuiText titleTxt(NULL, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetWText(titleStr);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);
	
	GuiText deleteBtnTxt("Delete", 20, (GXColor){255, 255, 255, 255});
	GuiImage deleteBtnImg(&btnOutline);
	GuiImage deleteBtnImgOver(&btnOutlineOver);
	GuiButton deleteBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	deleteBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	deleteBtn.SetPosition(0, -90);
	deleteBtn.SetLabel(&deleteBtnTxt);
	deleteBtn.SetImage(&deleteBtnImg);
	deleteBtn.SetImageOver(&deleteBtnImgOver);
	deleteBtn.SetTrigger(trigA);
	deleteBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(screenwidth, 6, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);

	if(netEditIndex < 0)
	{
		// find a share to put the data into
		for(i=0; i < 5; i++)
		{
			if(WiiSettings.smbConf[i].share[0] == 0)
			{
				netEditIndex = i;
				break;
			}
		}
	}
	else
	{
		w.Append(&deleteBtn);
	}
	
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_NETWORK_SMB && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				OnScreenKeyboard(WiiSettings.smbConf[netEditIndex].displayname, 40);
				break;

			case 1:
				if(WiiSettings.smbConf[netEditIndex].ip[0] == 0)
				{
					// pre-populate IP
					if(wiiIP[0] != 0)
					{
						strcpy(WiiSettings.smbConf[netEditIndex].ip, wiiIP);
						char* dot = strrchr(WiiSettings.smbConf[netEditIndex].ip,'.');
						if (dot != NULL) *++dot = 0; // strip last octet
					}
				}

				OnScreenKeyboard(WiiSettings.smbConf[netEditIndex].ip, 80);
				break;

			case 2:
				OnScreenKeyboard(WiiSettings.smbConf[netEditIndex].share, 40);
				break;

			case 3:
				OnScreenKeyboard(WiiSettings.smbConf[netEditIndex].user, 20);
				break;

			case 4:
				OnScreenKeyboard(WiiSettings.smbConf[netEditIndex].pwd, 14);
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			snprintf(options.value[0], 40, "%s", WiiSettings.smbConf[netEditIndex].displayname);
			snprintf(options.value[1], 40, "%s", WiiSettings.smbConf[netEditIndex].ip);
			snprintf(options.value[2], 40, "%s", WiiSettings.smbConf[netEditIndex].share);
			snprintf(options.value[3], 40, "%s", WiiSettings.smbConf[netEditIndex].user);
			snprintf(options.value[4], 40, "%s", WiiSettings.smbConf[netEditIndex].pwd);
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS_NETWORK);
		}
		if(deleteBtn.GetState() == STATE_CLICKED)
		{
			deleteBtn.ResetState();
			if (WindowPrompt("Delete Share", gettext("Are you sure that you want to delete this share?"), "OK", "Cancel"))
			{
				WiiSettings.smbConf[netEditIndex].displayname[0] = 0;
				WiiSettings.smbConf[netEditIndex].ip[0] = 0;
				WiiSettings.smbConf[netEditIndex].share[0] = 0;
				WiiSettings.smbConf[netEditIndex].user[0] = 0;
				WiiSettings.smbConf[netEditIndex].pwd[0] = 0;
				ChangeMenuNoHistory(MENU_SETTINGS_NETWORK);
			}
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	CloseShare(netEditIndex+1);
}

static bool isnumeric(char *str)
{
	while(*str)
	{
		if(!isdigit(*str)) return false;
		str++;
	}
	return true;
}

static void MenuSettingsNetworkFTP()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	wchar_t titleStr[100];
	char siteName[100];

	sprintf(options.name[i++], "Display Name");
	sprintf(options.name[i++], "IP");
	sprintf(options.name[i++], "Folder");
	sprintf(options.name[i++], "Username");
	sprintf(options.name[i++], "Password");
	sprintf(options.name[i++], "Port");
	sprintf(options.name[i++], "Mode");

	options.length = i;
		
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	if(netEditIndex < 0)
		sprintf(siteName, "New Site");
	else if(WiiSettings.ftpConf[netEditIndex].displayname[0] != 0)
		sprintf(siteName, "%s", WiiSettings.ftpConf[netEditIndex].displayname);
	else
		snprintf(siteName, 100, "%s@%s:%d/%s", 
		WiiSettings.ftpConf[netEditIndex].user, 
		WiiSettings.ftpConf[netEditIndex].ip,
		WiiSettings.ftpConf[netEditIndex].port,
		WiiSettings.ftpConf[netEditIndex].folder);

	swprintf(titleStr, 100, L"%s - %s", gettext("Settings - Network"), gettext(siteName));

	GuiText titleTxt(NULL, 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetWText(titleStr);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);
	
	GuiText deleteBtnTxt("Delete", 20, (GXColor){255, 255, 255, 255});
	GuiImage deleteBtnImg(&btnOutline);
	GuiImage deleteBtnImgOver(&btnOutlineOver);
	GuiButton deleteBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	deleteBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	deleteBtn.SetPosition(0, -90);
	deleteBtn.SetLabel(&deleteBtnTxt);
	deleteBtn.SetImage(&deleteBtnImg);
	deleteBtn.SetImageOver(&deleteBtnImgOver);
	deleteBtn.SetTrigger(trigA);
	deleteBtn.SetEffectGrow();

	int size = 7;

	if(netEditIndex >= 0)
		size = 6;

	GuiOptionBrowser optionBrowser(screenwidth, size, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	
	if(netEditIndex < 0)
	{
		// find a site to put the data into
		for(i=0; i < 5; i++)
		{
			if(WiiSettings.ftpConf[i].ip[0] == 0)
			{
				netEditIndex = i;
				break;
			}
		}
	}
	else
	{
		w.Append(&deleteBtn);
	}
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_NETWORK_FTP && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				OnScreenKeyboard(WiiSettings.ftpConf[netEditIndex].displayname, 40);
				break;
			case 1:
				OnScreenKeyboard(WiiSettings.ftpConf[netEditIndex].ip, 80);
				break;
			case 2:
				OnScreenKeyboard(WiiSettings.ftpConf[netEditIndex].folder, 80);
				break;
			case 3:
				OnScreenKeyboard(WiiSettings.ftpConf[netEditIndex].user, 20);
				break;
			case 4:
				OnScreenKeyboard(WiiSettings.ftpConf[netEditIndex].pwd, 20);
				break;
			case 5:
				char tmpPort[6];
				sprintf(tmpPort, "%d", WiiSettings.ftpConf[netEditIndex].port);
				OnScreenKeyboard(tmpPort, 5);
				if(tmpPort[0] == 0)
					ErrorPrompt("Port cannot be blank!");
				else if(!isnumeric(tmpPort))
					ErrorPrompt("Port is not a number!");
				else
				{
					int port = atoi(tmpPort);
					if(port < 4 || port > 49151)
						ErrorPrompt("Port is outside the allowed range (4-49151)!");
					else
						WiiSettings.ftpConf[netEditIndex].port = port;
				}
				break;
			case 6:
				//WiiSettings.ftpConf[netEditIndex].passive ^= 1;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			snprintf(options.value[0], 40, "%s", WiiSettings.ftpConf[netEditIndex].displayname);
			snprintf(options.value[1], 40, "%s", WiiSettings.ftpConf[netEditIndex].ip);
			snprintf(options.value[2], 40, "%s", WiiSettings.ftpConf[netEditIndex].folder);
			snprintf(options.value[3], 40, "%s", WiiSettings.ftpConf[netEditIndex].user);
			snprintf(options.value[4], 40, "%s", WiiSettings.ftpConf[netEditIndex].pwd);
			sprintf(options.value[5], "%d", WiiSettings.ftpConf[netEditIndex].port);
			sprintf(options.value[6], "%s", WiiSettings.ftpConf[netEditIndex].passive ? "Passive" : "Active");
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS_NETWORK);
		}
		if(deleteBtn.GetState() == STATE_CLICKED)
		{
			deleteBtn.ResetState();
			if (WindowPrompt("Delete Site", gettext("Are you sure that you want to delete this site?"), "OK", "Cancel"))
			{
				WiiSettings.ftpConf[netEditIndex].displayname[0] = 0;
				WiiSettings.ftpConf[netEditIndex].ip[0] = 0;
				WiiSettings.ftpConf[netEditIndex].folder[0] = 0;
				WiiSettings.ftpConf[netEditIndex].user[0] = 0;
				WiiSettings.ftpConf[netEditIndex].pwd[0] = 0;
				WiiSettings.ftpConf[netEditIndex].port = 21;
				WiiSettings.ftpConf[netEditIndex].passive = 0;
				ChangeMenuNoHistory(MENU_SETTINGS_NETWORK);
			}
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	CloseFTP(netEditIndex+1);
}

static int GetLangIndex()
{
	if(WiiSettings.subtitleLanguage[0] == 0)
		return 0;

	for(int i=1; i < LANGUAGE_SIZE; i++)
		if(strcmp(WiiSettings.subtitleLanguage, languages[i].abbrev) == 0)
			return i;
	return 0;
}

static int GetCodepageIndex()
{
	if(WiiSettings.subtitleCodepage[0] == 0)
		return 0;

	for(int i=1; i < CODEPAGE_SIZE; i++)
		if(strcmp(WiiSettings.subtitleCodepage, codepages[i].cpname) == 0)
			return i;
	return 0;
}

static void LanguageWindow()
{
	GuiWindow promptWindow(556,352);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_large_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetAlpha(220);

	GuiText titleTxt("Subtitle Language", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,18);

	GuiText okBtnTxt("OK", 20, (GXColor){255, 255, 255, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(20, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(trigA);
	okBtn.SetSelectable(false);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 20, (GXColor){255, 255, 255, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-20, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetSelectable(false);
	cancelBtn.SetEffectGrow();

	int i;
	OptionList options;
	sprintf(options.name[0], "Default");

	for(i=1; i < LANGUAGE_SIZE; i++)
		sprintf(options.name[i], "%s (%s)", languages[i].language, languages[i].abbrev);

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiOptionBrowser optionBrowser(544, 6, &options);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	optionBrowser.SetPosition(6, 70);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&optionBrowser);
	promptWindow.Append(&okBtn);
	promptWindow.Append(&cancelBtn);

	SuspendGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(disabled);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	int save = -1;

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
		else if(optionBrowser.GetClickedOption() >= 0)
			save = 1;
	}

	if(save && optionBrowser.GetSelectedOption() >= 0)
		strcpy(WiiSettings.subtitleLanguage, languages[optionBrowser.GetSelectedOption()].abbrev);

	SuspendGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->Remove(disabled);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

static void CodepageWindow()
{
	GuiWindow promptWindow(556,352);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_large_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetAlpha(220);

	GuiText titleTxt("Subtitle Codepage", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,18);

	GuiText okBtnTxt("OK", 20, (GXColor){255, 255, 255, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(20, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(trigA);
	okBtn.SetSelectable(false);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 20, (GXColor){255, 255, 255, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-20, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetSelectable(false);
	cancelBtn.SetEffectGrow();

	int i;
	OptionList options;
	sprintf(options.name[0], "Default");

	for(i=1; i < CODEPAGE_SIZE; i++)
		sprintf(options.name[i], "%s (%s)", codepages[i].cpname, codepages[i].language);

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiOptionBrowser optionBrowser(544, 6, &options);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	optionBrowser.SetPosition(6, 70);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&optionBrowser);
	promptWindow.Append(&okBtn);
	promptWindow.Append(&cancelBtn);

	SuspendGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(disabled);
	mainWindow->Append(&promptWindow);
	ResumeGui();

	int save = -1;

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
		else if(optionBrowser.GetClickedOption() >= 0)
			save = 1;
	}

	if(save && optionBrowser.GetSelectedOption() >= 0)
		strcpy(WiiSettings.subtitleCodepage, codepages[optionBrowser.GetSelectedOption()].cpname);

	SuspendGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->Remove(disabled);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

static void MenuSettingsSubtitles()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Visibility");
	sprintf(options.name[i++], "Delay");
	sprintf(options.name[i++], "Language");
	sprintf(options.name[i++], "Codepage");

	options.length = i;
		
	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings - Subtitles", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 7, &options);
	optionBrowser.SetPosition(0, 150);
	optionBrowser.SetCol2Position(275);
	optionBrowser.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	SuspendGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS_SUBTITLES && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				WiiSettings.subtitleVisibility ^= 1;
				break;
			case 1:
				WiiSettings.subtitleDelay += 0.1;
				if (WiiSettings.subtitleDelay > 2)
					WiiSettings.subtitleDelay = -2;
				break;
			case 2:
				LanguageWindow();
				break;
			case 3:
				CodepageWindow();
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			sprintf(options.value[0], "%s", WiiSettings.subtitleVisibility ? "On" : "Off");
			sprintf(options.value[1], "%.1f sec", WiiSettings.subtitleDelay);
			strcpy(options.value[2], languages[GetLangIndex()].language);
			if(GetCodepageIndex() == 0)
				sprintf(options.value[3], "Default");
			else
				sprintf(options.value[3], "%s (%s)", codepages[GetCodepageIndex()].cpname, codepages[GetCodepageIndex()].language);

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			ChangeMenuNoHistory(MENU_SETTINGS);
		}
	}
	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static void MenuSettings()
{
	int ret;
	int i = 0;
	
	OptionList options;

	sprintf(options.name[i++], "Global");
	sprintf(options.name[i++], "Videos");
	sprintf(options.name[i++], "Music");
	sprintf(options.name[i++], "Pictures");
	sprintf(options.name[i++], "DVD");
	sprintf(options.name[i++], "Online Media");
	sprintf(options.name[i++], "Network");
	sprintf(options.name[i++], "Subtitles");

	options.length = i;

	for(i=0; i < options.length; i++)
	{
		options.value[i][0] = 0;
		options.icon[i] = 0;
	}

	GuiText titleTxt("Settings", 28, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(30, 100);

	GuiImageData btnBottom(button_bottom_png);
	GuiImageData btnBottomOver(button_bottom_over_png);
	GuiImageData arrowRight(arrow_right_small_png);

	GuiText backBtnTxt("Go back", 18, (GXColor){255, 255, 255, 255});
	backBtnTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnTxt.SetPosition(-16, 10);
	GuiImage backBtnImg(&btnBottom);
	GuiImage backBtnImgOver(&btnBottomOver);
	GuiImage backBtnArrow(&arrowRight);
	backBtnArrow.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	backBtnArrow.SetPosition(26, 11);
	GuiButton backBtn(screenwidth, btnBottom.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(0, 0);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetIcon(&backBtnArrow);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trigB);

	GuiOptionBrowser optionBrowser(screenwidth, 8, &options);
	optionBrowser.SetPosition(0, 150);

	SuspendGui();
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&backBtn);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menuCurrent == MENU_SETTINGS && !guiShutdown)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				ChangeMenuNoHistory(MENU_SETTINGS_GLOBAL);
				break;
			case 1:
				ChangeMenuNoHistory(MENU_SETTINGS_VIDEOS);
				break;
			case 2:
				ChangeMenuNoHistory(MENU_SETTINGS_MUSIC);
				break;
			case 3:
				ChangeMenuNoHistory(MENU_SETTINGS_PICTURES);
				break;
			case 4:
				ChangeMenuNoHistory(MENU_SETTINGS_DVD);
				break;
			case 5:
				ChangeMenuNoHistory(MENU_SETTINGS_ONLINEMEDIA);
				break;
			case 6:
				ChangeMenuNoHistory(MENU_SETTINGS_NETWORK);
				break;
			case 7:
				ChangeMenuNoHistory(MENU_SETTINGS_SUBTITLES);
				break;
		}

		if(backBtn.GetState() == STATE_CLICKED)
			ChangeMenu(menuPrevious);
	}

	if(menuCurrent <= 4) // leaving settings area
		SaveSettings(NOTSILENT);

	SuspendGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&backBtn);
	mainWindow->Remove(&titleTxt);
}

static void VideoProgressCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;

	float total = wiiGetTimeLength();
	int done = wiiGetTimePos();
	float percent = 0;
	
	if(total > 0)
		percent = done/total;
	
	if(b->GetState() == STATE_CLICKED)
	{
		if(b->GetStateChan() >= 0)
		{
			percent = (userInput[b->GetStateChan()].wpad->ir.x - b->GetLeft())/560.0;
			if(percent > 100) percent = 100;
			else if(percent < 0) percent = 0;
			done = total*percent;
			ShutoffRumble();
			wiiSeekPos(done);
		}
		b->ResetState();
	}
	
	if(percent <= 0.02)
	{
		videobarProgressLeftImg->SetVisible(false);
		videobarProgressMidImg->SetTile(0);
		videobarProgressLineImg->SetVisible(false);
		videobarProgressRightImg->SetVisible(false);
	}
	else if(percent >= 0.99)
	{
		videobarProgressLeftImg->SetVisible(true);
		videobarProgressMidImg->SetTile(136);
		videobarProgressLineImg->SetVisible(false);
		videobarProgressRightImg->SetVisible(true);
	}
	else
	{
		int tile = 136*percent;
		videobarProgressLeftImg->SetVisible(true);
		videobarProgressMidImg->SetTile(tile);
		videobarProgressLineImg->SetPosition(8 + tile*4, 60);
		videobarProgressLineImg->SetVisible(true);
		videobarProgressRightImg->SetVisible(false);
	}
	char time[50];
	wiiGetTimeDisplay(time);
	videobarTime->SetText(time);
}

static void VideoVolumeLevelCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;

	if(b->GetState() == STATE_CLICKED)
	{
		if(b->GetStateChan() >= 0)
		{
			WiiSettings.volume = 100 - 100*((userInput[b->GetStateChan()].wpad->ir.y-5 - b->GetTop())/140.0);

			if(WiiSettings.volume > 94)
				WiiSettings.volume = 100;
			else if(WiiSettings.volume < 8)
				WiiSettings.volume = 0;

			wiiSetVolume(WiiSettings.volume);
		}
		b->ResetState();
	}

	float percent = WiiSettings.volume/100.0;
	int tile = 32*percent;

	if(percent < 0.05)
	{
		videobarVolumeLevelBottomImg->SetVisible(false);
		videobarVolumeLevelLineImg->SetVisible(false);
		videobarVolumeLevelTopImg->SetVisible(false);
	}
	else if(percent > 0.95)
	{
		videobarVolumeLevelBottomImg->SetVisible(true);
		videobarVolumeLevelLineImg->SetVisible(false);
		videobarVolumeLevelTopImg->SetVisible(true);
		tile = 32;
	}
	else
	{
		videobarVolumeLevelBottomImg->SetVisible(true);
		videobarVolumeLevelLineImg->SetPosition(20, -90-tile*4);
		videobarVolumeLevelLineImg->SetVisible(true);
		videobarVolumeLevelTopImg->SetVisible(false);
	}
	videobarVolumeLevelMidImg->SetTileVertical(tile);
	videobarVolumeLevelMidImg->SetPosition(20, -90-tile*4);
}

bool VolumeLevelBarVisible()
{
	return videobarVolumeLevelBtn->IsVisible();
}

void ShowVolumeLevelBar()
{
	videobarVolumeLevelBtn->SetVisible(true);
	videobarVolumeLevelBtn->SetState(STATE_DEFAULT);
}

void HideVolumeLevelBar()
{
	videobarVolumeLevelBtn->SetVisible(false);
	videobarVolumeLevelBtn->SetState(STATE_DISABLED);
	videobarVolumeLevelTopImg->SetVisible(false);
	videobarVolumeLevelMidImg->SetTileVertical(0);
	videobarVolumeLevelLineImg->SetVisible(false);
	videobarVolumeLevelBottomImg->SetVisible(false);
}

static void VideoVolumeCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();

		// show/hide volume level bar
		if(videobarVolumeLevelBtn->IsVisible())
			HideVolumeLevelBar();
		else
			ShowVolumeLevelBar();
	}
}

static void VideoBackwardCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ShutoffRumble();
		wiiRewind();
	}
}

static void VideoPauseCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ShutoffRumble();
		wiiPause();
	}
}

static void VideoForwardCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ShutoffRumble();
		wiiFastForward();
	}
}

static void AudioProgressCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;

	double total = wiiGetTimeLength();
	int done = wiiGetTimePos();
	double percent = 0;

	if(!wiiAudioOnly())
		total = 0; // values are from a loaded video - do not show!

	if(total > 0)
		percent = done/total;

	if(b->GetState() == STATE_CLICKED)
	{
		if(b->GetStateChan() >= 0)
		{
			percent = (userInput[b->GetStateChan()].wpad->ir.x - b->GetLeft())/300.0;
			if(percent > 100) percent = 100;
			else if(percent < 0) percent = 0;
			done = total*percent;
			wiiSeekPos(done);
		}
		b->ResetState();
	}

	if(percent <= 0.01)
	{
		audiobarProgressLeftImg->SetVisible(false);
		audiobarProgressMidImg->SetTile(0);
		audiobarProgressLineImg->SetVisible(false);
		audiobarProgressRightImg->SetVisible(false);
	}
	else if(percent <= 0.03)
	{
		audiobarProgressLeftImg->SetVisible(true);
		audiobarProgressMidImg->SetTile(0);
		audiobarProgressLineImg->SetPosition(8, 60);
		audiobarProgressLineImg->SetVisible(true);
		audiobarProgressRightImg->SetVisible(false);
	}
	else if(percent >= 0.98)
	{
		audiobarProgressLeftImg->SetVisible(true);
		audiobarProgressMidImg->SetTile(71);
		audiobarProgressLineImg->SetVisible(false);
		audiobarProgressRightImg->SetVisible(true);
	}
	else
	{
		audiobarProgressLeftImg->SetVisible(true);
		int tile = 71*percent;
		audiobarProgressMidImg->SetTile(tile);
		audiobarProgressLineImg->SetPosition(8 + tile*4, 60);
		audiobarProgressLineImg->SetVisible(true);
		audiobarProgressRightImg->SetVisible(false);
	}
}

static void AudioNowPlayingCallback(void * ptr)
{
	if(!wiiAudioOnly())
	{
		if(audiobarNowPlayingBtn->IsVisible())
			audiobarNowPlayingBtn->SetVisible(false);
		return;
	}
	else
	{
		if(!audiobarNowPlayingBtn->IsVisible())
			audiobarNowPlayingBtn->SetVisible(true);
	}

	double total = wiiGetTimeLength();

	// display ICY data
	if(total <= 0.02)
	{
		if(strncmp(loadedFile, "http:", 5) == 0)
		{
			if(streamtitle[0] != 0)
				audiobarNowPlaying[1]->SetText(streamtitle);
			else
				audiobarNowPlaying[1]->SetText("Internet Stream");
			if(streamurl[0] != 0)
				audiobarNowPlaying[3]->SetText(streamurl);
			else
				audiobarNowPlaying[3]->SetText(NULL);
		}
		else
		{
			audiobarNowPlaying[1]->SetText(NULL);
			audiobarNowPlaying[3]->SetText(NULL);
		}
		audiobarNowPlaying[2]->SetText(NULL);
		return;
	}

	if(!nowPlayingSet && total > 0)
	{
		nowPlayingSet = true;
		char *title = NULL;
		char *artist = NULL;
		char *album = NULL;
		char *year = NULL;
		int i;

		audiobarNowPlaying[2]->SetText(NULL);
		audiobarNowPlaying[3]->SetText(NULL);

		if(wiiGetMetaTitle() != NULL)
		{
			title = strdup(wiiGetMetaTitle());
			for(i=strlen(title)-1; i >= 0; i--)	if(title[i] == ' ') title[i] = '\0'; else break;

			if(strlen(title) > 0)
				audiobarNowPlaying[1]->SetText(title);
		}
		if(!title || title[0] == 0)
		{
			char tmp[MAXJOLIET+1];
			char *start = strrchr(loadedFile,'/');
			if(start != NULL) // start up starting part of path
				strcpy(tmp, &start[1]);
			else
				strcpy(tmp, loadedFile);
			StripExt(tmp);
			audiobarNowPlaying[1]->SetText(tmp);
		}

		if(wiiGetMetaArtist() != NULL)
		{
			artist = strdup(wiiGetMetaArtist());
			for(i=strlen(artist)-1; i >= 0; i--) if(artist[i] == ' ') artist[i] = '\0'; else break;
			
			if(strlen(artist) > 0)
				audiobarNowPlaying[2]->SetText(artist);
		}

		if(wiiGetMetaAlbum() != NULL)
		{
			album = strdup(wiiGetMetaAlbum());
			for(i=strlen(album)-1; i >= 0; i--) if(album[i] == ' ') album[i] = '\0'; else break;
		}

		if(wiiGetMetaYear() != NULL)
		{
			year = strdup(wiiGetMetaYear());
			for(i=strlen(year)-1; i >= 0; i--) if(year[i] == ' ') year[i] = '\0'; else break;
		}

		char txt[1024];

		if(album && strlen(album) > 0 && year && strlen(year) > 0)
		{
			sprintf(txt, "%s (%s)", album, year);
			audiobarNowPlaying[3]->SetText(txt);
		}
		else if(album && strlen(album) > 0)
		{
			audiobarNowPlaying[3]->SetText(album);
		}
		else if(year && strlen(year) > 0)
		{
			audiobarNowPlaying[3]->SetText(year);
		}

		if(title) free(title);
		if(artist) free(artist);
		if(album) free(album);
		if(year) free(year);
	}
}

static void PicturePreviousCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ChangePicture(-1);
	}
}

static void PictureNextCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ChangePicture(1);
	}
}

static void PictureSlideshowCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		ToggleSlideshow();
	}
}

static void PictureCloseCallback(void * ptr)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		b->ResetState();
		closePictureViewer = 1;
	}
}

static void SetupGui()
{	
	static int guiSetup = 0;
	
	if(guiSetup)
		return;
	
	// pointers
	
	pointer[0] = new GuiImageData(player1_point_png);
	pointer[1] = new GuiImageData(player2_point_png);
	pointer[2] = new GuiImageData(player3_point_png);
	pointer[3] = new GuiImageData(player4_point_png);

	// triggers

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);
	
	trigLeft = new GuiTrigger;
	trigLeft->SetButtonOnlyTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);
		
	trigRight = new GuiTrigger;
	trigRight->SetButtonOnlyTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);
	
	trigUp = new GuiTrigger;
	trigUp->SetButtonOnlyTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);

	trigDown = new GuiTrigger;
	trigDown->SetButtonOnlyTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);

	// images
	
	bg = new GuiImageData(bg_png);
	disabled = new GuiImage(screenwidth,screenheight,(GXColor){0, 0, 0, 100});

	actionbarLeft = new GuiImageData(actionbar_left_png);	
	actionbarMid = new GuiImageData(actionbar_mid_png);
	actionbarRight = new GuiImageData(actionbar_right_png);

	actionbarBackward = new GuiImageData(actionbar_backward_png);
	actionbarPause = new GuiImageData(actionbar_pause_png);
	actionbarPlay = new GuiImageData(actionbar_play_png);
	actionbarForward = new GuiImageData(actionbar_forward_png);
	actionbarSingle = new GuiImageData(actionbar_single_png);
	actionbarContinuous = new GuiImageData(actionbar_continuous_png);
	actionbarShuffle = new GuiImageData(actionbar_shuffle_png);
	actionbarLoop = new GuiImageData(actionbar_loop_png);	
	actionbarClose = new GuiImageData(actionbar_close_png);
	actionbarPlaylist = new GuiImageData(actionbar_playlist_png);
	actionbarVolume = new GuiImageData(actionbar_volume_png);

	// video bar

	videobarLeftImg = new GuiImage(actionbarLeft);
	videobarMidImg = new GuiImage(actionbarMid);
	videobarMidImg->SetPosition(20, 0);
	videobarMidImg->SetTile(25); // 20x25 = 520
	videobarRightImg = new GuiImage(actionbarRight);
	videobarRightImg->SetPosition(520, 0);

	videobarProgressImg = new GuiImage(&progressLongEmpty);
	videobarProgressLeftImg = new GuiImage(&progressLeft);
	videobarProgressLeftImg->SetPosition(0, 60);
	videobarProgressLeftImg->SetVisible(false);
	videobarProgressMidImg = new GuiImage(&progressMid);
	videobarProgressMidImg->SetPosition(8, 60);
	videobarProgressMidImg->SetTile(0);
	videobarProgressLineImg = new GuiImage(&progressLine);
	videobarProgressLineImg->SetPosition(8, 60);
	videobarProgressRightImg = new GuiImage(&progressRight);
	videobarProgressRightImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	videobarProgressRightImg->SetPosition(0, 60);
	videobarProgressRightImg->SetVisible(false);
	videobarVolumeImg = new GuiImage(actionbarVolume);
	videobarVolumeImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	videobarBackwardImg = new GuiImage(actionbarBackward);
	videobarBackwardImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	videobarPauseImg = new GuiImage(actionbarPause);
	videobarPauseImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	videobarForwardImg = new GuiImage(actionbarForward);
	videobarForwardImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	videobarVolumeLevelImg = new GuiImage(&volumeEmpty);
	videobarVolumeLevelImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	videobarVolumeLevelTopImg = new GuiImage(&volumeTop);
	videobarVolumeLevelTopImg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	videobarVolumeLevelTopImg->SetPosition(20, -220);
	videobarVolumeLevelTopImg->SetVisible(false);
	videobarVolumeLevelMidImg = new GuiImage(&volumeMid);
	videobarVolumeLevelMidImg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	videobarVolumeLevelMidImg->SetTileVertical(0);
	videobarVolumeLevelLineImg = new GuiImage(&volumeLine);
	videobarVolumeLevelLineImg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	videobarVolumeLevelLineImg->SetPosition(20, 0);
	videobarVolumeLevelLineImg->SetVisible(false);
	videobarVolumeLevelBottomImg = new GuiImage(&volumeBottom);
	videobarVolumeLevelBottomImg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	videobarVolumeLevelBottomImg->SetPosition(20, -90);
	videobarVolumeLevelBottomImg->SetVisible(false);
	
	videobarVolumeTip = new GuiTooltip("Volume");
	videobarBackwardTip = new GuiTooltip("Backward");
	videobarPauseTip = new GuiTooltip("Pause");
	videobarForwardTip = new GuiTooltip("Forward");
	
	videobarProgressBtn = new GuiButton(videobarProgressImg->GetWidth(), videobarProgressImg->GetHeight());
	videobarProgressBtn->SetImage(videobarProgressImg);
	videobarProgressBtn->SetPosition(0, 60);
	videobarProgressBtn->SetTrigger(trigA);
	videobarProgressBtn->SetSelectable(false);
	videobarProgressBtn->SetUpdateCallback(VideoProgressCallback);
	
	videobarVolumeLevelBtn = new GuiButton(videobarVolumeLevelImg->GetWidth(), videobarVolumeLevelImg->GetHeight());
	videobarVolumeLevelBtn->SetImage(videobarVolumeLevelImg);
	videobarVolumeLevelBtn->SetPosition(20, -90);
	videobarVolumeLevelBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	videobarVolumeLevelBtn->SetTrigger(trigA);
	videobarVolumeLevelBtn->SetSelectable(false);
	videobarVolumeLevelBtn->SetVisible(false);
	videobarVolumeLevelBtn->SetState(STATE_DISABLED);
	videobarVolumeLevelBtn->SetUpdateCallback(VideoVolumeLevelCallback);

	videobarVolumeBtn = new GuiButton(40, 40);
	videobarVolumeBtn->SetPosition(10, 4);
	videobarVolumeBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	videobarVolumeBtn->SetImage(videobarVolumeImg);
	videobarVolumeBtn->SetTooltip(videobarVolumeTip);
	videobarVolumeBtn->SetTrigger(trigA);
	videobarVolumeBtn->SetSelectable(false);
	videobarVolumeBtn->SetUpdateCallback(VideoVolumeCallback);
	videobarVolumeBtn->SetEffectGrow();

	videobarBackwardBtn = new GuiButton(40, 40);
	videobarBackwardBtn->SetPosition(-60, 4);
	videobarBackwardBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videobarBackwardBtn->SetImage(videobarBackwardImg);
	videobarBackwardBtn->SetTooltip(videobarBackwardTip);
	videobarBackwardBtn->SetTrigger(trigA);
	videobarBackwardBtn->SetSelectable(false);
	videobarBackwardBtn->SetUpdateCallback(VideoBackwardCallback);
	videobarBackwardBtn->SetEffectGrow();

	videobarPauseBtn = new GuiButton(40, 40);
	videobarPauseBtn->SetPosition(0, 4);
	videobarPauseBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videobarPauseBtn->SetImage(videobarPauseImg);
	videobarPauseBtn->SetTooltip(videobarPauseTip);
	videobarPauseBtn->SetTrigger(trigA);
	videobarPauseBtn->SetSelectable(false);
	videobarPauseBtn->SetUpdateCallback(VideoPauseCallback);
	videobarPauseBtn->SetEffectGrow();
	
	videobarForwardBtn = new GuiButton(40, 40);
	videobarForwardBtn->SetPosition(60, 4);
	videobarForwardBtn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videobarForwardBtn->SetImage(videobarForwardImg);
	videobarForwardBtn->SetTooltip(videobarForwardTip);
	videobarForwardBtn->SetTrigger(trigA);
	videobarForwardBtn->SetSelectable(false);
	videobarForwardBtn->SetUpdateCallback(VideoForwardCallback);
	videobarForwardBtn->SetEffectGrow();
	
	videobarTime = new GuiText(NULL, 16, (GXColor){255, 255, 255, 255});
	videobarTime->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	videobarTime->SetPosition(-20, 14);

	videobar = new GuiWindow(560, 80);

	videobar->Append(videobarLeftImg);
	videobar->Append(videobarMidImg);
	videobar->Append(videobarRightImg);
	videobar->Append(videobarProgressBtn);
	videobar->Append(videobarProgressLeftImg);
	videobar->Append(videobarProgressMidImg);
	videobar->Append(videobarProgressLineImg);
	videobar->Append(videobarProgressRightImg);
	videobar->Append(videobarVolumeBtn);
	videobar->Append(videobarVolumeLevelBtn);
	videobar->Append(videobarVolumeLevelTopImg);
	videobar->Append(videobarVolumeLevelMidImg);
	videobar->Append(videobarVolumeLevelLineImg);
	videobar->Append(videobarVolumeLevelBottomImg);
	videobar->Append(videobarBackwardBtn);
	videobar->Append(videobarPauseBtn);
	videobar->Append(videobarForwardBtn);
	videobar->Append(videobarTime);

	videobar->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	videobar->SetPosition(0, -30);
	
	// audio bar

	audiobarLeftImg = new GuiImage(actionbarLeft);
	audiobarMidImg = new GuiImage(actionbarMid);
	audiobarMidImg->SetPosition(20, 0);
	audiobarMidImg->SetTile(12); // 20x12 = 240
	audiobarRightImg = new GuiImage(actionbarRight);
	audiobarRightImg->SetPosition(260, 0);

	audiobarProgressImg = new GuiImage(&progressEmpty);
	audiobarProgressLeftImg = new GuiImage(&progressLeft);
	audiobarProgressLeftImg->SetPosition(0, 60);
	audiobarProgressLeftImg->SetVisible(false);
	audiobarProgressMidImg = new GuiImage(&progressMid);
	audiobarProgressMidImg->SetPosition(8, 60);
	audiobarProgressMidImg->SetTile(0);
	audiobarProgressLineImg = new GuiImage(&progressLine);
	audiobarProgressLineImg->SetPosition(12, 60);
	audiobarProgressLineImg->SetVisible(false);
	audiobarProgressRightImg = new GuiImage(&progressRight);
	audiobarProgressRightImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	audiobarProgressRightImg->SetPosition(0, 60);
	audiobarProgressRightImg->SetVisible(false);
	audiobarPlaylistImg = new GuiImage(actionbarPlaylist);
	audiobarPlaylistImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	audiobarBackwardImg = new GuiImage(actionbarBackward);
	audiobarBackwardImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	audiobarPauseImg = new GuiImage(actionbarPlay);
	audiobarPauseImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	audiobarForwardImg = new GuiImage(actionbarForward);
	audiobarForwardImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	audiobarModeImg = new GuiImage;
	audiobarModeImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	audiobarPlaylistTip = new GuiTooltip("Playlist");
	audiobarBackwardTip = new GuiTooltip("Restart");
	audiobarPauseTip = new GuiTooltip("Play");
	audiobarForwardTip = new GuiTooltip("Next");
	audiobarModeTip = new GuiTooltip("Single Play");

	UpdateAudiobarModeBtn();

	audiobarProgressBtn = new GuiButton(audiobarProgressImg->GetWidth(), audiobarProgressImg->GetHeight());
	audiobarProgressBtn->SetImage(audiobarProgressImg);
	audiobarProgressBtn->SetPosition(0, 60);
	audiobarProgressBtn->SetTrigger(trigA);
	audiobarProgressBtn->SetSelectable(false);
	audiobarProgressBtn->SetState(STATE_DISABLED);
	audiobarProgressBtn->SetAlpha(128);
	audiobarProgressBtn->SetUpdateCallback(AudioProgressCallback);
	
	audiobarPlaylistBtn = new GuiButton(40, 40);
	audiobarPlaylistBtn->SetPosition(10, 4);
	audiobarPlaylistBtn->SetImage(audiobarPlaylistImg);
	audiobarPlaylistBtn->SetTooltip(audiobarPlaylistTip);
	audiobarPlaylistBtn->SetTrigger(trigA);
	audiobarPlaylistBtn->SetEffectGrow();

	audiobarBackwardBtn = new GuiButton(40, 40);
	audiobarBackwardBtn->SetPosition(70, 4);
	audiobarBackwardBtn->SetImage(audiobarBackwardImg);
	audiobarBackwardBtn->SetTooltip(audiobarBackwardTip);
	audiobarBackwardBtn->SetTrigger(trigA);
	audiobarBackwardBtn->SetEffectGrow();
	audiobarBackwardBtn->SetState(STATE_DISABLED);
	audiobarBackwardBtn->SetAlpha(128);

	audiobarPauseBtn = new GuiButton(40, 40);
	audiobarPauseBtn->SetPosition(130, 4);
	audiobarPauseBtn->SetImage(audiobarPauseImg);
	audiobarPauseBtn->SetTooltip(audiobarPauseTip);
	audiobarPauseBtn->SetTrigger(trigA);
	audiobarPauseBtn->SetEffectGrow();
	audiobarPauseBtn->SetState(STATE_DISABLED);
	audiobarPauseBtn->SetAlpha(128);

	audiobarForwardBtn = new GuiButton(40, 40);
	audiobarForwardBtn->SetPosition(190, 4);
	audiobarForwardBtn->SetImage(audiobarForwardImg);
	audiobarForwardBtn->SetTooltip(audiobarForwardTip);
	audiobarForwardBtn->SetTrigger(trigA);
	audiobarForwardBtn->SetEffectGrow();
	audiobarForwardBtn->SetState(STATE_DISABLED);
	audiobarForwardBtn->SetAlpha(128);

	audiobarModeBtn = new GuiButton(40, 40);
	audiobarModeBtn->SetPosition(250, 4);
	audiobarModeBtn->SetImage(audiobarModeImg);
	audiobarModeBtn->SetTooltip(audiobarModeTip);
	audiobarModeBtn->SetTrigger(trigA);
	audiobarModeBtn->SetEffectGrow();
	
	audiobarNowPlaying[0] = new GuiText("now playing", 16, (GXColor){160, 160, 160, 255});
	audiobarNowPlaying[0]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	audiobarNowPlaying[0]->SetPosition(10, 0);
	
	for(int i=1; i < 4; i++)
	{
		audiobarNowPlaying[i] = new GuiText(NULL, 16, (GXColor){255, 255, 255, 255});
		audiobarNowPlaying[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		audiobarNowPlaying[i]->SetPosition(10, 20*i);
		audiobarNowPlaying[i]->SetMaxWidth(screenwidth-370);
	}

	audiobarNowPlayingBtn = new GuiButton(screenwidth-370,80);
	audiobarNowPlayingBtn->SetLabel(audiobarNowPlaying[0], 0);
	audiobarNowPlayingBtn->SetLabel(audiobarNowPlaying[1], 1);
	audiobarNowPlayingBtn->SetLabel(audiobarNowPlaying[2], 2);
	audiobarNowPlayingBtn->SetLabel(audiobarNowPlaying[3], 3);
	audiobarNowPlayingBtn->SetSelectable(false);
	audiobarNowPlayingBtn->SetClickable(false);
	audiobarNowPlayingBtn->SetUpdateCallback(AudioNowPlayingCallback);
	audiobarNowPlayingBtn->SetVisible(false);

	audiobar2 = new GuiWindow(300, 80);
	audiobar2->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);

	audiobar2->Append(audiobarLeftImg);
	audiobar2->Append(audiobarMidImg);
	audiobar2->Append(audiobarRightImg);
	audiobar2->Append(audiobarProgressBtn);
	audiobar2->Append(audiobarProgressLeftImg);
	audiobar2->Append(audiobarProgressMidImg);
	audiobar2->Append(audiobarProgressLineImg);
	audiobar2->Append(audiobarProgressRightImg);
	audiobar2->Append(audiobarPlaylistBtn);
	audiobar2->Append(audiobarBackwardBtn);
	audiobar2->Append(audiobarPauseBtn);
	audiobar2->Append(audiobarForwardBtn);
	audiobar2->Append(audiobarModeBtn);

	audiobar = new GuiWindow(screenwidth-60, 80);
	audiobar->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	audiobar->SetPosition(0, -30);
	audiobar->Append(audiobar2);

	audiobar->Append(audiobarNowPlayingBtn);

	// setup picture bar
	
	picturebarLeftImg = new GuiImage(actionbarLeft);
	picturebarMidImg = new GuiImage(actionbarMid);
	picturebarMidImg->SetPosition(20, 0);
	picturebarMidImg->SetTile(9); // 20x9 = 180
	picturebarRightImg = new GuiImage(actionbarRight);
	picturebarRightImg->SetPosition(200, 0);

	picturebarCloseImg = new GuiImage(actionbarClose);
	picturebarCloseImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	picturebarPreviousImg = new GuiImage(actionbarBackward);
	picturebarPreviousImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	picturebarSlideshowImg = new GuiImage(actionbarPlay);
	picturebarSlideshowImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	picturebarNextImg = new GuiImage(actionbarForward);
	picturebarNextImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	picturebarPreviousTip = new GuiTooltip("Previous");
	picturebarNextTip = new GuiTooltip("Next");
	picturebarSlideshowTip = new GuiTooltip("Start Slideshow");
	picturebarCloseTip = new GuiTooltip("Close");
	
	picturebarCloseBtn = new GuiButton(40, 40);
	picturebarCloseBtn->SetPosition(10, 4);
	picturebarCloseBtn->SetImage(picturebarCloseImg);
	picturebarCloseBtn->SetTooltip(picturebarCloseTip);
	picturebarCloseBtn->SetTrigger(trigA);
	picturebarCloseBtn->SetTrigger(trigB);
	picturebarCloseBtn->SetSelectable(false);
	picturebarCloseBtn->SetUpdateCallback(PictureCloseCallback);
	picturebarCloseBtn->SetEffectGrow();

	picturebarPreviousBtn = new GuiButton(40, 40);
	picturebarPreviousBtn->SetPosition(70, 4);
	picturebarPreviousBtn->SetImage(picturebarPreviousImg);
	picturebarPreviousBtn->SetTooltip(picturebarPreviousTip);
	picturebarPreviousBtn->SetTrigger(trigA);
	picturebarPreviousBtn->SetTrigger(trigLeft);
	picturebarPreviousBtn->SetSelectable(false);
	picturebarPreviousBtn->SetUpdateCallback(PicturePreviousCallback);
	picturebarPreviousBtn->SetEffectGrow();
	
	picturebarSlideshowBtn = new GuiButton(40, 40);
	picturebarSlideshowBtn->SetPosition(130, 4);
	picturebarSlideshowBtn->SetImage(picturebarSlideshowImg);
	picturebarSlideshowBtn->SetTooltip(picturebarSlideshowTip);
	picturebarSlideshowBtn->SetTrigger(trigA);
	picturebarSlideshowBtn->SetSelectable(false);
	picturebarSlideshowBtn->SetUpdateCallback(PictureSlideshowCallback);
	picturebarSlideshowBtn->SetEffectGrow();
	
	picturebarNextBtn = new GuiButton(40, 40);
	picturebarNextBtn->SetPosition(190, 4);
	picturebarNextBtn->SetImage(picturebarNextImg);
	picturebarNextBtn->SetTooltip(picturebarNextTip);
	picturebarNextBtn->SetTrigger(trigA);
	picturebarNextBtn->SetTrigger(trigRight);
	picturebarNextBtn->SetSelectable(false);
	picturebarNextBtn->SetUpdateCallback(PictureNextCallback);
	picturebarNextBtn->SetEffectGrow();
	
	picturebar = new GuiWindow(240, 48);
	picturebar->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	picturebar->SetPosition(0, -30);

	picturebar->Append(picturebarLeftImg);
	picturebar->Append(picturebarMidImg);
	picturebar->Append(picturebarRightImg);
	picturebar->Append(picturebarCloseBtn);
	picturebar->Append(picturebarPreviousBtn);
	picturebar->Append(picturebarSlideshowBtn);
	picturebar->Append(picturebarNextBtn);

	pictureImg = new GuiImage;
	pictureImg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	pictureBtn = new GuiButton(0, 0);
	pictureBtn->SetImage(pictureImg);
	pictureBtn->SetTrigger(trigA);
	pictureBtn->SetSelectable(false);
	pictureBtn->SetState(STATE_DISABLED);
	pictureBtn->SetVisible(false);
	pictureBtn->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	// initialize pictures struct
	for(int i=0; i < NUM_PICTURES; i++)
	{
		pictures[i].image = NULL;
		pictures[i].index = -1;
	}

	statusText = new GuiText(NULL, 24, (GXColor){255, 255, 255, 255});
	statusText->SetVisible(false);

	guiSetup = 1;
}

void GuiInit()
{
	guiHalt = 1;
	LWP_CreateThread (&guithread, GuiThread, NULL, guistack, GSTACK, 60);
}

static void StartGuiThreads()
{
	showProgress = 0;
	progressThreadHalt = 1;
	pictureThreadHalt = 1;
	creditsThreadHalt = 1;
	updateThreadHalt = 1;
	
	LWP_CreateThread (&progressthread, ProgressThread, NULL, NULL, 0, 60);
	LWP_CreateThread (&picturethread, PictureThread, NULL, NULL, 0, 60);
	LWP_CreateThread (&creditsthread, CreditsThread, NULL, NULL, 0, 60);
	LWP_CreateThread (&updatethread, UpdateThread, NULL, NULL, 0, 60);
}

static void StopGuiThreads()
{
	showProgress = 0;
	progressThreadHalt = 2;

	if(progressthread != LWP_THREAD_NULL)
	{
		if(LWP_ThreadIsSuspended(progressthread))
			LWP_ResumeThread (progressthread);
		
		// wait for thread to finish
		LWP_JoinThread(progressthread, NULL);
		progressthread = LWP_THREAD_NULL;
	}

	pictureThreadHalt = 2;

	if(picturethread != LWP_THREAD_NULL)
	{
		if(LWP_ThreadIsSuspended(picturethread))
			LWP_ResumeThread (picturethread);
		
		// wait for thread to finish
		LWP_JoinThread(picturethread, NULL);
		picturethread = LWP_THREAD_NULL;
	}

	creditsThreadHalt = 2;

	if(creditsthread != LWP_THREAD_NULL)
	{
		if(LWP_ThreadIsSuspended(creditsthread))
			LWP_ResumeThread (creditsthread);
		
		// wait for thread to finish
		LWP_JoinThread(creditsthread, NULL);
		creditsthread = LWP_THREAD_NULL;
	}

	updateThreadHalt = 2;

	if(updatethread != LWP_THREAD_NULL)
	{
		if(LWP_ThreadIsSuspended(updatethread))
			LWP_ResumeThread (updatethread);
		
		// wait for thread to finish
		LWP_JoinThread(updatethread, NULL);
		updatethread = LWP_THREAD_NULL;
	}
}

/****************************************************************************
 * Menu
 ***************************************************************************/
void WiiMenu()
{
	menuMode = 0; // switch to normal GUI mode
	guiShutdown = false;

	SetupGui();

	mainWindow = new GuiWindow(screenwidth, screenheight);

	if(videoScreenshot)
	{
		videoImg = new GuiImage(videoScreenshot, vmode->fbWidth, vmode->viHeight);
		videoImg->SetScaleX(screenwidth/(float)vmode->fbWidth);
		videoImg->SetScaleY(screenheight/(float)vmode->efbHeight);
		mainWindow->Append(videoImg);
		selectLoadedFile = 1; // video loaded - trigger browser to jump to it
	}

	GuiImage bgImg(bg);
	bgImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	bgImg.SetAlpha(200);
	GuiImageData navDivider(nav_divider_png);
	GuiImage navDividerImg(&navDivider);
	navDividerImg.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	navDividerImg.SetPosition(0, 85);

	mainWindow->Append(&bgImg);
	mainWindow->Append(&navDividerImg);

	GuiTooltip logoBtnTip("Credits");
	GuiImageData logo(logo_png);
	GuiImage logoBtnImg(&logo);
	logoBtn = new GuiButton(logo.GetWidth(), logo.GetHeight());
	logoBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	logoBtn->SetPosition(-32, 40);
	logoBtn->SetImage(&logoBtnImg);
	logoBtn->SetTrigger(trigA);
	logoBtn->SetSelectable(false);
	logoBtn->SetTooltip(&logoBtnTip);
	logoBtn->SetUpdateCallback(DisplayCredits);
	mainWindow->Append(logoBtn);

	GuiImageData navHighlight(nav_highlight_png);
	GuiImageData videos(nav_videos_png);
	GuiImageData videosOver(nav_videos_over_png);
	GuiImageData videosOn(nav_videos_on_png);
	GuiImageData music(nav_music_png);
	GuiImageData musicOver(nav_music_over_png);
	GuiImageData musicOn(nav_music_on_png);
	GuiImageData pictures(nav_pictures_png);
	GuiImageData picturesOver(nav_pictures_over_png);
	GuiImageData picturesOn(nav_pictures_on_png);
	GuiImageData dvd(nav_dvd_png);
	GuiImageData dvdOver(nav_dvd_over_png);
	GuiImageData dvdOn(nav_dvd_on_png);
	GuiImageData online(nav_onlinemedia_png);
	GuiImageData onlineOver(nav_onlinemedia_over_png);
	GuiImageData onlineOn(nav_onlinemedia_on_png);
	GuiImageData settings(nav_settings_png);
	GuiImageData settingsOver(nav_settings_over_png);
	GuiImageData settingsOn(nav_settings_on_png);

	GuiTooltip videosBtnTip("Videos");
	videosBtnImg = new GuiImage(&videos);
	videosBtnOnImg = new GuiImage(&videosOn);
	GuiImage videosBtnOverImg(&videosOver);
	GuiImage videosBtnHighlightImg(&navHighlight);
	videosBtnHighlightImg.SetPosition(-20, 30);
	videosBtnHighlightImg.SetAlpha(128);
	videosBtn = new GuiButton(videosBtnImg->GetWidth(), videosBtnImg->GetHeight());
	videosBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	videosBtn->SetPosition(30, 30);
	videosBtn->SetTooltip(&videosBtnTip);
	videosBtn->SetImage(videosBtnImg);
	videosBtn->SetImageOver(&videosBtnOverImg);
	videosBtn->SetIconOver(&videosBtnHighlightImg);
	videosBtn->SetTrigger(trigA);
	videosBtn->SetSelectable(false);
	videosBtn->SetEffectGrow();
	videosBtn->SetUpdateCallback(ChangeMenuVideos);

	GuiTooltip musicBtnTip("Music");
	musicBtnImg = new GuiImage(&music);
	musicBtnOnImg = new GuiImage(&musicOn);
	GuiImage musicBtnOverImg(&musicOver);
	GuiImage musicBtnHighlightImg(&navHighlight);
	musicBtnHighlightImg.SetPosition(-20, 30);
	musicBtnHighlightImg.SetAlpha(128);
	musicBtn = new GuiButton(musicBtnImg->GetWidth(), musicBtnImg->GetHeight());
	musicBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	musicBtn->SetPosition(85, 30);
	musicBtn->SetTooltip(&musicBtnTip);
	musicBtn->SetImage(musicBtnImg);
	musicBtn->SetImageOver(&musicBtnOverImg);
	musicBtn->SetIconOver(&musicBtnHighlightImg);
	musicBtn->SetTrigger(trigA);
	musicBtn->SetSelectable(false);
	musicBtn->SetEffectGrow();
	musicBtn->SetUpdateCallback(ChangeMenuMusic);

	GuiTooltip picturesBtnTip("Pictures");
	picturesBtnImg = new GuiImage(&pictures);
	picturesBtnOnImg = new GuiImage(&picturesOn);
	GuiImage picturesBtnOverImg(&picturesOver);
	GuiImage picturesBtnHighlightImg(&navHighlight);
	picturesBtnHighlightImg.SetPosition(-20, 30);
	picturesBtnHighlightImg.SetAlpha(128);
	picturesBtn = new GuiButton(picturesBtnImg->GetWidth(), picturesBtnImg->GetHeight());
	picturesBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	picturesBtn->SetPosition(140, 30);
	picturesBtn->SetTooltip(&picturesBtnTip);
	picturesBtn->SetImage(picturesBtnImg);
	picturesBtn->SetImageOver(&picturesBtnOverImg);
	picturesBtn->SetIconOver(&picturesBtnHighlightImg);
	picturesBtn->SetTrigger(trigA);
	picturesBtn->SetSelectable(false);
	picturesBtn->SetEffectGrow();
	picturesBtn->SetUpdateCallback(ChangeMenuPictures);

	GuiTooltip dvdBtnTip("DVD");
	dvdBtnImg = new GuiImage(&dvd);
	dvdBtnOnImg = new GuiImage(&dvdOn);
	GuiImage dvdBtnOverImg(&dvdOver);
	GuiImage dvdBtnHighlightImg(&navHighlight);
	dvdBtnHighlightImg.SetPosition(-20, 30);
	dvdBtnHighlightImg.SetAlpha(128);
	dvdBtn = new GuiButton(dvdBtnImg->GetWidth(), dvdBtnImg->GetHeight());
	dvdBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	dvdBtn->SetPosition(195, 30);
	dvdBtn->SetTooltip(&dvdBtnTip);
	dvdBtn->SetImage(dvdBtnImg);
	dvdBtn->SetImageOver(&dvdBtnOverImg);
	dvdBtn->SetIconOver(&dvdBtnHighlightImg);
	dvdBtn->SetTrigger(trigA);
	dvdBtn->SetSelectable(false);
	dvdBtn->SetEffectGrow();
	dvdBtn->SetUpdateCallback(ChangeMenuDVD);

	GuiTooltip onlineBtnTip("Online Media");
	onlineBtnImg = new GuiImage(&online);
	onlineBtnOnImg = new GuiImage(&onlineOn);
	GuiImage onlineBtnOverImg(&onlineOver);
	GuiImage onlineBtnHighlightImg(&navHighlight);
	onlineBtnHighlightImg.SetPosition(-20, 30);
	onlineBtnHighlightImg.SetAlpha(128);
	onlineBtn = new GuiButton(onlineBtnImg->GetWidth(), onlineBtnImg->GetHeight());
	onlineBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	onlineBtn->SetPosition(250, 30);
	onlineBtn->SetTooltip(&onlineBtnTip);
	onlineBtn->SetImage(onlineBtnImg);
	onlineBtn->SetImageOver(&onlineBtnOverImg);
	onlineBtn->SetIconOver(&onlineBtnHighlightImg);
	onlineBtn->SetTrigger(trigA);
	onlineBtn->SetSelectable(false);
	onlineBtn->SetEffectGrow();
	onlineBtn->SetUpdateCallback(ChangeMenuOnline);

	GuiTooltip settingsBtnTip("Settings");
	settingsBtnImg = new GuiImage(&settings);
	settingsBtnOnImg = new GuiImage(&settingsOn);
	GuiImage settingsBtnOverImg(&settingsOver);
	GuiImage settingsBtnHighlightImg(&navHighlight);
	settingsBtnHighlightImg.SetPosition(-20, 30);
	settingsBtnHighlightImg.SetAlpha(128);
	settingsBtn = new GuiButton(settingsBtnImg->GetWidth(), settingsBtnImg->GetHeight());
	settingsBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	settingsBtn->SetPosition(-200, 30);
	settingsBtn->SetImage(settingsBtnImg);
	settingsBtn->SetImageOver(&settingsBtnOverImg);
	settingsBtn->SetIconOver(&settingsBtnHighlightImg);
	settingsBtn->SetTooltip(&settingsBtnTip);
	settingsBtn->SetTrigger(trigA);
	settingsBtn->SetSelectable(false);
	settingsBtn->SetEffectGrow();
	settingsBtn->SetUpdateCallback(ChangeMenuSettings);

	UpdateMenuImages(-1, menuCurrent);

	mainWindow->Append(videosBtn);
	mainWindow->Append(musicBtn);
	mainWindow->Append(picturesBtn);
	mainWindow->Append(dvdBtn);
	mainWindow->Append(onlineBtn);
	mainWindow->Append(settingsBtn);

	StartGuiThreads();
	ResumeGui();
	EnableRumble();

	// Load settings (only happens once)
	if(!LoadSettings())
	{
		if(!SaveSettings(NOTSILENT))
		{
			ExitRequested = 2;
			while(1) usleep(THREAD_SLEEP);
		}
	}

	// Init MPlayer path and vars (only happens once)
	if(!InitMPlayer())
	{
		ExitRequested = 2;
		while(1) usleep(THREAD_SLEEP);
	}

	while(!guiShutdown)
	{
		switch (menuCurrent)
		{
			case MENU_BROWSE_VIDEOS:
			case MENU_BROWSE_MUSIC:
			case MENU_BROWSE_ONLINEMEDIA:
				MenuBrowse(menuCurrent);
				break;
			case MENU_BROWSE_PICTURES:
				MenuBrowsePictures();
				break;
			case MENU_DVD:
				MenuDVD();
				break;
			case MENU_SETTINGS:
				MenuSettings();
				break;
			case MENU_SETTINGS_GLOBAL:
				MenuSettingsGlobal();
				break;
			case MENU_SETTINGS_VIDEOS:
				MenuSettingsVideos();
				break;
			case MENU_SETTINGS_MUSIC:
				MenuSettingsMusic();
				break;
			case MENU_SETTINGS_PICTURES:
				MenuSettingsPictures();
				break;
			case MENU_SETTINGS_ONLINEMEDIA:
				MenuSettingsOnlineMedia();
				break;
			case MENU_SETTINGS_DVD:
				MenuSettingsDVD();
				break;
			case MENU_SETTINGS_NETWORK:
				MenuSettingsNetwork();
				break;
			case MENU_SETTINGS_NETWORK_SMB:
				MenuSettingsNetworkSMB();
				break;
			case MENU_SETTINGS_NETWORK_FTP:
				MenuSettingsNetworkFTP();
				break;
			case MENU_SETTINGS_SUBTITLES:
				MenuSettingsSubtitles();
				break;
			default: // unrecognized menu
				MenuBrowse(MENU_BROWSE_VIDEOS);
				break;
		}
		
		usleep(THREAD_SLEEP);
	}

	StopGuiThreads();
	SuspendGui();
	DisableRumble();

	delete mainWindow;
	mainWindow = NULL;

	delete videosBtn;
	videosBtn = NULL;
	delete musicBtn;
	musicBtn = NULL;
	delete picturesBtn;
	picturesBtn = NULL;
	delete dvdBtn;
	dvdBtn = NULL;
	delete onlineBtn;
	onlineBtn = NULL;
	delete settingsBtn;
	settingsBtn = NULL;
	
	delete videosBtnImg;
	delete videosBtnOnImg;
	delete musicBtnImg;
	delete musicBtnOnImg;
	delete picturesBtnImg;
	delete picturesBtnOnImg;
	delete dvdBtnImg;
	delete dvdBtnOnImg;
	delete onlineBtnImg;
	delete onlineBtnOnImg;
	delete settingsBtnImg;
	delete settingsBtnOnImg;

	delete logoBtn;
	logoBtn = NULL;

	if(videoImg)
	{
		delete videoImg;
		videoImg = NULL;
	}
	if(videoScreenshot)
	{
		free(videoScreenshot);
		videoScreenshot = NULL;
	}
	if(nowPlaying)
	{
		delete nowPlaying;
		nowPlaying = NULL;
	}
}

bool BufferingStatusSet()
{
	return statusText->IsVisible();
}

void SetBufferingStatus(int s)
{
	if(s == 0)
	{
		statusText->SetVisible(false);
		return;
	}

	statusText->SetVisible(true);
	wchar_t txt[50];
	swprintf(txt, 50, L"%s (%02d%%)", gettext("Buffering"), s);
	statusText->SetWText(txt);
}

/****************************************************************************
 * MPlayer Menu
 ***************************************************************************/
void MPlayerMenu()
{
	guiShutdown = false;

	mainWindow = new GuiWindow(screenwidth, screenheight);

	mainWindow->Append(videobar);
	mainWindow->Append(statusText);
	mainWindow->SetVisible(false);
	mainWindow->SetState(STATE_DISABLED);
	HideVolumeLevelBar();
	menuMode = 1; // switch to MPlayer GUI mode
	EnableRumble();

	videoPaused = !wiiIsPaused();

	while(controlledbygui == 0)
		usleep(300000);

	DisableRumble();

	delete mainWindow;
	mainWindow = NULL;
}