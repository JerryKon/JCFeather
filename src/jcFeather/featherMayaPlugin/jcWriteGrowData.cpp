
#include "jcWriteGrowData.h"
#include "featherInfo.h"

#include <maya/MGlobal.h> 
#include <maya/MStatus.h> 
#include <maya/MSelectionList.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnDependencyNode.h>

jcWriteGrowData::jcWriteGrowData()
{
}

jcWriteGrowData::~jcWriteGrowData()
{}

void *jcWriteGrowData::creator()
{
   return new jcWriteGrowData;
}
bool jcWriteGrowData::isUndoable() const
{
   return false;
}

void jcWriteGrowData::getScatterData(MDagPath &scatter,jcScatterPointData &stData)
{
	MPlug tempPlug(scatter.node(),jcScatter::outGrowData);

	MFnPluginData plugDFn(tempPlug.asMObject());
	scatterPointData *tempVData = (scatterPointData*)(plugDFn.constData());

	stData = tempVData->m_PointData;
}

void jcWriteGrowData::getVertexDirectionData(MDagPath &pGuid,verDirVector &vdData)
{
	MPlug tempPlug(pGuid.node(),jcPreGuid::outVertexDir);

	MFnPluginData plugDFn(tempPlug.asMObject());
	vertexDirectionData *tempVData = (vertexDirectionData*)(plugDFn.constData());

	vdData = tempVData->m_verDir;
}

void jcWriteGrowData::getComponentArrayData(MDagPath &comArray,MObject &nodePlug, std::vector<turtles> &outData)
{
	outData.clear();
	MPlug tempPlug(comArray.node(),nodePlug);//jcPreFeather::outFeatherTurtleArray);

	MFnPluginData plugDFn(tempPlug.asMObject());
	featherTurtleArrayData *tempData = (featherTurtleArrayData*)(plugDFn.constData());
	if(!tempData) return;
	
	turtles currentT;
	for(int ii=0;ii<tempData->m_turtleArray.size();++ii)
	{
		featherTools::getTurtles(tempData->m_turtleArray[ii],&currentT);
		outData.push_back(currentT);
	}
}

MStatus jcWriteGrowData::doIt(const MArgList& args)
{
	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MStatus status=MS::kSuccess;
	MString fileName="",nodeName="";
	MDagPath growNode;

	//-----------------------------------------get cmd infos
	int argLength=args.length();
	if(argLength==2)
	{
		fileName = args.asString( 0, &status );
		CHECK_MSTATUS( status );
		nodeName = args.asString( 1, &status );
		CHECK_MSTATUS( status );
		featherTools::dagPathFromName(nodeName,growNode);
	}
	else
	{
		MGlobal::displayError("Use format: jcWriteGrowData fileName jcPreGuid.");
		return MS::kFailure;
	}
	int fileNamelen = fileName.length();
	if(fileNamelen==0) 
	{
		MGlobal::displayError("JCFeather: Need a file name.");
		return MS::kFailure;
	}


	//----------------------------------------
	MFnDependencyNode dgNodeFn;
	if(growNode.hasFn(MFn::kTransform))
		growNode.extendToShape();
	dgNodeFn.setObject( growNode.node() );
	MString name=  dgNodeFn.name();


	if( dgNodeFn.typeId()== jcPreGuid::id )
	{	
		if(fileNamelen<=3) fileName+=".vd";
		if( fileNamelen>3 && fileName.substring(fileNamelen-3,fileNamelen)!=".vd")
		fileName+=".vd";

		verDirVector vertexData;
		getVertexDirectionData(growNode,vertexData);

		if( featherTools::writePreGuideFile(fileName,vertexData))
		{
			setResult(fileName);
			MGlobal::displayInfo("Write file "+fileName+" ok.");
			return MS::kSuccess;
		}
		else
		{
			MGlobal::displayError("JCFeather: Write file "+fileName+" error.");
			return MS::kFailure;
		}
	}
	else if(dgNodeFn.typeId()== jcScatter::id)
	{
		if(fileNamelen<=3) fileName+=".st";
		if( fileNamelen>3 && fileName.substring(fileNamelen-3,fileNamelen)!=".st")
		fileName+=".st";

		jcScatterPointData sctData;
		getScatterData(growNode,sctData);

		if( jcScatterPointData::writeDataTo(fileName,sctData))
		{
			setResult(fileName);
			MGlobal::displayInfo("Write file "+fileName+" ok.");
			return MS::kSuccess;
		}
		else
		{
			MGlobal::displayError("JCFeather: Write file "+fileName+" error.");
			return MS::kFailure;
		}
	}
	else if(dgNodeFn.typeId()== jcPreFeather::id)
	{
		if(fileNamelen<=4) fileName+=".tur";
		if( fileNamelen>4 && fileName.substring(fileNamelen-4,fileNamelen)!=".tur")
		fileName+=".tur";

		turtlesArrayIO outIO;
		getComponentArrayData(growNode,jcPreFeather::outFeatherTurtleArray,outIO._turtlesD);
		if( outIO.writeInfoTo(fileName.asChar()))
		{
			setResult(fileName);
			MGlobal::displayInfo("Write file "+fileName+" ok.");
			return MS::kSuccess;
		}
		else
		{
			MGlobal::displayError("JCFeather: Write file "+fileName+" error.");
			return MS::kFailure;
		}
		return MS::kSuccess;
	}
	else if(dgNodeFn.typeId()== jcFeatherComponentArray::id)
	{
		if(fileNamelen<=4) fileName+=".tur";
		if( fileNamelen>4 && fileName.substring(fileNamelen-4,fileNamelen)!=".tur")
		fileName+=".tur";
		
		turtlesArrayIO outIO;
		getComponentArrayData(growNode,jcFeatherComponentArray::outFeatherTurtleDataArray,outIO._turtlesD);
		if( outIO.writeInfoTo(fileName.asChar()))
		{
			setResult(fileName);
			MGlobal::displayInfo("Write file "+fileName+" ok.");
			return MS::kSuccess;
		}
		else
		{
			MGlobal::displayError("JCFeather: Write file "+fileName+" error.");
			return MS::kFailure;
		}
		return MS::kSuccess;
	}
	else
	{
		MGlobal::displayError("JCFeather: Need a jcPreGuide, jcScatter,jcPreFeather or jcFeatherComponentArray node.");
		return MS::kFailure;
	}

	
}
