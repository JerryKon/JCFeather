
#include "jcInstancerToMeshCmd.h"
#include "jcFeather.h"
#include "jcFeatherSystem.h"
#include "boostTools.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnInstancer.h>
#include <maya/MDagPathArray.h>
#include <maya/MMatrixArray.h>
jcInstancerToMeshCmd::jcInstancerToMeshCmd()
{}

jcInstancerToMeshCmd::~jcInstancerToMeshCmd()
{}

void *jcInstancerToMeshCmd::creator()
{
   return new jcInstancerToMeshCmd;
}

bool jcInstancerToMeshCmd::isUndoable() const
{
   return false;
}

MSyntax jcInstancerToMeshCmd::newSyntax()
{
   MSyntax syntax;
   CHECK_MSTATUS( syntax.addFlag("-n", "-nodes", MSyntax::kString) );
   return syntax;
}

bool jcInstancerToMeshCmd::getFlagData(const MArgList& args)
{
	MArgDatabase argData(syntax(), args);

	//-----------------------------------------get cmd infos
	MString objs;
	if( argData.isFlagSet("-nodes") )
	{
		argData.getFlagArgument("-nodes", 0, objs);
		objs.split(' ',objects);
	}
	else
		objs = "";

	return true;
}
bool jcInstancerToMeshCmd::convertInstancerToMesh(MDagPath &instancer,MObject &outMeshData)
{
	MStatus status = MS::kSuccess;
	if(!instancer.hasFn(MFn::kInstancer))
		return false;

	MFnDagNode dagNfn(instancer);

	MFnInstancer instanceFn(instancer);
	MDagPathArray allPaths;
    MMatrixArray allMatrices;
    MIntArray pathIndices;
    MIntArray pathStartIndices;
	int numParticles=instanceFn.particleCount();
	
	status = instanceFn.allInstances(allPaths, allMatrices, pathStartIndices, pathIndices);
	CHECK_MSTATUS(status);
	if(status!=MS::kSuccess)
		return false;

	polyObj instanceObj;
	polyObj finalObj;
	MDagPath curPath;
	float matAry[4][4];
	for( int p = 0; p < numParticles; p++ )
    {
            MMatrix particleMatrix = allMatrices[p];
            int numPaths = pathStartIndices[p+1]-pathStartIndices[p];
            int pathStart = pathStartIndices[p];

            //      loop through the instanced paths for this particle
            for( int i = pathStart; i < pathStart+numPaths; i++ )
            {
                int curPathIndex = pathIndices[i];
                curPath = allPaths[curPathIndex];
				curPath.extendToShape();
				if( !instanceObj.initWithMObject(curPath.node(),NULL))
					continue;

                MMatrix instancedPathMatrix = curPath.inclusiveMatrix();
                MMatrix finalMatrixForPath = instancedPathMatrix * particleMatrix;
				finalMatrixForPath.get(matAry);

				MFloatMatrix fMatrix(matAry);
                instanceObj.transform(fMatrix);                
				finalObj.appendMesh(instanceObj);
            }
    }

	return finalObj.convertToDagMObject(outMeshData,NULL);
}

void jcInstancerToMeshCmd::getInstancerNodes(MStringArray &resultObjs)
{
	MStatus status = MS::kFailure;

	MSelectionList selList;
	MDagPath dagP;
	resultObjs.clear();
	MFnDependencyNode dgNodeFn;

	if(objects.length()>0)
	{
	    for(int ii=0;ii<objects.length();++ii)
			selList.add(objects[ii],false);
	}
	else
		MGlobal::getActiveSelectionList(selList);
	
	MString curvesGrp;

	if(selList.length()>0)
	{
		MObject meshData;
		for(unsigned int ii=0;ii<selList.length();ii++)
		{
			if( selList.getDagPath(ii,dagP) != MS::kSuccess) continue;
			if( !convertInstancerToMesh(dagP,meshData) ) continue;
			dgNodeFn.setObject(meshData);
			dgNodeFn.setName((dagP.partialPathName()+"_outMesh#"));
			resultObjs.append(dgNodeFn.name());
		}
	}
}

MStatus jcInstancerToMeshCmd::doIt(const MArgList& args)
{
	if( !featherTools::checkLicense() )	return MS::kFailure;
	if( !this->getFlagData(args) ) return MS::kFailure;

	MStringArray returnResult;
	getInstancerNodes(returnResult);
	for(int ii=0;ii<returnResult.length();++ii)
		MGlobal::executeCommand("sets -e -forceElement initialShadingGroup "+returnResult[ii]);

	//----------set result
	setResult(returnResult);

	return MS::kSuccess;
}
