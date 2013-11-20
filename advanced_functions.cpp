#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h> //get the basename(const char* path)
//#include <cstdarg>
#include "bootloader.h"
#include "common.h"
#include "install.h"
#include "roots.h"
#include "recovery_ui.h"


extern "C" {
#include "miui/src/miui.h"
#include "miui_intent.h"
#include "dedupe/dedupe.h"
#include "minzip/DirUtil.h"
#include "mtdutils/mounts.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "libcrecovery/common.h"
#include "yaffs2_static/mkyaffs2image.h"
#include "yaffs2_static/unyaffs.h"
#include "flashutils/flashutils.h"
}

#include "nandroid.h"
#include "root_device.hpp"
#include "advanced_functions.hpp"

#include <iostream>

MIFunc::MIFunc() {
	nandroid_md5sum = true;
	skip_cdi = false;
	backup_method = "tar";

}

MIFunc::~MIFunc() {

}

/*
 * nandroid_backup_format=tar | dup | tgz 
 * skip_cdi=on | off
 * nandroid_md5sum=on | off 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */




int MIFuc::read_config_file(char* config_file, char *key, char *value, char *value_def) {
    int ret = 0;
    char line[PROPERTY_VALUE_MAX];
    ensure_path_mounted(config_file);
   
    FILE *fp = fopen(config_file, "rb");
    if (fp != NULL) {
        while(fgets(line, sizeof(line), fp) != NULL) {
            if (strstr(line, key) != NULL && strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == '=') {
                strcpy(value, strstr(line, "=") + 1);
                //remove trailing \n char
                if (value[strlen(value)-1] == '\n')
                    value[strlen(value)-1] = '\0';
                if (value[0] != '\0') {
                    fclose(fp);
                    LOGI("%s=%s\n", key, value);
                    return ret;
                }
            }
        }
        ret = 1;
        fclose(fp);
    } else {
        LOGI("Cannot open %s\n", config_file);
        ret = -1;
    }

    strcpy(value, value_def);
    LOGI("%s set to default (%s)\n", key, value_def);
    return ret;
}


int MIFunc::write_config_file(char* config_file, char* key, char* value) {
    if (ensure_path_mounted(config_file) != 0) {
        LOGE("Cannot mount path for settings file: %s\n", config_file);
        return -1;
    }

    char config_file_tmp[PATH_MAX];
    strcpy(config_file_tmp, config_file);
    ensure_directory(dirname(config_file_tmp));
    strcpy(config_file_tmp, config_file);
    strcat(config_file_tmp, ".tmp");
    delete_a_file(config_file_tmp);

    FILE *f_tmp = fopen(config_file_tmp, "wb");
    if (f_tmp == NULL) {
        LOGE("failed to create temporary settings file!\n");
        return -1;
    }

    FILE *fp = fopen(config_file, "rb");
    if (fp == NULL) {
        // we need to create a new settings file: write an info header
        const char* header[] = {
            "#MIUI RECOVERY v3.2 Settings File\n",
            "#Edit only in appropriate UNIX format (Notepad+++...)\n",
            "#Entries are in the form of:\n",
            "#key=value\n",
            "#Do not add spaces in between!\n",
            "\n",
            NULL
        };

        int i;
        for(i=0; header[i] != NULL; i++) {
            fwrite(header[i], 1, strlen(header[i]), f_tmp);
        }
    } else {
        // parse existing config file and write new temporary file.
        char line[PROPERTY_VALUE_MAX];
        while(fgets(line, sizeof(line), fp) != NULL) {
            // ignore any existing line with key we want to set
            if (strstr(line, key) != NULL && strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == '=')
                continue;
            // ensure trailing \n, in case some one got a bad editor...
            if (line[strlen(line)-1] != '\n')
                strcat(line, "\n");
            fwrite(line, 1, strlen(line), f_tmp);
        }
        fclose(fp);
    }

    // write new key=value entry
    char new_entry[PROPERTY_VALUE_MAX];
    sprintf(new_entry, "%s=%s\n", key, value);
    fwrite(new_entry, 1, strlen(new_entry), f_tmp);
    fclose(f_tmp);

    if (rename(config_file_tmp, config_file) !=0) {
        LOGE("failed to rename temporary settings file!\n");
        return -1;
    }
    LOGI("%s was set to %s\n", key, value);
    return 0;
}
//----- end file settings parser




