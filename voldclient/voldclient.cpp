
#include "voldclient.hpp"
#include "Volume.h"
#include "ResponseCode.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>

#include <pthread.h>
#include "../common.h"

int VoldClient::vold_mount_volume(const char* path, int wait) {
	const char *cmd[3] = { "volume", "mount", path };
    int state = vold_get_volume_state(path);

    if (state == Volume::State_Mounted) {
        LOGI("Volume %s already mounted\n", path);
        return 0;
    }

    if (state != Volume::State_Idle) {
        LOGI("Volume %s is not idle, current state is %d\n", path, state);
        return -1;
    }

    if (access(path, R_OK) != 0) {
        mkdir(path, 0000);
        chown(path, 1000, 1000);
    }
    return vold_command(3, cmd, wait);
}

int VoldClient::vold_update_volumes() {

    const char *cmd[2] = {"volume", "list"};
    return vold_command(2, cmd, 1);
}

int VoldClient::vold_unmount_volume(const char* path, int force, int wait) {

    const char *cmd[4] = { "volume", "unmount", path, "force" };
    int state = vold_get_volume_state(path);

    if (state <= Volume::State_Idle) {
        LOGI("Volume %s is not mounted\n", path);
        return 0;
    }

    if (state != Volume::State_Mounted) {
        LOGI("Volume %s cannot be unmounted in state %d\n", path, state);
        return -1;
    }

    return vold_command(force ? 4: 3, cmd, wait);
}

int VoldClient::vold_share_volume(const char* path) {

    const char *cmd[4] = { "volume", "share", path, "ums" };
    int state = vold_get_volume_state(path);

    if (state == Volume::State_Mounted)
        vold_unmount_volume(path, 0, 1);

    return vold_command(4, cmd, 1);
}

int VoldClient::vold_unshare_volume(const char* path, int mount) {

    const char *cmd[4] = { "volume", "unshare", path, "ums" };
    int state = vold_get_volume_state(path);
    int ret = 0;

    if (state != Volume::State_Shared) {
        LOGE("Volume %s is not shared - state=%d\n", path, state);
        return 0;
    }

    ret = vold_command(4, cmd, 1);

    if (mount)
        vold_mount_volume(path, 1);

    return ret;
}

int VoldClient::vold_format_volume(const char* path, int wait) {

    const char* cmd[3] = { "volume", "format", path };
    return vold_command(3, cmd, wait);
}

int VoldClient::vold_custom_format_volume(const char* path, const char* fstype, int wait) {
    const char* cmd[4] = { "volume", "format", path, fstype };
    return vold_command(4, cmd, wait);
}

const char* VoldClient::volume_state_to_string(int state) {
    if (state == Volume::State_Init)
        return "Initializing";
    else if (state == Volume::State_NoMedia)
        return "No-Media";
    else if (state == Volume::State_Idle)
        return "Idle-Unmounted";
    else if (state == Volume::State_Pending)
        return "Pending";
    else if (state == Volume::State_Mounted)
        return "Mounted";
    else if (state == Volume::State_Unmounting)
        return "Unmounting";
    else if (state == Volume::State_Checking)
        return "Checking";
    else if (state == Volume::State_Formatting)
        return "Formatting";
    else if (state == Volume::State_Shared)
        return "Shared-Unmounted";
    else if (state == Volume::State_SharedMnt)
        return "Shared-Mounted";

        return "Unknown-Error";
}


//dispatcher.c 
//

static struct vold_callbacks* callbacks = NULL;
static int should_automount = 0;

struct volume_node {
    const char *label;
    const char *path;
    int state;
    struct volume_node *next;
};

static struct volume_node *volume_head = NULL;
static struct volume_node *volume_tail = NULL;

static int num_volumes = 0;

static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void vold_set_callbacks(struct vold_callbacks* ev_callbacks) {
    callbacks = ev_callbacks;
}

void VoldClient::vold_set_automount(int automount) {
    should_automount = automount;
}

void VoldClient::vold_mount_all() {

    struct volume_node *node;

    pthread_rwlock_rdlock(&rwlock);
    for (node = volume_head; node; node = node->next) {
        if (node->state == Volume::State_Idle) {
            vold_mount_volume(node->path, 0);
        }
    }
    pthread_rwlock_unlock(&rwlock);
}

void VoldClient::vold_unmount_all() {

    struct volume_node *node;

    pthread_rwlock_rdlock(&rwlock);
    for (node = volume_head; node; node = node->next) {
        if (node->state >= Volume::State_Shared) {
            vold_unshare_volume(node->path, 0);
        }
        if (node->state == Volume::State_Mounted) {
            vold_unmount_volume(node->path, 1, 1);
        }
    }
    pthread_rwlock_unlock(&rwlock);
}

int VoldClient::vold_get_volume_state(const char *path) {

    int ret = 0;
    struct volume_node *node;

    pthread_rwlock_rdlock(&rwlock);
    for (node = volume_head; node; node = node->next) {
        if (strcmp(path, node->path) == 0) {
            ret = node->state;
            break;
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return ret;
}

int VoldClient::vold_get_num_volumes() {
    return num_volumes;
}

int VoldClient::vold_is_volume_available(const char *path) {
    return vold_get_volume_state(path) > 0;
}

static void free_volume_list_locked() {

    struct volume_node *node;

    node = volume_head;
    while (node) {
        struct volume_node *next = node->next;
        free((void *)node->path);
        free((void *)node->label);
        free(node);
        node = next;
    }
    volume_head = volume_tail = NULL;
}

static int is_listing_volumes = 0;

static void vold_handle_volume_list(const char* label, const char* path, int state) {

    struct volume_node *node;

    pthread_rwlock_wrlock(&rwlock);
    if (is_listing_volumes == 0) {
        free_volume_list_locked();
        num_volumes = 0;
        is_listing_volumes = 1;
    }

    node = (struct volume_node *)malloc(sizeof(struct volume_node));
    node->label = strdup(label);
    node->path = strdup(path);
    node->state = state;
    node->next = NULL;

    if (volume_head == NULL)
        volume_head = volume_tail = node;
    else {
        volume_tail->next = node;
        volume_tail = node;
    }

    num_volumes++;
    pthread_rwlock_unlock(&rwlock);
}

static void vold_handle_volume_list_done() {

    pthread_rwlock_wrlock(&rwlock);
    is_listing_volumes = 0;
    pthread_rwlock_unlock(&rwlock);
}

static void set_volume_state(char* path, int state) {

    struct volume_node *node;

    pthread_rwlock_rdlock(&rwlock);
    for (node = volume_head; node; node = node->next) {
        if (strcmp(node->path, path) == 0) {
            node->state = state;
            break;
        }
    }
    pthread_rwlock_unlock(&rwlock);
}

static void vold_handle_volume_state_change(char* label, char* path, int state) {

    set_volume_state(path, state);

    if (callbacks != NULL && callbacks->state_changed != NULL)
        callbacks->state_changed(label, path, state);
}

static void vold_handle_volume_inserted(char* label, char* path) {

    set_volume_state(path, Volume::State_Idle);

    if (callbacks != NULL && callbacks->disk_added != NULL)
        callbacks->disk_added(label, path);

    if (should_automount)
        VoldClient::vold_mount_volume(path, 0);
}

static void vold_handle_volume_removed(char* label, char* path) {

    set_volume_state(path, Volume::State_NoMedia);

    if (callbacks != NULL && callbacks->disk_removed != NULL)
        callbacks->disk_removed(label, path);
}

int vold_dispatch(int code, char** tokens, int len) {

    int i = 0;
    int ret = 0;

    if (code == ResponseCode::VolumeListResult) {
        // <code> <seq> <label> <path> <state>
        vold_handle_volume_list(tokens[2], tokens[3], atoi(tokens[4]));

    } else if (code == ResponseCode::VolumeStateChange) {
        // <code> "Volume <label> <path> state changed from <old_#> (<old_str>) to <new_#> (<new_str>)"
        vold_handle_volume_state_change(tokens[2], tokens[3], atoi(tokens[10]));

    } else if (code == ResponseCode::VolumeDiskInserted) {
        // <code> Volume <label> <path> disk inserted (<blk_id>)"
        vold_handle_volume_inserted(tokens[2], tokens[3]);

    } else if (code == ResponseCode::VolumeDiskRemoved || code == ResponseCode::VolumeBadRemoval) {
        // <code> Volume <label> <path> disk removed (<blk_id>)"
        vold_handle_volume_removed(tokens[2], tokens[3]);

    } else if (code == ResponseCode::CommandOkay && is_listing_volumes) {
        vold_handle_volume_list_done();

    } else {
        ret = 0;
    }

    return ret;
}



//event_loop.c 
//

// locking
static pthread_mutex_t mutex      = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  completion = PTHREAD_COND_INITIALIZER;

// command result status, read with mutex held
static int cmd_result = 0;

// commands currently in flight
static int cmd_inflight = 0;

// socket fd
static int sock = -1;


static int vold_connect() {

    int ret = 1;
    if (sock > 0) {
        return ret;
    }

    // socket connection to vold
    if ((sock = socket_local_client("vold",
                                     ANDROID_SOCKET_NAMESPACE_RESERVED,
                                     SOCK_STREAM)) < 0) {
        LOGE("Error connecting to Vold! (%s)\n", strerror(errno));
        ret = -1;
    } else {
        LOGI("Connected to Vold..\n");
    }
    return ret;
}

static int split(char *str, char **splitstr) {

    char *p;
    int i = 0;

    p = strtok(str, " ");

    while(p != NULL) {
        splitstr[i] = (char*)malloc(strlen(p) + 1);
        if (splitstr[i])
            strcpy(splitstr[i], p);
        i++;
        p = strtok (NULL, " ");
    }

    return i;
}


static int handle_response(char* response) {

    int code = 0, len = 0, i = 0;
    char *tokens[32] = { NULL };

    len = split(response, tokens);
    code = atoi(tokens[0]);

    if (len) {
        vold_dispatch(code, tokens, len);

        for (i = 0; i < len; i++)
            free(tokens[i]);
    }

    return code;
}

static int monitor_started = 0;

// wait for events and signal waiters when appropriate
static int monitor() {

    char *buffer = (char*)malloc(4096);
    int code = 0;

    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        if (!monitor_started) {
            pthread_mutex_lock(&mutex);
            monitor_started = 1;
            pthread_cond_signal(&completion);
            pthread_mutex_unlock(&mutex);
        }

        if ((rc = select(sock +1, &read_fds, NULL, NULL, &to)) < 0) {
            LOGE("Error in select (%s)\n", strerror(errno));
            goto out;

        } else if (!rc) {
            continue;

        } else if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, 4096);
            if ((rc = read(sock, buffer, 4096)) <= 0) {
                if (rc == 0)
                    LOGE("Lost connection to Vold - did it crash?\n");
                else
                    LOGE("Error reading data (%s)\n", strerror(errno));
                if (rc == 0)
                    return ECONNRESET;
                goto out;
            }

            int offset = 0;
            int i = 0;

            // dispatch each line of the response
            for (i = 0; i < rc; i++) {
                if (buffer[i] == '\0') {

                    LOGI("%s\n", buffer + offset);
                    code = handle_response(strdup(buffer + offset));

                    if (code >= 200 && code < 600) {
                        pthread_mutex_lock(&mutex);
                        cmd_result = code;
                        cmd_inflight--;
                        pthread_cond_signal(&completion);
                        pthread_mutex_unlock(&mutex);
                    }
                    offset = i + 1;
                }
            }
        }
    }
out:
    free(buffer);
    pthread_mutex_unlock(&mutex);
    return code;
}

static void *event_thread_func(void* v) {

    // if monitor() returns, it means we lost the connection to vold
    while (1) {

        if (vold_connect()) {
            monitor();

            if (sock)
                close(sock);
        }
        sleep(3);
    }
    return NULL;
}


// start the client thread
void VoldClient::vold_client_start(struct vold_callbacks* callbacks, int automount) {

    if (sock > 0) {
        return;
    }

    pthread_mutex_lock(&mutex);

    vold_set_callbacks(callbacks);

    pthread_t vold_event_thread;
    pthread_create(&vold_event_thread, NULL, &event_thread_func, NULL);
    pthread_cond_wait(&completion, &mutex);
    pthread_mutex_unlock(&mutex);

    vold_update_volumes();

    if (automount) {
        vold_mount_all();
    }
    vold_set_automount(automount);
}

// send a command to vold. waits for completion and returns result
// code if wait is 1, otherwise returns zero immediately.
int VoldClient::vold_command(int len, const char** command, int wait) {

    char final_cmd[255] = "0 "; /* 0 is a (now required) sequence number */
    int i;
    size_t sz;
    int ret = 0;

    if (!vold_connect()) {
        return -1;
    }

    for (i = 0; i < len; i++) {
        char *cmp;

        if (!index(command[i], ' '))
            asprintf(&cmp, "%s%s", command[i], (i == (len -1)) ? "" : " ");
        else
            asprintf(&cmp, "\"%s\"%s", command[i], (i == (len -1)) ? "" : " ");

        sz = strlcat(final_cmd, cmp, sizeof(final_cmd));

        if (sz >= sizeof(final_cmd)) {
            LOGE("command syntax error  sz=%d size=%d", sz, sizeof(final_cmd));
            free(cmp);
            return -1;
        }
        free(cmp);
    }

    // only one writer at a time
    pthread_mutex_lock(&mutex);
    if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
        LOGE("Unable to send command to vold!\n");
        ret = -1;
    }
    cmd_inflight++;

    if (wait) {
        while (cmd_inflight) {
            // wait for completion
            pthread_cond_wait(&completion, &mutex);
            ret = cmd_result;
        }
    }
    pthread_mutex_unlock(&mutex);

    return ret;
}


