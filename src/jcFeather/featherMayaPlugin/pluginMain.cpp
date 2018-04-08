
#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDGMessage.h>
#include <maya/MGlobal.h>
#include <maya/MSceneMessage.h>

#if defined(_OPENMP)
	#include <omp.h>
#endif

#include "jcFeather.h"
#include "jcFeatherSystem.h"
#include "jcWriteFeather.h"
#include "jcFeatherGlobals.h"
#include "featherGen.h"
#include "jcPreFeather.h"
#include "jcPreGuid.h"
#include "jcScatter.h"
#include "jcFeatherComponent.h"
#include "jcFeatherComponentArray.h"
#include "featherTurtleData.h"
#include "vertexDirectionData.h"
#include "scatterPointData.h"
#include "featherTurtleArrayData.h"
#include "jcWriteGrowData.h"
#include "jcPreController.h"
#include "jcFeatherAbout.h"
#include "jcFeatherToCurves.h"
#include "jcInstancerToMeshCmd.h"
#include "jcQueryData.h"

HashTable*  feaInfoDataBase = NULL;
HashTable*  turtlesDataBase = NULL;
featherGen* featherG = NULL;

bool jcFea_UseMultiThread = true ; //use multi thread or not
int jcFea_ProcessorNum = omp_get_num_procs() ;
int jcFea_MaxThreads = jcFea_ProcessorNum;//the max thread num,if 0, number of processes will be used.
int jcFea_FreeVersionMaxNodesNum = 3;

int jcFea_FreeVersionJCFeatherNodeIndex=0;
int jcFea_FreeVersionJCPreFeatherNodeIndex=0;

MCallbackIdArray jcFeaCallbackids;
const char* jcFeatherVersion="Free Version 2.8.6";

//------------------------------------------clear hashtable
inline void clearHashTable()
{
	if( feaInfoDataBase != NULL )
	{
		feaInfoDataBase->ClearFreePopID();
		feaInfoDataBase->maxFreeListCount=0;
		feaInfoDataBase->ClearFreeList();
		delete feaInfoDataBase;
		feaInfoDataBase= NULL;
	}
	if( turtlesDataBase != NULL )
	{
		turtlesDataBase->ClearFreePopID();
		turtlesDataBase->maxFreeListCount=0;
		turtlesDataBase->ClearFreeList();
		delete turtlesDataBase;
		turtlesDataBase= NULL;
	}
}
//------------------------------------------clear all jcFeather data library
inline void clearJCFeatherGlobalData()
{
	clearHashTable();

	if( featherG != NULL )
	{	
		if( featherG->_featherAttrs != NULL )
			featherG->_featherAttrs=NULL;
		delete featherG;
		featherG = NULL;
	}
}
//------------------------------------------reset for new scene
inline void resetJCFeatherGlobalData()
{
	clearHashTable();

	feaInfoDataBase = new HashTable(5);
	turtlesDataBase = new HashTable(10);

	feaInfoDataBase->maxFreeListCount = 20;
	turtlesDataBase->maxFreeListCount = 1024;

	if(!featherG)
		featherG = new featherGen;
	featherG->init();
	featherG->_featherAttrs = NULL;
}

//------------------------------------------init multithread for maya
void setJCFeatherOpenMP()
{
	jcFea_UseMultiThread = true ;
	jcFea_MaxThreads = omp_get_num_procs();

	MSelectionList sellist;
	MFnDependencyNode dgNodeFn;
	MObject globalObj;

	sellist.clear();
	if( sellist.add("jcFeatherGlobals")==MS::kSuccess )
	{
		CHECK_MSTATUS( sellist.getDependNode(0,globalObj) );
		CHECK_MSTATUS( dgNodeFn.setObject(globalObj) );
		if(dgNodeFn.typeId() == jcFeatherGlobals::id)
		{
			jcFea_UseMultiThread = dgNodeFn.findPlug(jcFeatherGlobals::useMultiThread).asBool();
			jcFea_MaxThreads = dgNodeFn.findPlug(jcFeatherGlobals::maxThreads).asInt();
			if(jcFea_MaxThreads==0)jcFea_MaxThreads= jcFea_ProcessorNum; 
		}
	}
	
}

//------------------------------------------maya callbacks
static void jcFeatherSceneUpdate(void * data)
{
	resetJCFeatherGlobalData();
	setJCFeatherOpenMP();
}
static void jcFeatherAfterOpen(void *data)
{
	feaInfoDataBase->maxFreeListCount=0;
	feaInfoDataBase->ClearFreeList();
	turtlesDataBase->maxFreeListCount=0;
	turtlesDataBase->ClearFreeList();

	feaInfoDataBase->maxFreeListCount = 20;
	turtlesDataBase->maxFreeListCount = 1024;

	setJCFeatherOpenMP();
	MGlobal::executeCommand("updateJCFeatherStartNurbSurface");
}
static void jcFeatherAfterInData(void *data)
{
	MGlobal::executeCommand("checkJCFeatherGlobals");
	MGlobal::executeCommand("updateJCFeatherStartNurbSurface");
	setJCFeatherOpenMP();
}
void registerjcFeaMessage()
{
	MStatus   status;

	jcFeaCallbackids.clear();

	//-------------scene update call back
	jcFeaCallbackids.append( MSceneMessage::addCallback( MSceneMessage::kAfterNew,jcFeatherSceneUpdate,NULL,&status ));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append( MSceneMessage::addCallback( MSceneMessage::kAfterOpen,jcFeatherAfterOpen,NULL,&status ));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append( MSceneMessage::addCallback( MSceneMessage::kAfterRemoveReference,jcFeatherAfterOpen,NULL,&status ));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append( MSceneMessage::addCallback( MSceneMessage::kAfterCreateReference,jcFeatherAfterInData,NULL,&status ));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append( MSceneMessage::addCallback( MSceneMessage::kAfterImport,jcFeatherAfterInData,NULL,&status ));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append(  MSceneMessage::addCallback( MSceneMessage::kAfterLoadReference,jcFeatherAfterInData,NULL,&status ));
	CHECK_MSTATUS(status);

	//--------node removed call back
	jcFeaCallbackids.append(  MDGMessage::addNodeRemovedCallback(jcFeather::nodeRemoved,jcFeather::typeName,&status));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append(  MDGMessage::addNodeRemovedCallback(jcFeatherGlobals::nodeRemoved,jcFeatherGlobals::typeName,&status));
	CHECK_MSTATUS(status);

	jcFeaCallbackids.append(  MDGMessage::addNodeRemovedCallback(jcPreFeather::nodeRemoved,jcPreFeather::typeName,&status));
	CHECK_MSTATUS(status);
}
void deRegisterjcFeaMessage()
{
	for(int ii=0;ii<jcFeaCallbackids.length();ii++)
			CHECK_MSTATUS( MMessage::removeCallback( jcFeaCallbackids[ii] ) );
	jcFeaCallbackids.clear();
}
//------------------------------------------maya callbacks

MStatus initializePlugin( MObject obj )
{
	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MStatus   status;
	MFnPlugin plugin( obj, "Jerry Kon", jcFeatherVersion, "Any",&status);
	CHECK_MSTATUS( status );
	
	CHECK_MSTATUS( plugin.registerData( featherTurtleData::typeName,
                                         featherTurtleData::id,//36
                                         featherTurtleData::creator ) );

	CHECK_MSTATUS( plugin.registerData( featherTurtleArrayData::typeName,
                                         featherTurtleArrayData::id,//38
                                         featherTurtleArrayData::creator ) );

	CHECK_MSTATUS( plugin.registerData( vertexDirectionData::typeName,
                                         vertexDirectionData::id,//37
                                         vertexDirectionData::creator ) );
	CHECK_MSTATUS( plugin.registerData( scatterPointData::typeName,
                                         scatterPointData::id,//42
                                         scatterPointData::creator ) );
	CHECK_MSTATUS( plugin.registerNode(jcFeather::typeName, jcFeather::id,//30
						jcFeather::creator, jcFeather::initialize,
						MPxNode::kLocatorNode) );

	CHECK_MSTATUS( plugin.registerNode( jcFeatherGlobals::typeName, jcFeatherGlobals::id, //31
										jcFeatherGlobals::creator,jcFeatherGlobals::initialize ) );

	CHECK_MSTATUS( plugin.registerNode(jcFeatherSystem::typeName, jcFeatherSystem::id,//32
						jcFeatherSystem::creator, jcFeatherSystem::initialize,
						MPxNode::kLocatorNode) );



	CHECK_MSTATUS( plugin.registerNode( jcFeatherComponent::typeName, jcFeatherComponent::id, //34
										jcFeatherComponent::creator, jcFeatherComponent::initialize,
										MPxNode::kLocatorNode ) );

	CHECK_MSTATUS( plugin.registerNode( jcFeatherComponentArray::typeName, jcFeatherComponentArray::id, //39
										jcFeatherComponentArray::creator, jcFeatherComponentArray::initialize,
										MPxNode::kLocatorNode ) );

	CHECK_MSTATUS( plugin.registerNode(jcScatter::typeName, jcScatter::id,//41
						jcScatter::creator, jcScatter::initialize,
						MPxNode::kLocatorNode) );

	CHECK_MSTATUS( plugin.registerNode(jcPreGuid::typeName, jcPreGuid::id,//35
						jcPreGuid::creator, jcPreGuid::initialize,
						MPxNode::kLocatorNode) );

	CHECK_MSTATUS( plugin.registerNode(jcPreFeather::typeName, jcPreFeather::id,//33
						jcPreFeather::creator, jcPreFeather::initialize,
						MPxNode::kLocatorNode) );
	CHECK_MSTATUS( plugin.registerNode( jcPreController::typeName, jcPreController::id, //40
										jcPreController::creator, jcPreController::initialize,
										MPxNode::kLocatorNode ) );

	CHECK_MSTATUS( plugin.registerCommand("jcWriteFeather", jcWriteFeather::creator,jcWriteFeather::newSyntax) );
	CHECK_MSTATUS( plugin.registerCommand("jcFeatherToCurves", jcFeatherToCurves::creator,jcFeatherToCurves::newSyntax) );
	CHECK_MSTATUS( plugin.registerCommand("jcInstancerToMesh", jcInstancerToMeshCmd::creator,jcInstancerToMeshCmd::newSyntax) );	
	CHECK_MSTATUS( plugin.registerCommand("jcWriteNodeData", jcWriteGrowData::creator) );
	CHECK_MSTATUS( plugin.registerCommand("jcQueryData", jcQueryData::creator,jcQueryData::newSyntax) );
	CHECK_MSTATUS( plugin.registerCommand("jcFeatherAbout", jcFeatherAbout::creator) );

	CHECK_MSTATUS( MGlobal::sourceFile("jcFeatherScripts.mel") );
	CHECK_MSTATUS( MGlobal::sourceFile("jcFeatherDelight.mel") );
	CHECK_MSTATUS( MGlobal::executeCommand("jcFeatherMenu") );

	registerjcFeaMessage();
	resetJCFeatherGlobalData();

	MGlobal::displayInfo("jcFeather "+MString(jcFeatherVersion)+" loaded");
	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	CHECK_MSTATUS( plugin.deregisterNode( jcFeather::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcFeatherSystem::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcFeatherGlobals::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcFeatherComponent::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcFeatherComponentArray::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcPreFeather::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcPreGuid::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcScatter::id ) );
	CHECK_MSTATUS( plugin.deregisterNode( jcPreController::id ) );
	CHECK_MSTATUS( plugin.deregisterData( featherTurtleData::id ) );
	CHECK_MSTATUS( plugin.deregisterData( featherTurtleArrayData::id ) );
	CHECK_MSTATUS( plugin.deregisterData( vertexDirectionData::id ) );
	CHECK_MSTATUS( plugin.deregisterData( scatterPointData::id ) );

	CHECK_MSTATUS( plugin.deregisterCommand("jcWriteFeather") );
	CHECK_MSTATUS( plugin.deregisterCommand("jcFeatherToCurves") );
	CHECK_MSTATUS( plugin.deregisterCommand("jcInstancerToMesh") );
	CHECK_MSTATUS( plugin.deregisterCommand("jcWriteNodeData") );
	CHECK_MSTATUS( plugin.deregisterCommand("jcQueryData") );
	CHECK_MSTATUS( plugin.deregisterCommand("jcFeatherAbout") );
	CHECK_MSTATUS( MGlobal::sourceFile("jcFeatherScripts.mel") );
	MGlobal::executeCommand("jcUninstalMenu");

	deRegisterjcFeaMessage();
	clearJCFeatherGlobalData();

	MGlobal::displayInfo("jcFeather "+MString(jcFeatherVersion)+" unloaded!");
	return status;
}
