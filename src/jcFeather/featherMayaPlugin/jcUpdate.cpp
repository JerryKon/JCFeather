#include "jcUpdate.h"
#include "featherInfo.h"
#include "jcFeather.h"
#include "jcPreFeather.h"
#include <maya/MGlobal.h>
#include <maya/MItDependencyNodes.h>

void* jcUpdate::creator()
{
    return new jcUpdate();
}

MSyntax jcUpdate::newSyntax()
{
   MSyntax syntax;
   CHECK_MSTATUS( syntax.addFlag("-at", "-allTexture", MSyntax::kNoArg) );
   return syntax;
}

void jcUpdate::jcUpdateFeatherColor()
{
	MObjectArray objs;
	jcFeather::getActiveJCFeatherNode(objs,false);
	MFnDependencyNode dgNodeFn;
	MObject updateObj;
	for(int ii=0;ii<objs.length();++ii)
	{
		dgNodeFn.setObject(objs[ii]);
		if( !dgNodeFn.findPlug(jcFeather::interactiveTexture).asBool() )
		{
			static_cast<jcFeather*>(dgNodeFn.userNode())->updateJCFTexture();
			dgNodeFn.findPlug(jcFeather::outputMesh).getValue(updateObj);
		}
	}
}
MStatus jcUpdate::doIt( const MArgList& args )
{
	if( !featherTools::checkLicense() )
		return MS::kFailure;
	
	MArgDatabase argData(syntax(), args);
	if(argData.isFlagSet("-at") )
		jcUpdateFeatherColor();

	return MS::kSuccess;
}