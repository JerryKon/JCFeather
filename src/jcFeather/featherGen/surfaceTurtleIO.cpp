#include "surfaceTurtleIO.h"
#include <maya/MGlobal.h>
#include <string>

bool surfaceTurtleIO::_ascii=false;
surfaceTurtleIO::surfaceTurtleIO()
{
	init();
}
void surfaceTurtleIO::init()
{ 
	_surfaces.clear();
	_turtlesG.clear();
}

bool surfaceTurtleIO::writeInfoTo(const surfaceTurtleIO &outData,const char* fileName)
{
	if(!_ascii)
	{
		ofstream ofs(fileName,ios::out|ios::binary);
		if(!ofs)return false;
		boost::archive::binary_oarchive  oa(ofs);
		oa<<outData;
	}
	else
	{
		ofstream ofs(fileName);
		if(!ofs)return false;
		boost::archive::text_oarchive  oa(ofs);
		oa<<outData;
	}
	return true;
}
bool surfaceTurtleIO::readInfoFrom(surfaceTurtleIO &inData,const char* fileName)
{
	inData._surfaces.clear();
	inData._turtlesG.clear();
	if(!_ascii)
	{
		ifstream ifs(fileName,ios::in|ios::binary);
		if(!ifs)return false;
		boost::archive::binary_iarchive  ia(ifs);
		ia>>inData;
	}
	else
	{
		ifstream ifs(fileName);
		if(!ifs)return false;
		boost::archive::text_iarchive  ia(ifs);
		ia>>inData;
	}
	return true;
}
bool surfaceTurtleIO::readInfoFrom(surfaceTurtleIO *inData,const char* fileName)
{
	inData->_surfaces.clear();
	inData->_turtlesG.clear();
	if(!_ascii)
	{
		ifstream ifs(fileName,ios::in|ios::binary);
		if(!ifs)return false;
		boost::archive::binary_iarchive  ia(ifs);
		ia>>inData;
	}
	else
	{
		ifstream ifs(fileName);
		if(!ifs)return false;
		boost::archive::text_iarchive  ia(ifs);
		ia>>inData;
	}
	return true;
}