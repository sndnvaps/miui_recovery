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
#include "twrpDigest.hpp"
#include "utils_func.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "common.h"

extern "C" {
        #include "roots.h"
}

utils::utils() {
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
	for (unsigned int i = 0; i < file_list.size(); i++) {
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
	for (unsigned int i = 0; i < file_list.size(); i++) {
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


bool utils::Path_Exists(string path) {
	//Check to see if the path exists
	struct stat st;
	if (stat(path.c_str(), &st) != 0) {
		return false;
	}

	return true;
}


int utils::Get_File_Type(string fn) {
	string::size_type i = 0;
	int firstbyte = 0, secondbyte = 0;
	char header[3];

	ifstream f;
	f.open(fn.c_str(), ios::in | ios::binary);
	f.get(header, 3);
	f.close();
	firstbyte = header[i] & 0xff;
	secondbyte = header[++i] & 0xff;

	if (firstbyte == 0x1f && secondbyte == 0x8b)
		return 1; // Compressed
	else if (firstbyte == 0x4f && secondbyte == 0x41)
		return 2; // Encrypted
	else
		return 0; // Unknown

	return 0;
}

int utils::Wait_For_Child(pid_t pid, int *status, string Child_Name) {
	pid_t rc_pid;

	rc_pid = waitpid(pid, status, 0);
	if (rc_pid > 0) {
		if (WEXITSTATUS(*status) == 0)
			LOGINFO("%s process ended with RC=%d\n", Child_Name.c_str(), WEXITSTATUS(*status)); // Success
		else if (WIFSIGNALED(*status)) {
			LOGINFO("%s process ended with signal: %d\n", Child_Name.c_str(), WTERMSIG(*status)); // Seg fault or some other non-graceful termination
			return -1;
		} else if (WEXITSTATUS(*status) != 0) {
			LOGINFO("%s process ended with ERROR=%d\n", Child_Name.c_str(), WEXITSTATUS(*status)); // Graceful exit, but there was an error
			return -1;
		}
	} else { // no PID returned
		if (errno == ECHILD)
			LOGINFO("%s no child process exist\n", Child_Name.c_str());
		else {
			LOGINFO("%s Unexpected error\n", Child_Name.c_str());
			return -1;
		}
	}
	return 0;
}


vector<string> utils::split_string(const string &in, char del, bool skip_empty) {
	vector<string> res;

	if (in.empty() || del == '\0')
		return res;

	string field;
	istringstream f(in);
	if (del == '\n') {
		while(getline(f, field)) {
			if (field.empty() && skip_empty)
				continue;
			res.push_back(field);
		}
	} else {
		while(getline(f, field, del)) {
			if (field.empty() && skip_empty)
				continue;
			res.push_back(field);
		}
	}
	return res;
}

void utils::Is_Data_Media(int has_data_media) {

	if (is_data_media()) {
		has_data_media = 1;
	} else {
		has_data_media = 0;
	}
}

// Returns "file.name" from a full /path/to/file.name
string utils::Get_Filename(string Path) {
	size_t pos = Path.find_last_of("/");
	if (pos != string::npos) {
		string Filename;
		Filename = Path.substr(pos + 1, Path.size() - pos - 1);
		return Filename;
	} else
		return Path;
}

// Returns "/path/to/" from a full /path/to/file.name
string utils::Get_Path(string Path) {
	size_t pos = Path.find_last_of("/");
	if (pos != string::npos) {
		string Pathonly;
		Pathonly = Path.substr(0, pos + 1);
		return Pathonly;
	} else
		return Path;
}

// Returns "/path" from a full /path/to/file.name
string utils::Get_Root_Path(string Path) {
	string Local_Path = Path;

	// Make sure that we have a leading slash
	if (Local_Path.substr(0, 1) != "/")
		Local_Path = "/" + Local_Path;

	// Trim the path to get the root path only
	size_t position = Local_Path.find("/", 2);
	if (position != string::npos) {
		Local_Path.resize(position);
	}
	return Local_Path;
}

unsigned long long utils::Get_Folder_Size(const string& Path, bool Display_Error) {
	DIR* d;
	struct dirent* de;
	struct stat st;
	unsigned long long dusize = 0;
	unsigned long long dutemp = 0;

	d = opendir(Path.c_str());
	if (d == NULL)
	{
		LOGERR("error opening '%s'\n", Path.c_str());
		LOGERR("error: %s\n", strerror(errno));
		return 0;
	}

	while ((de = readdir(d)) != NULL)
	{
		if (de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, "lost+found") != 0)
		{
			dutemp = Get_Folder_Size((Path + "/" + de->d_name), Display_Error);
			dusize += dutemp;
			dutemp = 0;
		}
		else if (de->d_type == DT_REG)
		{
			stat((Path + "/" + de->d_name).c_str(), &st);
			dusize += (unsigned long long)(st.st_size);
		}
	}
	closedir(d);
	return dusize;
}


unsigned long utils::Get_File_Size(string Path) {
	struct stat st;

	if (stat(Path.c_str(), &st) != 0)
		return 0;
	return st.st_size;
}

/* Execute a command */
int utils::Exec_Cmd(const string& cmd, string &result) {
	FILE* exec;
	char buffer[130];
	int ret = 0;
	exec = __popen(cmd.c_str(), "r");
	if (!exec) return -1;
	while(!feof(exec)) {
		memset(&buffer, 0, sizeof(buffer));
		if (fgets(buffer, 128, exec) != NULL) {
			buffer[128] = '\n';
			buffer[129] = NULL;
			result += buffer;
		}
	}
	ret = __pclose(exec);
	return ret;
}

int utils::Exec_Cmd(const string& cmd) {
	pid_t pid;
	int status;
	switch(pid = fork())
	{
		case -1:
			LOGERR("Exec_Cmd(): vfork failed!\n");
			return -1;
		case 0: // child
			execl("/sbin/sh", "sh", "-c", cmd.c_str(), NULL);
			_exit(127);
			break;
		default:
		{
			if (TWFunc::Wait_For_Child(pid, &status, cmd) != 0)
				return -1;
			else
				return 0;
		}
	}
}

