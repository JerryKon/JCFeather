#include "jcFeatherAbout.h"
#include <maya/MGlobal.h>
#include "featherInfo.h"

extern const char* jcFeatherVersion;

void* jcFeatherAbout::creator()
{
    return new jcFeatherAbout();
}

MStatus jcFeatherAbout::doIt( const MArgList& args )
{
	MString result;
	if( !featherTools::checkLicense() )
		return MS::kFailure;
	result +="JCFeather Version ";
	result += jcFeatherVersion;
	result += " For Maya";
	result += MGlobal::mayaVersion();
	setResult(result);
	return MS::kSuccess;
}