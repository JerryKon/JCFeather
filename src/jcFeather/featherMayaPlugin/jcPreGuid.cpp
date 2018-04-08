#include "jcPreGuid.h"
#include "randomc.h"
#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnTransform.h>
#include <maya/MVectorArray.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <GL/gl.h>

MTypeId  jcPreGuid::id(0x81035);
MString  jcPreGuid::typeName("jcPreGuide");

MObject  jcPreGuid::displayLocator;
MObject  jcPreGuid::locatorColorHue;
MObject  jcPreGuid::inGrowMesh;
MObject  jcPreGuid::inComponentList;
MObject  jcPreGuid::inGuidCurves;

MObject  jcPreGuid::guidsPerVertex;
MObject  jcPreGuid::searchCurveRadius;
MObject  jcPreGuid::curvePower;
MObject  jcPreGuid::maxAngle;

MObject  jcPreGuid::sampleTolerance;
MObject  jcPreGuid::outVertexDir;

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
jcPreGuid::jcPreGuid()
{}

jcPreGuid::~jcPreGuid()
{}

void * jcPreGuid::creator()
{
    return new jcPreGuid();
}

void   jcPreGuid::nodeRemoved(MObject& node, void *clientData)
{
	MFnDependencyNode fnNode(node);
	static_cast<jcPreGuid*>(fnNode.userNode())->guidResult.clear();
}

MStatus jcPreGuid::initialize()
{
	MStatus status=MS::kSuccess;

	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnNumericAttribute nAttr;
	displayLocator = nAttr.create( "displayLocator", "disploc", MFnNumericData::kBoolean,true);
	MAKE_INPUT(nAttr,true);
	locatorColorHue =nAttr.create( "colorHue", "colh", MFnNumericData::kInt,130);
	MAKE_INPUT(nAttr,true);	
	nAttr.setMin(0);
	nAttr.setMax(360);

	guidsPerVertex = nAttr.create( "curvesPerVertex", "cpv", MFnNumericData::kInt,2);
	MAKE_INPUT(nAttr,false);nAttr.setMin(1);
	searchCurveRadius = nAttr.create( "searchCurveRadius", "scr", MFnNumericData::kFloat,10.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);
	curvePower = nAttr.create( "curvePower", "cpr", MFnNumericData::kFloat,0.6f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	sampleTolerance = nAttr.create( "sampleTolerance", "samto", MFnNumericData::kFloat,0.001f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);

	maxAngle = nAttr.create( "maxAngle", "mag", MFnNumericData::kFloat,40.0f);
	MAKE_INPUT(nAttr,true);nAttr.setMin(0);nAttr.setMax(180);

	MFnTypedAttribute typAttr;
	inGrowMesh = typAttr.create("inGrowMesh","ingm",MFnMeshData::kMesh);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior(MFnAttribute::kReset);

	inGuidCurves = typAttr.create("inGuideCurves","ingc",MFnNurbsCurveData::kNurbsCurve);
	MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);
	typAttr.setArray(true);
	typAttr.setStorable(false);
	typAttr.setDisconnectBehavior(MFnAttribute::kDelete);

	inComponentList = typAttr.create("inComponentList", "inclist", MFnComponentListData::kComponentList);
    MAKE_INPUT(typAttr,false);
	typAttr.setHidden(true);


	outVertexDir = typAttr.create("outVertexDir",
									"outvd",
									vertexDirectionData::id,
									MObject::kNullObj,
									&status);
	CHECK_MSTATUS( status );
	MAKE_OUTPUT(typAttr);
	typAttr.setHidden(true);

	CHECK_MSTATUS( addAttribute(displayLocator) );
	CHECK_MSTATUS( addAttribute(locatorColorHue) );
	CHECK_MSTATUS( addAttribute(guidsPerVertex) );
	CHECK_MSTATUS( addAttribute(searchCurveRadius) );
	CHECK_MSTATUS( addAttribute(curvePower) );
	CHECK_MSTATUS( addAttribute(sampleTolerance) );
	CHECK_MSTATUS( addAttribute(maxAngle) );

	CHECK_MSTATUS( addAttribute(inGrowMesh) );
	CHECK_MSTATUS( addAttribute(inComponentList) );
	CHECK_MSTATUS( addAttribute(inGuidCurves) );
	CHECK_MSTATUS( addAttribute(outVertexDir) );

	CHECK_MSTATUS( attributeAffects( displayLocator, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( guidsPerVertex, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( searchCurveRadius, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( curvePower, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( inGrowMesh, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( inComponentList, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( inGuidCurves, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( maxAngle, outVertexDir ) );
	CHECK_MSTATUS( attributeAffects( sampleTolerance, outVertexDir ) );
	return status;
}

MStatus jcPreGuid::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;

	if(plug == outVertexDir)
	{
		//------preGuidInfo compute
		if(storeDataAndCompute(block)!=MS::kSuccess)
			return MS::kFailure;
		block.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return status;
}

MStatus jcPreGuid::storeDataAndCompute(MDataBlock& block)
{
		MStatus status = MS::kSuccess;
		guidResult.clear();
		vertexDirUVFn preGuidInfo;

		MArrayDataHandle inGuidCurves_Hnd   = block.inputArrayValue(inGuidCurves);
		unsigned int inCurveCount = inGuidCurves_Hnd.elementCount();
		for(unsigned int ii=0;ii<inCurveCount;++ii)
		{
			inGuidCurves_Hnd.jumpToArrayElement(ii);
			preGuidInfo._inGuidCurves.append( inGuidCurves_Hnd.inputValue().asNurbsCurve());
		}		

		MDataHandle guidsPerVertex_Hnd   = block.inputValue(guidsPerVertex);
		preGuidInfo._guidsPerVertex = guidsPerVertex_Hnd.asInt();
		MDataHandle searchCurveRadius_Hnd   = block.inputValue(searchCurveRadius);
		preGuidInfo._searchCurveRadius = searchCurveRadius_Hnd.asFloat();
		MDataHandle curvePower_Hnd   = block.inputValue(curvePower);
		preGuidInfo._curvePower = curvePower_Hnd.asFloat();
		MDataHandle sampleTolerance_Hnd   = block.inputValue(sampleTolerance);
		preGuidInfo._tolerance = sampleTolerance_Hnd.asFloat();
		MDataHandle maxAngle_Hnd   = block.inputValue(maxAngle);
		preGuidInfo._maxAngle = maxAngle_Hnd.asFloat()*radians_per_degree;

		MDataHandle inComponentList_Hnd   = block.inputValue(inComponentList);
		preGuidInfo._growComponent = inComponentList_Hnd.data();

		MDataHandle inGrowMesh_Hnd   = block.inputValue(inGrowMesh);
		preGuidInfo._inGrownMesh = inGrowMesh_Hnd.asMesh();
		if( preGuidInfo._inGrownMesh.isNull() || !preGuidInfo._inGrownMesh.hasFn(MFn::kMesh) ) return MS::kFailure;

		MFnPluginData fnDataCreator;
        fnDataCreator.create( vertexDirectionData::id, &status );
        vertexDirectionData *newTurtleData = (vertexDirectionData*)fnDataCreator.data( &status );
		
		preGuidInfo.getVertexDir( newTurtleData->m_verDir,guidResult);
		MDataHandle outVertexDirUV_Hnd   = block.outputValue(outVertexDir);
		outVertexDirUV_Hnd.set(newTurtleData);
		
		return MS::kSuccess;
}

void jcPreGuid::draw(M3dView &view,
						 const MDagPath &path,
						 M3dView::DisplayStyle style,
						 M3dView::DisplayStatus dispStatus)
{

	MObject thisNode = thisMObject();
	MPlug mePlug(thisNode,displayLocator);
	if(!mePlug.asBool()) return;

	mePlug.setAttribute(outVertexDir);
	MObject updateObject;
	mePlug.getValue(updateObject);

	mePlug.setAttribute(locatorColorHue);
	MColor drawCol(MColor::kHSV,(float)mePlug.asInt(),1,1);

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
					drawGuidVertexLine(drawCol,1);
				else if(dispStatus == M3dView::kDormant)
					drawGuidVertexLine(drawCol,0.6f);
				else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
					drawGuidVertexLine(colorRGB(dispStatus),1);

			glPopMatrix();
		glPopAttrib();
	view.endGL();
}

void jcPreGuid::drawGuidVertexLine(const MColor &col,float scale)
{
	int grs = guidResult.size();
	glColor3f( (float)col.r*scale,(float)col.g*scale,(float)col.b*scale );
	
	glPointSize(4);
	glBegin(GL_POINTS);
	for(int ii=0;ii<grs;++ii)
		glVertex3f((float)guidResult[ii]._startPt.x,(float)guidResult[ii]._startPt.y,(float)guidResult[ii]._startPt.z);
	glEnd();


	glLineWidth(2);
	glBegin(GL_LINES);
	for(int ii=0;ii<grs;++ii)
	{
		glVertex3f((float)guidResult[ii]._startPt.x,(float)guidResult[ii]._startPt.y,(float)guidResult[ii]._startPt.z);
		glVertex3f((float)guidResult[ii]._endPt.x,(float)guidResult[ii]._endPt.y,(float)guidResult[ii]._endPt.z);
	}
	glEnd();
}
