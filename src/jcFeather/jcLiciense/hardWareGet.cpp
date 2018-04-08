#include "hardWareGet.h"

void hardWareGet::getmacString()
{
	Macs.clear();
	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	char *mac_addr = (char*)malloc(17);

	AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		printf("JCFeather : Error allocating memory needed to call GetAdaptersinfo\n");
	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen     variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {

		AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			printf("JCFeather : Error allocating memory needed to call GetAdaptersinfo\n");
		 }
	 }

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
		do {
			sprintf(mac_addr, "%02X%02X%02X%02X%02X%02X",
				pAdapterInfo->Address[0], pAdapterInfo->Address[1],
				pAdapterInfo->Address[2], pAdapterInfo->Address[3],
				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

			string macs=string(mac_addr);
			transform(macs.begin(), macs.end(), macs.begin(), toupper);
			Macs.push_back(macs);

			pAdapterInfo = pAdapterInfo->Next;        
		}while(pAdapterInfo);                        
	}
	free(AdapterInfo);
}
string hardWareGet::getFileAscii(const char* path)
{
	string macAd;
	ifstream isf( path);
	if(!isf )
	{
		cerr<<"cannot open file "<<path<<endl;
		return "";
	}
	string result;
	while( getline(isf,macAd) )  
		result=macAd;
	return result;
}
bool hardWareGet::dezhengshuCheck()
{
	#ifdef JCTRIAL_VERSION
		unsigned char key[]	= {0xBC,0x3D,0xCF,0x48,0x34,0xDF,0xE2,0xE6,0x2E,0x1D,0xFE,0x0F,0xDC,0x03,0xC5,0xE6};
	#else
		unsigned char key[]	= {0xBB,0x3A,0xC0,0x4D,0x04,0xD9,0xC2,0xEE,0x0E,0xCA,0xFF,0x1F,0xBC,0x04,0x25,0x36};
	#endif		

	vector<string>::iterator striter;
	for(striter = Macs.begin();striter!= Macs.end();++striter)
	{
		string firstStr = *striter;
		string secondStr = MyDecryptFile(licenseFile.c_str(),key);
		int twoStr = firstStr.compare(secondStr);
		if( !twoStr )
			return true;
	}
	return false;
}
string hardWareGet::MyDecryptFile(const char* file, unsigned char key[])
{ 
	unsigned char iv[]	= {0xCA,0x04,0x73,0x0A,0xD0,0xC9,0xF7,0x00,0x0F,0x0C,0xC1,0xF2,0x2A,0xCC,0xC7,0x08};
	int keysize = 16;

	string result;
	std::ifstream infile(file, std::ios::binary);
	string content((std::istreambuf_iterator<char>(infile)),
                    std::istreambuf_iterator<char>());

	CBC_Mode<AES>::Decryption Decryptor1(key,keysize,iv); 
	StringSource(	content, 
					true,
					new StreamTransformationFilter( Decryptor1,
						new StringSink( result ),
						BlockPaddingSchemeDef::ONE_AND_ZEROS_PADDING,
						true)
			);
    return result;
}