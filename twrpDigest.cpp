/*
	Copyright 2012 bigbiff/Dees_Troy TeamWin
	This file is part of TWRP/TeamWin Recovery Project.

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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>


extern "C" {
	#include "digest/md5.h"
	#include "libcrecovery/common.h"
}

#include "twrpDigest.hpp"
#include "utils_func.hpp"
#include "common.h"

using namespace std;

void twrpDigest::setfn(string fn) {
	md5fn = fn;
}

int twrpDigest::computeMD5(void) {
	string line;
	struct MD5Context md5c;
	FILE *file;
	int len;
	unsigned char buf[1024];
	MD5Init(&md5c);
	file = fopen(md5fn.c_str(), "rb");
	if (NULL == file) 
		return -1;
	while ((len = fread(buf, 1, sizeof(buf), file)) > 0) {
		MD5Update(&md5c, buf, len);
	}
	MD5Final(md5sum, &md5c);
	return 0;
}

int twrpDigest::write_md5digest(void) {
	int i;
	string md5string, md5file;
	char hex[3];
	md5file = md5fn + ".md5";
	for (i = 0; i < 16; ++i) {
		snprintf(hex, 3, "%02x", md5sum[i]);
			md5string += hex;
	//	md5string += hex;
	}
	md5string += " ";
	md5string += basename((char*) md5fn.c_str());
	md5string += + "\n";
	utils::write_file(md5file, md5string);
	return 0;
}

int twrpDigest::read_md5digest(void) {
	string md5file = md5fn + ".md5";
	if (utils::read_file(md5file, line) != 0) 
		return -1;
	return 0;
}

int twrpDigest::verify_md5digest(void) {
	string buf;
	char hex[3];
	int i;
	string md5string;
	if (read_md5digest() != 0)
		return -1;
	stringstream ss(line);
	vector<string> tokens;
	while (ss >> buf)
		tokens.push_back(buf);
	computeMD5();
	for (i = 0; i < 16; ++i) {
		snprintf(hex, 3, "%02x", md5sum[i]);
		md5string += hex;
	}
	if (tokens.at(0) != md5string)
		return -2;
	return 0;
}



