#ifndef _surfaceTurtleIO_
#define _surfaceTurtleIO_

#include <fstream>
#include <string>
#include "surfaceTurtleArray.h"

using namespace std;
class surfaceTurtleIO
{
public:	
	friend class boost::serialization::access;
	surfaceTurtleIO();
	~surfaceTurtleIO(){};

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & _surfaces;
		ar & _turtlesG;
	}

	static bool writeInfoTo(const surfaceTurtleIO &outData,const char* fileName);
	static bool readInfoFrom(surfaceTurtleIO &inData,const char* fileName);
	static bool readInfoFrom(surfaceTurtleIO *inData,const char* fileName);
	void init();

public:
	surfaceArray _surfaces;
	turtlesArray _turtlesG;
	static bool _ascii;
};
#endif