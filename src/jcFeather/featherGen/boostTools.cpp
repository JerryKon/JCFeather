#include "boostTools.h"

namespace jerryC
{
	void memoryMapWriteFile(const char* file,const char* data,unsigned long len)
	{

		std::size_t dataSize=0;

		if(len==0)
			dataSize= strlen(data);
		else
			dataSize= len;

		//Create a file
		{
			bstInter::file_mapping::remove(file);

			std::filebuf fbuf;
			fbuf.open(file, std::ios_base::in | std::ios_base::out| std::ios_base::trunc);
			//Set the size
			fbuf.pubseekoff(dataSize-1, std::ios_base::beg);
			fbuf.sputc(0);
		}

		//Create a file mapping
		bstInter::file_mapping m_file(file, bstInter::read_write);

		//Map the whole file with read-write permissions in this process
		bstInter::mapped_region region(m_file,bstInter::read_write);

		//Get the address of the mapped region
		void * addr       = region.get_address();
		std::size_t size  = region.get_size();

		//copy memory to file
		std::memcpy(addr,data,size);

		region.flush();
	}

	bool getDir_Name(const char* origpath,bool remove,std::string& dir,std::string& fileName)
	{
		bstFs::path myPath(origpath);

		if(!myPath.is_absolute())
			return false;
		if(remove && bstFs::exists(myPath))
			bstFs::remove(myPath);

		dir = myPath.parent_path().generic_string();
		fileName = myPath.stem().generic_string();

		return true;
	}

	bool createDirs(const char* dir)
	{
		return bstFs::create_directories(bstFs::path(dir));
	}

	template <class T>
	void stringAppendNum(std::string &str,const T value)
	{
		 str.append( boost::lexical_cast<std::string>(value) );
	}
}
