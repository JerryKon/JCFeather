#include "jcFeather.h"
#include "featherGen.h"
#include "glDrawFeather.h"
#include "jcFeatherComponent.h"
#include "featherTurtleData.h"
#include "featherTurtleArrayData.h"

#include <iostream>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MRampAttribute.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MDagPath.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnPluginData.h>
#include <maya/MItDependencyNodes.h>

MTypeId  jcFeather::id(0x81030);
MString  jcFeather::typeName("jcFeather");

MObject  jcFeather::displayPercent;
MObject  jcFeather::displayAs;
MObject  jcFeather::uniformWidth;
MObject  jcFeather::render;
MObject  jcFeather::exactFeather;
MObject  jcFeather::active;
MObject  jcFeather::direction;
MObject  jcFeather::curveType;

MObject  jcFeather::rachisRenderAs;
MObject  jcFeather::rachisSegment;
MObject  jcFeather::rachisPosition;
MObject  jcFeather::rachisCrossSection;
MObject  jcFeather::rachisThick;
MObject  jcFeather::rachisStart;
MObject  jcFeather::rachisEnd;
MObject  jcFeather::rachisThickScale;

MObject  jcFeather::barbuleNum;
MObject  jcFeather::barbuleDensity;
MObject  jcFeather::forcePerSegment;
MObject  jcFeather::turnForce;

MObject  jcFeather::rotateLamda;
MObject  jcFeather::upDownLamda;

MObject  jcFeather::rachisNoiseFrequency;
MObject  jcFeather::barbuleNoiseFrequency;
MObject  jcFeather::noisePhase;

MObject  jcFeather::shapeSymmetry;
MObject  jcFeather::barbuleLengthRandScale;
MObject  jcFeather::barbuleSegments;
MObject  jcFeather::leftBarbuleLengthScale;
MObject  jcFeather::barbuleLength;

MObject  jcFeather::gapForce;
MObject  jcFeather::gapMaxForce;
MObject  jcFeather::gapSize;
MObject  jcFeather::leftGapDensity;
MObject  jcFeather::rightGapDensity;

MObject  jcFeather::rightBarbuleLengthScale;
MObject  jcFeather::upDownNoise;
MObject  jcFeather::barbuleUpDownScale;
MObject  jcFeather::barbuleUpDownRachis;
MObject  jcFeather::forceScale;
MObject  jcFeather::forceRotateScale;

MObject  jcFeather::keyBarbuleStepAngle;
MObject  jcFeather::keyBarbulePos1;
MObject  jcFeather::keyBarbulePos2;
MObject  jcFeather::keyBarbulePos3;
MObject  jcFeather::keyBarbulePos4;

MObject  jcFeather::keyBarbuleStartAngle1;
MObject  jcFeather::keyBarbuleStartAngle2;
MObject  jcFeather::keyBarbuleStartAngle3;
MObject  jcFeather::keyBarbuleStartAngle4;

MObject  jcFeather::kbRotate1;
MObject  jcFeather::kbRotate2;
MObject  jcFeather::kbRotate3;
MObject  jcFeather::kbRotate4;

MObject  jcFeather::barbuleThick;
MObject  jcFeather::barbuleThickScale;
MObject  jcFeather::barbuleRandThick;
MObject  jcFeather::barbuleThickAdjust;

MObject  jcFeather::displayProxyBoundingBox;
MObject  jcFeather::proxyBoundingBoxScale;

MObject  jcFeather::randSeed;

MObject  jcFeather::useOutShader;
MObject  jcFeather::projectTexUV;
MObject  jcFeather::uvProjectScale;
MObject  jcFeather::barbuleColorTex;
MObject  jcFeather::autoConvertTex;
MObject  jcFeather::textureProcedure;
MObject  jcFeather::interactiveTexture;
MObject  jcFeather::mainColor;
MObject  jcFeather::barbuleDiffuse;
MObject  jcFeather::barbuleSpecular;
MObject  jcFeather::barbuleSpecularColor;
MObject  jcFeather::barbuleGloss;
MObject  jcFeather::selfShadow;
MObject  jcFeather::barbuleHueVar;
MObject  jcFeather::barbuleSatVar;
MObject  jcFeather::barbuleValueVar;
MObject  jcFeather::varFrequency;

MObject  jcFeather::rootColor;
MObject  jcFeather::tipColor;
MObject  jcFeather::baseOpacity;
MObject  jcFeather::fadeOpacity;
MObject  jcFeather::fadeStart;

MObject  jcFeather::preFeatherRib;
MObject  jcFeather::preFeatherMeshRib;
MObject  jcFeather::postFeatherRib;
MObject  jcFeather::postFeatherMeshRib;

MObject  jcFeather::rachisRootColor;
MObject  jcFeather::rachisTipColor;

MObject  jcFeather::guidSurfaceInfo;
MObject  jcFeather::guidSurface;
MObject  jcFeather::guidUV;

MObject  jcFeather::inFeatherTurtleData;
MObject  jcFeather::inFeatherTurtleArrayData;

MObject  jcFeather::outputMesh;
MObject  jcFeather::outRenderFeather;

MObject  jcFeather::outputFeatherMesh;
MObject  jcFeather::meshUVScale;
MObject  jcFeather::outRachisMesh;
MObject  jcFeather::outBarbuleMesh;

#define MAKE_INPUT_VN(attr,keyable,stroable,readable,writable)\
    CHECK_MSTATUS( attr.setKeyable(keyable) );		\
    CHECK_MSTATUS( attr.setStorable(stroable) );	\
    CHECK_MSTATUS( attr.setReadable(readable) );	\
    CHECK_MSTATUS( attr.setWritable(writable) );

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

extern int jcFea_FreeVersionMaxNodesNum;
extern int jcFea_FreeVersionJCFeatherNodeIndex;

jcFeather::jcFeather()
{
	jcFea_FreeVersionJCFeatherNodeIndex+=1;
}

jcFeather::~jcFeather()
{
}

void	jcFeather::postConstructor()
{
    setExistWithoutOutConnections(false);
	featherID = new long;
	*featherID = feaInfoDataBase->Get_FreeID();
	featherData = new featherInfo();
	feaInfoDataBase->Add(featherID,featherData);
	surfaceIndexIDMap.clear();
	inTurtleIDMap.clear();
	featherDraw.clear();

	createDefaultRampData();
}

void	*jcFeather::creator()
{
	if(jcFea_FreeVersionJCFeatherNodeIndex < jcFea_FreeVersionMaxNodesNum)
		return new jcFeather();
	else
	{
		jcFea_FreeVersionJCFeatherNodeIndex-=1;
		if(jcFea_FreeVersionJCFeatherNodeIndex<0)
			jcFea_FreeVersionJCFeatherNodeIndex=0;

		MString str="JCFeather : With free version, you can only create ";
		str +=jcFea_FreeVersionMaxNodesNum;
		str +=" jcFeather nodes.";
		MGlobal::displayError(str);

		return NULL;
	}
}

MStatus jcFeather::initRampData(MObject& rampObj, 
								int index, 
								float position, 
								float value, 
								int interpolation)
{
	MStatus status;
	MPlug rampPlug( thisMObject(), rampObj );
	
	MPlug elementPlug = rampPlug.elementByLogicalIndex( index, &status );
	MPlug positionPlug = elementPlug.child(0, &status);
	
	status = positionPlug.setFloat(position);
	MPlug valuePlug = elementPlug.child(1);
	status = valuePlug.setFloat(value);
	MPlug interpPlug = elementPlug.child(2);
	interpPlug.setInt(interpolation);

	return MS::kSuccess;
}

void	jcFeather::createDefaultRampData()
{
	/*initRampData(leftGapDensity,0,0,1,MRampAttribute::kSpline);
	initRampData(rightGapDensity,0,0,1,MRampAttribute::kSpline);

	initRampData(leftGapDensity,0,0,1,MRampAttribute::kSpline);
	initRampData(rightGapDensity,0,0,1,MRampAttribute::kSpline);

	initRampData(forceRotateScale,0,0,1,MRampAttribute::kSpline);
	initRampData(forceScale,0,0,1,MRampAttribute::kSpline);

	initRampData(barbuleDensity,0,0,1,MRampAttribute::kSpline);
	initRampData(barbuleUpDownScale,0,0,1,MRampAttribute::kSpline);
	initRampData(barbuleUpDownRachis,0,0,1,MRampAttribute::kSpline);
	initRampData(barbuleThickAdjust,0,0,1,MRampAttribute::kSpline);
	initRampData(barbuleThickScale,0,0,0.2f,MRampAttribute::kSpline);
	initRampData(rachisThickScale,0,0,0.3f,MRampAttribute::kSpline);

	initRampData(kbRotate1,0,0,0.5f,MRampAttribute::kSpline);
	initRampData(kbRotate2,0,0,0.5f,MRampAttribute::kSpline);
	initRampData(kbRotate3,0,0,0.5f,MRampAttribute::kSpline);
	initRampData(kbRotate4,0,0,0.5f,MRampAttribute::kSpline);

	initRampData(leftBarbuleLengthScale,0,0,1,MRampAttribute::kSpline);
	initRampData(rightBarbuleLengthScale,0,0,1,MRampAttribute::kSpline);*/

	MObject me = thisMObject();

	MFloatArray pos(1,0),val(1,1);
	MIntArray intp(1,MRampAttribute::kSpline);

	MRampAttribute tempRamp(me,leftGapDensity);
	tempRamp.addEntries(pos,val,intp);

	tempRamp=MRampAttribute(me,rightGapDensity);
	tempRamp.addEntries(pos,val,intp);

	tempRamp=MRampAttribute(me,forceRotateScale);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,forceScale);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,barbuleDensity);
	tempRamp.addEntries(pos,val,intp);

	tempRamp=MRampAttribute(me,barbuleUpDownScale);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,barbuleUpDownRachis);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,barbuleThickAdjust);
	tempRamp.addEntries(pos,val,intp);
	
	val[0]=0.2f;
	tempRamp=MRampAttribute(me,barbuleThickScale);
	tempRamp.addEntries(pos,val,intp);
	val[0]=0.3f;
	tempRamp=MRampAttribute(me,rachisThickScale);
	tempRamp.addEntries(pos,val,intp);

	val[0]=0.5f;
	tempRamp=MRampAttribute(me,kbRotate1);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,kbRotate2);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,kbRotate3);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,kbRotate4);
	tempRamp.addEntries(pos,val,intp);

	val[0]=1;
	tempRamp=MRampAttribute(me,leftBarbuleLengthScale);
	tempRamp.addEntries(pos,val,intp);
	tempRamp=MRampAttribute(me,rightBarbuleLengthScale);
	tempRamp.addEntries(pos,val,intp);
}

void	jcFeather::nodeRemoved(MObject& node, void *clientData)
{
	jcFea_FreeVersionJCFeatherNodeIndex-=1;

	if(jcFea_FreeVersionJCFeatherNodeIndex<0)
		jcFea_FreeVersionJCFeatherNodeIndex=0;
	
	MFnDependencyNode fnNode(node);

	static_cast<jcFeather*>(fnNode.userNode())->featherDraw.clear();

	//---remove surface feather
	std::map <int,long>::iterator m1_Iter;
	for ( m1_Iter = static_cast<jcFeather*>(fnNode.userNode())->surfaceIndexIDMap.begin(); m1_Iter != static_cast<jcFeather*>(fnNode.userNode())->surfaceIndexIDMap.end(); m1_Iter++ )
		removeTurtlesDataBaseKey(&(m1_Iter->second));
	static_cast<jcFeather*>(fnNode.userNode())->surfaceIndexIDMap.clear();

	//---remove inTurtleFeater
	for ( m1_Iter = static_cast<jcFeather*>(fnNode.userNode())->inTurtleIDMap.begin(); m1_Iter != static_cast<jcFeather*>(fnNode.userNode())->inTurtleIDMap.end(); m1_Iter++ )
		removeTurtlesDataBaseKey(&(m1_Iter->second));
	static_cast<jcFeather*>(fnNode.userNode())->inTurtleIDMap.clear();

	//---remove inTurtleArray Feather
	std::map<std::pair<int,int>,long>::iterator m2_Iter;
	for ( m2_Iter = static_cast<jcFeather*>(fnNode.userNode())->inTurtleArrayIDMap.begin(); m2_Iter != static_cast<jcFeather*>(fnNode.userNode())->inTurtleArrayIDMap.end(); m2_Iter++ )
		removeTurtlesDataBaseKey(&(m2_Iter->second));
	static_cast<jcFeather*>(fnNode.userNode())->inTurtleArrayIDMap.clear();

	//----remove featherID and featherData
	removeFeaInfoDataBase(static_cast<jcFeather*>(fnNode.userNode())->featherID);
	static_cast<jcFeather*>(fnNode.userNode())->featherID=NULL;
	static_cast<jcFeather*>(fnNode.userNode())->featherData=NULL;
}

void	jcFeather::getActiveJCFeatherNode(MObjectArray &featherAry,bool visibleOnly)
{
	MStatus status=MS::kSuccess;
	featherAry.clear();
	MItDependencyNodes itNode(MFn::kPluginDependNode,&status);
	MFnDependencyNode dgNodeFn;

	CHECK_MSTATUS(status);
	for( itNode.reset(MFn::kPluginDependNode);!itNode.isDone();itNode.next() )
	{
		MObject node_it=itNode.thisNode();
		if( !featherTools::isObjectVisible(node_it) && visibleOnly )continue;

		dgNodeFn.setObject(itNode.thisNode());
		if( dgNodeFn.typeId() != jcFeather::id ) continue;
		if(!(dgNodeFn.findPlug(jcFeather::active,false).asBool())) continue;
		
		featherAry.append( itNode.thisNode() );
	}
}

MStatus jcFeather::initialize()
{
	MStatus status;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

    MFnNumericAttribute nAttr;

	outRenderFeather = nAttr.create( "featherInfoID", "feaid", MFnNumericData::kLong,-1);
	MAKE_OUTPUT(nAttr);
	nAttr.setHidden(true);
	uniformWidth = nAttr.create( "uniformWidth", "uniwi", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	useOutShader= nAttr.create( "useOutShader", "useos", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	interactiveTexture= nAttr.create( "interactiveColor", "intcol", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);

	displayPercent = nAttr.create( "displayQuality", "dispqua", MFnNumericData::kInt,20);
	MAKE_INPUT(nAttr,true);nAttr.setMax(100);nAttr.setMin(0);
	outputFeatherMesh = nAttr.create( "outputFeatherMesh", "outfeamesh", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,true);
	shapeSymmetry = nAttr.create( "shapeSymmetry", "shapesym", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	active = nAttr.create( "active", "active", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	render = nAttr.create( "render", "render", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	exactFeather = nAttr.create( "surfaceFeather", "surfea", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,true);
	rachisSegment = nAttr.create( "rachisSegment", "racseg", MFnNumericData::kInt,10);
	MAKE_INPUT(nAttr,true);nAttr.setMin(1);
	rachisPosition = nAttr.create( "rachisPosition", "racpos", MFnNumericData::kDouble,0.5);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);
	rachisCrossSection = nAttr.create( "rachisSides", "racsides", MFnNumericData::kInt,4);
	MAKE_INPUT(nAttr,true);nAttr.setMin(3);
	rachisThick = nAttr.create( "rachisThick", "racthick", MFnNumericData::kDouble,0.5);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	rachisStart = nAttr.create( "rachisStart", "racstart", MFnNumericData::kDouble,0.0);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);
	rachisEnd = nAttr.create( "rachisEnd", "racend", MFnNumericData::kDouble,1.0);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);
	barbuleNum = nAttr.create( "barbuleNum", "barn", MFnNumericData::kInt,100);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	forcePerSegment = nAttr.create( "forcePerSegment", "forceperseg", MFnNumericData::kDouble,0);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	turnForce = nAttr.create( "turnAtForce", "turnaf", MFnNumericData::kDouble,1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);

	rotateLamda = nAttr.create( "rotateDegree", "rotdeg", MFnNumericData::kDouble,5);
	MAKE_INPUT(nAttr,true);
	upDownNoise = nAttr.create( "upDownNoise", "updown", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);
	upDownLamda = nAttr.create( "upDownLength", "updowl", MFnNumericData::kDouble,0.00);
	MAKE_INPUT(nAttr,true);
		
	gapForce = nAttr.create( "gapForce", "gapForce", MFnNumericData::k2Double,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0,0);
	gapMaxForce = nAttr.create( "gapMaxForce", "gapMaxForce", MFnNumericData::k2Double,1);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0,0);
	gapSize = nAttr.create( "gapSize", "gapSize", MFnNumericData::k2Double,3);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0,0);
	nAttr.setMax(180,180);

	rachisNoiseFrequency = nAttr.create( "rachisNoiseFrequency", "racnoisefre", MFnNumericData::kDouble,10);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	barbuleNoiseFrequency = nAttr.create( "barbuleNoiseFrequency", "barnoisefre", MFnNumericData::kDouble,20);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	noisePhase = nAttr.create( "noisePhase", "noisephase", MFnNumericData::k2Double,0);
	MAKE_INPUT(nAttr,true);
	meshUVScale= nAttr.create( "meshUVScale", "muvsc", MFnNumericData::k2Double,1);
	MAKE_INPUT(nAttr,true);

	keyBarbuleStepAngle = nAttr.create( "keyBarbuleStepAngle", "kbsd", MFnNumericData::kDouble,10);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-360);
	nAttr.setMax(360);
	keyBarbulePos1 = nAttr.create( "keyBarbulePos1", "kbp1", MFnNumericData::kDouble,0.0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(1);
	keyBarbulePos2 = nAttr.create( "keyBarbulePos2", "kbp2", MFnNumericData::kDouble,0.2);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(1);
	keyBarbulePos3 = nAttr.create( "keyBarbulePos3", "kbp3", MFnNumericData::kDouble,0.8);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(1);
	keyBarbulePos4 = nAttr.create( "keyBarbulePos4", "kbp4", MFnNumericData::kDouble,1.0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(1);

	keyBarbuleStartAngle1 = nAttr.create( "keyBarbuleStartAngle1", "kbsa1", MFnNumericData::kDouble,60);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-180);
	nAttr.setMax(180);
	keyBarbuleStartAngle2 = nAttr.create( "keyBarbuleStartAngle2", "kbsa2", MFnNumericData::kDouble,40);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-180);
	nAttr.setMax(180);
	keyBarbuleStartAngle3 = nAttr.create( "keyBarbuleStartAngle3", "kbsa3", MFnNumericData::kDouble,20);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-180);
	nAttr.setMax(180);
	keyBarbuleStartAngle4 = nAttr.create( "keyBarbuleStartAngle4", "kbsa4", MFnNumericData::kDouble,10);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(-180);
	nAttr.setMax(180);


	barbuleLengthRandScale = nAttr.create( "barbuleLengthRandScale", "barls", MFnNumericData::kDouble,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0.0f);
	nAttr.setMax(1.0f);
	barbuleSegments = nAttr.create( "barbuleSegments", "bars", MFnNumericData::kInt,8);
	MAKE_INPUT(nAttr,true);nAttr.setMin(1);
	barbuleLength = nAttr.create( "barbuleLength", "barl", MFnNumericData::kDouble,0.2f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	barbuleThick = nAttr.create( "barbuleThick", "barth", MFnNumericData::kDouble,0.3);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	barbuleRandThick = nAttr.create( "barbuleRandThick", "barrath", MFnNumericData::kDouble,0);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);
	randSeed = nAttr.create( "randSeed", "randseed", MFnNumericData::kInt,4352);
	MAKE_INPUT(nAttr,false);

	uvProjectScale = nAttr.create( "uvProjectScale", "uvprosc", MFnNumericData::kDouble,1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	projectTexUV =	nAttr.create( "projectTexUV", "protuv", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	autoConvertTex =	nAttr.create( "autoConvertTex", "autoct", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,true);
	barbuleSpecular = nAttr.create( "barbuleSpecular", "barsp", MFnNumericData::kDouble,1);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	barbuleGloss = nAttr.create( "barbuleGloss", "bargl", MFnNumericData::kDouble,0.09);
	MAKE_INPUT(nAttr,true);nAttr.setMax(0.1);nAttr.setMin(0);
	barbuleDiffuse = nAttr.create( "barbuleDiffuse", "bardif", MFnNumericData::kDouble,0.5);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	selfShadow = nAttr.create( "selfShadow", "selfshadow", MFnNumericData::kDouble,1);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	barbuleSpecularColor = nAttr.createColor("barbuleSpecularColor", "barspecc");
	nAttr.setDefault(1.0,1.0,1.0);
	MAKE_INPUT(nAttr,true);
	rootColor= nAttr.createColor("rootColor", "rtc");
	nAttr.setDefault(0.7,0.7,0.7);
	MAKE_INPUT(nAttr,true);
	tipColor= nAttr.createColor("tipColor", "tpc");
	nAttr.setDefault(1.0,1.0,1.0);
	MAKE_INPUT(nAttr,true);
	baseOpacity = nAttr.create( "rootOpacity", "rooto", MFnNumericData::kDouble,1.0);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	fadeOpacity = nAttr.create( "tipOpacity", "tipo", MFnNumericData::kDouble,1.0);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	fadeStart = nAttr.create( "fadeStart", "fadestart", MFnNumericData::kDouble,0.9);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);

	barbuleHueVar = nAttr.create( "hueVar", "huevar", MFnNumericData::kInt,0);
	MAKE_INPUT(nAttr,true);nAttr.setMax(360);nAttr.setMin(0);
	barbuleSatVar = nAttr.create( "satVar", "satvar", MFnNumericData::kDouble,0.0);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	barbuleValueVar = nAttr.create( "valueVar", "valvar", MFnNumericData::kDouble,0.0);
	MAKE_INPUT(nAttr,true);nAttr.setMax(1.0);nAttr.setMin(0);
	varFrequency = nAttr.create( "varFrequency", "varfreq", MFnNumericData::kDouble,325.12);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	rachisRootColor= nAttr.createColor("rachisRootColor", "rartc");
	nAttr.setDefault(0.3,0.3,0.3);
	MAKE_INPUT(nAttr,true);
	rachisTipColor= nAttr.createColor("rachisTipColor", "rattc");
	nAttr.setDefault(0.8,0.8,0.8);
	MAKE_INPUT(nAttr,true);
	mainColor = nAttr.createColor("mainColor", "mainc");
	MAKE_INPUT_VN(nAttr,false,false,true,true);

	MFnTypedAttribute typAttr;

	MFnStringData strData;
	MObject tempObj = strData.create("");
	preFeatherRib = typAttr.create( "preFeatherRib", "prefr", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	preFeatherMeshRib = typAttr.create( "preFeatherMeshRib", "prefmr", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	postFeatherRib = typAttr.create( "postFeatherRib", "postfr", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	postFeatherMeshRib = typAttr.create( "postFeatherMeshRib", "postfmr", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);


	tempObj = strData.create("");
	barbuleColorTex = typAttr.create( "barbuleColorTex", "barcoltex",MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);
	tempObj = strData.create("");
	textureProcedure = typAttr.create( "textureProcedure", "texpro",MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	outputMesh = typAttr.create("outputMesh","outputm",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	outRachisMesh = typAttr.create("outRachisMesh","outrame",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	outBarbuleMesh = typAttr.create("outBarbuleMesh","outbame",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	MFnEnumAttribute enumAttr;

	direction = enumAttr.create("direction","dir",0);
	enumAttr.addField("U",0);
	enumAttr.addField("V",1);
	MAKE_INPUT(enumAttr,true);

	displayAs = enumAttr.create("displayAs","disas",0);
	enumAttr.addField("FinalFeather",0);
	enumAttr.addField("KeyBarbule",1);
	enumAttr.addField("None",2);
	MAKE_INPUT(enumAttr,true);

	rachisRenderAs = enumAttr.create("rachisRenderAs","rachra",0);
	enumAttr.addField("Nurbs Curve",0);
	enumAttr.addField("Subdivision Mesh",1);
	enumAttr.addField("None",3);
	MAKE_INPUT(enumAttr,true);

	curveType = enumAttr.create("curveType","curvetype",1);
	enumAttr.addField("Linear Curve",0);
	enumAttr.addField("Cubic Curve",1);
	MAKE_INPUT(enumAttr,true);

	rachisThickScale =  MRampAttribute::createCurveRamp("rachisThickScale", "racthS");

	leftBarbuleLengthScale = MRampAttribute::createCurveRamp("leftBarbuleLengthScale", "lbarbls");

	rightBarbuleLengthScale =  MRampAttribute::createCurveRamp("rightBarbuleLengthScale", "rbarbls");

	barbuleUpDownScale =  MRampAttribute::createCurveRamp("barbuleUpDownBarbule", "barbudb");

	barbuleUpDownRachis =  MRampAttribute::createCurveRamp("barbuleUpDownRachis", "barbudrs");

	leftGapDensity =  MRampAttribute::createCurveRamp("leftGapDensity", "lgpden");

	rightGapDensity =  MRampAttribute::createCurveRamp("rightGapDensity", "rgpden");
	
	barbuleThickScale = MRampAttribute::createCurveRamp("barbuleThickScale", "barbts");

	barbuleThickAdjust = MRampAttribute::createCurveRamp("barbuleThickAdjust", "barbtadj");

	barbuleDensity = MRampAttribute::createCurveRamp( "barbuleDensity", "barden");


	forceScale = MRampAttribute::createCurveRamp("forcePerSegmentScale", "forpss");

	forceRotateScale = MRampAttribute::createCurveRamp("forceRotateScale", "forrs");

	kbRotate1 = MRampAttribute::createCurveRamp("kbRotate1", "kbr1");
	kbRotate2 = MRampAttribute::createCurveRamp("kbRotate2", "kbr2");
	kbRotate3 = MRampAttribute::createCurveRamp("kbRotate3", "kbr3");
	kbRotate4 = MRampAttribute::createCurveRamp("kbRotate4", "kbr4");

	//------------guid surface info
	guidSurface = typAttr.create("guidSurface","guis",MFnNurbsSurfaceData::kNurbsSurface);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kReset) );

	guidUV = nAttr.create( "guidUV", "guv", MFnNumericData::k2Float,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0,0);nAttr.setMax(1,1);

	MFnCompoundAttribute compoundAttr;
	guidSurfaceInfo = compoundAttr.create("guidSurfaceInfo","gsinf");
	compoundAttr.addChild(guidSurface);
	compoundAttr.addChild(guidUV);
	compoundAttr.setArray(true);
	compoundAttr.setHidden(true);
	CHECK_MSTATUS( compoundAttr.setDisconnectBehavior(MFnAttribute::kDelete) );

	//-------------------------guid mesh info
	inFeatherTurtleData = typAttr.create("inFeatherTurtleData",
										  "infeatd",
										    featherTurtleData::id,
											MObject::kNullObj,
											&status);
	MAKE_INPUT(typAttr,false);
	typAttr.setArray(true);
	typAttr.setHidden(true);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kDelete) );

	inFeatherTurtleArrayData = typAttr.create("inFeatherTurtleArrayData",
											  "infeatarrayd",
												featherTurtleArrayData::id,
												MObject::kNullObj,
												&status);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setArray(true);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kDelete) );
	//-------------------------guid mesh info


	displayProxyBoundingBox =	nAttr.create( "displayProxyBoundingBox", "dispPBB", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,true);
	proxyBoundingBoxScale = nAttr.create( "proxyBoundingBoxScale", "pbbs", MFnNumericData::k3Double,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setDefault(0.0,0.0,0.1);

	addAffect();

    return MS::kSuccess;
}

void    jcFeather::addAffect()
{
	CHECK_MSTATUS( addAttribute(displayPercent) );
	CHECK_MSTATUS( addAttribute(displayAs) );
	CHECK_MSTATUS( addAttribute(uniformWidth) );
	CHECK_MSTATUS( addAttribute(active) );
	CHECK_MSTATUS( addAttribute(render) );
	CHECK_MSTATUS( addAttribute(curveType) );
	CHECK_MSTATUS( addAttribute(exactFeather) );
	CHECK_MSTATUS( addAttribute(direction) );
	CHECK_MSTATUS( addAttribute(rachisRenderAs) );
	CHECK_MSTATUS( addAttribute(rachisSegment) );
	CHECK_MSTATUS( addAttribute(rachisPosition) );
	CHECK_MSTATUS( addAttribute(rachisCrossSection) );
	CHECK_MSTATUS( addAttribute(rachisThick) );
	CHECK_MSTATUS( addAttribute(rachisStart) );
	CHECK_MSTATUS( addAttribute(rachisEnd) );
	CHECK_MSTATUS( addAttribute(rachisThickScale) );
	CHECK_MSTATUS( addAttribute(barbuleNum) );
	CHECK_MSTATUS( addAttribute(barbuleDensity) );
	CHECK_MSTATUS( addAttribute(forceScale) );
	CHECK_MSTATUS( addAttribute(forceRotateScale) );
	CHECK_MSTATUS( addAttribute(forcePerSegment) );
	CHECK_MSTATUS( addAttribute(turnForce) );
	CHECK_MSTATUS( addAttribute(rotateLamda) );
	CHECK_MSTATUS( addAttribute(rachisNoiseFrequency) );
	CHECK_MSTATUS( addAttribute(barbuleNoiseFrequency) );
	CHECK_MSTATUS( addAttribute(noisePhase) );

	CHECK_MSTATUS( addAttribute(upDownLamda) );
	CHECK_MSTATUS( addAttribute(keyBarbuleStepAngle) );

	CHECK_MSTATUS( addAttribute(gapForce) );
	CHECK_MSTATUS( addAttribute(gapMaxForce) );
	CHECK_MSTATUS( addAttribute(gapSize) );

	CHECK_MSTATUS( addAttribute(keyBarbulePos1) );
	CHECK_MSTATUS( addAttribute(keyBarbulePos2) );
	CHECK_MSTATUS( addAttribute(keyBarbulePos3) );
	CHECK_MSTATUS( addAttribute(keyBarbulePos4) );

	CHECK_MSTATUS( addAttribute(keyBarbuleStartAngle1) );
	CHECK_MSTATUS( addAttribute(keyBarbuleStartAngle2) );
	CHECK_MSTATUS( addAttribute(keyBarbuleStartAngle3) );
	CHECK_MSTATUS( addAttribute(keyBarbuleStartAngle4) );

	CHECK_MSTATUS( addAttribute(kbRotate1) );
	CHECK_MSTATUS( addAttribute(kbRotate2) );
	CHECK_MSTATUS( addAttribute(kbRotate3) );
	CHECK_MSTATUS( addAttribute(kbRotate4) );

	CHECK_MSTATUS( addAttribute(barbuleLengthRandScale) );
	CHECK_MSTATUS( addAttribute(shapeSymmetry) );
	CHECK_MSTATUS( addAttribute(barbuleSegments) );
	CHECK_MSTATUS( addAttribute(leftBarbuleLengthScale) );
	CHECK_MSTATUS( addAttribute(barbuleLength) );
	CHECK_MSTATUS( addAttribute(rightBarbuleLengthScale) );
	CHECK_MSTATUS( addAttribute(barbuleThick) );
	CHECK_MSTATUS( addAttribute(barbuleRandThick) );
	CHECK_MSTATUS( addAttribute(barbuleThickScale) );
	CHECK_MSTATUS( addAttribute(barbuleThickAdjust) );
	CHECK_MSTATUS( addAttribute(barbuleUpDownScale) );
	CHECK_MSTATUS( addAttribute(barbuleUpDownRachis) );

	CHECK_MSTATUS( addAttribute(leftGapDensity) );
	CHECK_MSTATUS( addAttribute(rightGapDensity) );

	CHECK_MSTATUS( addAttribute(upDownNoise) );

	CHECK_MSTATUS( addAttribute(randSeed) );

	CHECK_MSTATUS( addAttribute(displayProxyBoundingBox) );
	CHECK_MSTATUS( addAttribute(proxyBoundingBoxScale) );
	
	CHECK_MSTATUS( addAttribute(useOutShader) );
	CHECK_MSTATUS( addAttribute(uvProjectScale) );
	CHECK_MSTATUS( addAttribute(projectTexUV) );
	CHECK_MSTATUS( addAttribute(autoConvertTex) );
	CHECK_MSTATUS( addAttribute(barbuleColorTex) );
	CHECK_MSTATUS( addAttribute(textureProcedure) );
	CHECK_MSTATUS( addAttribute(rootColor) );
	CHECK_MSTATUS( addAttribute(tipColor) );
	CHECK_MSTATUS( addAttribute(fadeStart) );
	CHECK_MSTATUS( addAttribute(interactiveTexture) );
	CHECK_MSTATUS( addAttribute(mainColor) );
	CHECK_MSTATUS( addAttribute(baseOpacity) );
	CHECK_MSTATUS( addAttribute(fadeOpacity) );
	CHECK_MSTATUS( addAttribute(barbuleDiffuse) );
	CHECK_MSTATUS( addAttribute(barbuleSpecular) );
	CHECK_MSTATUS( addAttribute(barbuleSpecularColor) );
	CHECK_MSTATUS( addAttribute(barbuleGloss) );
	CHECK_MSTATUS( addAttribute(selfShadow) );
	CHECK_MSTATUS( addAttribute(barbuleHueVar) );
	CHECK_MSTATUS( addAttribute(barbuleSatVar) );
	CHECK_MSTATUS( addAttribute(barbuleValueVar) );
	CHECK_MSTATUS( addAttribute(varFrequency) );

	CHECK_MSTATUS( addAttribute(preFeatherRib) );
	CHECK_MSTATUS( addAttribute(postFeatherRib) );
	CHECK_MSTATUS( addAttribute(preFeatherMeshRib) );
	CHECK_MSTATUS( addAttribute(postFeatherMeshRib) );

	CHECK_MSTATUS( addAttribute(rachisRootColor) );
	CHECK_MSTATUS( addAttribute(rachisTipColor) );
	CHECK_MSTATUS( addAttribute(guidSurfaceInfo) );
	CHECK_MSTATUS( addAttribute(inFeatherTurtleData) );
	CHECK_MSTATUS( addAttribute(inFeatherTurtleArrayData) );

	CHECK_MSTATUS( addAttribute(outputMesh) );
	CHECK_MSTATUS( addAttribute(outRenderFeather) );
	CHECK_MSTATUS( addAttribute(outputFeatherMesh) );
	CHECK_MSTATUS( addAttribute(meshUVScale) );
	CHECK_MSTATUS( addAttribute(outRachisMesh) );
	CHECK_MSTATUS( addAttribute(outBarbuleMesh) );

	//--------------------------------------
	CHECK_MSTATUS( attributeAffects( displayPercent, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( displayAs, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( uniformWidth, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( active, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( exactFeather, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( direction, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( curveType, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisSegment, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisPosition, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisCrossSection, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisStart, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisEnd, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisThick, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisThickScale, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( barbuleNum, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleDensity, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( forceScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( forceRotateScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( forcePerSegment, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( turnForce, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rotateLamda, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisNoiseFrequency, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleNoiseFrequency, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( noisePhase, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( upDownLamda, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( upDownNoise, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStepAngle, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos1, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos2, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos3, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos4, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle1, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle2, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle3, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle4, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate1, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate2, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate3, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate4, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( shapeSymmetry, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleSegments, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleLength, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( leftBarbuleLengthScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rightBarbuleLengthScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( leftGapDensity, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rightGapDensity, outputMesh ) );	
	
	CHECK_MSTATUS( attributeAffects( barbuleLengthRandScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleUpDownScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleUpDownRachis, outputMesh ) );
		
	CHECK_MSTATUS( attributeAffects( gapForce, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( gapMaxForce, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( gapSize, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( barbuleThick, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleThickScale, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleRandThick, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleThickAdjust, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( randSeed, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( displayProxyBoundingBox, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( proxyBoundingBoxScale, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( rootColor, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( tipColor, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( interactiveTexture, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( mainColor, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( rachisRootColor, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisTipColor, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( uvProjectScale, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( guidSurfaceInfo, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( guidSurface, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( guidUV, outputMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherTurtleData, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( inFeatherTurtleArrayData, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( outputFeatherMesh, outputMesh ) );
	CHECK_MSTATUS( attributeAffects( meshUVScale, outputMesh ) );
	//----------------

	//----------------
	CHECK_MSTATUS( attributeAffects( displayPercent, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( uniformWidth, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( active, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( exactFeather, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( direction, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( curveType, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisSegment, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisThick, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisThickScale, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisPosition, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisCrossSection, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisStart, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisEnd, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( randSeed, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( guidSurfaceInfo, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( guidSurface, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( guidUV, outRachisMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherTurtleData, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( inFeatherTurtleArrayData, outRachisMesh ) );
	CHECK_MSTATUS( attributeAffects( outputFeatherMesh, outRachisMesh ) );
	//----------------



	//---------------
	CHECK_MSTATUS( attributeAffects( displayPercent, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( active, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( uniformWidth, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( exactFeather, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( direction, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( curveType, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisSegment, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisPosition, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisStart, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisEnd, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleNum, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleDensity, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( forceScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( forceRotateScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( forcePerSegment, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( turnForce, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rotateLamda, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rachisNoiseFrequency, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleNoiseFrequency, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( noisePhase, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( upDownLamda, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( upDownNoise, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStepAngle, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos1, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos2, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos3, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbulePos4, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle1, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle2, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle3, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( keyBarbuleStartAngle4, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate1, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate2, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate3, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( kbRotate4, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( shapeSymmetry, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleSegments, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleLength, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleThick, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleThickScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleRandThick, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleThickAdjust, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( leftGapDensity, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rightGapDensity, outBarbuleMesh ) );
	
	CHECK_MSTATUS( attributeAffects( leftBarbuleLengthScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( rightBarbuleLengthScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleLengthRandScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleUpDownScale, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( barbuleUpDownRachis, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( gapForce, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( gapMaxForce, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( gapSize, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( randSeed, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( guidSurfaceInfo, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( guidSurface, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( guidUV, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( inFeatherTurtleData, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( inFeatherTurtleArrayData, outBarbuleMesh ) );

	CHECK_MSTATUS( attributeAffects( outputFeatherMesh, outBarbuleMesh ) );
	CHECK_MSTATUS( attributeAffects( meshUVScale, outBarbuleMesh ) );

}

void	jcFeather::cleanOutMesh(MDataBlock& block)
{
	MDataHandle    outputFeatherMesh_Hnd  = block.inputValue(outputFeatherMesh);
	MDataHandle    outputRachisMeshHnd    = block.outputValue( outRachisMesh );
	MDataHandle    outputBarbuleMeshHnd   = block.outputValue( outBarbuleMesh );
	
	MFnMeshData meshDataFn;
	MFnMesh meshFn;
	polyObj jcOutDisplayMesh;

	jcOutDisplayMesh.init();
	MObject newMeshData = meshDataFn.create();
	meshFn.create(0,0,jcOutDisplayMesh.pa,jcOutDisplayMesh.faceCounts,jcOutDisplayMesh.faceConnects,newMeshData);
	meshFn.updateSurface();

	outputRachisMeshHnd.set(newMeshData);
	outputBarbuleMeshHnd.set(newMeshData);
	outputRachisMeshHnd.setClean();
	outputBarbuleMeshHnd.setClean();
}

MStatus jcFeather::compute(const MPlug& plug, MDataBlock& block)
{
	//-----------------����featherinfoid
	MDataHandle    outRenderFeaHnd  = block.outputValue( outRenderFeather );
	int feaId   = *featherID;
	outRenderFeaHnd.set(feaId);
	outRenderFeaHnd.setClean();

	//----------�鿴�Ƿ񼤻�
	MDataHandle active_Hnd = block.inputValue(active);
	bool activeornot = active_Hnd.asBool();
	if( !activeornot )
	{
		featherDraw.clear();
		return MS::kSuccess;
	}

	MFnMeshData meshDataFn;
	MFnMesh meshFn;
	polyObj jcOutDisplayMesh;

	MDataHandle    outputFeatherMesh_Hnd  = block.inputValue(outputFeatherMesh);
	if( plug == outputMesh )
	{
		if( !makeFeathers( block ) ) return MS::kFailure;
		MDataHandle    outputMeshHnd    = block.outputValue( outputMesh );

		MObject newMeshData = meshDataFn.create();

		meshFn.create(0,0,jcOutDisplayMesh.pa,jcOutDisplayMesh.faceCounts,jcOutDisplayMesh.faceConnects,newMeshData);
		meshFn.updateSurface();
		outputMeshHnd.set(newMeshData);
		outputMeshHnd.setClean();
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
	else if( plug == outRachisMesh || plug == outBarbuleMesh )
	{
		if( outputFeatherMesh_Hnd.asBool() !=0 )
		{
			MDataHandle    outputRachisMeshHnd    = block.outputValue( outRachisMesh );
			MDataHandle    outputBarbuleMeshHnd   = block.outputValue( outBarbuleMesh );

			polyObj jcOutRachis,jcOutBarbule;
			MObject rachisData,barbuleData;

			if(plug == outRachisMesh)
			{
				for(unsigned int ii=0;ii<featherDraw.size();++ii)
				{
					jcOutRachis.appendMesh(featherDraw[ii].rachisCylinder);
					featherDraw[ii].rachisCylinder.init();
				}
				if( jcOutRachis.convertToMObject(rachisData,NULL) )
				{
					outputRachisMeshHnd.set(rachisData);
					outputRachisMeshHnd.setClean();
				}
			}
			if(plug == outBarbuleMesh)
			{
				for(unsigned int ii=0;ii<featherDraw.size();++ii)
				{
					jcOutBarbule.appendMesh(featherDraw[ii].barbuleFace);
					featherDraw[ii].barbuleFace.init();
				}
				if( jcOutBarbule.convertToMObject(barbuleData,NULL) )
				{
					outputBarbuleMeshHnd.set(barbuleData);
					outputBarbuleMeshHnd.setClean();
				}
			}
		}
		else
			cleanOutMesh(block);

		return MS::kSuccess;
	}
	else
		return MS::kUnknownParameter;
}

void    jcFeather::draw(M3dView &view,
						 const MDagPath &path,
						 M3dView::DisplayStyle style,
						 M3dView::DisplayStatus dispStatus)
{
	MObject thisNode = thisMObject();

	MPlug tPlug(thisNode,displayAs);
	MPlug dpbbPlug(thisNode,displayProxyBoundingBox);
	MPlug renderPlug(thisNode,render);

	MFnDagNode fnDagNode(thisNode);
	MFnTransform fnParentTransform(fnDagNode.parent(0));
	m_worldMatrix = fnParentTransform.transformation().asMatrix();

	MColorArray store4Color(4,MColor(1,1,1));
	store4Color[0] = featherData->_shader._rachisRootColor;
	store4Color[1] = featherData->_shader._rachisTipColor;
	store4Color[2] = featherData->_shader._rootColor;
	store4Color[3] = featherData->_shader._tipColor;
	view.beginGL();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
				double m[4][4];
				m_worldMatrix.inverse().get(m);
				glMultMatrixd(&(m[0][0]));
				if(renderPlug.asBool())
				{
					if( dispStatus == M3dView::kLead )
						jerryC::drawFeather(featherDraw,1,store4Color);
					else if(dispStatus == M3dView::kDormant)
						jerryC::drawFeather(featherDraw,0.6f,store4Color);
					else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
						jerryC::drawFeatherWithColor(featherDraw,colorRGB(dispStatus));
				}
				else//�����Ⱦ����ë��ʾΪ��ɫ
				{
					if( dispStatus == M3dView::kLead )
						jerryC::drawFeatherWithColor(featherDraw,MColor(1,1,0));
					else if(dispStatus == M3dView::kDormant)
						jerryC::drawFeatherWithColor(featherDraw,MColor(0.6f,0.6f,0));
					else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
						jerryC::drawFeatherWithColor(featherDraw,colorRGB(dispStatus));
				}
				if(dpbbPlug.asBool())//����proxyboundingbox
					jerryC::drawFeatherProxyBB(featherDraw,MColor(0,1,0));
			glPopMatrix();
		glPopAttrib();
	view.endGL();
}

MBoundingBox   jcFeather::boundingBox() const//���boundingboxû�д�С���Ͳ�����viewport�е���draw������ʾ
{
	MBoundingBox result;//boundingbox�Ĵ�С��proxyboundingbox�����档
	for(unsigned int ii=0;ii<featherDraw.size();++ii)
		result.expand(featherDraw[ii].proxyBBox);

	return result;
}

bool	jcFeather::makeFeathers( MDataBlock& block )
{
	MStatus status=MS::kSuccess;

	MDataHandle displayP_Hnd          = block.inputValue(displayPercent);
	int displayPer= displayP_Hnd.asInt();

	MDataHandle direction_Hnd          = block.inputValue(direction);
	featherData->_direction = direction_Hnd.asShort();

	MDataHandle curveType_Hnd          = block.inputValue(curveType);
	featherData->_curveType = curveType_Hnd.asShort();

	MDataHandle rachisRenderAs_Hnd			  = block.inputValue(rachisRenderAs);
	featherData->_rachisRenderAs = rachisRenderAs_Hnd.asShort();

	MDataHandle outputFeatherMesh_Hnd		  = block.inputValue(outputFeatherMesh);
	featherData->_outputFeatherMesh = outputFeatherMesh_Hnd.asBool();

	MDataHandle uniformWidth_Hnd			  = block.inputValue(uniformWidth);
	featherData->_uniformWidth = uniformWidth_Hnd.asBool();

	MDataHandle rachisS_Hnd			  = block.inputValue(rachisSegment);
	featherData->_rachisSegment = rachisS_Hnd.asInt()+1;

	MDataHandle rachisPosition_Hnd          = block.inputValue(rachisPosition);
	featherData->_rachisPos = (float)(rachisPosition_Hnd.asDouble());

	MDataHandle rachisCs_Hnd			  = block.inputValue(rachisCrossSection);
	featherData->_rachisSides = rachisCs_Hnd.asInt();

	MDataHandle rachisThick_Hnd			  = block.inputValue(rachisThick);
	featherData->_rachisThick = (float)rachisThick_Hnd.asDouble();

	MDataHandle barbuleRandThick_Hnd			  = block.inputValue(barbuleRandThick);
	featherData->_barbuleRandThick = (float)barbuleRandThick_Hnd.asDouble();

	MDataHandle rachisStart_Hnd          = block.inputValue(rachisStart);
	featherData->_rachisStart = (float)(rachisStart_Hnd.asDouble());

	MDataHandle rachisEnd_Hnd          = block.inputValue(rachisEnd);
	featherData->_rachisEnd = (float)(rachisEnd_Hnd.asDouble());

	MDataHandle barbuleNum_Hnd         = block.inputValue(barbuleNum);
	featherData->_renderBarbuleNum = barbuleNum_Hnd.asInt();
	featherData->_displayPercent=(float)(displayPer)/100.0f;
	featherData->_barbuleNum  = (int)( (float)featherData->_renderBarbuleNum * featherData->_displayPercent );

	MDataHandle forcePerSegment_Hnd          = block.inputValue(forcePerSegment);
	featherData->_forcePerSegment = (float)forcePerSegment_Hnd.asDouble();

	MDataHandle turnForce_Hnd          = block.inputValue(turnForce);
	featherData->_turnForce = (float)turnForce_Hnd.asDouble();

	MDataHandle rotateLamda_Hnd          = block.inputValue(rotateLamda);
	featherData->_rotateLamda = (float)rotateLamda_Hnd.asDouble();

	MDataHandle rachisNoiseFrequency_Hnd          = block.inputValue(rachisNoiseFrequency);
	featherData->_rachisNoiseFrequency = (float)rachisNoiseFrequency_Hnd.asDouble();

	MDataHandle barbuleNoiseFrequency_Hnd          = block.inputValue(barbuleNoiseFrequency);
	featherData->_barbuleNoiseFrequency = (float)barbuleNoiseFrequency_Hnd.asDouble();

	MDataHandle noisePhase_Hnd          = block.inputValue(noisePhase);
	double2& _noisePhase = noisePhase_Hnd.asDouble2();
	featherData->_noisePhase[0]	  = (float)_noisePhase[0];
	featherData->_noisePhase[1]	  = (float)_noisePhase[1];

	MDataHandle meshUVScale_Hnd       = block.inputValue(meshUVScale);
	double2& _meshUVScale = meshUVScale_Hnd.asDouble2();
	featherData->_meshUVScale[0]	  = (float)_meshUVScale[0];
	featherData->_meshUVScale[1]	  = (float)_meshUVScale[1];

	MDataHandle upDownLamda_Hnd          = block.inputValue(upDownLamda);
	featherData->_upDownLamda = (float)upDownLamda_Hnd.asDouble();

	MDataHandle gapForce_Hnd          = block.inputValue(gapForce);
	double2& gapTemp1=gapForce_Hnd.asDouble2();
	featherData->_gapForce[0] = (float)gapTemp1[0];
	featherData->_gapForce[1] = (float)gapTemp1[1];

	MDataHandle gapMaxForce_Hnd          = block.inputValue(gapMaxForce);
	double2& gapTemp2 = gapMaxForce_Hnd.asDouble2();
	featherData->_gapMaxForce[0] = (float)gapTemp2[0];
	featherData->_gapMaxForce[1] = (float)gapTemp2[1];

	MDataHandle gapSize_Hnd          = block.inputValue(gapSize);
	double2& gapTemp3 = gapSize_Hnd.asDouble2();
	featherData->_gapSize[0] = (float)gapTemp3[0];
	featherData->_gapSize[1] = (float)gapTemp3[1];


	MDataHandle shapeSymmetry_Hnd       = block.inputValue(shapeSymmetry);
	featherData->_shapeSymmetry = shapeSymmetry_Hnd.asBool();

	MDataHandle barbuleSegments_Hnd          = block.inputValue(barbuleSegments);
	featherData->_barbuleSegments = barbuleSegments_Hnd.asInt()+1;

	MDataHandle barbuleLength_Hnd          = block.inputValue(barbuleLength);
	featherData->_barbuleLength = (float)barbuleLength_Hnd.asDouble();

	MDataHandle barbuleThick_Hnd          = block.inputValue(barbuleThick);
	featherData->_barbuleThick = (float)barbuleThick_Hnd.asDouble();

	MDataHandle barbuleLengthRandScale_Hnd          = block.inputValue(barbuleLengthRandScale);
	featherData->_barbuleLengthRandScale = (float)barbuleLengthRandScale_Hnd.asDouble()+0.001f;

	MDataHandle rootColor_Hnd          = block.inputValue(rootColor);
	MFloatVector rootC = rootColor_Hnd.asFloatVector();
	featherData->_shader._rootColor = MColor(rootC.x,rootC.y,rootC.z);

	MDataHandle tipColor_Hnd          = block.inputValue(tipColor);
	MFloatVector tipC = tipColor_Hnd.asFloatVector();
	featherData->_shader._tipColor = MColor(tipC.x,tipC.y,tipC.z);

	MDataHandle rachisRootColor_Hnd          = block.inputValue(rachisRootColor);
	MFloatVector rachisRc = rachisRootColor_Hnd.asFloatVector();
	featherData->_shader._rachisRootColor = MColor(rachisRc.x,rachisRc.y,rachisRc.z);

	MDataHandle rachisTipColor_Hnd          = block.inputValue(rachisTipColor);
	MFloatVector rachisTc = rachisTipColor_Hnd.asFloatVector();
	featherData->_shader._rachisTipColor = MColor(rachisTc.x,rachisTc.y,rachisTc.z);

	MDataHandle uvProjectScale_Hnd          = block.inputValue(uvProjectScale);
	featherData->_shader._uvProjectScale = (float)uvProjectScale_Hnd.asDouble();

	MDataHandle randSeed_Hnd          = block.inputValue(randSeed);
	featherData->_randSeed = randSeed_Hnd.asInt();

	MDataHandle displayAs_Hnd          = block.inputValue(displayAs);
	short _displayAs = displayAs_Hnd.asShort();

	MDataHandle exactFeather_Hnd          = block.inputValue(exactFeather);
	featherData->_exactFeather = exactFeather_Hnd.asBool();

	MDataHandle proxyBoundingBoxScale_Hnd     = block.inputValue(proxyBoundingBoxScale);
	double3& _proxyBoundingBoxScale = proxyBoundingBoxScale_Hnd.asDouble3();
	featherData->_proxyBoundingBoxScale[0]	  = (float)_proxyBoundingBoxScale[0];
	featherData->_proxyBoundingBoxScale[1]	  = (float)_proxyBoundingBoxScale[1];
	featherData->_proxyBoundingBoxScale[2]	  = (float)_proxyBoundingBoxScale[2];


	MDataHandle upDownNoise_Hnd          = block.inputValue(upDownNoise);
	featherData->_upDownNoise = upDownNoise_Hnd.asBool();

	featherData->_keyBarAttrs.clear();

	MDataHandle keyBarbulePos1_Hnd          = block.inputValue(keyBarbulePos1);
	featherData->_keyBarAttrs.barbule.push_back( (float)keyBarbulePos1_Hnd.asDouble() );
	MDataHandle keyBarbulePos2_Hnd          = block.inputValue(keyBarbulePos2);
	featherData->_keyBarAttrs.barbule.push_back( (float)keyBarbulePos2_Hnd.asDouble() );
	MDataHandle keyBarbulePos3_Hnd          = block.inputValue(keyBarbulePos3);
	featherData->_keyBarAttrs.barbule.push_back( (float)keyBarbulePos3_Hnd.asDouble() );
	MDataHandle keyBarbulePos4_Hnd          = block.inputValue(keyBarbulePos4);
	featherData->_keyBarAttrs.barbule.push_back( (float)keyBarbulePos4_Hnd.asDouble() );

	MDataHandle keyBarbuleStartAngle1_Hnd          = block.inputValue(keyBarbuleStartAngle1);
	featherData->_keyBarAttrs.startAngle.push_back( (float)keyBarbuleStartAngle1_Hnd.asDouble() );
	MDataHandle keyBarbuleStartAngle2_Hnd          = block.inputValue(keyBarbuleStartAngle2);
	featherData->_keyBarAttrs.startAngle.push_back( (float)keyBarbuleStartAngle2_Hnd.asDouble() );
	MDataHandle keyBarbuleStartAngle3_Hnd          = block.inputValue(keyBarbuleStartAngle3);
	featherData->_keyBarAttrs.startAngle.push_back( (float)keyBarbuleStartAngle3_Hnd.asDouble() );
	MDataHandle keyBarbuleStartAngle4_Hnd          = block.inputValue(keyBarbuleStartAngle4);
	featherData->_keyBarAttrs.startAngle.push_back( (float)keyBarbuleStartAngle4_Hnd.asDouble() );

	MDataHandle keyBarbuleStepAngle_Hnd          = block.inputValue(keyBarbuleStepAngle);
	featherData->_keyBarAttrs.stepAngle = (float)keyBarbuleStepAngle_Hnd.asDouble();

	//�õ�rampAttribute�еĸ�ֵ
	MObject me = thisMObject();

	//get key barbule ramps
	MRampAttribute rmp1=MRampAttribute(me,kbRotate1,&status);
	MRampAttribute rmp2=MRampAttribute(me,kbRotate2,&status);
	MRampAttribute rmp3=MRampAttribute(me,kbRotate3,&status);
	MRampAttribute rmp4=MRampAttribute(me,kbRotate4,&status);
	featherData->_keyBarAttrs.pushRamp( rmp1,rmp2,rmp3,rmp4);
	
	featherData->_barbuleUpDownScale = MRampAttribute(me,barbuleUpDownScale,&status);if(!status) return false;
	featherData->_barbuleUpDownRachis = MRampAttribute(me,barbuleUpDownRachis,&status);if(!status) return false;
	featherData->_rachisThickScale = MRampAttribute(me,rachisThickScale,&status);if(!status) return false;
	featherData->_barbuleThickScale = MRampAttribute(me,barbuleThickScale,&status);if(!status) return false;
	featherData->_barbuleThickAdjust = MRampAttribute(me,barbuleThickAdjust,&status);if(!status) return false;
	featherData->_barbuleDensity = MRampAttribute(me,barbuleDensity,&status);if(!status) return false;
	featherData->_gapDensity[0] = MRampAttribute(me,leftGapDensity,&status);if(!status) return false;
	featherData->_gapDensity[1] = MRampAttribute(me,rightGapDensity,&status);if(!status) return false;
	
	featherData->_forceRotateScale = MRampAttribute(me,forceRotateScale,&status);if(!status) return false;
	featherData->_forceScale = MRampAttribute(me,forceScale,&status);if(!status) return false;
	featherData->_leftBarbuleLengthScale = MRampAttribute(me,leftBarbuleLengthScale,&status);if(!status) return false;

	if( featherData->_shapeSymmetry )
		featherData->_rightBarbuleLengthScale = MRampAttribute(me,leftBarbuleLengthScale,&status);
	else
	{
		featherData->_rightBarbuleLengthScale = MRampAttribute(me,rightBarbuleLengthScale,&status);
		if(!status) return false;
	}


	//-------------------------declare some useful variables
	//-------------------------
	//-------------------------
	featherG->assignFeatherInfo(featherData);//�洢���ڵ��featherInfo��Ϣ
	featherG->_feaExpInf._onlyGetBBox=false;
	featherDraw.clear();
	singleFeather sgf;

	MObject newObj = MObject::kNullObj;
	turtles *tempTurtle=NULL;
	Node* tempNodePt=NULL;
	jcU.clear();
	jcV.clear();
	//-----


	//-------׼��UV�ĸ���
	std::map<int,long>::iterator iter;
	//-------------------------��surface ��ȡuv ���
	MArrayDataHandle    guidSurfaceInfo_Hnd = block.inputArrayValue(guidSurfaceInfo);
	for( iter = surfaceIndexIDMap.begin();iter!=surfaceIndexIDMap.end();++iter)
	{
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;

		guidSurfaceInfo_Hnd.jumpToElement(iter->first);
		float2 &uvg = guidSurfaceInfo_Hnd.inputValue().child(guidUV).asFloat2();
		jcU.append(uvg[0]);
		jcV.append(uvg[1]);
	}

	//-----------------------��inTurtleData ��ȡuv���
	MArrayDataHandle    inFeatherTurtleData_Hnd = block.inputArrayValue(inFeatherTurtleData);

	featherTurtleData* currentTurtleD=NULL;
	for(iter = inTurtleIDMap.begin();iter != inTurtleIDMap.end();++iter)
	{
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;
		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);
		tempTurtle->clearKeyTurtle();

		inFeatherTurtleData_Hnd.jumpToElement(iter->first);
		currentTurtleD = (featherTurtleData*)(inFeatherTurtleData_Hnd.inputValue().asPluginData());
		if(!currentTurtleD)continue;
		//----�õ�turtle�Լ�����growid
        if(!featherTools::getTurtles(currentTurtleD->m_turtleValue,tempTurtle)) continue;

		jcU.append(currentTurtleD->m_turtleValue._featherUV[0]);
		jcV.append(currentTurtleD->m_turtleValue._featherUV[1]);
	}

	//-----------------------------��inTurtleArrayData ������ݸ���
	MArrayDataHandle inFeatherTurtleArrayData_Hnd = block.inputArrayValue(inFeatherTurtleArrayData);
	int inTurtArrayCount = inFeatherTurtleArrayData_Hnd.elementCount();

	featherTurtleArrayData* inTurtleArrayPxData=NULL;
	std::map<std::pair<int,int>,long>::iterator pairIter;
	for ( pairIter = inTurtleArrayIDMap.begin(); pairIter != inTurtleArrayIDMap.end(); pairIter++ )
		removeTurtlesDataBaseKey(&(pairIter->second));
	inTurtleArrayIDMap.clear();

	std::pair<int,int> tempPair(-1,-1);
	int intapdSize=0;
	for(int kk=0;kk<inTurtArrayCount;kk++)
	{
		inFeatherTurtleArrayData_Hnd.jumpToArrayElement(kk);
		inTurtleArrayPxData = (featherTurtleArrayData*)inFeatherTurtleArrayData_Hnd.inputValue().asPluginData();
		if(!inTurtleArrayPxData) continue;

		intapdSize = (int)inTurtleArrayPxData->m_turtleArray.size();

		for(int jj=0;jj < intapdSize;++jj)
		{
			long *newID = new long;
			*newID = turtlesDataBase->Get_FreeID();

			tempPair.first = kk;tempPair.second = jj;
			inTurtleArrayIDMap[tempPair] = *newID;

			featherData->_turtleChildrenID.push_back(*newID);
			turtles *newTurs = new turtles;
			turtlesDataBase->Add(newID,newTurs);

			//----�õ�turtle�Լ�����growid
			if(!featherTools::getTurtles(inTurtleArrayPxData->m_turtleArray[jj],newTurs))	continue;

			jcU.append( inTurtleArrayPxData->m_turtleArray[jj]._featherUV[0] );
			jcV.append( inTurtleArrayPxData->m_turtleArray[jj]._featherUV[1] );
		}
	}
	if( block.inputValue(interactiveTexture).asBool())
		updateJCFTexture();

	//---��ȡ��ͼ��ɫ������feather��mainColor
	/*MPlugArray plugA;
	MString texName("");
	
	MPlug texPlug(me,mainColor);
	if(texPlug.connectedTo(plugA,1,0))
	{
		texName=plugA[0].name();
		if( featherTools::sampleTexture(texName,&jcU,&jcV,jcMainColor,jcMainTrans))
		{
			unsigned int ulen=jcU.length();
			for(unsigned int l=0;l<ulen;l++)
				mainColorArray.append(jcMainColor[l].x,jcMainColor[l].y,jcMainColor[l].z);
		}
	}*/

	//-----------------------surface feather����nurbssurface�ϵ���ë
	unsigned int ii=0;
	unsigned int mcl=jcU.length();
	unsigned int mcolorlen=mainColorArray.length();
	for( iter = surfaceIndexIDMap.begin();iter!=surfaceIndexIDMap.end();++iter,++ii)
	{
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;
		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);

		guidSurfaceInfo_Hnd.jumpToElement(iter->first);
		newObj = guidSurfaceInfo_Hnd.inputValue().child(guidSurface).asNurbsSurface();
		if(!newObj.hasFn(MFn::kNurbsSurface)) continue;

		sgf.clear();
		if(ii<mcl)
		{
			sgf.surfaceUV[0]=jcU[ii];
			sgf.surfaceUV[1]=jcV[ii];
		}
		if(ii<mcolorlen)sgf.mainColor = mainColorArray[ii];

		if(_displayAs==0)//��ʾΪ��ʵ����ë������ë����barbule
		{
			if( featherData->_exactFeather )
				featherG->getJCFeather( newObj,sgf, ii);
			else
			{
				featherG->getTurtles(newObj,tempTurtle);
				featherG->getTurtleJCFeather( *tempTurtle,sgf, ii);
			}
			featherDraw.push_back(sgf);	//����ë��ݴ���
		}
		else if(_displayAs==1)
		{
			if(featherData->_exactFeather)//��ȡ��ë����barbule
				featherG->getKeyBarbule( newObj,sgf );
			else
			{
				featherG->getTurtles(newObj,tempTurtle);
				featherG->getKeyBarbule(*tempTurtle,sgf );
			}
			featherDraw.push_back(sgf);	//����ë��ݴ���
		}
		
	}

	//------------------------guidMesh feather�������ε��ϵ���ë
	for(iter = inTurtleIDMap.begin();iter != inTurtleIDMap.end();++iter,++ii)
	{
		//---��ʼ��
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;
		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);

		sgf.clear();
		if(ii<mcl)
		{
			sgf.surfaceUV[0] = jcU[ii];
			sgf.surfaceUV[1] = jcV[ii];
		}
		if(ii<mcolorlen) sgf.mainColor = mainColorArray[ii];

		if(_displayAs==0)
		{
			featherG->getJCFeather( *tempTurtle,sgf, ii);
			featherDraw.push_back(sgf);
		}
		else if(_displayAs==1)
		{
			if(!tempTurtle->_exMeshFea)
				featherG->getKeyBarbule(*tempTurtle,sgf );
			else
				featherG->getExKeyBarbule(*tempTurtle,sgf );
			featherDraw.push_back(sgf);
		}
	}

	//---------------------------------��inTurtleArrayData����ë���м���
	for(pairIter = inTurtleArrayIDMap.begin();pairIter != inTurtleArrayIDMap.end();++pairIter,++ii)
	{
		//---��ʼ��
		tempNodePt = turtlesDataBase->Find( &pairIter->second );
		if(!tempNodePt) continue;
		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);

		sgf.clear();
		if(ii<mcl)
		{
			sgf.surfaceUV[0] = jcU[ii];
			sgf.surfaceUV[1] = jcV[ii];
		}
		if(ii<mcolorlen) sgf.mainColor = mainColorArray[ii];

		if(_displayAs==0)
		{
			featherG->getJCFeather( *tempTurtle,sgf, ii);
			featherDraw.push_back(sgf);
		}
		else if(_displayAs==1)
		{
			if(!tempTurtle->_exMeshFea)
				featherG->getKeyBarbule(*tempTurtle,sgf );
			else
				featherG->getExKeyBarbule(*tempTurtle,sgf );
			featherDraw.push_back(sgf);
		}
	}
	return true;
}

MStatus jcFeather::connectionMade( const MPlug & plug, const MPlug & otherPlug, bool asSrc )
{
	MStatus status;
	MFnDependencyNode dgNodeFn;
	dgNodeFn.setObject(otherPlug.node());

	if( plug == jcFeather::guidSurface &&
		otherPlug.asMObject().hasFn(MFn::kNurbsSurface) &&
		asSrc == false
		)//������ӵ���nurbssurface
	{
		addPlugTurtleData(plug,true,surfaceIndexIDMap);
	}
	if( plug == jcFeather::inFeatherTurtleData &&
		dgNodeFn.typeId() == jcFeatherComponent::id &&
		asSrc == false )
	{
		addPlugTurtleData(plug,false,inTurtleIDMap);
	}
	return MPxNode::connectionMade( plug, otherPlug, asSrc );
}

MStatus jcFeather::connectionBroken ( const MPlug & plug, const MPlug & otherPlug, bool asSrc )
{
	MStatus status;
	MFnDependencyNode dgNodeFn;
	dgNodeFn.setObject(otherPlug.node());

	if( otherPlug.asMObject().hasFn(MFn::kNurbsSurface) &&
		plug == jcFeather::guidSurface &&
		asSrc == false
		)
	{
		removePlugTurtleData(plug,true,surfaceIndexIDMap);
	}
	if( plug == jcFeather::inFeatherTurtleData &&
		dgNodeFn.typeId() == jcFeatherComponent::id &&
		asSrc == false )
	{
		removePlugTurtleData(plug,false,inTurtleIDMap);
	}

	return MPxNode::connectionBroken( plug, otherPlug, asSrc );
}

void	jcFeather::addPlugTurtleData(const MPlug &plug,bool surface,std::map<int,long> &indexMap)
{
		long *newID = new long;
		*newID = turtlesDataBase->Get_FreeID();
		int plugIndex=-1;
		if(surface)
			plugIndex = plug.parent().logicalIndex();
		else
			plugIndex = plug.logicalIndex();

		indexMap[plugIndex] = *newID;
		featherData->_turtleChildrenID.push_back(*newID);

		turtles *newTurs = new turtles;
		turtlesDataBase->Add(newID,newTurs);
}

void	jcFeather::removePlugTurtleData(const MPlug &plug,bool surface,std::map<int,long> &indexMap)
{
	int plugIndex=-1;
	if(surface)
		plugIndex = plug.parent().logicalIndex();
	else
		plugIndex = plug.logicalIndex();

	featherData->_turtleChildrenID.remove( indexMap[plugIndex] );
	removeTurtlesDataBaseKey(&indexMap[plugIndex] );
	indexMap.erase(plugIndex);
}

void	jcFeather::removeTurtlesDataBaseKey(long* id)
{
	turtlesDataBase->Remove(id);
	Node *nodeRemoved=turtlesDataBase->GetFreeList();
	if( nodeRemoved != NULL )
	{
		if(nodeRemoved->Key != NULL)
		{
			delete reinterpret_cast<long*>(nodeRemoved->Key);
			nodeRemoved->Key=NULL;
		}
		if(nodeRemoved->Value != NULL)
		{
			delete reinterpret_cast<turtles*>(nodeRemoved->Value);
			nodeRemoved->Value=NULL;
		}
	}
}

void	jcFeather::removeFeaInfoDataBase(long* id)
{
	feaInfoDataBase->Remove(id);
	Node *nodeRemoved=feaInfoDataBase->GetFreeList();
	if( nodeRemoved != NULL )
	{
		if(nodeRemoved->Key != NULL)
		{
			delete reinterpret_cast<long*>(nodeRemoved->Key);
			nodeRemoved->Key=NULL;
		}
		if(nodeRemoved->Value != NULL)
		{
			delete reinterpret_cast<featherInfo*>(nodeRemoved->Value);
			nodeRemoved->Value=NULL;
		}
	}
}

void	jcFeather::updateJCFTexture()
{
	MPlugArray plugA;
	MString texName("");
	MFloatVectorArray jcMainColor,jcMainTrans;
	MObject me = thisMObject();
	MPlug texPlug(me,mainColor);
	int featherSize =featherDraw.size();
	mainColorArray.clear();
	bool texOk=false;
	
	if(texPlug.connectedTo(plugA,1,0))
	{
		texName=plugA[0].name();
		if( featherTools::sampleTexture(texName,&jcU,&jcV,jcMainColor,jcMainTrans))
		{
			texOk =true;
			unsigned int ulen=jcU.length();
			for(unsigned int l=0;l<ulen;l++)
				mainColorArray.append(jcMainColor[l].x,jcMainColor[l].y,jcMainColor[l].z);
			if(ulen==featherSize)
			{
				for(int ii=0;ii<ulen;++ii)
					featherDraw[ii].mainColor = mainColorArray[ii];
			}
		}
	}

	if(!texOk)
	{
		for(int ii=0;ii<featherSize;++ii)
			featherDraw[ii].mainColor = MColor(1,1,1);
	}
}
