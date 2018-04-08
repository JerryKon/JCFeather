#include "surfaceTurtleArray.h"

surfaceData::surfaceData(){init();}
surfaceData::~surfaceData(){init();}
void surfaceData::init()
{
	_degreeInU = _degreeInV=3;
	_formU = MFnNurbsSurface::kOpen;
	_formV = MFnNurbsSurface::kOpen;
	_createRational=false;
	_uKnotSequences.clear();
	_vKnotSequences.clear();
	_controlVertices.clear();
}


cacheFeatherData::cacheFeatherData(){init();}
cacheFeatherData::~cacheFeatherData(){init();}
void cacheFeatherData::init()
{
	_randSeed = 0;
	_mainColor=MColor(1,1,1);
	_surfaceUV[0] = 0;
	_surfaceUV[1] = 0;
	_colorTexOverride.clear();
}
void cacheFeatherData::setDataWithFeather(const singleFeather &f)
{
	_randSeed = f.randSeed;
	_mainColor = f.mainColor;
	_surfaceUV[0] = f.surfaceUV[0];
	_surfaceUV[1] = f.surfaceUV[1];
	_colorTexOverride = f.colorTexOverride;
	_bbox=f.proxyBBox;
}


surfaceArray::surfaceArray(){}
surfaceArray::~surfaceArray(){}
void surfaceArray::addObject( const MObject &obj ,const singleFeather &f)
{
	MFnNurbsSurface surFn;
	surfaceData tempData;

	if( surFn.setObject(obj)!=MS::kSuccess ) return ;

	surFn.getCVs(tempData._controlVertices,MSpace::kWorld);
	surFn.getKnotsInU(tempData._uKnotSequences);
	surFn.getKnotsInV(tempData._vKnotSequences);
	tempData._degreeInU = surFn.degreeU();
	tempData._degreeInV = surFn.degreeV();
	tempData._formU = surFn.formInU();
	tempData._formV = surFn.formInV();
	tempData._createRational = false;

	_surfaceD.push_back(tempData);

	cacheFeatherData tempD;
	tempD.setDataWithFeather(f);
	_surfaceFeaInfo.push_back(tempD);
}
void surfaceArray::clear(){ 
		_surfaceD.clear();
		_surfaceFeaInfo.clear();
}

//------------------

turtlesArray::turtlesArray(){}
turtlesArray::~turtlesArray(){}
void turtlesArray::addObject( const turtles &t ,const singleFeather &f)
{
	_turtlesD.push_back(t);

	cacheFeatherData tempD;
	tempD.setDataWithFeather(f);

	_turtlesFeaInfo.push_back(tempD);
}
void turtlesArray::clear()
{
	_turtlesD.clear();
	_turtlesFeaInfo.clear();
}


//------------------------------


turtlesArrayIO::turtlesArrayIO(){}
turtlesArrayIO::~turtlesArrayIO(){}

void turtlesArrayIO::clear()
{
	_turtlesD.clear();
}
bool turtlesArrayIO::writeInfoTo(const char* fileName)
{
	ofstream ofs(fileName,ios::out|ios::binary);
	if(!ofs)return false;
	boost::archive::binary_oarchive  oa(ofs);
	oa<<_turtlesD;
	return true;
}
bool turtlesArrayIO::readInfoFrom(const char* fileName)
{
	ifstream ifs(fileName,ios::in|ios::binary);
	if(!ifs)return false;
	boost::archive::binary_iarchive  ia(ifs);
	ia>>_turtlesD;
	return true;
}