
#ifndef _jcQueryData_
#define _jcQueryData_

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MStatus.h>
#include <maya/MArgDatabase.h>
#include "tinyxml.h"

/*
	this cmd is used to query the feather cache data, currently ,you can use it to query the bounding box of the xml file.
	more data can be queryed in the future.
*/
class jcQueryData : public MPxCommand
{
public:
        jcQueryData() {};
		~jcQueryData(){}; 

        MStatus  doIt( const MArgList& args );
        static void*    creator();
		static MSyntax newSyntax();
public:
		TiXmlDocument _feaXMLDoc;//xml file doc
		TiXmlElement *_baseEle;//jcFeather_CacheFile element
};

#endif