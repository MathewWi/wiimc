/****************************************************************************
 * WiiMC
 * Tantric 2009-2010
 ***************************************************************************/

#include <malloc.h>
#include <stdlib.h>
#include <ogc/machine/processor.h>
#include <sys/iosupport.h>

#include "pngu.h"
#include "fileop.h"

int mload_init();
int mload_close();
bool load_ehci_module();
void USB2Enable(bool e);
void USB2Storage_Close();
int GetUSB2LanPort();
void SetUSB2Mode(int mode); 
extern void __exception_closeall();
typedef void (*entrypoint) (void);
u32 load_dol_image (void *dolstart, struct __argv *argv);

extern const u8		background_png[];
extern const u32	background_png_size;
extern const u8		background_wide_png[];
extern const u32	background_wide_png_size;

void InitVideo();
void StopGX();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF );

static bool FindIOS(u32 ios)
{
	s32 ret;
	u32 n;

	u64 *titles = NULL;
	u32 num_titles=0;

	ret = ES_GetNumTitles(&num_titles);
	if (ret < 0)
		return false;

	if(num_titles < 1) 
		return false;

	titles = (u64 *)memalign(32, num_titles * sizeof(u64) + 32);
	if (!titles)
		return false;

	ret = ES_GetTitles(titles, num_titles);
	if (ret < 0)
	{
		free(titles);
		return false;
	}
		
	for(n=0; n < num_titles; n++)
	{
		if((titles[n] & 0xFFFFFFFF)==ios) 
		{
			free(titles); 
			return true;
		}
	}
    free(titles); 
	return false;
}

#define ROUNDDOWN32(v)				(((u32)(v)-0x1f)&~0x1f)

static bool USBLANDetected()
{
	u8 dummy;
	u8 i;
	u16 vid, pid;

	USB_Initialize();
	u8 *buffer = (u8*)ROUNDDOWN32(((u32)SYS_GetArena2Hi() - (32*1024)));
	memset(buffer, 0, 8 << 3);

	if(USB_GetDeviceList("/dev/usb/oh0", buffer, 8, 0, &dummy) < 0)
		return false;

	for(i = 0; i < 8; i++)
	{
		memcpy(&vid, (buffer + (i << 3) + 4), 2);
		memcpy(&pid, (buffer + (i << 3) + 6), 2);
		if( (vid == 0x0b95) && (pid == 0x7720))
			return true;
	}
	return false;
}

static ssize_t __out_write(struct _reent *r, int fd, const char *ptr, size_t len)
{
	return -1;
}

const devoptab_t phony_out = 
{ "stdout",0,NULL,NULL,__out_write,
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, NULL,0,NULL,NULL,NULL,NULL,NULL };

int main(int argc, char **argv)
{
	void *buffer = (void *)0x92000000;
	devoptab_list[STD_OUT] = &phony_out; // to keep libntfs happy
	devoptab_list[STD_ERR] = &phony_out; // to keep libntfs happy

	bool usblan = USBLANDetected();

	// try to load IOS 202
	if(FindIOS(202))
		IOS_ReloadIOS(202);
	else if(IOS_GetVersion() < 61 && FindIOS(61))
		IOS_ReloadIOS(61);

	if(IOS_GetVersion() == 202)
	{
		// enable DVD and USB2
		if(mload_init() >= 0 && load_ehci_module())
		{
			int mode = 3;

			if(usblan)
			{
				int usblanport = GetUSB2LanPort();

				if(usblanport >= 0)
				{
					if(usblanport == 1)
						mode = 1;
					else
						mode = 2;

					USB2Storage_Close();
					mload_close();
					IOS_ReloadIOS(202);
					mload_init();
					load_ehci_module();
				}
			}
			SetUSB2Mode(mode);
			USB2Enable(true);
		}
	}

	InitVideo();

	u8 *bg;
	int bgWidth, bgHeight;
	int a,i,j;

	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		bg = DecodePNG(background_wide_png, &bgWidth, &bgHeight);
	else
		bg = DecodePNG(background_png, &bgWidth, &bgHeight);

	for(a = 0; a <= 255; a+=15)
	{
		Menu_DrawImg(0, 0, bgWidth, bgHeight, bg, 0, 1, 1, a);
		Menu_Render();
	}

	// mount devices and look for file
	MountAllDevices();
	char filepath[1024] = { 0 };
	FILE *fp = NULL;

	for(i=0; i < 2; i++)
	{
		for(j=0; j < MAX_DEVICES; j++)
		{
			if(part[i][j].type == 0)
				continue;

			sprintf(filepath, "%s:/apps/wiimc/boot.dol", part[i][j].mount);
			fp = fopen(filepath, "rb");
			if(fp)
				goto found;
		}
	}

	if(!fp)
	{
		StopGX();
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}
found:
	fseek (fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek (fp, 0, SEEK_SET);
	fread(buffer, 1, len, fp);
	fclose (fp);
	UnmountAllDevices();

	// load entry point
	struct __argv args;
	bzero(&args, sizeof(args));
	args.argvMagic = ARGV_MAGIC;
	args.length = strlen(filepath) + 2;
	args.commandLine = (char*)malloc(args.length);
	strcpy(args.commandLine, filepath);
	args.commandLine[args.length - 1] = '\0';
	args.argc = 1;
	args.argv = &args.commandLine;
	args.endARGV = args.argv + 1;

	u32 exeEntryPointAddress = load_dol_image(buffer, &args);

	entrypoint exeEntryPoint = (entrypoint) exeEntryPointAddress;

	for(a = 255; a >= 0; a-=15)
	{
		Menu_DrawImg(0, 0, bgWidth, bgHeight, bg, 0, 1, 1, a);
		Menu_Render();
	}

	StopGX();
	VIDEO_WaitVSync();

	// cleanup
	if(IOS_GetVersion() == 202)
		mload_close();

	u32 level;
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable(level);
	__exception_closeall();
	exeEntryPoint();
	_CPU_ISR_Restore(level);
	return 0;
}
