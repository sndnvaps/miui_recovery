/*
 * Copyright (c) 2014 The Miui Recovery Project
 * Copyright (c) 2014 dnv aps<sndnvaps@gmail.com>
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

#ifndef _VOLDCLIENT_HPP
#define _VOLDCLIENT_HPP

using namespace std;

struct vold_callbacks {
    int (*state_changed)(char* label, char* path, int state);
    int (*disk_added)(char* label, char* path);
    int (*disk_removed)(char* label, char* path);
};

class VoldClient {
	public:
static int vold_mount_volume(const char* path, int wait);
static int vold_unmount_volume(const char* path, int force, int wait);

static int vold_share_volume(const char* path);
static int vold_unshare_volume(const char* path, int remount);

static int vold_format_volume(const char* path, int wait);
static int vold_custom_format_volume(const char* path, const char* fstype, int wait);

static int vold_is_volume_available(const char* path);
static int vold_get_volume_state(const char* path);

static int vold_update_volumes();
static int vold_get_num_volumes();
static void vold_mount_all();
static void vold_unmount_all();

static void vold_client_start(struct vold_callbacks* callbacks, int automount);
static void vold_set_automount(int enabled);
static int vold_command(int len, const char** command, int wait);
static const char* volume_state_to_string(int state);

};



#endif

