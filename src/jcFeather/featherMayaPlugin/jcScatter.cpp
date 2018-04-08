#include "jcScatter.h"
#include "featherInfo.h"
#include "cell.h"

#include <iostream>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MPlugArray.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
MTypeId  jcScatter::id(0x81041);
MString  jcScatter::typeName("jcScatter");

MObject  jcScatter::displayLocator;
MObject  jcScatter::locatorColorHue;
MObject  jcScatter::scatterMode;

MObject  jcScatter::numPerArea;
MObject  jcScatter::inPosition;
MObject  jcScatter::maxDistance;
MObject  jcScatter::uvSegment;
MObject  jcScatter::uvOffset;

MObject  jcScatter::jitterDistance;
MObject  jcScatter::jitterFrequency;

MObject  jcScatter::randomSeed;
MObject  jcScatter::uvSetName;
MObject  jcScatter::baldnessTex;

MObject  jcScatter::customerUVData;
MObject  jcScatter::customerUArrayData;
MObject  jcScatter::customerVArrayData;

MObject  jcScatter::inComponentList;
MObject  jcScatter::inputGrowMesh;

MObject  jcScatter::outGrowData;
MObject  jcScatter::pointSize;
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

jcScatter::jcScatter()
{
}

jcScatter::~jcScatter()
{
}

void jcScatter::postConstructor()
{
}

void * jcScatter::creator()
{
    return new jcScatter();
}

MStatus jcScatter::initialize()
{
	MStatus status;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnEnumAttribute enumAttr;

	scatterMode = enumAttr.create("scatterMode","sctm",1);
	enumAttr.addField("Area",0);
	enumAttr.addField("UV",1);
	enumAttr.addField("InPosition",2);
	MAKE_INPUT(enumAttr,true);

    MFnNumericAttribute nAttr;

	displayLocator = nAttr.create( "displayLocator", "disploc", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);

	numPerArea = nAttr.create( "numPerArea", "npa", MFnNumericData::kDouble,1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	maxDistance = nAttr.create( "maxDistance", "maxd", MFnNumericData::kDouble,0.01);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	uvSegment =nAttr.create( "uvSegment", "uvseg", MFnNumericData::k2Int,10);
	MAKE_INPUT(nAttr,true);

	uvOffset =nAttr.create( "uvOffset", "uvOffset", MFnNumericData::k2Double,0);
	MAKE_INPUT(nAttr,true);

	jitterDistance =nAttr.create( "jitterDistance", "jitterdis", MFnNumericData::kFloat,0.1);
	MAKE_INPUT(nAttr,true);

	jitterFrequency =nAttr.create( "jitterFrequency", "jitterfre", MFnNumericData::kFloat,10.1f);
	MAKE_INPUT(nAttr,true);

	baldnessTex =nAttr.create( "baldnessTex", "baldnessTex", MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,false);

	customerUVData = nAttr.create( "userUvData", "usdat", MFnNumericData::k2Float);
	MAKE_INPUT(nAttr,false);
	nAttr.setArray(true);

	randomSeed =nAttr.create( "randomSeed", "rands", MFnNumericData::kInt,5);
	MAKE_INPUT(nAttr,true);
	locatorColorHue =nAttr.create( "colorHue", "colh", MFnNumericData::kInt,180);
	MAKE_INPUT(nAttr,true);	
	nAttr.setMin(0);
	nAttr.setMax(360);

	pointSize =nAttr.create( "pointSize", "pts", MFnNumericData::kInt);
	MAKE_OUTPUT(nAttr);

	MFnTypedAttribute typAttr;	
	inComponentList = typAttr.create("inComponentList", "inclist", MFnComponentListData::kComponentList);
    MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	inputGrowMesh = typAttr.create("inGrowMesh","ingrm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	CHECK_MSTATUS( typAttr.setDisconnectBehavior(MFnAttribute::kReset) );
	
	MVectorArray defaultVectArray;
    MFnVectorArrayData vectArrayDataFn;
    vectArrayDataFn.create( defaultVectArray );
    inPosition = typAttr.create( "inPosition",
									"ipos",
									MFnData::kVectorArray,
									vectArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);

	outGrowData = typAttr.create("outGrowData",
									"outgd",
									scatterPointData::id,
									MObject::kNullObj,
									&status);
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	MFnStringData strData;
	MObject tempObj = strData.create("map1");
	uvSetName = typAttr.create( "uvSetName", "uvsn", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	MDoubleArray defaultDoubArray;
    MFnDoubleArrayData doubArrayDataFn;
    doubArrayDataFn.create( defaultDoubArray );
    customerUArrayData = typAttr.create( "userUArray",
									"usua",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	CHECK_MSTATUS( typAttr.setHidden(true));

    customerVArrayData = typAttr.create( "userVArray",
									"usva",
									MFnData::kDoubleArray,
									doubArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	CHECK_MSTATUS( typAttr.setHidden(true));

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	CHECK_MSTATUS( addAttribute(displayLocator) );
	CHECK_MSTATUS( addAttribute(locatorColorHue) );
	CHECK_MSTATUS( addAttribute(scatterMode) );
	CHECK_MSTATUS( addAttribute(numPerArea) );
	CHECK_MSTATUS( addAttribute(maxDistance) );
	CHECK_MSTATUS( addAttribute(inPosition) );

	CHECK_MSTATUS( addAttribute(uvSegment) );
	CHECK_MSTATUS( addAttribute(jitterDistance) );
	CHECK_MSTATUS( addAttribute(jitterFrequency) );
	CHECK_MSTATUS( addAttribute(uvOffset) );
	
	CHECK_MSTATUS( addAttribute(baldnessTex) );
	
	CHECK_MSTATUS( addAttribute(randomSeed) );
	CHECK_MSTATUS( addAttribute(uvSetName) );
	CHECK_MSTATUS( addAttribute(customerUVData) );
	CHECK_MSTATUS( addAttribute(customerUArrayData) );
	CHECK_MSTATUS( addAttribute(customerVArrayData) );
	CHECK_MSTATUS( addAttribute(inputGrowMesh) );
	CHECK_MSTATUS( addAttribute(inComponentList) );
	CHECK_MSTATUS( addAttribute(outGrowData) );
	CHECK_MSTATUS( addAttribute(pointSize) );
	//--------------------------------------
	CHECK_MSTATUS( attributeAffects( scatterMode, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( displayLocator, outGrowData ) );

	CHECK_MSTATUS( attributeAffects( numPerArea, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( maxDistance, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( inPosition, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( uvSegment, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( uvOffset, outGrowData ) );

	CHECK_MSTATUS( attributeAffects( jitterDistance, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( jitterFrequency, outGrowData ) );

	CHECK_MSTATUS( attributeAffects( baldnessTex, outGrowData ) );

	CHECK_MSTATUS( attributeAffects( randomSeed, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( customerUVData, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( customerUArrayData, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( customerVArrayData, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMesh, outGrowData ) );
	CHECK_MSTATUS( attributeAffects( inComponentList, outGrowData ) );

	//-----------------
	CHECK_MSTATUS( attributeAffects( scatterMode, pointSize ) );
	CHECK_MSTATUS( attributeAffects( displayLocator, pointSize ) );

	CHECK_MSTATUS( attributeAffects( numPerArea, pointSize ) );
	CHECK_MSTATUS( attributeAffects( maxDistance, pointSize ) );
	CHECK_MSTATUS( attributeAffects( inPosition, pointSize ) );
	CHECK_MSTATUS( attributeAffects( uvSegment, pointSize ) );
	CHECK_MSTATUS( attributeAffects( uvOffset, pointSize ) );

	CHECK_MSTATUS( attributeAffects( jitterDistance, pointSize ) );
	CHECK_MSTATUS( attributeAffects( jitterFrequency, pointSize ) );

	CHECK_MSTATUS( attributeAffects( baldnessTex, pointSize ) );

	CHECK_MSTATUS( attributeAffects( randomSeed, pointSize ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, pointSize ) );
	CHECK_MSTATUS( attributeAffects( customerUVData, pointSize ) );
	CHECK_MSTATUS( attributeAffects( customerUArrayData, pointSize ) );
	CHECK_MSTATUS( attributeAffects( customerVArrayData, pointSize ) );
	CHECK_MSTATUS( attributeAffects( inputGrowMesh, pointSize ) );
	CHECK_MSTATUS( attributeAffects( inComponentList, pointSize ) );
   return MS::kSuccess;
}

MStatus jcScatter::compute(const MPlug& plug, MDataBlock& data) 
{
	MStatus status = MS::kSuccess;

	if(plug == outGrowData || plug == pointSize)
		return getDataCompute(data);
	else
		return MS::kUnknownParameter;
}

void    jcScatter::draw(M3dView &view, 
						 const MDagPath &path, 
						 M3dView::DisplayStyle style, 
						 M3dView::DisplayStatus dispStatus)
{
	MObject thisNode = thisMObject();

	MPlug mePlug(thisNode,locatorColorHue);
	MColor drawCol(MColor::kHSV,(float)mePlug.asInt(),1,1);

	MPlug outGrowPlug(thisNode,displayLocator);
	if(!outGrowPlug.asBool()) return;

	outGrowPlug.setAttribute(outGrowData);
	MObject updateObj;
	outGrowPlug.getValue(updateObj);

	if( _displayPts.length()==0 )
		return;
	
    MFnDagNode fnDagNode(thisNode);
	MFnTransform fnParentTransform(fnDagNode.parent(0));
	m_worldMatrix = fnParentTransform.transformation().asMatrix();

	view.beginGL(); 
		glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
				double m[4][4];
				m_worldMatrix.inverse().get(m);
				glMultMatrixd(&(m[0][0]));
				if( dispStatus == M3dView::kLead )
					drawScatterPoints(drawCol,1);
				else if(dispStatus == M3dView::kDormant)
					drawScatterPoints(drawCol,0.6f);
				else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
					drawScatterPoints(colorRGB(dispStatus),1);
			glPopMatrix();
		glPopAttrib();
	view.endGL();
}

void jcScatter::drawScatterPoints(const MColor &col,float scale)
{
	int grs = _displayPts.length();
	glColor3f( (float)col.r*scale,(float)col.g*scale,(float)col.b*scale );
	glPointSize(4);
	glBegin(GL_POINTS);
	for(int ii=0;ii<grs;++ii)		
		glVertex3f(_displayPts[ii].x,_displayPts[ii].y,_displayPts[ii].z);
	glEnd();
}

MStatus jcScatter::getDataCompute( MDataBlock& block )
{
	MStatus status = MS::kSuccess;
	
	scatterPoints scatterPt;

	MObject me = thisMObject();

	MDataHandle scatterMode_Hnd          = block.inputValue(scatterMode,&status);
	scatterPt._scatterMode=scatterMode_Hnd.asShort();

	MDataHandle numPerArea_Hnd          = block.inputValue(numPerArea,&status);
	scatterPt._ptNumPerArea=numPerArea_Hnd.asDouble();

	MDataHandle maxDistance_Hnd          = block.inputValue(maxDistance,&status);
	scatterPt._maxDistance=maxDistance_Hnd.asDouble();

	MDataHandle inputGrowMesh_Hnd          = block.inputValue(inputGrowMesh,&status);
	scatterPt._inMesh= inputGrowMesh_Hnd.asMesh();

	MDataHandle inComponentList_Hnd          = block.inputValue(inComponentList,&status);
	scatterPt._inComponentList= inComponentList_Hnd.data();

	MDataHandle uvSetName_Hnd          = block.inputValue(uvSetName,&status);
	scatterPt._uv = uvSetName_Hnd.asString();
	
	//----------------------------------------------customer uv data
	//-----------------------------------------
	MArrayDataHandle    customerUVData_Hnd = block.inputArrayValue(customerUVData);
	int usrDataSize=customerUVData_Hnd.elementCount();
	for(int ii=0;ii<usrDataSize;++ii)
	{
		customerUVData_Hnd.jumpToArrayElement(ii);
		float2 &usrUVValue= customerUVData_Hnd.inputValue().asFloat2();
		if(usrUVValue[0]>=0 && usrUVValue[1]>=0)
		{
			scatterPt._usrU.append(usrUVValue[0]);
			scatterPt._usrV.append(usrUVValue[1]);
		}
	}
	MFnDoubleArrayData dataDoubArrayFn;
	MDoubleArray dary0,dary1;

	MDataHandle customerUArrayData_Hnd  = block.inputValue(customerUArrayData,&status);
	status = dataDoubArrayFn.setObject( customerUArrayData_Hnd.data());
	CHECK_MSTATUS( status );
	if( status )	dary0=dataDoubArrayFn.array();

	MDataHandle customerVArrayData_Hnd  = block.inputValue(customerVArrayData,&status);
	status = dataDoubArrayFn.setObject( customerVArrayData_Hnd.data());
	CHECK_MSTATUS( status );
	if( status )	dary1=dataDoubArrayFn.array();
	usrDataSize=dary0.length();
	if(usrDataSize==dary1.length())
		for(int ii=0;ii<usrDataSize;++ii)
		{
			scatterPt._usrU.append((float)dary0[ii]);
			scatterPt._usrV.append((float)dary1[ii]);
		}
	if(scatterPt._scatterMode==2)
	{
		MFnVectorArrayData dataVectorArrayFn;
		MDataHandle inPosition_Hnd  = block.inputValue(inPosition,&status);
		dataVectorArrayFn.setObject( inPosition_Hnd.data() );
		MVectorArray inPos=dataVectorArrayFn.array(&status);
		for(int ii=0;ii<inPos.length();++ii)
			scatterPt._usrInPosition.append(inPos[ii].x,inPos[ii].y,inPos[ii].z);
	}

	//-----------------------------------------


	//-----------------��jerryC::jCell�������㣬���������ý�ȥ
	MDataHandle uvSegment_Hnd          = block.inputValue(uvSegment,&status);
	int2 &uvsegData=uvSegment_Hnd.asInt2();
	jerryC::jCell::m_xSegment = uvsegData[0];
	jerryC::jCell::m_zSegment = uvsegData[1];

	MDataHandle randomSeed_Hnd          = block.inputValue(randomSeed,&status);
	jerryC::jCell::m_seed = randomSeed_Hnd.asInt();

	MDataHandle jitterDistance_Hnd    = block.inputValue(jitterDistance,&status);
	jerryC::jCell::m_jitter = (double)(jitterDistance_Hnd.asFloat());

	MDataHandle jitterFrequency_Hnd    = block.inputValue(jitterFrequency,&status);
	jerryC::jCell::m_frequency= (double)(jitterFrequency_Hnd.asFloat());

	MDataHandle uvOffset_Hnd    = block.inputValue(uvOffset,&status);
	double2 &uvo = uvOffset_Hnd.asDouble2();
	jerryC::jCell::m_xOffset = uvo[0];
	jerryC::jCell::m_zOffset = uvo[1];

	//-------------------------�õ�texture�����
	MPlug texPlug(me,baldnessTex);
	MPlugArray texConnect;
	if(texPlug.connectedTo(texConnect,true,false,&status))
		scatterPt._baldTex = texConnect[0].name();
	
	//-------------------
	if(!scatterPt.getFinalPosition(_displayPts))
		return MS::kFailure;

	MFnPluginData fnDataCreator;
    fnDataCreator.create( scatterPointData::id, &status );
    scatterPointData *newPtData = (scatterPointData*)fnDataCreator.data( &status );
	newPtData->m_PointData = scatterPt._outScatterData;

	MDataHandle outGrowData_Hnd   = block.outputValue(outGrowData);
	outGrowData_Hnd.set(newPtData);
	outGrowData_Hnd.setClean();

	MDataHandle pointSize_Hnd   = block.outputValue(pointSize);
	pointSize_Hnd.setInt(scatterPt._outScatterData._u.length());
	pointSize_Hnd.setClean();

	return status;
}
