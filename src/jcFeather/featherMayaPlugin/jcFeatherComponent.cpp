#include "jcFeatherComponent.h"
#include "preFeatherMesh.h"

#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MFnPluginData.h>
#include <GL/gl.h>
MTypeId  jcFeatherComponent::id(0x81034);
MString  jcFeatherComponent::typeName("jcFeatherComponent");

MObject  jcFeatherComponent::displayLocator;
MObject  jcFeatherComponent::inMesh;
MObject  jcFeatherComponent::vertexGroup;
MObject  jcFeatherComponent::featherUV;
MObject  jcFeatherComponent::locatorSize;

MObject  jcFeatherComponent::leftVertexGroup;
MObject  jcFeatherComponent::rightVertexGroup;
MObject  jcFeatherComponent::exMeshFeather;

MObject  jcFeatherComponent::outVertexGroup;
MObject  jcFeatherComponent::outFeatherTurtleData;

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
jcFeatherComponent::jcFeatherComponent()
{}
jcFeatherComponent::~jcFeatherComponent()
{}
void jcFeatherComponent::postConstructor()
{
	m_drawPoints.clear();
}
void * jcFeatherComponent::creator()
{
    return new jcFeatherComponent();
}
void   jcFeatherComponent::nodeRemoved(MObject& node, void *clientData)
{
	MFnDependencyNode fnNode(node);

	static_cast<jcFeatherComponent*>(fnNode.userNode())->m_drawPoints.clear();
}
MStatus jcFeatherComponent::initialize()
{
	MStatus status=MS::kSuccess;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnTypedAttribute typAttr;
	inMesh = typAttr.create("inMesh","inm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior( MFnAttribute::kReset );

	MFnNumericAttribute nAttr;
	displayLocator = nAttr.create( "displayLocator", "dispLoc", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);

	exMeshFeather = nAttr.create( "exMeshFeather", "exmf", MFnNumericData::kBoolean,false);
	MAKE_INPUT(nAttr,true);

	featherUV = nAttr.create( "featherUV", "feauv", MFnNumericData::k2Float,0.0f);
	MAKE_INPUT(nAttr,true);
	nAttr.setDefault(0,0);
	nAttr.setMin(0,0);
	nAttr.setMax(1,1);

	locatorSize=nAttr.create( "locatorSize", "locs", MFnNumericData::kFloat,1.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(1);

	MFnStringData strData;
	MObject tempObj = strData.create("");
	vertexGroup = typAttr.create( "vertexGroup", "vergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	leftVertexGroup = typAttr.create( "leftVertexGroup", "lvergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	rightVertexGroup = typAttr.create( "rightVertexGroup", "rvergrp", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	outVertexGroup = typAttr.create( "outVertexGroup", "outvergrp", MFnData::kString,tempObj);
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);
	typAttr.setConnectable(false);

	outFeatherTurtleData = typAttr.create("outFeatherTurtleData",
											"outfeatd",
											featherTurtleData::id,
											MObject::kNullObj,
											&status);
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	CHECK_MSTATUS( addAttribute(displayLocator) );
	CHECK_MSTATUS( addAttribute(inMesh) );
	CHECK_MSTATUS( addAttribute(vertexGroup) );
	CHECK_MSTATUS( addAttribute(exMeshFeather) );
	CHECK_MSTATUS( addAttribute(leftVertexGroup) );
	CHECK_MSTATUS( addAttribute(rightVertexGroup) );

	CHECK_MSTATUS( addAttribute(featherUV) );
	CHECK_MSTATUS( addAttribute(locatorSize));
	CHECK_MSTATUS( addAttribute(outVertexGroup) );
	CHECK_MSTATUS( addAttribute(outFeatherTurtleData) );
	
	CHECK_MSTATUS( attributeAffects( displayLocator, outVertexGroup ) );
	CHECK_MSTATUS( attributeAffects( vertexGroup, outVertexGroup ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outVertexGroup ) );
	CHECK_MSTATUS( attributeAffects( locatorSize, outVertexGroup ) );

	CHECK_MSTATUS( attributeAffects( vertexGroup, outFeatherTurtleData ) );
	CHECK_MSTATUS( attributeAffects( inMesh, outFeatherTurtleData ) );
	CHECK_MSTATUS( attributeAffects( featherUV, outFeatherTurtleData ) );
	CHECK_MSTATUS( attributeAffects( exMeshFeather, outFeatherTurtleData ) );
	CHECK_MSTATUS( attributeAffects( leftVertexGroup, outFeatherTurtleData ) );
	CHECK_MSTATUS( attributeAffects( rightVertexGroup, outFeatherTurtleData ) );
	return status;
}
MStatus jcFeatherComponent::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;
	if( plug==outVertexGroup||
		plug==outFeatherTurtleData)
	{
		MDataHandle vertexGroup_Hnd   = block.inputValue(vertexGroup,&status);
		MString vstr= vertexGroup_Hnd.asString();
		if(vstr=="") return MS::kSuccess;

		MDataHandle exMeshFeather_Hnd  = block.inputValue(exMeshFeather,&status);
		bool useExMeshFea = exMeshFeather_Hnd.asBool();
		
		MDataHandle leftVertexGroup_Hnd   = block.inputValue(leftVertexGroup,&status);
		MString lvstr= leftVertexGroup_Hnd.asString();
		
		MDataHandle rightVertexGroup_Hnd   = block.inputValue(rightVertexGroup,&status);
		MString rvstr= rightVertexGroup_Hnd.asString();

		MDataHandle featherUV_Hnd  = block.inputValue(featherUV,&status);
		float2 &fuv = featherUV_Hnd.asFloat2();

		MDataHandle locatorSize_Hnd  = block.inputValue(locatorSize,&status);
		float locaS = locatorSize_Hnd.asFloat();

		MDataHandle inMesh_Hnd  = block.inputValue(inMesh,&status);
		MObject inMeshObject = inMesh_Hnd.asMesh();
		if(inMeshObject.isNull()) return MS::kSuccess;

		//-----------�õ�mesh�ĵ����Լ����id��ϣ���������õ�λ������ͷ�������
		MFnMesh meshFn(inMeshObject);
		unsigned int vernum = meshFn.numVertices();
		if(vernum==0) return MS::kSuccess;

		//---------------
        MFnPluginData fnDataCreator;
		MIntArray resultID;
		MString vertexStr="";

        fnDataCreator.create( featherTurtleData::id, &status );
        featherTurtleData *newTurtleData = (featherTurtleData*)fnDataCreator.data( &status );
		featherTools::computeStringIDArray(vstr,resultID,vernum);//������id�ŷŽ�resultID
		featherTools::getPNFromMesh(meshFn,resultID,
									newTurtleData->m_turtleValue._pos,
									newTurtleData->m_turtleValue._nor,vertexStr);//����λ�úͷ���
		newTurtleData->m_turtleValue._featherUV[0] = fuv[0];
		newTurtleData->m_turtleValue._featherUV[1] = fuv[1];
		newTurtleData->m_turtleValue._exMeshFea = useExMeshFea;

		if(useExMeshFea)
		{
			featherTools::computeStringIDArray(lvstr,resultID,vernum);//������id�ŷŽ�resultID
			featherTools::getPFromMesh(meshFn,resultID,newTurtleData->m_turtleValue._leftPt);//����λ��

			featherTools::computeStringIDArray(rvstr,resultID,vernum);//������id�ŷŽ�resultID
			featherTools::getPFromMesh(meshFn,resultID,newTurtleData->m_turtleValue._rightPt);//����λ��
		}

		MDataHandle outFeatherTurtleData_Hnd  = block.outputValue(outFeatherTurtleData,&status);
		outFeatherTurtleData_Hnd.set(newTurtleData);
		outFeatherTurtleData_Hnd.setClean();

		MDataHandle outVertexGroup_Hnd  = block.outputValue(outVertexGroup,&status);
		outVertexGroup_Hnd.set(vertexStr);
		outVertexGroup_Hnd.setClean();

		m_drawPoints.clear();
		featherTools::getFeaComponentDrawPoints(newTurtleData->m_turtleValue._pos,
												newTurtleData->m_turtleValue._nor,
												locaS,m_drawPoints);//������Ҫ���ĵ��λ��
	}
	else
		return MS::kUnknownParameter;

	return status;
}
void    jcFeatherComponent::draw(M3dView &view,
						 const MDagPath &path,
						 M3dView::DisplayStyle style,
						 M3dView::DisplayStatus dispStatus)
{
	MObject thisNode = thisMObject();
	MPlug dispPlug(thisNode,displayLocator);
	if(!dispPlug.asBool()) return;

	MPlug mePlug(thisNode,outVertexGroup);
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

inline  void jcFeatherComponent::drawComponent(const MColor &drawColor)
{
	if(m_drawPoints.length()!=3) return;

	glBegin(GL_LINE_LOOP);
		glColor3f((float)drawColor.r,(float)drawColor.g,(float)drawColor.b);
		glVertex3f((float)m_drawPoints[0].x,(float)m_drawPoints[0].y,(float)m_drawPoints[0].z);
		glVertex3f((float)m_drawPoints[1].x,(float)m_drawPoints[1].y,(float)m_drawPoints[1].z);
		glVertex3f((float)m_drawPoints[2].x,(float)m_drawPoints[2].y,(float)m_drawPoints[2].z);
	glEnd();
}
