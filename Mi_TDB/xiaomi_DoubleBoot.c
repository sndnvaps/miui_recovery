#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libcrecovery/common.h"


//#include "roots.h" //需要在cwm源代码中编译，并且需要源代码支持双系统启动

/* // this part is define in roots.h 
int is_dualsystem() {
	int i;
	for (i = 0; i < num_volumes; i++) {
		Volume *vol = device_volumes + i;
		if (strcmp(vol->mount_point, "/system1") == 0)
			return 1;
	}
	return 0;
}
*/



int is_dualsystem() {
	FILE *fstab = fopen("/etc/recovery.fstab", "r");
	if (fstab == NULL) {
		printf("failed to open '/etc/recovery.fstab' (%s)\n", strerror(errno));
		return -1;
	}
	char buffer[1024];
	int i;
	while(fgets(buffer, sizeof(buffer) - 1, fstab)) {
 for (i = 0; buffer[i] && isspace(buffer[i]); ++i);
        if (buffer[i] == '\0' || buffer[i] == '#') continue;

	char *mount_point = strtok(buffer+i, " \t\n");

	if (strcmp(mount_point, "/system1") == 0) //此为判断系统是否为支持双系统 
		return 1;
	}

	fclose(fstab);

	return 0;
}


int SetBootmode(const char *bootmode) {

	//open misc partition
	FILE *misc = fopen("/dev/block/platform/msm_sdcc.1/by-name/misc", "wb");
	if (NULL == misc) {
		printf("Error opening misc partition. \n");
		return -1;
	}

	//write bootmode 
	fseek(misc, 0x1000, SEEK_SET);
	if (fputs(bootmode, misc) < 0) {
		printf("Error writing bootmode to misc partition. \n");
		return -1;
	}

	//close 
	fclose(misc);
	return 0;
}

int getBootmode(char *bootmode) {

	//open misc partition
	FILE *misc = fopen("/dev/block/platform/msm_sdcc.1/by-name/misc", "rb");
	if (misc == NULL) {
		printf("Error opening misc partition .\n");
		return -1;
	}

	//write bootmode 
	fseek(misc, 0x1000, SEEK_SET);
	if (fgets(bootmode, 13, misc) == NULL) {
		printf("Error reading bootmode from misc partition. \n");
		return -1;
	}

	//close 
	fclose(misc);

	return 0;
}


//检查哪个系统为被系统的系统，或者不支持双系统启动
int check_active_system() {
	if (is_dualsystem() == 1) {
		char bootmode[13];
		getBootmode(bootmode);
		if (strcmp(bootmode, "boot-system0") == 0)
			return 1; //表示当前的系统为系统1
		else if (strcmp(bootmode, "boot-system1") == 0)
			return 2; //表示当前的系统为系统2 
	}

	return 0; //返回0，表示不支持双系统启动
}

//对得到的数据判断，再输出到终端中
void get_active_system(int num) {
	if (num == 1) 
		printf("Active system is system0\n");
	else if (num == 2) 
		printf("Active system is system1\n");
	else if (num == 0)
		printf("Not support dualsystem !\n");
}


//激动双系统中的其中一个系统
//Active dualsystem 

//开启和关闭双系统共存

//重点判断 
//当开启双系统共存的时候，会生成如下的文件和目录
//cwm '/data_root'                -> folder 
//cwm '/data_root/.truedualboot'  -> files    此文件用于判断双系统共存
//cwm '/data_root/system0'        -> folder 
//cwm '/data_root/system1'        -> folder 
//

#define DUALBOOT_FILE_TRUEDUALBOOT "/data_root/.truedualboot"
#define USERDATA_SYSTEM0           "/data_root/system0"
#define USERDATA_SYSTEM1           "/data_root/system1"
#define TDB_USERDATA_ROOT          "/data_root"

int isTDBEnabled() {
	struct stat st;
	int ret = stat(DUALBOOT_FILE_TRUEDUALBOOT, &st);
	return (ret == 0); //返回1或者是0 ， 1表示已经开启了双系统共存，0表示没有开启双系统共存
}

int EnableTDB(int enable) {
	//1为打开
	//0 为关闭
	char tmp[1024];
	__system("mount /data");
	printf("Formating the 'data' partition first. \n");
	snprintf(tmp, 1024, "%s", "cd /data ; for f in $(ls -a | grep -v ^media$); do rm -rf $f; done");
	__system(tmp);

	struct stat st;

	if (0 == stat("/data/media/0", &st)) {
		char *layout_version = "2";
           FILE* f = fopen("/data/.layout_version", "wb");
            if (NULL != f) {
                fwrite(layout_version, 1, 2, f);
                fclose(f);
            }
            else {
                printf("error opening /data/.layout_version for write.\n");
            }

	} else {
		printf("/data/media/0 not found\n");
	}

      if (enable) { 
         FILE *pFile = fopen(DUALBOOT_FILE_TRUEDUALBOOT, "w");
         if (pFile == NULL) {
	printf("TrueDualBoot: failed creating file");
	 } 
 fclose(pFile);
      } else {
	     remove(DUALBOOT_FILE_TRUEDUALBOOT);
      }

     return 0;
}



int E_D_TDB(char *cmd) {
	if (strcmp(cmd, "enabled") == 0 && !isTDBEnabled()) {
		EnableTDB(1);
		 mkdir(TDB_USERDATA_ROOT,0777);//build in linux need to add ,the second parse to mkdir(const char *path, int mode)

                 mkdir(USERDATA_SYSTEM0,0777);         
                 mkdir(USERDATA_SYSTEM1,0777); 
	} else if (strcmp(cmd, "disabled") == 0) { //
		EnableTDB(0);
		remove(TDB_USERDATA_ROOT);
	}
     return 0;
}

int usages() {
	        printf("DBS is the tools for set the bootmode from cmd line\n"
		       "DBS -s -mutiboot enabled       ,Enable dualboot shared the same userdata partition\n"
		       "DBS -s -mutiboot disabled      ,Disable dualboot shared the same userdata partition\n"
		       "DBS -s -active boot-system0    ,Set bootmode is system0\n"
		       "DBS -s -active boot-system1    ,Set bootmode is system1\n"
		       "DBS -c                         ,Check which system is the active system when in dualsystem\n"
		       "DBS  [ -h | --help | -help]    ,Show this message \n");

	return 0;
}
	

int main(int argc, char *argv[]) {

	if (argc < 2 ) {

		usages();	

	} else {

		if (strcmp(argv[1], "-s") == 0) {
			if (strcmp(argv[2], "-active") == 0) {
			SetBootmode(argv[3]);
			__system("sync");
			} else  if (strcmp(argv[2], "-mutiboot") == 0) {
				 E_D_TDB(argv[3]);
				 __system("sync");
			}

		} else if (strcmp(argv[1], "-c") == 0) {
			get_active_system(check_active_system());//用于显示哪个系统为激动的系统，或者是不支持双系统启动

		} else if (strcmp(argv[1], "-h") == 0 || 
				strcmp(argv[1], "-help") == 0  ||
			       	strcmp(argv[1], "--help") == 0) {
		       usages();	
	

	}
   }

return 0;
}








