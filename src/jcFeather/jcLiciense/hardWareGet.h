#ifndef _hardWareGet_
#define _hardWareGet_

#include <windows.h> 
#include <wincon.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

#include "cryptlib.h"
#include "filters.h"
#include "bench.h"
#include "osrng.h"
#include "hex.h"
#include "modes.h"
#include "files.h"

#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

using namespace std;
using namespace CryptoPP;

class hardWareGet
{
public:
	hardWareGet(){
		#ifdef JCTRIAL_VERSION
			licenseFile=string("C:/flexlm/jcFeatherTrial2.lic");
		#else
			licenseFile = string("C:/flexlm/jcFeather2.lic");
		#endif
	};
	~hardWareGet(){};

	void getmacString();
	string MyDecryptFile(const char* file,unsigned char key[]);
	bool dezhengshuCheck();
	string getFileAscii(const char* path);
public:
	string licenseFile;
	string CrypFile;
	vector<string> Macs;
};

#endif