#include "jcFeatherSystem.h"
#include "featherGen.h"
#include "HashTable.h"
#include "cell.h"
#include "glDrawFeather.h"

#include <iostream>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
MTypeId  jcFeatherSystem::id(0x81032);
MString  jcFeatherSystem::typeName("jcFeatherSystem");

MObject  jcFeatherSystem::displayPercent; 
MObject  jcFeatherSystem::displayBarbulePercent; 
MObject  jcFeatherSystem::renderQuality; 
MObject  jcFeatherSystem::active; 
MObject  jcFeatherSystem::outputFeatherMesh; 

MObject  jcFeatherSystem::useUVIFile;
MObject  jcFeatherSystem::uviFile;

MObject  jcFeatherSystem::uSegment;
MObject  jcFeatherSystem::vSegment;

MObject  jcFeatherSystem::uvOffset;
MObject  jcFeatherSystem::uvTranslate;

MObject  jcFeatherSystem::jitterDistance;
MObject  jcFeatherSystem::jitterFrequency;

MObject  jcFeatherSystem::guidsPerFeather;
MObject  jcFeatherSystem::gloalScale;
MObject  jcFeatherSystem::randScale;
MObject  jcFeatherSystem::radius;
MObject  jcFeatherSystem::power;

MObject  jcFeatherSystem::colorTex;
MObject  jcFeatherSystem::scaleTex;
MObject  jcFeatherSystem::baldnessTex;
MObject  jcFeatherSystem::radiusTex;

MObject  jcFeatherSystem::randomSeed;
MObject  jcFeatherSystem::uvSetName;

MObject  jcFeatherSystem::inRenderFeather;
MObject  jcFeatherSystem::growthSet;
MObject  jcFeatherSystem::inFeatherMesh;
MObject  jcFeatherSystem::outFeatherMesh;

MObject  jcFeatherSystem::inputGrowSurface;
MObject  jcFeatherSystem::inputGrowMesh;
MObject  jcFeatherSystem::inputGrowMeshID;
MObject  jcFeatherSystem::outRachisMesh;
MObject  jcFeatherSystem::outBarbuleMesh;
MObject  jcFeatherSystem::displayProxyBoundingBox;

#define MAKE_INPUT(attr,keyable)	\
    CHECK_MSTATUS( attr.setKeyable(keyable) );		\
    CHECK_MSTATUS( attr.setStorable(true) );	\
    CHECK_MSTATUS( attr.setReadable(true) );	\
    CHECK_MSTATUS( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)	\
    CHECK_MSTATUS( attr.setKeyable(false) ); 	\
    CHECK_MSTATUS( attr.setStorable(false) );	\
    CHECK_MSTATUS( attr.setReadable(true) );	\
    CHECK_MSTATUS( attr.setWritable(false) );

extern HashTable*    feaInfoDataBase;
extern HashTable*    turtlesDataBase;
extern featherGen*   featherG;

jcFeatherSystem::jcFeatherSystem()
{
}

jcFeatherSystem::~jcFeatherSystem()
{
}
void jcFeatherSystem::postConstructor()
{
	featherDraw.clear();
	store4Color.setLength(4);
}
void * jcFeatherSystem::creator()
{
    return new jcFeatherSystem();
}
void   jcFeatherSystem::nodeRemoved(MObject& node, void *clientData)
{
	MFnDependencyNode fnNode(node);

	static_cast<jcFeatherSystem*>(fnNode.userNode())->featherDraw.clear();
	static_cast<jcFeatherSystem*>(fnNode.userNode())->store4Color.clear();
}
bool jcFeatherSystem::excludeAsLocator() const
{
	return false;
}
MStatus jcFeatherSystem::initialize()
{
	MStatus status;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

    MFnNumericAttribute nAttr;

	active = nAttr.create( "active", "active", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	outputFeatherMesh = nAttr.create( "outputFeatherMesh", "outfeamesh", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);

	useUVIFile = nAttr.create( "useUVIFile", "uuvi", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,true);

	displayPercent = nAttr.create( "displayQuality", "dispqua", MFnNumericData::kInt,20);
	MAKE_INPUT(nAttr,true);nAttr.setMax(100);nAttr.setMin(0);
	displayBarbulePercent = nAttr.create( "displayBarbuleQuality", "dispbarqua", MFnNumericData::kInt,10);
	MAKE_INPUT(nAttr,true);nAttr.setMax(100);nAttr.setMin(0);
	renderQuality=nAttr.create( "renderQuality", "renqua", MFnNumericData::kInt,100);
	MAKE_INPUT(nAttr,true);nAttr.setMax(100);nAttr.setMin(0);
	inRenderFeather =nAttr.create( "inRenderFeather", "inrf", MFnNumericData::kInt,-1);
	MAKE_INPUT(nAttr,false);
	nAttr.setHidden(true);

	uSegment =nAttr.create( "uSegment", "useg", MFnNumericData::kInt,10);
	MAKE_INPUT(nAttr,true);

	vSegment =nAttr.create( "vSegment", "vseg", MFnNumericData::kInt,10);
	MAKE_INPUT(nAttr,true);

	uvOffset =nAttr.create( "uvOffset", "uvOffset", MFnNumericData::k2Double,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-1);
	nAttr.setMax(1);

	uvTranslate =nAttr.create( "uvTranslate", "uvTranslate", MFnNumericData::k2Float,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-1);
	nAttr.setMax(1);

	jitterDistance =nAttr.create( "jitterDistance", "jitterdis", MFnNumericData::kFloat,0.1);
	MAKE_INPUT(nAttr,true);

	jitterFrequency =nAttr.create( "jitterFrequency", "jitterfre", MFnNumericData::kFloat,15);
	MAKE_INPUT(nAttr,true);

	guidsPerFeather =nAttr.create( "guidsPerFeather", "gpf", MFnNumericData::kInt,2);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(1);

	gloalScale =nAttr.create( "gloalScale", "gloalScale", MFnNumericData::kFloat,1.0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);

	randScale =nAttr.create( "randScale", "randScale", MFnNumericData::kFloat,0.0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(1);

	radius =nAttr.create( "radius", "radius", MFnNumericData::kFloat,10.0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);

	power =nAttr.create( "power", "power", MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);

	colorTex = nAttr.createColor("colorTex", "colorTex");
	MAKE_INPUT(nAttr,false);

	scaleTex =nAttr.create( "scaleTex", "scaleTex", MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,false);

	baldnessTex =nAttr.create( "baldnessTex", "baldnessTex", MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,false);

	radiusTex =nAttr.create( "radiusTex", "radiusTex", MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,false);

	randomSeed =nAttr.create( "randomSeed", "rands", MFnNumericData::kInt,5);
	MAKE_INPUT(nAttr,true);

	displayProxyBoundingBox =nAttr.create( "displayProxyBoundingBox", "dispPBB", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,true);

	inputGrowMeshID = nAttr.create( "inputGrowMeshID", "inmeshid", MFnNumericData::kInt,-1);
	MAKE_INPUT(nAttr,false);
	nAttr.setHidden(true);
	
	MFnTypedAttribute typAttr;
	inputGrowSurface = typAttr.create("inputGrowSurface","ingrs",MFnMeshData::kNurbsSurface);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	inputGrowMesh = typAttr.create("inputGrowMesh","ingrm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);

	inFeatherMesh = typAttr.create("inFeatherMesh","infm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	outFeatherMesh = typAttr.create("outFeatherMesh","outfm",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	outRachisMesh = typAttr.create("outRachisMesh","outrame",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	outBarbuleMesh = typAttr.create("outBarbuleMesh","outbame",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	
	MFnStringData strData;
	MObject tempObj = strData.create("map1");
	uvSetName = typAttr.create( "uvSetName", "uvsn", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	tempObj = strData.create("");
	uviFile = typAttr.create( "uviFile", "uvif", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);


	MFnMessageAttribute messageAttr;
	growthSet=messageAttr.create("growthSet","growthSet");
	MAKE_INPUT(messageAttr,false);
	messageAttr.setHidden(true);

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	CHECK_MSTATUS( addAttribute(active) );
	CHECK_MSTATUS( addAttribute(displayPercent) );
	CHECK_MSTATUS( addAttribute(displayBarbulePercent) );
	CHECK_MSTATUS( addAttribute(renderQuality) );
	CHECK_MSTATUS( addAttribute(useUVIFile) );
	CHECK_MSTATUS( addAttribute(uviFile) );

	CHECK_MSTATUS( addAttribute(uSegment) );
	CHECK_MSTATUS( addAttribute(vSegment) );
	CHECK_MSTATUS( addAttribute(jitterDistance) );
	CHECK_MSTATUS( addAttribute(jitterFrequency) );

	CHECK_MSTATUS( addAttribute(guidsPerFeather) );
	CHECK_MSTATUS( addAttribute(gloalScale) );
	CHECK_MSTATUS( addAttribute(randScale) );
	CHECK_MSTATUS( addAttribute(radius) );
	CHECK_MSTATUS( addAttribute(power) );

	CHECK_MSTATUS( addAttribute(colorTex) );
	CHECK_MSTATUS( addAttribute(scaleTex) );
	CHECK_MSTATUS( addAttribute(baldnessTex) );
	CHECK_MSTATUS( addAttribute(radiusTex) );

	CHECK_MSTATUS( addAttribute(uvOffset) );
	CHECK_MSTATUS( addAttribute(uvTranslate) );
	CHECK_MSTATUS( addAttribute(randomSeed) );
	CHECK_MSTATUS( addAttribute(displayProxyBoundingBox) );
	CHECK_MSTATUS( addAttribute(uvSetName) );

	CHECK_MSTATUS( addAttribute(inRenderFeather) );
	CHECK_MSTATUS( addAttribute(growthSet) );
	CHECK_MSTATUS( addAttribute(inputGrowSurface) );
	CHECK_MSTATUS( addAttribute(inputGrowMesh) );
	CHECK_MSTATUS( addAttribute(inputGrowMeshID) );
	CHECK_MSTATUS( addAttribute(inFeatherMesh) );
	CHECK_MSTATUS( addAttribute(outFeatherMesh) );
	CHECK_MSTATUS( addAttribute(outRachisMesh) );
	CHECK_MSTATUS( addAttribute(outBarbuleMesh) );
	CHECK_MSTATUS( addAttribute(outputFeatherMesh) );
	//--------------------------------------
	CHECK_MSTATUS( attributeAffects( active, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( displayPercent, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( displayBarbulePercent, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inRenderFeather, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( growthSet, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowSurface, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMesh, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMeshID, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( useUVIFile, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( uviFile, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( uSegment, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( vSegment, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterDistance, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterFrequency, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( guidsPerFeather, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( gloalScale, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( randScale, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( radius, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( power, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( colorTex, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( scaleTex, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( baldnessTex, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( radiusTex, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( uvOffset, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( uvTranslate, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( randomSeed, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( displayProxyBoundingBox, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherMesh, outFeatherMesh ) );

	//-----------------------------------
	CHECK_MSTATUS( attributeAffects( active, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( outputFeatherMesh, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( displayPercent, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( inRenderFeather, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( growthSet, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowSurface, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMesh, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMeshID, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( useUVIFile, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( uviFile, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( uSegment, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( vSegment, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterDistance, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterFrequency, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( guidsPerFeather, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( gloalScale, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( randScale, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( radius, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( power, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( colorTex, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( scaleTex, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( baldnessTex, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( radiusTex, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( uvOffset, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( uvTranslate, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( randomSeed, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherMesh, outRachisMesh ) );

	//----------------------------------------

	CHECK_MSTATUS( attributeAffects( active, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( outputFeatherMesh, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( displayPercent, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( displayBarbulePercent, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( inRenderFeather, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( growthSet, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowSurface, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMesh, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMeshID, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outBarbuleMesh ) );


	CHECK_MSTATUS( attributeAffects( useUVIFile, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( uviFile, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( uSegment, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( vSegment, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterDistance, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( jitterFrequency, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( guidsPerFeather, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( gloalScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( randScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( radius, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( power, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( colorTex, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( scaleTex, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( baldnessTex, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( radiusTex, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( uvOffset, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( uvTranslate, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( randomSeed, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherMesh, outBarbuleMesh ) );

   return MS::kSuccess;
}
void jcFeatherSystem::updateJCFeather()
{
	MObject me = thisMObject();
	MPlug plugInFeather(me,inRenderFeather);

	if( !plugInFeather.isConnected() ) return ;

	//force the jcfeather recompute 
	MPlugArray plgAry;
	if( plugInFeather.connectedTo(plgAry,true,false) )
	{
		MObject update;
		if( plgAry[0].node().hasFn(MFn::kPluginDependNode) )
			MPlug(plgAry[0].node(),jcFeather::outputMesh).getValue(update);
	}
}

MStatus jcFeatherSystem::compute(const MPlug& plug, MDataBlock& data) 
{
	MStatus status = MS::kSuccess;
	
	MDataHandle active_Hnd          = data.inputValue(active);
	bool activeornot = active_Hnd.asBool();
	MDataHandle displayP_Hnd          = data.inputValue(displayPercent,&status);

	if( !activeornot ||displayP_Hnd.asInt()==0) 
	{
			featherDraw.clear();
			return MS::kSuccess;
	}	

	//updateJCFeather();
	MFnMeshData meshDataFn;
	MFnMesh meshFn;	
	if(plug == outFeatherMesh )
	{
		makeFeathers( data );

		MDataHandle    inFeaHandle = data.inputValue( inFeatherMesh );
		MObject        inputFeaObj = inFeaHandle.asMesh();
		MDataHandle    outputHnd  = data.outputValue( outFeatherMesh );

		//��������ʾmesh�����
		outputHnd.set(inputFeaObj);
		data.setClean( plug );
		//�Ż�feather������Ҫ�����ȥ�����ͷ��ڴ�
		for(unsigned int ii=0;ii<featherDraw.size();++ii)
		{
			featherDraw[ii].leftBarbuleLenPer.clear();
			featherDraw[ii].leftBarbuleRachisPos.clear();
			featherDraw[ii].rightBarbuleLenPer.clear();
			featherDraw[ii].rightBarbuleRachisPos.clear();
		}
		return MS::kSuccess;
	}
	else if(plug == outRachisMesh || plug == outBarbuleMesh)
	{
		MDataHandle    outputRachisMeshHnd    = data.outputValue( outRachisMesh );
		MDataHandle    outputBarbuleMeshHnd   = data.outputValue( outBarbuleMesh );

		MDataHandle outputFeatherMesh_Hnd    = data.inputValue(outputFeatherMesh,&status);
		polyObj jcOutRachis,jcOutBarbule;		
		if( outputFeatherMesh_Hnd.asBool() !=0 )
		{
			MObject rachisData=meshDataFn.create(),barbuleData=meshDataFn.create();			

			if(plug == outRachisMesh)
			{
				for(unsigned int ii=0;ii<featherDraw.size();++ii)
				{
					jcOutRachis.appendMesh(featherDraw[ii].rachisCylinder);
					featherDraw[ii].rachisCylinder.init();
				}
				meshFn.create(  jcOutRachis.pa.length(),
								jcOutRachis.faceCounts.length(),
								jcOutRachis.pa,
								jcOutRachis.faceCounts,
								jcOutRachis.faceConnects,
								rachisData,&status);
				CHECK_MSTATUS( status );
				meshFn.clearUVs();
				meshFn.setUVs( jcOutRachis.ua,jcOutRachis.va );
				meshFn.assignUVs( jcOutRachis.faceCounts,jcOutRachis.uvids);
				meshFn.updateSurface() ;
				outputRachisMeshHnd.set(rachisData);
		    }
		    else if(plug == outBarbuleMesh)
		    {
				for(unsigned int ii=0;ii<featherDraw.size();++ii)
				{
					jcOutBarbule.appendMesh(featherDraw[ii].barbuleFace);
					featherDraw[ii].barbuleFace.init();
				}
				meshFn.create(  jcOutBarbule.pa.length(),
								jcOutBarbule.faceCounts.length(),
								jcOutBarbule.pa,
								jcOutBarbule.faceCounts,
								jcOutBarbule.faceConnects,
								barbuleData,&status);
				CHECK_MSTATUS( status );
				meshFn.clearUVs();
				meshFn.setUVs( jcOutBarbule.ua,jcOutBarbule.va );
				meshFn.assignUVs( jcOutBarbule.faceCounts,jcOutBarbule.uvids);
				meshFn.updateSurface();
				outputBarbuleMeshHnd.set(barbuleData);				
		    }
	    }//end----if( outputFeatherMesh_Hnd.asBool() !=0 )
		else
		{
			MObject newMeshData = meshDataFn.create();
			meshFn.create(0,0,jcOutRachis.pa,jcOutRachis.faceCounts,jcOutRachis.faceConnects,newMeshData);
			meshFn.updateSurface();

			outputRachisMeshHnd.set(newMeshData);
			outputBarbuleMeshHnd.set(newMeshData);	
		}

	    data.setClean( plug );
		return MS::kSuccess;
	}
	else
		return MS::kUnknownParameter;
}

void    jcFeatherSystem::draw(M3dView &view, 
						 const MDagPath &path, 
						 M3dView::DisplayStyle style, 
						 M3dView::DisplayStatus dispStatus)
{
	if( featherDraw.size()==0 )
		return;
	
	MObject thisNode = thisMObject();
	MPlug dpbbPlug(thisNode,displayProxyBoundingBox);

    MFnDagNode fnDagNode(thisNode);
	MFnTransform fnParentTransform(fnDagNode.parent(0));
	m_worldMatrix = fnParentTransform.transformation().asMatrix();
	glLineWidth(1.0f);
	view.beginGL(); 
		glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
				double m[4][4];
				m_worldMatrix.inverse().get(m);
				glMultMatrixd(&(m[0][0]));
				if( dispStatus == M3dView::kLead )
					jerryC::drawFeather(featherDraw,1,store4Color);
				else if(dispStatus == M3dView::kDormant)
					jerryC::drawFeather(featherDraw,0.6f,store4Color);
				else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
					jerryC::drawFeatherWithColor(featherDraw,colorRGB(dispStatus));
				if(dpbbPlug.asBool())
					jerryC::drawFeatherProxyBB(featherDraw,MColor(0,1,0));
			glPopMatrix();
		glPopAttrib();
	view.endGL();
}
bool  jcFeatherSystem::isBounded() const
{
	return true;
}
MBoundingBox   jcFeatherSystem::boundingBox() const
{
	MBoundingBox result;
	size_t fs=featherDraw.size();
	for(unsigned int ii=0;ii<fs;++ii)
		result.expand(featherDraw[ii].proxyBBox);

	return result;
}
void jcFeatherSystem::makeFeathers( MDataBlock& block )
{
	MStatus status = MS::kSuccess;

	MObject me = thisMObject();
	featherDraw.clear();

	MDataHandle displayP_Hnd          = block.inputValue(displayPercent,&status);
	float _displayPer = (float)displayP_Hnd.asInt()/100.f;

	MDataHandle displayBarbuleP_Hnd          = block.inputValue(displayBarbulePercent,&status);
	float _displayBarPer = (float)displayBarbuleP_Hnd.asInt()/100.f;

	//-----------------��jerryC::jCell�������㣬���������ý�ȥ
	MDataHandle uSegment_Hnd          = block.inputValue(uSegment,&status);
	jerryC::jCell::m_xSegment = uSegment_Hnd.asInt();

	MDataHandle vSegment_Hnd          = block.inputValue(vSegment,&status);
	jerryC::jCell::m_zSegment = vSegment_Hnd.asInt();

	MDataHandle randomSeed_Hnd          = block.inputValue(randomSeed,&status);
	jerryC::jCell::m_seed = randomSeed_Hnd.asInt();

	MDataHandle jitterDistance_Hnd    = block.inputValue(jitterDistance,&status);
	jerryC::jCell::m_jitter = (double)(jitterDistance_Hnd.asFloat());

	MDataHandle jitterFrequency_Hnd    = block.inputValue(jitterFrequency,&status);
	jerryC::jCell::m_frequency= (double)(jitterFrequency_Hnd.asFloat());

	MDataHandle uvOffset_Hnd    = block.inputValue(uvOffset,&status);
	double2 &uvo = uvOffset_Hnd.asDouble2();
	jerryC::jCell::m_xOffset = (uvo[0]);
	jerryC::jCell::m_zOffset = (uvo[1]);

	MDataHandle uvTranslate_Hnd    = block.inputValue(uvTranslate,&status);
	float2 &_uvTranslate=uvTranslate_Hnd.asFloat2();

	//-----------------


	MDataHandle inRenderFeather_Hnd    = block.inputValue(inRenderFeather,&status);
	MDataHandle guidsPerFeather_Hnd    = block.inputValue(guidsPerFeather,&status);
	MDataHandle gloalScale_Hnd    = block.inputValue(gloalScale,&status);
	MDataHandle randScale_Hnd    = block.inputValue(randScale,&status);
	MDataHandle radius_Hnd    = block.inputValue(radius,&status);
	MDataHandle power_Hnd    = block.inputValue(power,&status);
	MDataHandle outputFeatherMesh_Hnd    = block.inputValue(outputFeatherMesh,&status);
	MDataHandle useUVIFile_Hnd          = block.inputValue(useUVIFile,&status);
	MDataHandle uviFile_Hnd          = block.inputValue(uviFile,&status);
	
	MDataHandle colorTex_Hnd          = block.inputValue(colorTex,&status);colorTex_Hnd.asFloatVector();
	MDataHandle scaleTex_Hnd          = block.inputValue(scaleTex,&status);scaleTex_Hnd.asFloat();
	MDataHandle baldnessTex_Hnd       = block.inputValue(baldnessTex,&status);baldnessTex_Hnd.asFloat();
	MDataHandle radiusTex_Hnd         = block.inputValue(radiusTex,&status);radiusTex_Hnd.asFloat();	
	
	MDataHandle inputGrowSurface_Hnd          = block.inputValue(inputGrowSurface,&status);
	MObject inGrowSurface = inputGrowSurface_Hnd.asNurbsSurface();

	MDataHandle inputGrowMesh_Hnd          = block.inputValue(inputGrowMesh,&status);
	MObject inGrowMesh = inputGrowMesh_Hnd.asMesh();

	MDataHandle inputGrowMeshID_Hnd          = block.inputValue(inputGrowMeshID,&status);
	int inGrowMeshID = inputGrowMeshID_Hnd.asInt();
	
	MDataHandle uvSetName_Hnd          = block.inputValue(uvSetName,&status);

	//-------------------------�õ�texture�����
	MPlug texPlug(me,scaleTex);
	MPlugArray texConnect;
	
	MStringArray texs(4,MString(""));
	texPlug.setAttribute(scaleTex);
	if(texPlug.connectedTo(texConnect,true,false,&status))
		texs[0]=texConnect[0].name();
	texPlug.setAttribute(baldnessTex);
	if(texPlug.connectedTo(texConnect,true,false,&status))
		texs[1]=texConnect[0].name();
	texPlug.setAttribute(colorTex);
	if(texPlug.connectedTo(texConnect,true,false,&status))
		texs[2]=texConnect[0].name();
	texPlug.setAttribute(radiusTex);
	if(texPlug.connectedTo(texConnect,true,false,&status))
		texs[3]=texConnect[0].name();
	
	Node *tempNodePt=NULL;
	long fid= (long)(inRenderFeather_Hnd.asInt());
	if(fid<0) return;
	tempNodePt = feaInfoDataBase->Find( &fid );
	if(!tempNodePt) return;
	featherG->_featherAttrs = reinterpret_cast<featherInfo*>(tempNodePt->Value);
	featherG->_feaExpInf._onlyGetBBox=false;

	//------��������ӽ�featherGen��
	featherG->_jcSysD._featherID = fid;
	featherG->_jcSysD._outputFeatherMesh= outputFeatherMesh_Hnd.asBool();
	featherG->_jcSysD._guidPF= guidsPerFeather_Hnd.asInt();
	featherG->_jcSysD._globalScale=gloalScale_Hnd.asFloat();
	featherG->_jcSysD._randScale= randScale_Hnd.asFloat();
	featherG->_jcSysD._radius=  radius_Hnd.asFloat() + 0.00000001f;
	featherG->_jcSysD._power= power_Hnd.asFloat();
	featherG->_jcSysD._translate[0]=  _uvTranslate[0];
	featherG->_jcSysD._translate[1]=  _uvTranslate[1];
	featherG->_jcSysD._seed=  randomSeed_Hnd.asInt();
	featherG->_jcSysD._textures= texs;
	featherG->_jcSysD._useUVI = useUVIFile_Hnd.asBool();
	featherG->_jcSysD._uviFile = uviFile_Hnd.asString();
	featherG->_jcSysD._uvSet = uvSetName_Hnd.asString();
	if(featherG->_jcSysD._uvSet.length()==0) featherG->_jcSysD._uvSet = "map1";

	store4Color[0] = featherG->_featherAttrs->_shader._rachisRootColor;
	store4Color[1] = featherG->_featherAttrs->_shader._rachisTipColor;
	store4Color[2] = featherG->_featherAttrs->_shader._rootColor;
	store4Color[3] = featherG->_featherAttrs->_shader._tipColor;

	//-------------------------�õ����λ�ã��Լ���
	MFloatPointArray positions;
	MFloatArray u,v;
	MDagPath growObjPath;
	MObject  growObjComp;
	texPlug.setAttribute(growthSet);
	if(texPlug.connectedTo(texConnect,true,false,&status))
	{
		MFnSet set(texConnect[0].node());
		MSelectionList selList;
		if(set.getMembers(selList,false) == MS::kSuccess)
			selList.getDagPath(0,growObjPath,growObjComp);
		else
			return;
	}
	else
		return;

	if(growObjPath.extendToShape()!=MS::kSuccess) return;

	int orignalNum = featherG->_featherAttrs->_barbuleNum;
	
	MPlug plug( me,inputGrowSurface );
	if( plug.isConnected() && growObjPath.apiType() == MFn::kNurbsSurface)
	{
		if( featherG->getPositions( growObjPath,positions,u,v)!=MS::kSuccess )
			return;
		featherG->getDisplayPositions(positions,u,v,_displayPer);
		featherG->_featherAttrs->_barbuleNum = (int)(featherG->_featherAttrs->_renderBarbuleNum*_displayBarPer);
		featherG->getFeatherFromSys( positions,u,v,featherDraw);
	}

	plug.setAttribute( inputGrowMesh );
	if( plug.isConnected() && growObjPath.apiType()==MFn::kMesh)
	{
		MItMeshPolygon itMeshPoly(growObjPath,growObjComp);
		if( featherG->getPositions(&itMeshPoly,positions,u,v)!=MS::kSuccess)
			return;
		featherG->getDisplayPositions(positions,u,v,_displayPer);
		featherG->_featherAttrs->_barbuleNum = (int)(featherG->_featherAttrs->_renderBarbuleNum*_displayBarPer);
		featherG->getFeatherFromSys(positions,u,v,featherDraw);			
	}
	featherG->_featherAttrs->_barbuleNum = orignalNum;

	positions.clear();
	u.clear();
	v.clear();
}
