/*
 * Calculate the MD5 from file
 * Verification the MD5 from file
 */
#include <string>
#include <stdio.h>
#include <vector>
#include "twrpDigest.hpp"

using namespace std;

class utils : public twrpDigest {
	public:
		utils();
		~utils();
		void get_file_in_folder(const char *backup_path);
		vector <string> get_files(string backup_path);
		bool Make_MD5(string backup_path);
		bool Check_MD5(string backup_path);

		 static int read_file(string fn, vector<string>& result); //read from file
	    static int read_file(string fn, string& result); //read from file
	    static int write_file(string fn, string& line); // write from file
	    //Return true if the path exists 
	    static bool Path_Exists(string path);
	
	    //Return file type of tar 
	    static int Get_File_Type(string fn);

	    //Return "file.name" from a full /path/to/file.name
	    static string Get_Filename(string Path);

	    //Returns "path/to/" from a full /path/to/file.name
	    static string Get_Path(string Path);

	    //Returns "/path" from a full /path/to/file.name
	    static string Get_Root_Path(string Path);

	    //Get folder size 
	    static  unsigned long long Get_Folder_Size(const string& Path, bool Display_Error);

	    //Get file size 
	    static unsigned long Get_File_Size(string Path); 
	    	
	    static int Exec_Cmd(const string& cmd, string &result); //execute a command and return the result as a string by reference
	static int Exec_Cmd(const string& cmd); //execute a command


	    static  int Wait_For_Child(pid_t pid, int *status, string Child_Name)

	   static vector<string> split_string(const string &in, char del, bool skip_empty)

	  static void Is_Data_Media(int has_data_media); 



};




