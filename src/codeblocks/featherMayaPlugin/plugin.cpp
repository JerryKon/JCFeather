#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, "LCA", "0.0", "Any");

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	return status;
}
