#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "miui_func.hpp"
#include "utils_func.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

#include "common.h"
#include "advanced_functions.hpp"
class MIFunc;

utils::utils() {
	enable_md5sum = true;
}

utils::~utils() {
	}


vector <string> utils::get_files(string backup_path) {
	DIR* dir;
	dir = opendir(backup_path.c_str());
	vector <string> files;
	if (dir == NULL) {
		cout << "Error opening " << backup_path << endl;
		return files;
	}

	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		int name_len = strlen(entry->d_name);
		if (entry->d_type == DT_DIR ||
				strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strstr(entry->d_name, ".md5") != NULL)
			continue;
		if (entry->d_type == DT_REG && name_len >= 4) {
			files.push_back(entry->d_name);
		}

	
	}
	closedir(dir);
	return files;
}


bool utils::Make_MD5(string backup_path) {
	string name;
	string Full_File;
	vector <string> file_list;
	file_list = get_files(backup_path);
	if (file_list.empty()) {
		cout << "empty folder" << endl;
		return false;
	} else {
       cout << "* Generating MD5 *" << endl;
	for (int i = 0; i < file_list.size(); i++) {
		Full_File = backup_path + "/" + file_list.at(i);

			if (access(Full_File.c_str(), F_OK) == 0) {
                                setfn(Full_File);
				if (computeMD5() == 0) {
				     if (write_md5digest() == 0) {
					printf("File: <%s>\n",file_list.at(i).c_str());
					printf(" * MD5 Create. \n");
				} else {
					printf("File: <%s>\n",file_list.at(i).c_str());
					printf(" * MD5 Error!\n");
					return false;
				}
			}
		}
	     }
	}
	return true;
}


bool utils::Check_MD5(string backup_path) {
	string Full_Filename, md5file;
	int index = 0;
	vector <string> file_list;

	file_list = get_files(backup_path);
	if (file_list.empty()) {
		cout << "empty folder" << endl;
		return false;
	} else {
       cout << "* Generating MD5 *" << endl;
	for (int i = 0; i < file_list.size(); i++) {
		Full_Filename = backup_path + "/" + file_list.at(i);

			if (access(Full_Filename.c_str(), F_OK) == 0) {
				md5file = Full_Filename;
				md5file += ".md5";
				if (access(md5file.c_str(), F_OK) != 0) {
					printf("No md5 file found for '%s'\n", file_list.at(i).c_str());
					printf("Please unselect Enable MD5 verification to restore.\n");
					return false;
				}
				setfn(Full_Filename);
				if (verify_md5digest() != 0) {
					printf("MD5 failed to match on '%s' \n", file_list.at(i).c_str());
					return false;
				} else {
					printf("File: <%s>\n",file_list.at(i).c_str());
					printf("MD5 match...\n");
				}
			}
		}
			return true;
	}
	return false;
}

int utils::read_file(string fn, string& results) {
	ifstream file;
	file.open(fn.c_str(), ios::in);
	if (file.is_open()) {
		file >> results;
		file.close();
		return 0;
	}
	LOGE("Cannot find file '%s'\n", fn.c_str());
	return -1;
}

int utils::read_file(string fn, vector<string>& result) {
	ifstream file;
	string line;
	file.open(fn.c_str(), ios::in);
	if (file.is_open()) {
		while (getline(file, line))
			result.push_back(line);
		file.close();
		return 0;
	}
	LOGE("Cannot find file '%s'\n", fn.c_str());
	return -1;
}

int utils::write_file(string fn, string& line) {
	FILE* file;
	file = fopen(fn.c_str(), "w");
	if (file != NULL) {
		fwrite(line.c_str(), line.size(), 1, file);
		fclose(file);
		return 0;
	}
	LOGE("Cannot find file '%s'\n", fn.c_str());
	return -1;
}


bool  utils::enabled_md5sum() {
	MIFunc *get_value;
	char val[PROPERTY_VALUE_MAX];
	get_value->read_config_file(MIUI_SETTINGS_FILE, "nandroid_md5sum",val, "on");
	if (strcmp(val, "off") == 0 || strcmp(val, "0") == 0) {
		enable_md5sum = false;
		return false;
	}
	enable_md5sum = true;
	return true;
}




