#ifndef _ADVANCED_FUNCTIONS_HPP
#define _ADVANCED_FUNCTIONS_HPP

#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIUI_SETTINGS_FILE "/cache/miui_recovery/miui_recovery_config.ini"

using namespace std;

class MIFunc {
	public:
		MIFunc();
		~MIFunc();
/**********************************/
/*       Start file parser        */
/*    Original source by PhilZ    */
/**********************************/
// todo: parse settings file in one pass and make pairs of key:value
// get value of key from a given config file
		int read_config_file(char* config_file, char *key, char *value, char *value_def);

		// set value of key in config file
		int write_config_file(char* config_file, char* key, char* value);
	private:
		bool nandroid_md5sum;
		bool skip_cdi;
		string backup_method;
		
};

#endif





