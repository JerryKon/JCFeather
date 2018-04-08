#include "jcQueryData.h"
#include <maya/MGlobal.h>
#include "featherInfo.h"

extern const char* jcFeatherVersion;

void* jcQueryData::creator()
{
    return new jcQueryData();
}

MSyntax jcQueryData::newSyntax()
{
   MSyntax syntax;

   CHECK_MSTATUS( syntax.addFlag("-p", "-path", MSyntax::kString) );
   CHECK_MSTATUS( syntax.addFlag("-bb", "-boundbox", MSyntax::kNoArg) );
   return syntax;
}
MStatus jcQueryData::doIt( const MArgList& args )
{
	if( !featherTools::checkLicense() )
		return MS::kFailure;
	
	MArgDatabase argData(syntax(), args);
	MString fileName;
	if(argData.isFlagSet("-path") )
		argData.getFlagArgument("-path", 0, fileName);
	else
	{
		MGlobal::displayError("Need a xml file.");
		return MS::kFailure;
	}
	int len=fileName.length();
	MString postfix=fileName.substring(len-4,len-1);
	if(postfix!=".xml") return MS::kFailure;

	//-------open xml
	if(!_feaXMLDoc.LoadFile(fileName.asChar())) return MS::kFailure;
	_baseEle=_feaXMLDoc.RootElement();
	
	//----get bounding box
	if(argData.isFlagSet("-boundbox") )
	{
		MDoubleArray resultData(6,0);

		double box[6];
		_baseEle->Attribute("minX",&resultData[0]);
		_baseEle->Attribute("maxX",&resultData[1]);

		_baseEle->Attribute("minY",&resultData[2]);
		_baseEle->Attribute("maxY",&resultData[3]);

		_baseEle->Attribute("minZ",&resultData[4]);
		_baseEle->Attribute("maxZ",&resultData[5]);

		setResult(resultData);
	}

	return MS::kSuccess;
}