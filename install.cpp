/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"
#include "install.h"
extern "C" {
#include "mincrypt/rsa.h"
#include "minui/minui.h"
#include "minzip/SysUtil.h"
#include "minzip/Zip.h"
#include "mtdutils/mounts.h"
#include "mtdutils/mtdutils.h"
#include "miui/src/miui.h"
#include "iniparser/iniparser.h"
}

#include "roots.h"
#include "verifier.h"
#include "root_device.hpp"

#include "firmware.h"

#define ASSUMED_UPDATE_BINARY_NAME  "META-INF/com/google/android/update-binary"
#define ASSUMED_UPDATE_SCRIPT_NAME  "META-INF/com/google/android/update-script"
#define PUBLIC_KEYS_FILE "/res/keys"

#define UPDATER_API_VERSION 3 // this should equal RECOVERY_API_VERSION , define in Android.mk 



// The update binary ask us to install a firmware file on reboot.  Set
// that up.  Takes ownership of type and filename.
static int  handle_firmware_update(char* type, char* filename, ZipArchive* zip) {
    unsigned int data_size;
    const ZipEntry* entry = NULL;

    if (strncmp(filename, "PACKAGE:", 8) == 0) {
        entry = mzFindZipEntry(zip, filename+8);
        if (entry == NULL) {
            LOGE("Failed to find \"%s\" in package", filename+8);
            return INSTALL_ERROR;
        }
        data_size = entry->uncompLen;
    } else {
        struct stat st_data;
        if (stat(filename, &st_data) < 0) {
            LOGE("Error stat'ing %s: %s\n", filename, strerror(errno));
            return INSTALL_ERROR;
        }
        data_size = st_data.st_size;
    }

    LOGI("type is %s; size is %d; file is %s\n",
         type, data_size, filename);

    char* data = (char*)malloc(data_size);
    if (data == NULL) {
        LOGI("Can't allocate %d bytes for firmware data\n", data_size);
        return INSTALL_ERROR;
    }

    if (entry) {
        if (mzReadZipEntry(zip, entry, data, data_size) == false) {
            LOGE("Failed to read \"%s\" from package", filename+8);
            return INSTALL_ERROR;
        }
    } else {
        FILE* f = fopen(filename, "rb");
        if (f == NULL) {
            LOGE("Failed to open %s: %s\n", filename, strerror(errno));
            return INSTALL_ERROR;
        }
        if (fread(data, 1, data_size, f) != data_size) {
            LOGE("Failed to read firmware data: %s\n", strerror(errno));
            return INSTALL_ERROR;
        }
        fclose(f);
    }

    if (remember_firmware_update(type, data, data_size)) {
        LOGE("Can't store %s image\n", type);
        free(data);
        return INSTALL_ERROR;
    }

    free(filename);

    return INSTALL_SUCCESS;
}


static const char *LAST_INSTALL_FILE = "/cache/recovery/last_install";

dictionary * ini_install;

int load_cotsettings()
{
    ini_install = iniparser_load("/sdcard/miui_recovery/settings.ini");
    if (ini_install==NULL)
        return 1;
        
    return 0;
}



bool skip_check_device_info(char *ignore_device_info);
			
// If the package contains an update binary, extract it and run it.
static int try_update_binary(const char *path, ZipArchive *zip) {
    const ZipEntry* binary_entry =
            mzFindZipEntry(zip, ASSUMED_UPDATE_BINARY_NAME);
    struct stat st;
    if (binary_entry == NULL) {
	    const ZipEntry* update_script_entry = 
		    mzFindZipEntry(zip, ASSUMED_UPDATE_SCRIPT_NAME);
	    if (update_script_entry != NULL) {
		     ui_print("Amend scripting (update-script) is no longer supported.\n");
            ui_print("Amend scripting was deprecated by Google in Android 1.5.\n");
            ui_print("It was necessary to remove it when upgrading to the ClockworkMod 3.0 Gingerbread based recovery.\n");
            ui_print("Please switch to Edify scripting (updater-script and update-binary) to create working update zip packages.\n");
            return INSTALL_ERROR;
        }

        mzCloseZipArchive(zip);
        return INSTALL_UPDATE_BINARY_MISSING;
    }

    string binary_str = "/tmp/updater";
    unlink(binary_str.c_str());
   
   int fd;
  if (0 > ( fd = open(binary_str.c_str(), (O_CREAT|O_WRONLY|O_TRUNC), 0755))) {
		   printf("create /tmp/updater error (%s)\n", strerror(errno));
  }


	   
    if (fd < 0) {
        mzCloseZipArchive(zip);
        LOGE("Can't make %s\n", binary_str.c_str());
        return 1;
    }


    bool ok = mzExtractZipEntryToFile(zip, binary_entry, fd);

    close(fd);
    mzCloseZipArchive(zip);

    if (!ok) {
        LOGE("Can't copy %s\n", ASSUMED_UPDATE_BINARY_NAME);
	mzCloseZipArchive(zip);
        return 1;
    }


 
    int currstatus;
    if (1==load_cotsettings()) {
        return INSTALL_CORRUPT;
    }
    
    currstatus = iniparser_getboolean(ini_install, "zipflash:CDI", -1);
    iniparser_freedict(ini_install);


       


    int pipefd[2];
    pipe(pipefd);
    char tmpbuf[256];

    // When executing the update binary contained in the package, the
    // arguments passed are:
    //
    //   - the version number for this interface
    //
    //   - an fd to which the program can write in order to update the
    //     progress bar.  The program can write single-line commands:
    //
    //        progress <frac> <secs>
    //            fill up the next <frac> part of of the progress bar
    //            over <secs> seconds.  If <secs> is zero, use
    //            set_progress commands to manually control the
    //            progress of this segment of the bar
    //
    //        set_progress <frac>
    //            <frac> should be between 0.0 and 1.0; sets the
    //            progress bar within the segment defined by the most
    //            recent progress command.
    //
    //        firmware <"hboot"|"radio"> <filename>
    //            arrange to install the contents of <filename> in the
    //            given partition on reboot.
    //
    //            (API v2: <filename> may start with "PACKAGE:" to
    //            indicate taking a file from the OTA package.)
    //
    //            (API v3: this command no longer exists.)
    //
    //        ui_print <string>
    //            display <string> on the screen.
    //
    //   - the name of the package zip file.
    //

    const char** args = ( const char**)malloc(sizeof(char*) * 5);
    args[0] = binary_str.c_str();
    //args[1] = EXPAND(RECOVERY_API_VERSION);   // defined in Android.mk
    args[1] = (char*)EXPAND(UPDATER_API_VERSION);
    char *temp = (char*)malloc(10);
    sprintf(temp, "%d", pipefd[1]);
    args[2] = temp;
    args[3] = (char*)path;
    args[4] = NULL;

    pid_t pid = fork();
    if (pid == 0) {
	setenv("UPDATE_PACKAGE", path, 1);
        close(pipefd[0]);
        execv(binary_str.c_str(),(char* const *) args);
        fprintf(stdout, "E:Can't run %s (%s)\n", binary_str.c_str(), strerror(errno));

        _exit(-1);
    }
    close(pipefd[1]);
    
    char *firmware_type = NULL;
    char *firmware_filename = NULL;

    char buffer[1024];

    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
         } else if (strcmp(command, "assert") == 0) {
		 if (currstatus) {
                char *ignore_device_info = strtok(NULL, " \n");
                if (skip_check_device_info(ignore_device_info)) 
                        continue;
                char *ignore_device_info_part_two = strtok(NULL, "||");
                if (skip_check_device_info(ignore_device_info_part_two))
                        continue;     
		 } else {
		 printf("assert command checking \n");
		 }	 
        } else if (strcmp(command, "progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            char* seconds_s = strtok(NULL, " \n");

            float fraction = strtof(fraction_s, NULL);
            int seconds = strtol(seconds_s, NULL, 10);

            ui_show_progress(fraction * (1-VERIFICATION_PROGRESS_FRACTION),
                             seconds);
        } else if (strcmp(command, "set_progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            float fraction = strtof(fraction_s, NULL);
            ui_set_progress(fraction);
	} else if (strcmp(command, "firmware") == 0) {
            char* type = strtok(NULL, " \n");
            char* filename = strtok(NULL, " \n");

            if (type != NULL && filename != NULL) {
                if (firmware_type != NULL) {
                    LOGE("ignoring attempt to do multiple firmware updates");
                } else {
                    firmware_type = strdup(type);
                    firmware_filename = strdup(filename);
                }
            }
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str)
                snprintf(tmpbuf, 255, "<#selectbg_g><b>%s</b></#>", str);
            else 
                snprintf(tmpbuf, 255, "<#selectbg_g><b>\n</b></#>");
            miuiInstall_set_text(tmpbuf);
        } else if (strcmp(command, "wipe_cache") == 0) {
           // *wipe_cache = 1;
        } else if (strcmp(command, "minzip:") == 0) {
            char* str = strtok(NULL, "\n");
            miuiInstall_set_info(str);
        } 
        else {
#if 0 
            snprintf(tmpbuf, 255, "%s", command);
	        miuiInstall_set_text(tmpbuf);
            char* str = strtok(NULL, "\n");
            if (str)
            {
                snprintf(tmpbuf, 255, "%s", str);
                miuiInstall_set_text(tmpbuf);
            }
#endif
            char * str = strtok(NULL, "\n");
            if (str)
                LOGD("[%s]:%s\n",command, str);
        }
    }
    fclose(from_child);
    

    int status;
    
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", path, WEXITSTATUS(status));
        snprintf(tmpbuf, 255, "<#selectbg_g><b>Error in%s\n(Status %d)\n</b></#>", path, WEXITSTATUS(status));
	mzCloseZipArchive(zip);
        miuiInstall_set_text(tmpbuf);
        return INSTALL_ERROR;
    }


    if (firmware_type != NULL) {
        int ret = handle_firmware_update(firmware_type, firmware_filename, zip);
        mzCloseZipArchive(zip);
        return ret;
    }

    mzCloseZipArchive(zip); 
    return INSTALL_SUCCESS;
}



static int really_install_package(const char *path)

{
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    ui_print("Finding update package...\n");
    ui_show_indeterminate_progress();

     // Resolve symlink in case legacy /sdcard path is used
    // Requires: symlink uses absolute path
    char new_path[PATH_MAX];
    if (strlen(path) > 1) {
        char *rest = strchr(path + 1, '/');
        if (rest != NULL) {
            int readlink_length;
            int root_length = rest - path;
            char *root = (char*)malloc(root_length + 1);
            strncpy(root, path, root_length);
            root[root_length] = 0;
            readlink_length = readlink(root, new_path, PATH_MAX);
            if (readlink_length > 0) {
                strncpy(new_path + readlink_length, rest, PATH_MAX - readlink_length);
                path = new_path;
            }
            free(root);
        }
    }

    LOGI("Update location: %s\n", path);

    if (ensure_path_mounted(path) != 0) {
        LOGE("Can't mount %s\n", path);
        return INSTALL_CORRUPT;
    }

    ui_print("Opening update package...\n");


 // check the signature 
     int currstatus;
    if (1==load_cotsettings()) {
        return INSTALL_CORRUPT;
    }
    
    currstatus = iniparser_getboolean(ini_install, "dev:signaturecheck", -1);
    iniparser_freedict(ini_install);

     int err = 0;	     

    if (currstatus == 1) {

        int numKeys;
        Certificate* loadedKeys = load_keys(PUBLIC_KEYS_FILE, &numKeys);
        if (loadedKeys == NULL) {
            LOGE("Failed to load keys\n");
            return INSTALL_CORRUPT;
        }
        LOGI("%d key(s) loaded from %s\n", numKeys, PUBLIC_KEYS_FILE);

        // Give verification half the progress bar...
        ui_print("Verifying update package...\n");
        ui_show_progress(
                VERIFICATION_PROGRESS_FRACTION,
                VERIFICATION_PROGRESS_TIME);

        err = verify_file(path, loadedKeys, numKeys);
        free(loadedKeys);
        LOGI("verify_file returned %d\n", err);
        if (err != VERIFY_SUCCESS) {
            LOGE("signature verification failed\n");
                return INSTALL_CORRUPT;
        }
    }
  

    /* Try to open the package.
     */
 
    ZipArchive zip;
     err = 0;
    err = mzOpenZipArchive(path, &zip);
    if (err != 0) {
        LOGE("Can't open %s\n(%s)\n", path, err != -1 ? strerror(err) : "bad");
        return INSTALL_CORRUPT;
    }

    /* Verify and install the contents of the package.
     */
    ui_print("Installing update...\n");

    return try_update_binary(path, &zip);

}

int
install_package(const char* path)
{
    FILE* install_log = fopen_path(LAST_INSTALL_FILE, "w");
    if (install_log) {
        fputs(path, install_log);
        fputc('\n', install_log);
    } else {
        LOGE("failed to open last_install: %s\n", strerror(errno));
    }
    int result = really_install_package(path);
    if (install_log) {
        fputc(result == INSTALL_SUCCESS ? '1' : '0', install_log);
        fputc('\n', install_log);
        fclose(install_log);
	chmod(LAST_INSTALL_FILE, 0644);
    }
    return result;
}

/*
 * This func is for check the device info
 * @ignore_device_info is the parse from strtok(NULL, " \n");
 * return true if found the device info
 * return false if not found the device info
 */
bool skip_check_device_info(char *ignore_device_info) {
        char tmpbuf[256];
        if (strstr(ignore_device_info, "ro.product.device") != NULL ||
                        strstr(ignore_device_info, "ro.build.product") != NULL ||
                        strstr(ignore_device_info, "ro.product.board") != NULL ||
                        strstr(ignore_device_info, "ro.sdupdate.Check_info") != NULL) {
                snprintf(tmpbuf, 255, "<#selectbg_g><b>Ignore device_info_check \n</b></#>");
                miuiInstall_set_text(tmpbuf);
                return true;
        }
                return false;
}

