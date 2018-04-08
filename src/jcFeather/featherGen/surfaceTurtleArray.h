#ifndef _surfaceTurtleArray_
#define _surfaceTurtleArray_

#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MString.h>
#include <maya/MColor.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>

#include <vector>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "featherInfo.h"

using namespace std;
class turtles;
//存储nurbs surface的信息和一根羽毛所需的turtles信息
class surfaceData
{
public:
	friend class boost::serialization::access;
	surfaceData();
	~surfaceData();
	void init();

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar << _degreeInU;
		ar << _degreeInV;
		ar << _formU;
		ar << _formV;
		ar << _createRational;
		unsigned int len=_uKnotSequences.length();
		double tempD=0;
		ar << len;
		for(unsigned int ii=0;ii<len;++ii)
		{
			tempD = _uKnotSequences[ii];
			ar << tempD;
		}

		len=_vKnotSequences.length();
		ar << len;
		for(unsigned int ii=0;ii<len;++ii)
		{
			tempD = _vKnotSequences[ii];
			ar << tempD;
		}

		len=_controlVertices.length();
		ar << len;
		for(unsigned int ii=0;ii<len;++ii)
		{
			tempD = _controlVertices[ii].x;
			ar << tempD;
			tempD = _controlVertices[ii].y;
			ar << tempD;
			tempD = _controlVertices[ii].z;
			ar << tempD;
		}
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar >> _degreeInU;
		ar >> _degreeInV;
		ar >> _formU;
		ar >> _formV;
		ar >> _createRational;
		unsigned int len=0;
		double tempD=0;
		ar >> len;
		_uKnotSequences.setLength(len);
		for(unsigned int ii=0;ii<len;++ii)
		{
			ar >> tempD;
			_uKnotSequences[ii]=tempD;
		}

		ar >> len;
		_vKnotSequences.setLength(len);
		for(unsigned int ii=0;ii<len;++ii)
		{
			ar >> tempD;
			_vKnotSequences[ii]=tempD;
		}

		ar >> len;
		_controlVertices.setLength(len);
		double tempD2=0,tempD3=0;
		for(unsigned int ii=0;ii<len;++ii)
		{
			ar >> tempD; ar >> tempD2; ar >> tempD3;
			_controlVertices[ii] = MPoint(tempD,tempD2,tempD3);
		}
	}
	template<class Archive>
	void serialize(Archive & ar,const unsigned int file_version )
	{
		boost::serialization::split_member(ar, *this, file_version);
	}

public:
	unsigned int _degreeInU, _degreeInV;
	MFnNurbsSurface::Form _formU, _formV;
	bool _createRational;
	MDoubleArray _uKnotSequences, _vKnotSequences;
	MPointArray _controlVertices;
};

class cacheFeatherData
{
public:
	friend class boost::serialization::access;
	unsigned int _randSeed;
	MColor _mainColor;
	float _surfaceUV[2];
	MString _colorTexOverride;
	MBoundingBox _bbox;

public:
	cacheFeatherData();
	~cacheFeatherData();
	void init();
	void setDataWithFeather(const singleFeather &f);

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar<<_randSeed;
		double tempD=0;
		tempD = _mainColor.r;	ar<<tempD;
		tempD = _mainColor.g;	ar<<tempD;
		tempD = _mainColor.b;	ar<<tempD;
		ar<<_surfaceUV;
		string tempS="None";
		if(_colorTexOverride.length()!=0)
			tempS = string(_colorTexOverride.asChar());
		ar<<tempS;
		tempD = _bbox.min().x;ar<<tempD;
		tempD = _bbox.max().x;ar<<tempD;
		tempD = _bbox.min().y;ar<<tempD;
		tempD = _bbox.max().y;ar<<tempD;
		tempD = _bbox.min().z;ar<<tempD;
		tempD = _bbox.max().z;ar<<tempD;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar>>_randSeed;
		double tempD=0;
		ar>>tempD;_mainColor.r=tempD ;
		ar>>tempD;_mainColor.g=tempD ;
		ar>>tempD;_mainColor.b=tempD;
		ar>>_surfaceUV;

		string tempS="";
		_colorTexOverride="";
		ar>>tempS;
		_colorTexOverride.set(tempS.c_str());

		MPoint pt1,pt2;
		ar>>tempD;pt1.x=tempD;
		ar>>tempD;pt2.x=tempD;
		ar>>tempD;pt1.y=tempD;
		ar>>tempD;pt2.y=tempD;
		ar>>tempD;pt1.z=tempD;
		ar>>tempD;pt2.z=tempD;
		_bbox=MBoundingBox(pt1,pt2);
	}
	template<class Archive>
	void serialize(Archive & ar,const unsigned int file_version )
	{
		boost::serialization::split_member(ar, *this, file_version);
	}
};

class surfaceArray
{
public:
	friend class boost::serialization::access;
	surfaceArray();
	~surfaceArray();

	void addObject( const MObject &obj ,const singleFeather &f);
	void clear();

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _surfaceD;
		ar & _surfaceFeaInfo;
	}
public:
	std::vector<surfaceData> _surfaceD;
	std::vector<cacheFeatherData> _surfaceFeaInfo;
};

class turtlesArray
{
	public:
		friend class boost::serialization::access;
		turtlesArray();
		~turtlesArray();

		void addObject( const turtles &t ,const singleFeather &f);
		void clear();

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & _turtlesD;
			ar & _turtlesFeaInfo;
		}
	public:
		std::vector<turtles> _turtlesD;
		std::vector<cacheFeatherData> _turtlesFeaInfo;
};

//-------used in jcWriteGrowData cmd,write turtles data to a file.
class turtlesArrayIO
{
	public:
		//friend class boost::serialization::access;
		turtlesArrayIO();
		~turtlesArrayIO();

		void clear();
		bool writeInfoTo(const char* fileName);
		bool readInfoFrom(const char* fileName);
	public:
		std::vector<turtles> _turtlesD;

};
#endif