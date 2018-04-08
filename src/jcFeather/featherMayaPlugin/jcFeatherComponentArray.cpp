#include "jcFeatherComponentArray.h"
#include "preFeatherMesh.h"
#include "featherTurtleArrayData.h"

#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnStringData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MFnPluginData.h>
#include <GL/gl.h>

MTypeId  jcFeatherComponentArray::id(0x81039);
MString  jcFeatherComponentArray::typeName("jcFeatherComponentArray");

MObject  jcFeatherComponentArray::displayLocator;
MObject  jcFeatherComponentArray::walkStep;
MObject  jcFeatherComponentArray::walkTimes;
MObject  jcFeatherComponentArray::vertexGroup;

MObject  jcFeatherComponentArray::leftVertexGroup;
MObject  jcFeatherComponentArray::rightVertexGroup;
MObject  jcFeatherComponentArray::exMeshFeather;

MObject  jcFeatherComponentArray::uvType;
MObject  jcFeatherComponentArray::locatorSize;
MObject  jcFeatherComponentArray::uvSetName;
MObject  jcFeatherComponentArray::uArrayData;
MObject  jcFeatherComponentArray::vArrayData;

MObject  jcFeatherComponentArray::inMesh;
MObject  jcFeatherComponentArray::inGrowMesh;
MObject  jcFeatherComponentArray::outFeatherTurtleDataArray;
MObject  jcFeatherComponentArray::outFeatherNumArray;
MObject  jcFeatherComponentArray::outFeatherUArray;
MObject  jcFeatherComponentArray::outFeatherVArray;
MObject  jcFeatherComponentArray::outRootIdArray;
MObject  jcFeatherComponentArray::outRootPositionArray;

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
jcFeatherComponentArray::jcFeatherComponentArray()
{}
jcFeatherComponentArray::~jcFeatherComponentArray()
{}
void jcFeatherComponentArray::postConstructor()
{
	m_drawPoints.clear();
}
void * jcFeatherComponentArray::creator()
{
    return new jcFeatherComponentArray();
}
void   jcFeatherComponentArray::nodeRemoved(MObject& node, void *clientData)
{
	MFnDependencyNode fnNode(node);

	static_cast<jcFeatherComponentArray*>(fnNode.userNode())->m_drawPoints.clear();
}
MStatus jcFeatherComponentArray::initialize()
{
	MStatus status=MS::kSuccess;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnTypedAttribute typAttr;
	inMesh = typAttr.create("inMesh","inm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setArray(true);
	typAttr.setDisconnectBehavior( MFnAttribute::kDelete );

	inGrowMesh = typAttr.create("inGrowMesh","ingm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior( MFnAttribute::kReset );

	MFnNumericAttribute nAttr;
	displayLocator=nAttr.create( "displayLocator", "dispLoc", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);

	exMeshFeather = nAttr.create( "exMeshFeather", "exmf", MFnNumericData::kBoolean,false);
	MAKE_INPUT(nAttr,true);

	locatorSize=nAttr.create( "locatorSize", "locs", MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);

	walkStep=nAttr.create( "walkStep", "walks", MFnNumericData::kInt,1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(1);

	walkTimes=nAttr.create( "walkTimes", "walkt", MFnNumericData::kInt,-1);
	MAKE_INPUT(nAttr,true);nAttr.setMin(-1);
	
	MFnEnumAttribute enumAttr;
	uvType = enumAttr.create("uvType","uvt",0);
	enumAttr.addField("Grow Mesh UV",0);
	enumAttr.addField("Cache UV",1);
	MAKE_INPUT(enumAttr,true);

	MFnStringData strData;
	MObject tempObj = strData.create("");
	vertexGroup = typAttr.create( "vertexGroup", "vergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	leftVertexGroup = typAttr.create( "leftVertexGroup", "lvergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	rightVertexGroup = typAttr.create( "rightVertexGroup", "rvergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	tempObj = strData.create("map1");
	uvSetName = typAttr.create( "uvSet", "uvs", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

    MDoubleArray defaultDoubleArray;
    MFnDoubleArrayData dbArrayDataFn;
    dbArrayDataFn.create( defaultDoubleArray );
    uArrayData = typAttr.create( "uArrayData",
									"uad",
									MFnData::kDoubleArray,
									dbArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setStorable(true);

	vArrayData = typAttr.create( "vArrayData",
									"vad",
									MFnData::kDoubleArray,
									dbArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_INPUT(typAttr,false);
	typAttr.setStorable(true);

	outFeatherUArray = typAttr.create( "outFeatherUArray",
									"ofua",
									MFnData::kDoubleArray,
									dbArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);

	outFeatherVArray = typAttr.create( "outFeatherVArray",
									"ofva",
									MFnData::kDoubleArray,
									dbArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);

    MIntArray defaultIntArray;
    MFnIntArrayData intArrayDataFn;
    intArrayDataFn.create( defaultIntArray );
	outFeatherNumArray = typAttr.create( "outFeatherNumArray",
									"ofna",
									MFnData::kIntArray,
									intArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );

	MAKE_OUTPUT(typAttr);
	outRootIdArray = typAttr.create( "outRootIdArray",
									"orida",
									MFnData::kIntArray,
									intArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);

	MVectorArray defaultVectArray;
    MFnVectorArrayData vectArrayDataFn;
    vectArrayDataFn.create( defaultVectArray );
    outRootPositionArray = typAttr.create( "outRootPositionArray",
									"orpa",
									MFnData::kVectorArray,
									vectArrayDataFn.object(),
									&status );
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);

	outFeatherTurtleDataArray = typAttr.create("outFeatherTurtleDataArray",
											"outfeatda",
											featherTurtleArrayData::id,
											MObject::kNullObj,
											&status);
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	addAndAffect();

	return status;
}

void jcFeatherComponentArray::addAndAffect()
{
	CHECK_MSTATUS( addAttribute(displayLocator) );
	CHECK_MSTATUS( addAttribute(walkStep) );
	CHECK_MSTATUS( addAttribute(walkTimes) );
	CHECK_MSTATUS( addAttribute(vertexGroup) );
	CHECK_MSTATUS( addAttribute(exMeshFeather) );
	CHECK_MSTATUS( addAttribute(leftVertexGroup) );
	CHECK_MSTATUS( addAttribute(rightVertexGroup) );

	CHECK_MSTATUS( addAttribute(uvType) );
	CHECK_MSTATUS( addAttribute(locatorSize));
	CHECK_MSTATUS( addAttribute(uvSetName) );
	CHECK_MSTATUS( addAttribute(uArrayData));
	CHECK_MSTATUS( addAttribute(vArrayData) );
	CHECK_MSTATUS( addAttribute(inMesh) );
	CHECK_MSTATUS( addAttribute(inGrowMesh) );
	CHECK_MSTATUS( addAttribute(outFeatherTurtleDataArray) );

	CHECK_MSTATUS( addAttribute(outFeatherNumArray) );
	CHECK_MSTATUS( addAttribute(outFeatherUArray) );
	CHECK_MSTATUS( addAttribute(outFeatherVArray) );
	CHECK_MSTATUS( addAttribute(outRootIdArray) );
	CHECK_MSTATUS( addAttribute(outRootPositionArray) );

	CHECK_MSTATUS( attributeAffects( displayLocator, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( walkStep, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( locatorSize, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outFeatherTurtleDataArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outFeatherTurtleDataArray ) );

	CHECK_MSTATUS( attributeAffects( walkStep, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outFeatherNumArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outFeatherNumArray ) );

	CHECK_MSTATUS( attributeAffects( walkStep, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outFeatherUArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outFeatherUArray ) );


	CHECK_MSTATUS( attributeAffects( walkStep, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outFeatherVArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outFeatherVArray ) );

	CHECK_MSTATUS( attributeAffects( walkStep, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outRootIdArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outRootIdArray ) );

	CHECK_MSTATUS( attributeAffects( walkStep, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( walkTimes, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( uvType, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( uvSetName, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( uArrayData, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( vArrayData, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outRootPositionArray ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outRootPositionArray ) );
}

MStatus jcFeatherComponentArray::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;
	if( plug == outFeatherTurtleDataArray ||
		plug == outFeatherNumArray||
		plug == outFeatherUArray||
		plug == outFeatherVArray||
		plug == outRootIdArray||
		plug == outRootPositionArray)
	{
		MDataHandle vertexGroup_Hnd   = block.inputValue(vertexGroup,&status);
		MString vstr= vertexGroup_Hnd.asString();
		if(vstr=="") return MS::kSuccess;

		MDataHandle uvSetName_Hnd   = block.inputValue(uvSetName,&status);
		MString uvN= uvSetName_Hnd.asString();
		if(uvN=="") uvN = "map1";

		MDataHandle walkStep_Hnd  = block.inputValue(walkStep,&status);
		int walks = walkStep_Hnd.asInt();
		MDataHandle walkTimes_Hnd  = block.inputValue(walkTimes,&status);
		int walkt = walkTimes_Hnd.asInt();
		MDataHandle uvType_Hnd  = block.inputValue(uvType,&status);
		short useUVType = uvType_Hnd.asShort();
		MDataHandle locatorSize_Hnd  = block.inputValue(locatorSize,&status);
		float locaS = locatorSize_Hnd.asFloat();
		
		MFnDoubleArrayData dataDbArrayFn;
		MDataHandle uArrayData_Hnd  = block.inputValue(uArrayData,&status);
		dataDbArrayFn.setObject( uArrayData_Hnd.data() );
		MDoubleArray uAD=dataDbArrayFn.array(&status);

		MDataHandle vArrayData_Hnd  = block.inputValue(vArrayData,&status);
		dataDbArrayFn.setObject( vArrayData_Hnd.data() );
		MDoubleArray vAD=dataDbArrayFn.array(&status);
		
		//-------
		MDataHandle exMeshFeather_Hnd  = block.inputValue(exMeshFeather,&status);
		bool useExMeshFea = exMeshFeather_Hnd.asBool();

		MDataHandle leftVertexGroup_Hnd   = block.inputValue(leftVertexGroup,&status);
		MString lvstr= leftVertexGroup_Hnd.asString();
		
		MDataHandle rightVertexGroup_Hnd   = block.inputValue(rightVertexGroup,&status);
		MString rvstr= rightVertexGroup_Hnd.asString();
		//-------

		MArrayDataHandle inMesh_Hnd  = block.inputArrayValue(inMesh,&status);
		unsigned int inMSize = inMesh_Hnd.elementCount();

		MDataHandle inGrowMesh_Hnd  = block.inputValue(inGrowMesh,&status);
		MObject inGrowMeshObject = inGrowMesh_Hnd.asMesh();
		
		MDoubleArray outFeaU,outFeaV;
		MIntArray outFeaNum,outVerIds;
		MVectorArray outPositions;
		//-----------�õ�mesh�ĵ����Լ����id��ϣ���������õ�λ������ͷ�������

		MFnPluginData fnDataCreator;
        fnDataCreator.create( featherTurtleArrayData::id, &status );
        featherTurtleArrayData *newTurtleData = (featherTurtleArrayData*)fnDataCreator.data( &status );

		MObject inMeshObject;
		MFnMesh meshFn;
		turtleArrayData currentTurtleAD;
		MPointArray tempPA;
		m_drawPoints.clear();
		unsigned int vernum=0;
		int tds=0;
		intArrayVector resultID;
		MFloatArray turtleU,turtleV;
		MIntArray turtleVerId;
		unsigned int currentUVArrayIndex=0;
		for(unsigned int ii=0;ii<inMSize;++ii)
		{
			currentTurtleAD.clear();

			inMesh_Hnd.jumpToArrayElement(ii);
			inMeshObject = inMesh_Hnd.inputValue().asMesh();
			meshFn.setObject(inMeshObject);

			vernum = meshFn.numVertices();
			if(vernum==0) continue;
			featherTools::computeStringMultiArray(walks,walkt,vstr,resultID,vernum);
			for(int ii=0;ii<resultID.size();++ii)
				outVerIds.append(resultID[ii][0]);

			//read uv value
			if(useUVType==0)
			{
				featherTools::getMultiPNFromMesh(meshFn,resultID,currentTurtleAD);
				if( !inGrowMeshObject.isNull() )
				{
					meshFn.setObject(inGrowMeshObject);
					featherTools::getTurtle_UVData(currentTurtleAD,meshFn,&uvN);
				}
			}
			else if(useUVType==1)
			{
				featherTools::getMultiPNFromMesh(meshFn,resultID,currentTurtleAD);
				tds=currentTurtleAD.size();
				if( uAD.length()== vAD.length() && uAD.length()>=(currentUVArrayIndex + tds) )
				{
					for(unsigned int kk=0;kk<tds;++kk)
					{
						currentTurtleAD[kk]._featherUV[0] = uAD[currentUVArrayIndex+kk];
						currentTurtleAD[kk]._featherUV[1] = vAD[currentUVArrayIndex+kk];
					}
					currentUVArrayIndex += tds;
				}
			}
			if(useExMeshFea)
			{
				featherTools::computeStringMultiArray(walks,walkt,lvstr,resultID,vernum);
				featherTools::getMultiSidePFromMesh(meshFn,resultID,currentTurtleAD,true);

				featherTools::computeStringMultiArray(walks,walkt,rvstr,resultID,vernum);
				featherTools::getMultiSidePFromMesh(meshFn,resultID,currentTurtleAD,false);

			}
			tds = currentTurtleAD.size();
			for(int jj=0;jj<tds;++jj)
			{
				tempPA.clear();
				featherTools::getFeaComponentDrawPoints(currentTurtleAD[jj]._pos,
													  currentTurtleAD[jj]._nor,
													  locaS,
													  tempPA);
				int posl= tempPA.length();
				for(int kk=0;kk<posl;++kk)
					m_drawPoints.append(tempPA[kk]);
			}
			//-----------
			for(int jj=0;jj<tds;++jj)
			{
				newTurtleData->m_turtleArray.push_back(currentTurtleAD[jj]);
				outFeaU.append(currentTurtleAD[jj]._featherUV[0]);
				outFeaV.append(currentTurtleAD[jj]._featherUV[1]);

				outPositions.append(currentTurtleAD[jj]._pos[0]);
			}
			outFeaNum.append(tds);
		}
		if(plug == outFeatherTurtleDataArray)
		{
			MDataHandle outFeatherTurtleDataArray_Hnd  = block.outputValue(outFeatherTurtleDataArray,&status);
			outFeatherTurtleDataArray_Hnd.set(newTurtleData);
			block.setClean(plug);
		}

		if(plug == outFeatherNumArray)
		{
			MFnIntArrayData dataIntArrayFn;
			MDataHandle outFeatherNumArray_Hnd  = block.outputValue(outFeatherNumArray,&status);
			dataIntArrayFn.setObject(outFeatherNumArray_Hnd.data());
			dataIntArrayFn.set(outFeaNum);
			block.setClean(plug);
		}
		if(plug == outFeatherUArray)
		{
			MFnDoubleArrayData dataDoubleArrayFn;
			MDataHandle outFeatherUArray_Hnd    = block.outputValue(outFeatherUArray,&status);
			dataDoubleArrayFn.setObject(outFeatherUArray_Hnd.data());
			dataDoubleArrayFn.set(outFeaU);
			block.setClean(plug);
		}
		if(plug == outFeatherVArray)
		{
			MFnDoubleArrayData dataDoubleArrayFn;
			MDataHandle outFeatherVArray_Hnd    = block.outputValue(outFeatherVArray,&status);
			dataDoubleArrayFn.setObject(outFeatherVArray_Hnd.data());
			dataDoubleArrayFn.set(outFeaV);
			block.setClean(plug);
		}

		if(plug == outRootIdArray)
		{
			MFnIntArrayData dataIntArrayFn;
			MDataHandle outRootIdArray_Hnd  = block.outputValue(outRootIdArray,&status);
			dataIntArrayFn.setObject(outRootIdArray_Hnd.data());
			dataIntArrayFn.set(outVerIds);
			block.setClean(plug);
		}

		if(plug == outRootPositionArray)
		{
			MFnVectorArrayData dataVectorArrayFn;
			MDataHandle outRootPositionArray_Hnd  = block.outputValue(outRootPositionArray,&status);
			dataVectorArrayFn.setObject(outRootPositionArray_Hnd.data());
			dataVectorArrayFn.set(outPositions);
			block.setClean(plug);
		}
	}
	else
		return MS::kUnknownParameter;

	return status;
}

void    jcFeatherComponentArray::draw(M3dView &view,
						 const MDagPath &path,
						 M3dView::DisplayStyle style,
						 M3dView::DisplayStatus dispStatus)
{
	MObject thisNode = thisMObject();
	MPlug dispPlug(thisNode,displayLocator);
	if(!dispPlug.asBool()) return;

	MPlug mePlug(thisNode,outFeatherTurtleDataArray);
	MObject updateObject;
	mePlug.getValue(updateObject);


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
					drawComponent(MColor(0,1,1));
				else if(dispStatus == M3dView::kDormant)
					drawComponent(MColor(0,0.6f,1));
				else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
					drawComponent(colorRGB(dispStatus));

			glPopMatrix();
		glPopAttrib();
	view.endGL();
}

inline  void jcFeatherComponentArray::drawComponent(const MColor &drawColor)
{
	int ptl=m_drawPoints.length();
	if(ptl<3) return;
	glColor3f((float)drawColor.r,(float)drawColor.g,(float)drawColor.b);
	for(int ii=0;ii<ptl-2;ii+=3)
	{
		glBegin(GL_LINE_LOOP);
			glVertex3f((float)m_drawPoints[ii].x,(float)m_drawPoints[ii].y,(float)m_drawPoints[ii].z);
			glVertex3f((float)m_drawPoints[ii+1].x,(float)m_drawPoints[ii+1].y,(float)m_drawPoints[ii+1].z);
			glVertex3f((float)m_drawPoints[ii+2].x,(float)m_drawPoints[ii+2].y,(float)m_drawPoints[ii+2].z);
		glEnd();
	}
}
