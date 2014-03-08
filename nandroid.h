#ifndef NANDROID_H
#define NANDROID_H

#ifdef __cplusplus 
extern "C" {
#endif

void nandroid_generate_timestamp_path(char* backup_path);
int nandroid_backup(const char* backup_path);
int nandroid_restore(const char* backup_path, int restore_boot, int restore_system, int restore_data, int restore_cache, int restore_sdext, int restore_wimax, int restore_boot1, int restore_system1);

int nandroid_advanced_backup(const char* backup_path, const char *root);
/* for dedupe backup method */
void nandroid_dedupe_gc(const char* blob_dir);
void nandroid_force_backup_format(const char* fmt);
unsigned nandroid_get_default_backup_format();



#define NANDROID_BACKUP_FORMAT_FILE "/sdcard/miui_recovery/.default_backup_format"
#define NANDROID_BACKUP_FORMAT_TAR 0
#define NANDROID_BACKUP_FORMAT_DUP 1
#define NANDROID_BACKUP_FORMAT_TGZ 2

#define TAR_FORMAT 0
/*end of dedupe method */

#ifdef __cplusplus
}
#endif

#endif

