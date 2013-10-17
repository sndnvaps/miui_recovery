/*
	Copyright 2012 bigbiff/Dees_Troy TeamWin
	Copyright 2013 sndnvaps Gaojiquan.com GTO 
	This file is part of TWRP/TeamWin Recovery Project.
        This file is part of MIUI RECOVERY Project.

	TWRP is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	TWRP is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with TWRP.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MIUI_FUNC_HPP
#define _MIUI_FUNC_HPP

extern "C" {
#include "digest/md5.h"
}
#include <string>
#include <vector>

using namespace std;

class miui_func {
	public:
		int read_md5digest(void);

                void setfn(string fn); //for md5
                void setdir(string dir);
	        int computeMD5(void);
         	int verify_md5digest(void);
	        int write_md5digest(void);

	private:
	//	int read_md5digest(void);
		string md5fn;
		string line;
		unsigned char md5sum[MD5LENGTH];
};

#endif // _MIUI_FUNC_HPP




