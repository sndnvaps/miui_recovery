/*
*  Copyright (c) 2013 The software  Author sndnvaps. 
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of  sndnvaps nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>

#include "../miui_inter.h"
#include "../miui.h"
#include "../utils.h" 
#include "../../../miui_intent.h"
#include "../../../iniparser/iniparser.h" // for iniparser 

#include "../libs/miui_screen.h"
#include "../libs/miui_libs.h"

#include "../../../minadbd/adb.h" //for ADB_SIDELOAD_FILENAME 
#include "../../../sideload.h"

#define ROOT_DEVICE 0x8
#define DISABLE_OFFICAL_REC 0x9
#define FREE_SDCARD_SPACE 0xA


//INTENT_RUN_ORS ,1, filename
//
#define AUTHOR_INFO "/tmp/author_info.log"


#define MIUI_SETTINGS_FILE "/sdcard/miui_recovery/settings.ini"

dictionary * ini;

int load_settings()
{
    miuiIntent_send(INTENT_MOUNT, 1, "/sdcard");
    ini = iniparser_load(MIUI_SETTINGS_FILE);
    if (ini==NULL)
        return 1;
    
    return 0;
}

void write_settings()
{
    char *ini_name;
    ini_name = "/sdcard/miui_recovery/settings.ini";
    iniparser_dump_ini(ini, ini_name);
    iniparser_freedict(ini);
}



static STATUS battary_menu_show(struct _menuUnit* p)
{
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        miuiIntent_send(INTENT_MOUNT, 1, "/data");
        unlink("/data/system/batterystats.bin");
        miuiIntent_send(INTENT_UNMOUNT, 1, "/data");
        miui_printf("Battery Stats wiped.\n");
    }
    return MENU_BACK;
}

static STATUS permission_menu_show(struct _menuUnit* p)
{
    miuiIntent_send(INTENT_MOUNT, 1, "/system");
    miuiIntent_send(INTENT_MOUNT, 1, "/data");
    miuiIntent_send(INTENT_SYSTEM, 1, "fix_permissions");
    miui_alert(4, p->name, "<~global_done>", "@alert", acfg()->text_ok);
    return MENU_BACK;
}

static STATUS log_menu_show(struct _menuUnit* p)
{
    char desc[512];
    char file_name[PATH_MAX];
    struct stat st;
    time_t timep;
    struct tm *time_tm;
    time(&timep);
    time_tm = gmtime(&timep);
    if (stat(RECOVERY_PATH, &st) != 0)
    {
        mkdir(RECOVERY_PATH, 0755);
    }
    snprintf(file_name, PATH_MAX - 1, "%s/log", RECOVERY_PATH);
    if (stat(file_name, &st) != 0)
    {
        mkdir(file_name, 0755);
    }
    snprintf(file_name, PATH_MAX - 1, "%s/log/recovery-%02d%02d%02d-%02d%02d.log", RECOVERY_PATH,
           time_tm->tm_year, time_tm->tm_mon + 1, time_tm->tm_mday,
           time_tm->tm_hour, time_tm->tm_min);
    snprintf(desc, 511, "%s%s?",p->desc, file_name);
    if (RET_YES == miui_confirm(3, p->name, desc, p->icon)) {
        miuiIntent_send(INTENT_MOUNT, 1, "/sdcard");
        miuiIntent_send(INTENT_COPY, 2, MIUI_LOG_FILE, file_name);
    }
    return MENU_BACK;
}

static STATUS root_device_item_show(menuUnit *p) {
	if(RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
		miui_busy_process();
		switch(p->result) {
			case ROOT_DEVICE:
				miuiIntent_send(INTENT_MOUNT, 1 , "/system");
				miuiIntent_send(INTENT_ROOT, 1, "root_device");
				miuiIntent_send(INTENT_UNMOUNT, 1 , "/system");
				break;
			case DISABLE_OFFICAL_REC:
				miuiIntent_send(INTENT_MOUNT, 1, "/system");
				miuiIntent_send(INTENT_ROOT, 1, "un_of_rec");
				miuiIntent_send(INTENT_UNMOUNT, 1, "/system");
				break;
			case FREE_SDCARD_SPACE:
				miuiIntent_send(INTENT_MOUNT, 1, "/sdcard");
				miuiIntent_send(INTENT_ROOT, 1, "dedupe_gc");
				break;
			 default:
				assert_if_fail(0);
				break;
		}
	}
	return MENU_BACK;
}

static STATUS brightness_menu_show(struct _menuUnit* p) {
	switch (p->result) {
		/*
		case  0:
			screen_set_brightness("0");
			break;
			*/
		case 15:
			screen_set_brightness("15");
			break;
		case 25:
			screen_set_brightness("25");
			break;
		case 50:
			screen_set_brightness("50");
			break;
		case 75:
			screen_set_brightness("75");
			break;
		case 100:
			screen_set_brightness("100");
			break;
		default:
			//we should never get here!
			break;
	}
	return MENU_BACK;
}




/* 
 *  _sd_show_dir show file system on screen 
 *  return MENU_BACK when press backmenu
 */

#define _SD_MAX_PATH 256
//callback function, success return 0, non-zero fail
 int ors_file_run(char *file_name, int file_len, void* data) {
	 return_val_if_fail(file_name != NULL, RET_FAIL);
	 return_val_if_fail((int)strlen(file_name) <= file_len, RET_INVALID_ARG);
	 return_val_if_fail(data != NULL, RET_FAIL);
	 struct _menuUnit* p = (pmenuUnit)data;
	  if(RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
		  miuiIntent_send(INTENT_RUN_ORS, 1, file_name);
		  return 0;

	  }else return -1;

	  return 0;
 }

//callback function file filter, if access, return 0, other return -1
int ors_file_filter(char* file, int file_len) {
	return_val_if_fail(file != NULL, RET_FAIL);
	int len = strlen(file);
	return_val_if_fail(len <= file_len, RET_INVALID_ARG);
	if (len >= 4 && strncasecmp(file + len -4, ".ors", 4) == 0)
		return 0;
	return -1;
}

static STATUS ors_sd_menu_show(menuUnit *p) {
	//ensure mounte sd path 
	struct _intentResult* result = miuiIntent_send(INTENT_MOUNT, 1, "/sdcard");
	//whatever wether sd is mount, scan sdcard and go on 
	int ret;
	ret = file_scan("/sdcard",sizeof("/sdcard"), p->name, strlen(p->name), &ors_file_run, (void*)p, &ors_file_filter);
	if (ret == -1) return MENU_BACK;
	return ret;
}

static STATUS ors_sdext_menu_show(menuUnit* p) {
	//ensure_mounte sdext path
	struct _intentResult* result = miuiIntent_send(INTENT_MOUNT, 1, "/external_sd");
	//whatever wether sd-ext is mounted ,scan sd-ext and go on
	int ret;
	ret = file_scan("/external_sd", sizeof("/external_sd"), p->name, strlen(p->name), &ors_file_run, (void*)p, &ors_file_filter);
	if (ret == -1) return MENU_BACK;
	return ret;
}

static STATUS about_author_menu_show(menuUnit* p) {
	 FILE* fp;
         char author_info[4084];
	 char file_name[PATH_MAX];
	 snprintf(author_info, 4084, "\nModify by Gaojiquan.com LaoYang\nBuild Date: %s\nSina Weibo: http://weibo.com/210124187\n",acfg()->rom_date);
	 fp = fopen(AUTHOR_INFO, "w+");
	 if (fp != NULL) {
		 fputs(author_info, fp);
		 fclose(fp);
	 } else {
		 LOGE("can not open %s!",AUTHOR_INFO);
		 fclose(fp);
	 } 
	 snprintf(file_name, PATH_MAX, "%s", AUTHOR_INFO);
	 miui_textbox(p->name, p->title_name, p->icon, miui_readfromfs(file_name));
	 return MENU_BACK;
}


//for adb sideload functions
//
static STATUS sideload_menu_show(struct _menuUnit *p) {
	miui_sideload_process();
	miuiIntent_send(INTENT_SIDELOAD, 1, NULL);
	if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
		miuiIntent_send(INTENT_INSTALL, 3, "/tmp/update.zip", "0", "1");
	}

	char cmd[1024];
	snprintf(cmd, 1024, "%s", "rm -rf /tmp/update.zip");
	miuiIntent_send(INTENT_SYSTEM, 1, cmd);
	return MENU_BACK;
}


//skip CDI 
//skip check device info 
//on | off 

static STATUS skip_CDI_menu_show(struct _menuUnit* p) {
	int currstatus;
	if (1 == load_settings()) {
		return MENU_BACK;
	}
	currstatus = iniparser_getboolean(ini, "zipflash:CDI", -1);
	char *statusdlg;
	char *btntext;
	if (currstatus == 0) {
		statusdlg = "Check device info is enabled.";
		btntext = "Disable";
	} else {
		statusdlg = "Check device info is Disable.";
		btntext = "Enable";
	}

        if (RET_YES == miui_confirm(5, p->name, statusdlg, p->icon, btntext, "Cancel")) {
        if (currstatus == 0) {
		//disable check device info 
		iniparser_set(ini, "zipflash:CDI", "1");
	} else if (currstatus == 1) {
		//enable check device info
		iniparser_set(ini, "zipflash:CDI", "0");
	}

	write_settings();
	}

	return MENU_BACK;
}


static STATUS set_md5sum_state(struct _menuUnit* p) {

	int currstatus;
	if (1 == load_settings()) {
		return MENU_BACK;
	}
	currstatus = iniparser_getboolean(ini, "zipflash:md5sum", -1);
	char *statusdlg;
	char *btntext;

	if (currstatus == 0) {
		statusdlg = "Generate md5sum are currently disabled.";
		btntext = "Enable";
	} else {
		statusdlg = "Generate md5sum are currently enabled.";
		btntext = "Disable";
	}

if (RET_YES == miui_confirm(5, p->name, statusdlg, p->icon, btntext, "Cancel")) {
        if (currstatus == 0) {
            // enable md5sum
            iniparser_set(ini, "zipflash:md5sum", "1");
        } else if (currstatus == 1) {
            // disable md5sum
            iniparser_set(ini, "zipflash:md5sum", "0");
        }
        write_settings();
    }
    return MENU_BACK;
}

static STATUS sigcheck_menu_show(struct _menuUnit* p)
{
    int currstatus;
    if (1==load_settings()) {
        return MENU_BACK;
    }
    
    currstatus = iniparser_getboolean(ini, "dev:signaturecheck", -1);
    char *statusdlg;
    char *btntext;
    
    if (currstatus == 0) {
        statusdlg = "Signature checks are currently disabled.";
        btntext = "Enable";
    } else {
        statusdlg = "Signature checks are currently enabled.";
        btntext = "Disable";
    }
    
    if (RET_YES == miui_confirm(5, p->name, statusdlg, p->icon, btntext, "Cancel")) {
        if (currstatus == 0) {
            // enable ors wipe prompt
            iniparser_set(ini, "dev:signaturecheck", "1");
        } else if (currstatus == 1) {
            // disable ors wipe prompt
            iniparser_set(ini, "dev:signaturecheck", "0");
        }
        write_settings();
    }
    return MENU_BACK;
}



struct _menuUnit* ors_ui_init() {
	struct _menuUnit* p = common_ui_init();
	return_null_if_fail(p != NULL);
	menuUnit_set_name(p, "<~root.ors>");
	menuUnit_set_title(p, "run ORS script");
	menuUnit_set_icon(p, "@root");
	assert_if_fail(menuNode_init(p) != NULL);
	//select ORS from sd card 
	struct _menuUnit* temp = common_ui_init();
	menuUnit_set_name(temp, "<~root.choose.sd.ors>");
	menuUnit_set_show(temp, &ors_sd_menu_show);
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	
	//select ORS from external_sd
	if (acfg()->sd_ext == 1) {
	temp = common_ui_init();
	menuUnit_set_name(temp, "<~root.choose.sdin.ors>");
	menuUnit_set_show(temp, &ors_sdext_menu_show);
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	}
	

	return p;
}



struct _menuUnit* brightness_ui_init() {
	struct _menuUnit* p = common_ui_init();
	return_null_if_fail(p != NULL);
	menuUnit_set_name(p, "<~root.set.brightness>");
	menuUnit_set_title(p, "Set Brightness");
	menuUnit_set_icon(p, "@root");
	assert_if_fail(menuNode_init(p) != NULL);
	//0% brightness
	/*
	struct _menuUnit* temp = common_ui_init();
	menuUnit_set_name(temp, "0% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 0;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	*/
	//15% brightness 
	struct _menuUnit* temp = common_ui_init();
	menuUnit_set_name(temp, "15% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 15;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	//25% brightness
	temp = common_ui_init();
	menuUnit_set_name(temp, "25% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 25;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	//50% brightness
        temp = common_ui_init();
	menuUnit_set_name(temp, "50% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 50;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	//75% brightness
	temp = common_ui_init();
	menuUnit_set_name(temp, "75% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 75;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	//100% brightness
	temp = common_ui_init();
	menuUnit_set_name(temp, "100% Brightness");
	menuUnit_set_show(temp, &brightness_menu_show);
	temp->result = 100;
	assert_if_fail(menuNode_add(p, temp) == RET_OK);
	return p;
}

struct _menuUnit* dev_ui_init() {
	struct _menuUnit *p = common_ui_init();
	return_null_if_fail(p != NULL);
	menuUnit_set_name(p, "<~root.dev>");
	menuUnit_set_title(p, "dev opts");
	menuUnit_set_icon(p, "@root");
	assert_if_fail(menuNode_init(p) != NULL);

	// skip check device info (SKIP_CDI) 
	struct _menuUnit *tmp = common_ui_init();
	menuUnit_set_name(tmp, "<~root.CDI>");
	menuUnit_set_show(tmp, &skip_CDI_menu_show);
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);

        // generate md5sum 
	tmp = common_ui_init();
	menuUnit_set_name(tmp, "<~root.md5sum>");
	menuUnit_set_show(tmp, &set_md5sum_state);
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);

	//sigcheck 
	tmp = common_ui_init();
	menuUnit_set_name(tmp, "<~root.sigcheck>");
	menuUnit_set_show(tmp, &sigcheck_menu_show);
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);

	//set Brightness
	tmp = brightness_ui_init();
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);
           
	//ORS function 
	tmp = ors_ui_init();
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);

	return p;
}


struct _menuUnit* root_ui_init() {

	struct _menuUnit *p = common_ui_init();
	return_null_if_fail(p != NULL);
        menuUnit_set_name(p, "<~root.name>");
	menuUnit_set_title(p, "<~root.title>");
	menuUnit_set_icon(p, "@root");
	

	//show info of the author 
	struct _menuUnit *tmp = common_ui_init();
	return_null_if_fail(tmp != NULL);
	strncpy(tmp->name, "<~root.about.author>",MENU_LEN);
	menuUnit_set_icon(tmp, "@info");
        menuUnit_set_show(tmp, &about_author_menu_show);
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);

	//show dev opts 
	 tmp = dev_ui_init();
	 assert_if_fail(menuNode_add(p,tmp) == RET_OK);

        //root device 
	tmp = common_ui_init();
	return_null_if_fail(tmp != NULL);
	strncpy(tmp->name, "<~root.device>", MENU_LEN);
	menuUnit_set_icon(tmp, "@root.device");
	tmp->result = ROOT_DEVICE;
	tmp->show = &root_device_item_show;
	assert_if_fail(menuNode_add(p,tmp) == RET_OK);

	 //fix permission
         tmp = common_ui_init();
	 menuUnit_set_name(tmp, "<~tool.permission.name>"); 
         menuUnit_set_icon(tmp, "@tool.permission");
         menuUnit_set_show(tmp, &permission_menu_show);
         assert_if_fail(menuNode_add(p, tmp) == RET_OK);

	//root disable_restore_official_recovery 	
	tmp = common_ui_init();
	return_null_if_fail(tmp != NULL);
	strncpy(tmp->name, "<~root.un_of_rec>", MENU_LEN);
	menuUnit_set_icon(tmp, "@root.un_of_rec");
	tmp->result = DISABLE_OFFICAL_REC;
	tmp->show = &root_device_item_show;
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);
          

         // adb sideload 
	 tmp = common_ui_init();
	 menuUnit_set_name(tmp, "<~tool.sideload.name>"); 
         menuUnit_set_icon(tmp, "@alert");
         menuUnit_set_show(tmp, &sideload_menu_show);
         menuUnit_set_desc(tmp, "<~tool.sideload.desc>");
         assert_if_fail(menuNode_add(p, tmp) == RET_OK);
	 

	//Free the sdcard space 
	tmp = common_ui_init();
	return_null_if_fail(tmp != NULL);
	strncpy(tmp->name, "<~root.free.sd.space>", MENU_LEN);
	menuUnit_set_icon(tmp,"@root");
	tmp->result = FREE_SDCARD_SPACE;
	tmp->show = &root_device_item_show;
	assert_if_fail(menuNode_add(p, tmp) == RET_OK);


	//batarry wipe
          tmp = common_ui_init();
          menuUnit_set_name(tmp, "<~tool.battary.name>"); 
          menuUnit_set_icon(tmp, "@tool.battery");
          menuUnit_set_show(tmp, &battary_menu_show);
          assert_if_fail(menuNode_add(p, tmp) == RET_OK);
	   //copy log
          tmp = common_ui_init();
          menuUnit_set_name(tmp, "<~tool.log.name>"); 
          menuUnit_set_show(tmp, &log_menu_show);
          menuUnit_set_icon(tmp, "@tool.log");
          menuUnit_set_desc(tmp, "<~tool.log.desc>");
          assert_if_fail(menuNode_add(p, tmp) == RET_OK);


	return p;
}


