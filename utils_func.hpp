/*
 * Calculate the MD5 from file
 * Verification the MD5 from file
 */
#include <string>
#include <stdio.h>
#include <vector>
#include "miui_func.hpp"

using namespace std;
//class miui_func;
//class utils {
class utils : public miui_func {
	public:
		utils();
		~utils();
		void get_file_in_folder(const char *backup_path);
		vector <string> get_files(string backup_path);
		bool Make_MD5(string backup_path);
		bool Check_MD5(string backup_path);
		bool enabled_md5sum();

		 static int read_file(string fn, vector<string>& result); //read from file
	    static int read_file(string fn, string& result); //read from file
	    static int write_file(string fn, string& line); // write from file
	
	private:
	    bool enable_md5sum;



};




