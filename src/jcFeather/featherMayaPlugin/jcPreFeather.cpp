#include "jcPreFeather.h"

#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MAngle.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnPluginData.h>
MTypeId  jcPreFeather::id(0x81033);
MString  jcPreFeather::typeName("jcPreFeather");

MObject  jcPreFeather::active;

MObject  jcPreFeather::guideFileName; 
MObject  jcPreFeather::scatterFileName; 

MObject  jcPreFeather::inScatterData;
MObject  jcPreFeather::inVertexDir; 

MObject  jcPreFeather::globalScale; 
MObject  jcPreFeather::randScale; 
MObject  jcPreFeather::rotateOrder; 
MObject  jcPreFeather::rotateOffset; 
MObject  jcPreFeather::useRotateNoise;
MObject  jcPreFeather::rotateNoiseFrequency;
MObject  jcPreFeather::rotateNoiseAmplitude;
MObject  jcPreFeather::rotateNoisePhase;

MObject  jcPreFeather::bendActive;
MObject  jcPreFeather::bendPosition;
MObject  jcPreFeather::bendCurvature;
MObject  jcPreFeather::bendRandCurvature;

MObject  jcPreFeather::randSeed; 
MObject  jcPreFeather::uvSetName; 
MObject  jcPreFeather::interactiveScale; 
MObject  jcPreFeather::scaleTexture; 

MObject  jcPreFeather::paintRotateX;
MObject  jcPreFeather::paintRotateY;
MObject  jcPreFeather::paintRotateZ;
MObject  jcPreFeather::paintScaleX;
MObject  jcPreFeather::paintScaleY;
MObject  jcPreFeather::paintScaleZ;
MObject  jcPreFeather::paintBendCurvature;

MObject  jcPreFeather::controlInfo;
MObject  jcPreFeather::controlRotate;
MObject  jcPreFeather::controlRotateX;
MObject  jcPreFeather::controlRotateY;
MObject  jcPreFeather::controlRotateZ;
MObject  jcPreFeather::controlScale;
MObject  jcPreFeather::controlScaleX;
MObject  jcPreFeather::controlScaleY;
MObject  jcPreFeather::controlScaleZ;
MObject  jcPreFeather::controlBend;
MObject  jcPreFeather::controlWeightMap;

MObject  jcPreFeather::inGrowMesh; 
MObject  jcPreFeather::instanceMesh; 
MObject  jcPreFeather::instanceFeatherTurtle; 
MObject  jcPreFeather::outFeatherMesh; 
MObject  jcPreFeather::outFeatherTurtleArray; 

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

extern int jcFea_FreeVersionMaxNodesNum;
extern int jcFea_FreeVersionJCPreFeatherNodeIndex;

jcPreFeather::jcPreFeather()
{
	jcFea_FreeVersionJCPreFeatherNodeIndex+=1;
}

jcPreFeather::~jcPreFeather()
{}
void  jcPreFeather::postConstructor()
{
	useVerDirFile=false;
	useScatterFile=false;
}
void * jcPreFeather::creator()
{
	if(jcFea_FreeVersionJCPreFeatherNodeIndex < jcFea_FreeVersionMaxNodesNum)
		return new jcPreFeather();
	else
	{
		//jcFea_FreeVersionJCPreFeatherNodeIndex-=1;
		//if(jcFea_FreeVersionJCPreFeatherNodeIndex<0)
		//	jcFea_FreeVersionJCPreFeatherNodeIndex=0;

		MString str="JCFeather Free : With free version, you can only create ";
		str +=jcFea_FreeVersionMaxNodesNum;
		str +=" jcPreFeather nodes.";
		MGlobal::displayError(str);
		return NULL;
	}
}
void   jcPreFeather::nodeRemoved(MObject& node, void *clientData)
{
	//MFnDependencyNode fnNode(node);

	//static_cast<jcPreFeather*>(fnNode.userNode())->preFeatherInfo.init();

	jcFea_FreeVersionJCPreFeatherNodeIndex-=1;
	if(jcFea_FreeVersionJCPreFeatherNodeIndex<0)
		jcFea_FreeVersionJCPreFeatherNodeIndex=0;
}

MStatus jcPreFeather::initialize()
{
	MStatus status=MS::kSuccess;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnEnumAttribute enumAttr;

	rotateOrder = enumAttr.create("rotateOrder","rotod",5);
	enumAttr.addField("xyz",0);
	enumAttr.addField("yzx",1);
	enumAttr.addField("zxy",2);
	enumAttr.addField("xzy",3);
	enumAttr.addField("yxz",4);
	enumAttr.addField("zyx",5);
	MAKE_INPUT(enumAttr,true);

	MFnNumericAttribute nAttr;
	active = nAttr.create( "active", "active", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);
	interactiveScale= nAttr.create( "interactiveScale", "intsca", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);
	globalScale = nAttr.create( "globalScale", "glsc", MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	randScale = nAttr.create( "randScale", "rasc", MFnNumericData::kFloat,0.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);

	rotateOffset = nAttr.create( "rotateOffset", "rooff", MFnNumericData::k3Float);
	MAKE_INPUT(nAttr,true);
	nAttr.setDefault(0,0,10);
	useRotateNoise = nAttr.create( "useRotateNoise", "urotn", MFnNumericData::kBoolean,false);
	MAKE_INPUT(nAttr,true);
	rotateNoiseFrequency = nAttr.create( "rotateNoiseFrequency", "rotnf", MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	rotateNoiseAmplitude = nAttr.create( "rotateNoiseAmplitude", "rotna", MFnNumericData::k3Float,1.0f);
	MAKE_INPUT(nAttr,true);
	nAttr.setDefault(0,0,5);
	rotateNoisePhase = nAttr.create( "rotateNoisePhase", "rotnph", MFnNumericData::k3Float,0.0f);
	MAKE_INPUT(nAttr,true);

	randSeed = nAttr.create( "randSeed", "raseed", MFnNumericData::kInt,1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	scaleTexture =nAttr.createColor( "scaleTexture", "scaleTex");
	nAttr.setDefault(1,1,1);
	MAKE_INPUT(nAttr,false);


	//------------------controls attrs
	controlBend = nAttr.create("controlBend", "conbnd",MFnNumericData::kFloat,0);
	MAKE_INPUT(nAttr,true);
	CHECK_MSTATUS( nAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	controlScaleX = nAttr.create("controlScaleX", "conscax",MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);
	CHECK_MSTATUS( nAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	controlScaleY = nAttr.create("controlScaleY", "conscay",MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);
	CHECK_MSTATUS( nAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	controlScaleZ = nAttr.create("controlScaleZ", "conscaz",MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);
	CHECK_MSTATUS( nAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	MFnCompoundAttribute compoundAttr;
	controlScale = compoundAttr.create("controlScale","consca");
	compoundAttr.addChild(controlScaleX);
	compoundAttr.addChild(controlScaleY);
	compoundAttr.addChild(controlScaleZ);
	compoundAttr.setHidden(true);
	CHECK_MSTATUS( compoundAttr.setDisconnectBehavior(MFnAttribute::kReset) );

	bendActive = nAttr.create( "activeBend", "actb", MFnNumericData::kBoolean,false);
	MAKE_INPUT(nAttr,true);
	bendPosition = nAttr.create("bendPosition", "bdpos",MFnNumericData::kFloat,0.5);
	MAKE_INPUT(nAttr,true); nAttr.setMin(0); nAttr.setMax(1);
	bendCurvature = nAttr.create("bendCurvature", "bdcur",MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,true);
	bendRandCurvature = nAttr.create("bendRandCurvature", "dbrdcur",MFnNumericData::kFloat,0);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	

	MFnUnitAttribute unitAttr;
	controlRotateX = unitAttr.create("controlRotateX", "conrotx",MFnUnitAttribute::kAngle,0.0f);
	MAKE_INPUT(unitAttr,true);
	CHECK_MSTATUS( unitAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	controlRotateY = unitAttr.create("controlRotateY", "conroty",MFnUnitAttribute::kAngle,0.0f);
	MAKE_INPUT(unitAttr,true);
	CHECK_MSTATUS( unitAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	controlRotateZ = unitAttr.create("controlRotateZ", "conrotz",MFnUnitAttribute::kAngle,0.0f);
	MAKE_INPUT(unitAttr,true);
	CHECK_MSTATUS( unitAttr.setDisconnectBehavior(MFnAttribute::kReset) );


	controlRotate = compoundAttr.create("controlRotate","conrot");
	compoundAttr.addChild(controlRotateX);
	compoundAttr.addChild(controlRotateY);
	compoundAttr.addChild(controlRotateZ);
	compoundAttr.setHidden(true);
	CHECK_MSTATUS( compoundAttr.setDisconnectBehavior(MFnAttribute::kReset) );

	MFnStringData strData;
	MFnTypedAttribute typAttr;
	MObject tempObj = strData.create("");
	controlWeightMap = typAttr.create( "controlWeightMap", "conwema", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kReset) );

	controlInfo = compoundAttr.create("controlInfo","coninfo");
	compoundAttr.addChild(controlRotate);
	compoundAttr.addChild(controlScale);
	compoundAttr.addChild(controlBend);
	compoundAttr.addChild(controlWeightMap);
	compoundAttr.setArray(true);
	compoundAttr.setHidden(true);
	CHECK_MSTATUS( compoundAttr.setDisconnectBehavior(MFnAttribute::kDelete) );


    MDoubleArray defaultDoubArray;
    MFnDoubleArrayData doubArrayDataFn;
    doubArrayDataFn.create( defaultDoubArray );
    paintRotateX = typAttr.create( "paintRoll",
									"pntrol",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	CHECK_MSTATUS( typAttr.setHidden(true));

	paintRotateY = typAttr.create( "paintYaw",
									"pntyaw",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	paintRotateZ = typAttr.create( "paintPitch",
									"pntpit",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	//-----------------
	paintScaleX = typAttr.create( "paintLength",
									"pntlen",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	paintScaleY = typAttr.create( "paintThick",
									"pntthick",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	paintScaleZ = typAttr.create( "paintWidth",
									"pntwd",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	paintBendCurvature = typAttr.create( "paintBendCurvature",
									"pntbcur",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	//----------------------------
	inGrowMesh = typAttr.create("inGrowMesh","ingm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior(MFnAttribute::kReset);

	instanceMesh = typAttr.create("instanceMesh","insm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior(MFnAttribute::kReset);

	instanceFeatherTurtle = typAttr.create("instanceFeatherTurtle",
										  "infeat",
										    featherTurtleData::id,
											MObject::kNullObj,
											&status);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kReset) );


	outFeatherTurtleArray = typAttr.create("outFeatherTurtleDataArray",
										  "otfeatda",
										    featherTurtleArrayData::id,
											MObject::kNullObj,
											&status);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	outFeatherMesh = typAttr.create("outFeatherMesh","ofm",MFnMeshData::kMesh);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	tempObj = strData.create("map1");
	uvSetName = typAttr.create( "uvSetName", "uvsn", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	inScatterData = typAttr.create("inScatterData",
									"insd",
									scatterPointData::id,
									MObject::kNullObj,
									&status);
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);

	inVertexDir = typAttr.create("inVertexDir",
									"invd",
									vertexDirectionData::id,
									MObject::kNullObj,
									&status);
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);

	tempObj = strData.create("");
	guideFileName = typAttr.create( "guideFile", "gdf", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	tempObj = strData.create("");
	scatterFileName = typAttr.create( "scatterFile", "scatf", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);
	
	addMayaAttributes();

	return status;
}

void jcPreFeather::addMayaAttributes()
{
	CHECK_MSTATUS( addAttribute(active) );
	CHECK_MSTATUS( addAttribute(guideFileName) );
	CHECK_MSTATUS( addAttribute(scatterFileName) );

	CHECK_MSTATUS( addAttribute(globalScale) );
	CHECK_MSTATUS( addAttribute(randScale) );
	CHECK_MSTATUS( addAttribute(rotateOrder) );
	CHECK_MSTATUS( addAttribute(rotateOffset) );
	CHECK_MSTATUS( addAttribute(useRotateNoise) );
	CHECK_MSTATUS( addAttribute(rotateNoiseFrequency) );
	CHECK_MSTATUS( addAttribute(rotateNoiseAmplitude) );
	CHECK_MSTATUS( addAttribute(rotateNoisePhase) );

	CHECK_MSTATUS( addAttribute(bendActive) );
	CHECK_MSTATUS( addAttribute(bendPosition) );
	CHECK_MSTATUS( addAttribute(bendCurvature) );
	CHECK_MSTATUS( addAttribute(bendRandCurvature) );
	CHECK_MSTATUS( addAttribute(paintBendCurvature) );
	CHECK_MSTATUS( addAttribute(controlInfo) );

	CHECK_MSTATUS( addAttribute(randSeed) );
	CHECK_MSTATUS( addAttribute(uvSetName) );
	CHECK_MSTATUS( addAttribute(interactiveScale) );
	CHECK_MSTATUS( addAttribute(scaleTexture) );

	CHECK_MSTATUS( addAttribute(paintRotateX) );
	CHECK_MSTATUS( addAttribute(paintRotateY) );
	CHECK_MSTATUS( addAttribute(paintRotateZ) );
	CHECK_MSTATUS( addAttribute(paintScaleX) );
	CHECK_MSTATUS( addAttribute(paintScaleY) );
	CHECK_MSTATUS( addAttribute(paintScaleZ) );

	CHECK_MSTATUS( addAttribute(inGrowMesh) );
	CHECK_MSTATUS( addAttribute(instanceMesh) );
	CHECK_MSTATUS( addAttribute(inScatterData) );
	CHECK_MSTATUS( addAttribute(inVertexDir) );
	CHECK_MSTATUS( addAttribute(outFeatherMesh) );
	CHECK_MSTATUS( addAttribute(instanceFeatherTurtle) );
	CHECK_MSTATUS( addAttribute(outFeatherTurtleArray) );

	CHECK_MSTATUS( attributeAffects( active, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( guideFileName, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( scatterFileName, outFeatherMesh ) );
	
	CHECK_MSTATUS( attributeAffects( globalScale, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( randScale, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( instanceMesh, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( rotateOrder, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( useRotateNoise, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( rotateNoiseFrequency, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( rotateNoiseAmplitude, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( rotateNoisePhase, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( bendActive, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( bendPosition, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( bendCurvature, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( bendRandCurvature, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( paintBendCurvature, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( rotateOffset, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( randSeed, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( scaleTexture, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( interactiveScale, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( paintRotateX, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( paintRotateY, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( paintRotateZ, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( paintScaleX, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( paintScaleY, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( paintScaleZ, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( inScatterData, outFeatherMesh ) );
	CHECK_MSTATUS( attributeAffects( inVertexDir, outFeatherMesh ) );

	CHECK_MSTATUS( attributeAffects( controlInfo, outFeatherMesh ) );


	CHECK_MSTATUS( attributeAffects( active, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( guideFileName, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( scatterFileName, outFeatherTurtleArray ) );
	
	CHECK_MSTATUS( attributeAffects( globalScale, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( randScale, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( instanceFeatherTurtle, outFeatherTurtleArray ) );
	
	CHECK_MSTATUS( attributeAffects( rotateOrder, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( useRotateNoise, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( rotateNoiseFrequency, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( rotateNoiseAmplitude, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( rotateNoisePhase, outFeatherTurtleArray ) );

	CHECK_MSTATUS( attributeAffects( rotateOffset, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( randSeed, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( scaleTexture, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( interactiveScale, outFeatherTurtleArray ) );

	CHECK_MSTATUS( attributeAffects( paintRotateX, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( paintRotateY, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( paintRotateZ, outFeatherTurtleArray ) );

	CHECK_MSTATUS( attributeAffects( paintScaleX, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( paintScaleY, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( paintScaleZ, outFeatherTurtleArray ) );

	CHECK_MSTATUS( attributeAffects( inScatterData, outFeatherTurtleArray ) );
	CHECK_MSTATUS( attributeAffects( inVertexDir, outFeatherTurtleArray ) );

	CHECK_MSTATUS( attributeAffects( controlInfo, outFeatherTurtleArray ) );
}

MStatus jcPreFeather::initPreFeatherInfo(MDataBlock& block)
{
	MStatus status = MS::kSuccess;
	preFeatherInfo.init();//in this init function preFeatherInfo._texScaleData will not be cleared

	MDataHandle interactiveScale_Hnd   = block.inputValue(interactiveScale);
	if(interactiveScale_Hnd.asBool()) preFeatherInfo._texScaleData.clear();


	MDataHandle globalScale_Hnd   = block.inputValue(globalScale);
	preFeatherInfo._globalScale = globalScale_Hnd.asFloat();
	MDataHandle randScale_Hnd   = block.inputValue(randScale);
	preFeatherInfo._randScale = randScale_Hnd.asFloat();
	MDataHandle rotateOffset_Hnd   = block.inputValue(rotateOffset);
	float3& rotoff= rotateOffset_Hnd.asFloat3();
	preFeatherInfo._rotateOffset[0]=rotoff[0]*0.017453292f;
	preFeatherInfo._rotateOffset[1]=rotoff[1]*0.017453292f;
	preFeatherInfo._rotateOffset[2]=rotoff[2]*0.017453292f;

	//----------------------------control solution		
	MObject me = thisMObject();
	MPlug texPlug(me,scaleTexture);
	MPlugArray texConnect;
	texPlug.setAttribute(scaleTexture);
	if(texPlug.connectedTo(texConnect,true,false,&status))
		preFeatherInfo._scaleTexture = texConnect[0].name();

	MArrayDataHandle controlInfo_Hnd = block.inputArrayValue(controlInfo);
	int inControlNum = controlInfo_Hnd.elementCount();

	MPlug conPlug(me,controlInfo);
	MVector tempV;
	for(int ii=0;ii<inControlNum;++ii)
	{
		MPlug conPlugIndex=conPlug.elementByPhysicalIndex( ii, &status );
		if( !conPlugIndex.child(controlWeightMap).connectedTo(texConnect,true,false,&status) )
			continue;

		controlInfo_Hnd.jumpToElement(ii);
		tempV.x = controlInfo_Hnd.inputValue().child(controlRotate).child(controlRotateX).asAngle().asRadians();
		tempV.y = controlInfo_Hnd.inputValue().child(controlRotate).child(controlRotateY).asAngle().asRadians();
		tempV.z = controlInfo_Hnd.inputValue().child(controlRotate).child(controlRotateZ).asAngle().asRadians();
		preFeatherInfo._rotateArray.append(tempV);

		tempV.x = controlInfo_Hnd.inputValue().child(controlScale).child(controlScaleX).asFloat();
		tempV.y = controlInfo_Hnd.inputValue().child(controlScale).child(controlScaleY).asFloat();
		tempV.z = controlInfo_Hnd.inputValue().child(controlScale).child(controlScaleZ).asFloat();
		preFeatherInfo._scaleArray.append(tempV);

		preFeatherInfo._controlBend.append(controlInfo_Hnd.inputValue().child(controlBend).asFloat());

		preFeatherInfo._controlColorMaps.append(controlInfo_Hnd.inputValue().child(controlWeightMap).asString());
	}
	//------------------------------------------

	//------------------------
	MDataHandle bendActive_Hnd   = block.inputValue(bendActive);
	preFeatherInfo._bendActive = bendActive_Hnd.asBool();
	MDataHandle bendPosition_Hnd   = block.inputValue(bendPosition);
	preFeatherInfo._bendPosition = bendPosition_Hnd.asFloat();
	MDataHandle bendCurvature_Hnd   = block.inputValue(bendCurvature);
	preFeatherInfo._bendCurvature = bendCurvature_Hnd.asFloat();
	MDataHandle bendRandCurvature_Hnd   = block.inputValue(bendRandCurvature);
	preFeatherInfo._bendRandomCurvature = bendRandCurvature_Hnd.asFloat();

	MDataHandle useRotateNoise_Hnd   = block.inputValue(useRotateNoise);
	preFeatherInfo._useRotNoise = useRotateNoise_Hnd.asBool();

	MDataHandle rotateNoiseFrequency_Hnd   = block.inputValue(rotateNoiseFrequency);
	preFeatherInfo._rotNoiseFre= rotateNoiseFrequency_Hnd.asFloat();
		
	MDataHandle rotateOrder_Hnd   = block.inputValue(rotateOrder);
	short rotoder=rotateOrder_Hnd.asShort();
	switch(rotoder)
	{
		case 0:
			preFeatherInfo._rotOrder = MTransformationMatrix::kXYZ;
			break;
		case 1:
			preFeatherInfo._rotOrder = MTransformationMatrix::kYZX;
			break;
		case 2:
			preFeatherInfo._rotOrder = MTransformationMatrix::kZXY;
			break;
		case 3:
			preFeatherInfo._rotOrder = MTransformationMatrix::kXZY;
			break;
		case 4:
			preFeatherInfo._rotOrder = MTransformationMatrix::kYXZ;
			break;
		case 5:
			preFeatherInfo._rotOrder = MTransformationMatrix::kZYX;
			break;
		default:
			preFeatherInfo._rotOrder = MTransformationMatrix::kZYX;
			break;
	}

	MDataHandle rotateNoiseAmplitude_Hnd   = block.inputValue(rotateNoiseAmplitude);
	float3& rotna= rotateNoiseAmplitude_Hnd.asFloat3();
	preFeatherInfo._rotNoiseAmp[0]=rotna[0];
	preFeatherInfo._rotNoiseAmp[1]=rotna[1];
	preFeatherInfo._rotNoiseAmp[2]=rotna[2];

	MDataHandle rotateNoisePhase_Hnd   = block.inputValue(rotateNoisePhase);
	float3& rotnp= rotateNoisePhase_Hnd.asFloat3();
	preFeatherInfo._rotNoisePha[0]=rotnp[0];
	preFeatherInfo._rotNoisePha[1]=rotnp[1];
	preFeatherInfo._rotNoisePha[2]=rotnp[2];

	MDataHandle randSeed_Hnd   = block.inputValue(randSeed);
	preFeatherInfo._randSeed = randSeed_Hnd.asInt();

	MDataHandle inGrowMesh_Hnd   = block.inputValue(inGrowMesh);
	preFeatherInfo._inGrownMesh = inGrowMesh_Hnd.asMesh();
	if(!preFeatherInfo._inGrownMesh.hasFn(MFn::kMesh)) return MS::kFailure;

	MDataHandle uvSetName_Hnd   = block.inputValue(uvSetName);
	preFeatherInfo._uvSetName = uvSetName_Hnd.asString();
	
	
	//-------------------
	MDoubleArray dary0,dary1,dary2,dary3,dary4,dary5,dary6;
	int daryLen=0;
	MFnDoubleArrayData dataDoubArrayFn;

	status=MS::kSuccess;
	MDataHandle paintRotateX_Hnd  = block.inputValue(paintRotateX);
	status=dataDoubArrayFn.setObject( paintRotateX_Hnd.data() );
	if( status )		dary0=dataDoubArrayFn.array();

	MDataHandle paintRotateY_Hnd  = block.inputValue(paintRotateY);
	status=dataDoubArrayFn.setObject( paintRotateY_Hnd.data() );
	if( status )		dary1=dataDoubArrayFn.array();

	MDataHandle paintRotateZ_Hnd  = block.inputValue(paintRotateZ);
	status=dataDoubArrayFn.setObject( paintRotateZ_Hnd.data() );
	if( status )		dary2=dataDoubArrayFn.array();

	daryLen=dary0.length();
	if(daryLen==dary1.length()&&daryLen==dary2.length())
		for(int ii=0;ii<daryLen;++ii)
			preFeatherInfo._vertexRotate.append( MVector(dary0[ii], dary1[ii],dary2[ii] ));

	MDataHandle paintScaleX_Hnd  = block.inputValue(paintScaleX);
	status=dataDoubArrayFn.setObject( paintScaleX_Hnd.data());
	if( status )		dary3=dataDoubArrayFn.array();

	MDataHandle paintScaleY_Hnd  = block.inputValue(paintScaleY);
	status=dataDoubArrayFn.setObject( paintScaleY_Hnd.data()) ;
	if(  status )		dary4=dataDoubArrayFn.array();

	MDataHandle paintScaleZ_Hnd  = block.inputValue(paintScaleZ);
	status=dataDoubArrayFn.setObject( paintScaleZ_Hnd.data() );
	if( status )		dary5=dataDoubArrayFn.array();
	daryLen=dary3.length();
	if(daryLen==dary4.length()&&daryLen==dary5.length())
		for(int ii=0;ii<daryLen;++ii)
			preFeatherInfo._vertexScale.append( MVector(dary3[ii],dary4[ii],dary5[ii]) );
	
	MDataHandle paintBendCurvature_Hnd  = block.inputValue(paintBendCurvature);
	status=dataDoubArrayFn.setObject( paintBendCurvature_Hnd.data()) ;
	if(  status )	dary6= dataDoubArrayFn.array();
	preFeatherInfo._vertexBendCurvature.copy(dary6);

	return status;
}

MStatus jcPreFeather::getInGuideScatter(MDataBlock& block,
										vertexDirectionData* verDirPoint,
										scatterPointData*  sctPoint)
{
	MDataHandle vertexDirFileName_Hnd   = block.inputValue(guideFileName);
	MString vdf = vertexDirFileName_Hnd.asString();

	MDataHandle scatterFileName_Hnd   = block.inputValue(scatterFileName);
	MString scatf = scatterFileName_Hnd.asString();

	if(vdf.length()==0)
	{
		MDataHandle inVertexDir_Hnd   = block.inputValue(inVertexDir);		
		verDirPoint = (vertexDirectionData*)(inVertexDir_Hnd.asPluginData());
		if(!verDirPoint) return MS::kFailure;
		useVerDirFile=false;
	}
	else
	{
		verDirPoint = new vertexDirectionData;
		if( !featherTools::getPreGuideFromFile(vdf,verDirPoint->m_verDir))
		{
			delete verDirPoint;
			verDirPoint=NULL;
			return MS::kFailure;
		}
		useVerDirFile=true;
	}
	//--------------get vertex direction data
	preFeatherInfo.initVerDirTable(verDirPoint->m_verDir);	

	if(scatf.length()==0)
	{
		MDataHandle inScatterData_Hnd   = block.inputValue(inScatterData);		
		sctPoint = (scatterPointData*)(inScatterData_Hnd.asPluginData());
		if(!sctPoint) return MS::kFailure;	

		useScatterFile=false;
	}
	else
	{
		sctPoint = new scatterPointData;
		if( !jcScatterPointData::readDataFrom(scatf,sctPoint->m_PointData))
		{
			delete verDirPoint;
			verDirPoint=NULL;
			return MS::kFailure;
		}		
		useScatterFile=true;
	}
	preFeatherInfo.initScatterData(&sctPoint->m_PointData);

	return MS::kSuccess;
}

MStatus jcPreFeather::compute(const MPlug& plug, MDataBlock& block) 
{
	MStatus status = MS::kSuccess;


	if( plug == outFeatherMesh || plug==outFeatherTurtleArray)
	{
		//-------if not active
		MDataHandle active_Hnd   = block.inputValue(active);
		if(!active_Hnd.asBool()) 
			return MS::kSuccess;

		//-----get attribute data
		if( ! initPreFeatherInfo( block ) )
			return MS::kFailure;

		vertexDirectionData* verDirPointer=NULL;
		scatterPointData*  sctPointer=NULL;

		//----get scatter and preguide data
		if( !getInGuideScatter(block,verDirPointer,sctPointer) )return MS::kFailure;
		
		
		preFeatherInfo.prepareData();

		if( plug == outFeatherMesh )
		{
			MDataHandle instanceMesh_Hnd   = block.inputValue(instanceMesh);
			preFeatherInfo._instanceMesh = instanceMesh_Hnd.asMesh();
			if(!preFeatherInfo._instanceMesh.hasFn(MFn::kMesh)) return MS::kFailure;

			MDataHandle outFeatherMesh_Hnd   = block.outputValue(outFeatherMesh);
		
			//---------compute and set output data
			MObject outMeshData;
			if(preFeatherInfo.GenMesh(outMeshData))
				outFeatherMesh_Hnd.set(outMeshData);
			else
				outFeatherMesh_Hnd.set(MObject::kNullObj);
			block.setClean(outFeatherMesh);
		}
		else
		{
			MDataHandle instanceFeatherTurtle_Hnd = block.inputValue(instanceFeatherTurtle,&status);
			instanceFeatherTurtle_Hnd.data();
			featherTurtleData* currentTurtleD=(featherTurtleData*)(instanceFeatherTurtle_Hnd.asPluginData());
			if(!currentTurtleD) return MS::kFailure;
			preFeatherInfo._instanceTurtleData.assign(	currentTurtleD->m_turtleValue );


			MFnPluginData fnDataCreator;
			fnDataCreator.create( featherTurtleArrayData::id, &status );
			featherTurtleArrayData *newTurtleData = (featherTurtleArrayData*)fnDataCreator.data( &status );
			
			MDataHandle outFeatherTurtleArray_Hnd  = block.outputValue(outFeatherTurtleArray,&status);			
			
			preFeatherInfo.GenFeatherTurtleArray(newTurtleData->m_turtleArray);
			outFeatherTurtleArray_Hnd.set(newTurtleData);
			block.setClean(outFeatherTurtleArray);
		}

		//-----clear memory
		if( useVerDirFile && !verDirPointer ) 
		{
			delete verDirPointer;
			verDirPointer=NULL;
		}
		if( useScatterFile && !sctPointer ) 
		{
			delete sctPointer;
			sctPointer=NULL;
		}
	}
	else
		return MS::kUnknownParameter;

	return status;
}

void    jcPreFeather::draw(M3dView &view, 
						 const MDagPath &path, 
						 M3dView::DisplayStyle style, 
						 M3dView::DisplayStatus dispStatus)
{}
