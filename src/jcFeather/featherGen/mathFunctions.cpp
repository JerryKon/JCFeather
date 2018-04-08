#include "mathFunctions.h"
#include "HashTable.h"

extern HashTable*   turtlesDataBase;

namespace jerryC{
	//------------------
	idDist::idDist(){_id=-1;_distance=0;}
	idDist::~idDist(){}
	//------------------
	jcString::jcString()
	{
		_data=NULL;
		_length=0;
	}
	jcString::~jcString(){ clear(); }
	bool jcString::getSpace(unsigned long size)
	{
		_data = (char*)malloc(size);
		if(_data==NULL)
			return false;
		return true;
	}
	void jcString::clear(){
		if(_data!=NULL)
		{
			free(_data);
			_data= NULL;
			_length=0;
		}
	}
	//------------------

	void idDistSort(std::vector<idDist> &s, int l, int r)
	{
			int i, j;
			long xid;
			float x;
			if (l < r)
			{
				i = l;
				j = r;

				x = s[i]._distance;

				xid = s[i]._id;
				while (i < j)
				{
					while(i < j && s[j]._distance > x) j--; /* 从右向左找第一个小于x的数 */
					if(i < j) 
					{ 
						s[i]._id = s[j]._id; 
						s[i]._distance = s[j]._distance;
						i++; 
					}
					while(i < j && s[i]._distance < x) i++;  /* 从左向右找第一个大于x的数 */
					if(i < j) 
					{ 
						s[j]._id = s[i]._id; 
						s[j]._distance = s[i]._distance;
						j--; 
					}
				}

				s[i]._id = xid;
				s[i]._distance = x;

				idDistSort(s, l, i-1); /* 递归调用 */
				idDistSort(s, i+1, r);
			}
	}

	jcFeaNearID::jcFeaNearID(){_id = NULL;_size=0;_realNum=0;}
	jcFeaNearID::jcFeaNearID(int newSize)
	{
		_size = 0;
		_realNum = 0;
		_id = reinterpret_cast<long*>(malloc(newSize*sizeof(long)));
		if( _id==NULL ) return;

		for(int i=0;i<newSize;++i)
			_id[i] = -1;
		_size = newSize;
	}
	jcFeaNearID::~jcFeaNearID(){clear();}
	void jcFeaNearID::clear()
	{
		if(_id!=NULL) free(_id);
		_id = NULL;
		_size = 0;
		_realNum = 0;
	}
	long* jcFeaNearID::getValue(int index)
	{
		if(index<_size&&index>=0) return (_id+index);
		else return NULL;
	}
	void jcFeaNearID::setValue(unsigned int index,long newID)
	{
		if( index<(unsigned int)_size )
		{
			_id[index] = newID;
			++_realNum;
		}
		else return;
	}
	int jcFeaNearID::getRealCount(){return _realNum;}
	int jcFeaNearID::getSize(){return _size;}
};