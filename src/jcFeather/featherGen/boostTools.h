#ifndef _boostTools_
#define _boostTools_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/lexical_cast.hpp>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <fstream>

using namespace boost;
namespace bstFs= boost::filesystem;
namespace bstInter=boost::interprocess;
namespace jerryC
{
	void memoryMapWriteFile(const char* file,const char* data,unsigned long len);

	bool getDir_Name(const char* origpath,bool remove,std::string& dir,std::string& fileName);

	bool createDirs(const char* dir);
	
	template <class T> 
	void stringAppendVector(std::string &str,const T value)
	{
		str.append(boost::lexical_cast<std::string>(value.x));
		str.append(" ");
		str.append(boost::lexical_cast<std::string>(value.y));
		str.append(" ");
		str.append(boost::lexical_cast<std::string>(value.z));
	}

	template <class T> 
	void stringAppendColor(std::string &str,const T value)
	{
		str.append(boost::lexical_cast<std::string>(value.r));
		str.append(" ");
		str.append(boost::lexical_cast<std::string>(value.g));
		str.append(" ");
		str.append(boost::lexical_cast<std::string>(value.b));
	}
};
#endif
