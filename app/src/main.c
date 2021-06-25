#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vitasdk.h>
#include <taihen.h>

#include <cex2rex_user.h>

#include "ctrl.h"
#include "debugScreen.h"
#include "pup.h"

#define printf psvDebugScreenPrintf
 
int ret;
int left = -1;

int getFileSize(const char *file) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);
	sceIoClose(fd);
	return fileSize;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDWR | SCE_O_CREAT, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0777);
	if (fd < 0)
		return fd;

	int readed = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return readed;
}

int CopyFile(char *src, char *dst)
{
	int size = getFileSize(src);
	char *data = malloc(size);
	memset(data,0,size);
	ret = ReadFile(src,data,size);
	if(ret < 0){
			psvDebugScreenPrintf("ReadFile() failed. ret = 0x%x\n", ret);
			while(1){};
	}
	ret = WriteFile(dst,data,size);
	if(ret < 0){
			psvDebugScreenPrintf("WriteFile() failed. ret = 0x%x\n", ret);
	}
	return 0;
}

void load_modules()
{
	char titleid[12];
	char kplugin_path[0x200];
	char uplugin_path[0x200];
	
	sceAppMgrUmount("app0:");
	sceAppMgrAppParamGetString(0, 12, titleid , 256);

	sprintf(kplugin_path, "ux0:app/%s/kernel.skprx", titleid);
	sprintf(uplugin_path, "ux0:app/%s/user.suprx", titleid);
	printf("kplugin_path: %s\n",kplugin_path);
	printf("uplugin_path: %s\n",uplugin_path);
	int kernel_modid, user_modid;

	kernel_modid = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
	if(kernel_modid < 0){
			printf("taiLoadStartKernelModule() failed. ret = 0x%x\n", kernel_modid);
	}	
	user_modid = sceKernelLoadStartModule(uplugin_path, 0, NULL, 0, NULL, NULL);
	if(user_modid < 0){
			printf("sceKernelLoadStartModule() failed. ret = 0x%x\n", user_modid);
	}
	psvDebugScreenClear(0);
	return;	
}


void config()
{
	int spoofer = 0;
	int activator = 0;
	psvDebugScreenClear(0);
		
	psvDebugScreenPrintf("Installation Type\n");
	psvDebugScreenPrintf("X: Full Install (spoofer + activator + testkit vsh)\n");
	psvDebugScreenPrintf("O: Warning Message Install (spoofer + testkit vsh)\n");
	psvDebugScreenPrintf("[]: Enso-less Install (testkit vsh)\n");
	
	sceKernelDelayThread(100000);

	switch(get_key(0)) {
			case SCE_CTRL_CROSS:
				activator = 1;
				spoofer = 1;
				break;
			case SCE_CTRL_CIRCLE:
				spoofer = 1;
				break;
			case SCE_CTRL_SQUARE:
				break;
			default:
				sceKernelExitProcess(0);
				break;
			}
	
	
	psvDebugScreenClear();
	sceIoRemove("ur0:tai/testkit.skprx");
	sceIoRemove("ur0:tai/kmspico.skprx");
	
	if(spoofer)
	{
		psvDebugScreenPrintf("Writing testkit.skprx");
		CopyFile("app0:/testkit.skprx","ur0:tai/testkit.skprx");
		
		//int pstv = vshSblAimgrIsGenuineDolce(); - my PSTV was not genuine :'( I need to buy a product key for it.. 
		//In the meantime, sceKernelIsPSVitaTV() it is!
		int pstv = sceKernelIsPSVitaTV();
		if(pstv)
		{
			CopyFile("app0:/pstvConfig.txt","ur0:tai/boot_config.txt");
		}
		else
		{
			CopyFile("app0:/vitaConfig.txt","ur0:tai/boot_config.txt");
		}
	}
	
	if(activator)
	{
		psvDebugScreenPrintf("Writing kmspico.skprx");
		CopyFile("app0:/kmspico.skprx","ur0:tai/kmspico.skprx");
	}
}

void cleanup()
{
	char path[0x1028];
	SceUID dfd = sceIoDopen("ur0:/pup_out");
	if (dfd >= 0) {
		int ret = 0;
		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));
			ret = sceIoDread(dfd, &dir);
			if (ret > 0) { 
				snprintf(path, 0x1028, "ur0:/pup_out/%s", dir.d_name);
				sceIoRemove(path);
			}
		}
		while(ret > 0);
	}
	sceIoRemove("ur0:/vs0.img");
	sceIoRmdir("ur0:/pup_out");
}

void lock()
{
	sceShellUtilInitEvents(0);
	sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED |
					SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER |
					SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
	scePowerSetCallbackState(0); // Magical NID engineered by FAPS TEAM ;)
}

void extract()
{
	cleanup();
	psvDebugScreenClear();
	psvDebugScreenPrintf("Extracting DEX.PUP...\n");
	scePlayStartionUpdatePackageExtractStage1("ux0:/DEX.PUP");
}

void vercheck()
{
	static char version[16];
	char target_version[16];
	
	memset(version, 0x00, 16);
	memset(target_version, 0x00, 16);
	
	psvDebugScreenClear();
	
	SceKernelFwInfo data;
	data.size = sizeof(SceKernelFwInfo);
	
	_vshSblGetSystemSwVersion(&data);
	
	snprintf(version, 16, "%s", data.versionString);
	ReadFile("ur0:/pup_out/version.txt",target_version,4);
	
	if(strcmp(target_version,version) != 0)
	{
		psvDebugScreenPrintf("ux0:/DEX.PUP version is %s\nBut your current version is %s. cannot continue\nPlease update your console to retail %s first.\nor download the DEX %s.PUP",target_version,version,target_version,version);
		sceKernelDelayThread(10000000);
		cleanup();
		sceKernelExitProcess(0);
	}
	
}

void decrypt()
{
	char path[0x1028];
	char pathout[0x1028];
	
	int vs0Count = 0;
	int type = 0;

	psvDebugScreenClear();
	psvDebugScreenPrintf("Decrypting DEX.PUP\n");
	
	for(int i = 0; i <= 26; i++)
	{
		memset(path,0x00,0x1028);
		memset(pathout,0x00,0x1028);
		sprintf(path,"ur0:/pup_out/package_data%02i.pkg",i);
		sprintf(pathout,"ur0:/pup_out/pkg-%i.dec",i);
		int sz = getFileSize(path);
		if(sz > 0)
		{
			
			psvDebugScreenPrintf("Decrypting %s\n",path);
			int ret = DecryptSpkgFile(path,pathout);
			if(ret < 0)
			{
				psvDebugScreenClear();
				psvDebugScreenPrintf("Error decrypting %s",path);
				sceKernelDelayThread(10000000);
				cleanup();
				sceKernelExitProcess(0);
			}
			sceIoRemove(path);
			
			int fd = sceIoOpen(pathout, SCE_O_RDONLY, 0777);
			sceIoLseek(fd,0x404,SCE_SEEK_SET);
			sceIoRead(fd,&type, sizeof(int));
			sceIoClose(fd);
			
			if(type == 0x0A)
			{
				char newName[0x1028];
				memset(newName,0x00,0x1028);
				sprintf(newName,"ur0:/pup_out/vs0-%i.dec",vs0Count);
				sceIoRename(pathout, newName);
				vs0Count += 1;
			}
			sceIoRemove(pathout);
			
		}
		
	}
	return;
}

void join()
{
	psvDebugScreenClear();
	psvDebugScreenPrintf("Concatenating package data...\n");
	char path[0x1028];
	int fd = 0;
	int sz = 0;
	char buffer[0x8000];
	int vs0Img = sceIoOpen("ur0:/vs0.img", SCE_O_RDWR | SCE_O_CREAT, 0777);
	
	for(int i = 0; i <= 26; i++)
	{
		memset(path,0x00,0x1028);
		sprintf(path,"ur0:/pup_out/vs0-%i.dec",i);
		psvDebugScreenPrintf("Adding %s to vs0.img\n",path);
		
		sz = getFileSize(path) - 0x480;
		
		fd = sceIoOpen(path,SCE_O_RDONLY, 0777);
		sceIoLseek(fd, 0x480, SCE_SEEK_SET);
	
		while(!(sz <= 0))
		{
			memset(buffer,0x00,0x8000);
			if(sz < 0x8000)
			{
				sceIoRead(fd, buffer, sz);
				ret = sceIoWrite(vs0Img, buffer, sz);
				sz -= sz;
			}
			else
			{
				sceIoRead(fd, buffer, 0x8000);
				ret = sceIoWrite(vs0Img, buffer, 0x8000);
				sz -= 0x8000;
			}
		}
		sceIoClose(fd);
		sceIoRemove(path);
		
	}
	sceIoClose(vs0Img);
	psvDebugScreenClear();
}

void restore()
{
	psvDebugScreenClear();
	if(getFileSize("ur0:/vs0-orig.img") < 0){
		psvDebugScreenPrintf("ur0:/vs0-orig.img MISSING, cannot restore\nIf you must go back,\nReinstall your firmware w official updater");
		sceKernelDelayThread(10000000);
		sceKernelExitProcess(0);
	}
	
	psvDebugScreenPrintf("Beginning restore in 10 seconds,\nDO NOT TURN OFF YOUR CONSOLE\nOR EXIT THIS APP WHILE RESTORING!\n");
	sceKernelDelayThread(10000000);
	
	psvDebugScreenClear();
	
	dd("ur0:/vs0-orig.img","sdstor0:int-lp-ign-vsh", 0x10000000); // start dd
	do
	{
		left = getTotalLeft();
		
		int total = 0x10000000 - left;
		float percent = (float)total / 268435456 * 100.0;
		
		psvDebugScreenPrintf("Restoring ur0:/vs0-orig.img %i/268435456 - %.2f%%\n",total, percent);
		
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
		coordX = 0;
		coordY = 0;
	}
	while(left != 0x50000001);

	psvDebugScreenClear();
	
	sceIoRemove("ur0:tai/testkit.skprx");
	sceIoRemove("ur0:tai/kmspico.skprx");	
	sceIoRemove("ur0:/vs0-orig.img");
	
	psvDebugScreenClear();
	psvDebugScreenPrintf("Restore complete~ Your console is now a retail again");
	sceKernelDelayThread(10000000);
	scePowerRequestColdReset();
}

void backup()
{
	sceIoRemove("ur0:/vs0-orig.img");
	WriteFile("ur0:/vs0-orig.img",NULL,0x00); // create vs0.img
	
	dd("sdstor0:int-lp-ign-vsh","ur0:/vs0-orig.img", 0x10000000); //start dd
	do
	{
		left = getTotalLeft();
		
		int total = 0x10000000 - left;
		float percent = (float)total / 268435456 * 100.0;
		
		psvDebugScreenPrintf("Creating ur0:/vs0-orig.img %i/268435456 - %.2f%%\n",total, percent);
		
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
		coordX = 0;
		coordY = 0;
	}
	while(left != 0x50000001);
	
	psvDebugScreenClear();
}

void flash()
{
	psvDebugScreenPrintf("Beginning flash in 10 seconds,\nDO NOT TURN OFF YOUR CONSOLE OR EXIT THIS APP WHILE FLASHING\n");
	sceKernelDelayThread(10000000);
						
	psvDebugScreenClear();
	
	dd("ur0:/vs0.img","sdstor0:int-lp-ign-vsh", 0x10000000); // start dd	
	do
	{
		left = getTotalLeft();
		
		int total = 0x10000000 - left;
		float percent = (float)total / 268435456 * 100.0;
		
		psvDebugScreenPrintf("Flashing ur0:/vs0.img %i/268435456 - %.2f%%\n",total, percent);
		
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
		coordX = 0;
		coordY = 0;
	}
	while(left != 0x50000001);
	
	cleanup();
	psvDebugScreenClear();
	psvDebugScreenPrintf("Flash complete~ Your console is now a \"testkit\"");
	sceKernelDelayThread(10000000);
	scePowerRequestColdReset();
}

void uninstall()
{
	load_modules();
	lock();
	restore();
}

void install()
{
	extract();
	vercheck();
	config();
	load_modules();
	lock();
	decrypt();
	join();
	backup();
	flash();
}

int isRex()
{
	if(getFileSize("vs0:/app/NPXS10104/eboot.bin") > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void copyappinfo() // Setup Epic "interlectural property notices" thing
{
	if(getFileSize("ux0:/psm/CEXR20202/RO/Application/app.info") != getFileSize("app0:/app.info"))
	{
		sceIoMkdir("ux0:/psm", 0777);
		sceIoMkdir("ux0:/psm/CEXR20202", 0777);
		sceIoMkdir("ux0:/psm/CEXR20202/RO", 0777);
		sceIoMkdir("ux0:/psm/CEXR20202/RO/Application", 0777);
		CopyFile("app0:/app.info","ux0:/psm/CEXR20202/RO/Application/app.info");
	}
}


int main() {
		copyappinfo();
		psvDebugScreenInit();
        psvDebugScreenClear(0);
			
		int pup_type = 0;
		int fd = sceIoOpen("ux0:/DEX.PUP", SCE_O_RDONLY, 0777);
		if(fd > 0)
		{
			sceIoLseek(fd,0x3c,SCE_SEEK_SET);
			sceIoRead(fd,&pup_type, sizeof(int));
			sceIoClose(fd);
			if(pup_type != 0x4)
			{
				psvDebugScreenPrintf("ux0:/DEX.PUP is NOT a TESTKIT UPDATE! (pup_type 0x%x expected 0x4)",pup_type);
			}
		}
		else
		{
			char version[16];
			SceKernelFwInfo data;
			data.size = sizeof(SceKernelFwInfo);
			_vshSblGetSystemSwVersion(&data);
			snprintf(version, 16, "%s", data.versionString);
			
			psvDebugScreenPrintf("ux0:/DEX.PUP NOT FOUND!\nPlease download the DEX %s firmware update file (.PUP)\nAnd place it in ux0:/DEX.PUP",version);
			while(1){};
		}
		
		
		if(isRex() == 0)
		{
			psvDebugScreenPrintf("CEX-2-REX 2.0 - Currently CEX\n");
			psvDebugScreenPrintf("X: Install TestKit Firmware\n");
			
			sceKernelDelayThread(100000);
			while(1)
			{
				switch(get_key(0)) {
						case SCE_CTRL_CROSS:
							install();
							break;
						default:
							break;
						}
			}
		}
		else
		{
			psvDebugScreenPrintf("CEX-2-REX 2.0 - Currently REX\n");
			psvDebugScreenPrintf("X: Uninstall TestKit Firmware\n");
			psvDebugScreenPrintf("O: Change Installation Type\n");
			
			sceKernelDelayThread(100000);
			while(1)
			{
				switch(get_key(0)) {
						case SCE_CTRL_CROSS:
							uninstall();
							break;
						case SCE_CTRL_CIRCLE:
							config();
							scePowerRequestColdReset();
							break;
						default:
							break;
						}
			}
		}
		
        

        

        
    return 0;
}