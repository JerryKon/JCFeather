#include "jcPreController.h"
#include "mathFunctions.h"
#include "featherInfo.h"

#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MPlugArray.h>
#include <maya/MFnEnumAttribute.h>
#include <GL/gl.h>


MTypeId  jcPreController::id(0x81040);
MString  jcPreController::typeName("jcPreController");

MObject  jcPreController::controllerColorHue;
MObject  jcPreController::displayShape;
MObject  jcPreController::locatorSize;
MObject  jcPreController::controllerRotate;
MObject  jcPreController::controllerWeightMap;
MObject  jcPreController::bendCurvature;
MObject  jcPreController::outMapName;
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

jcPreController::jcPreController()
{}

jcPreController::~jcPreController()
{}

void jcPreController::postConstructor()
{
	shapeEdge.setLength(29);
	shapeEdge[0] = MFloatVector(0.008411245755f,0,-0.09287172218f);
	shapeEdge[1] = MFloatVector(0.09909653234f,0,-0.1285294547f);
	shapeEdge[2] = MFloatVector(0.1850318692f,0,-0.1528029964f);
	shapeEdge[3] = MFloatVector(0.2725931467f,0,-0.1694780515f);
	shapeEdge[4] = MFloatVector(0.3651867967f,0,-0.1815747035f);
	shapeEdge[5] = MFloatVector(0.4584273462f,0,-0.1901040349f);
	shapeEdge[6] = MFloatVector(0.5467126421f,0,-0.1957329729f);
	shapeEdge[7] = MFloatVector(0.6284416521f,0,-0.1975065631f);
	shapeEdge[8] = MFloatVector(0.7090626651f,0,-0.1916123599f);
	shapeEdge[9] = MFloatVector(0.7944425142f,0,-0.1739523549f);
	shapeEdge[10] = MFloatVector(0.8805694026f,0,-0.1406038046f);
	shapeEdge[11] = MFloatVector(0.9515262336f,0,-0.08785518779f);
	shapeEdge[12] = MFloatVector(0.9912728155f,0,-0.01288336283f);
	shapeEdge[13] = MFloatVector(0.9936066454f,0,0.07242402863f);
	shapeEdge[14] = MFloatVector(0.960488214f,0,0.1439728161f);
	shapeEdge[15] = MFloatVector(0.8948458911f,0,0.179839688f);
	shapeEdge[16] = MFloatVector(0.8066891146f,0,0.1830576228f);
	shapeEdge[17] = MFloatVector(0.7100464626f,0,0.1708245295f);
	shapeEdge[18] = MFloatVector(0.617221705f,0,0.158590527f);
	shapeEdge[19] = MFloatVector(0.5295896212f,0,0.1500162654f);
	shapeEdge[20] = MFloatVector(0.444360004f,0,0.1442694843f);
	shapeEdge[21] = MFloatVector(0.3588054823f,0,0.1405679803f);
	shapeEdge[22] = MFloatVector(0.2704828234f,0,0.1383651569f);
	shapeEdge[23] = MFloatVector(0.1770305208f,0,0.1371660484f);
	shapeEdge[24] = MFloatVector(0.08151817897f,0,0.1327820256f);
	shapeEdge[25] = MFloatVector(0.001266724393f,0,0.111332229f);
	shapeEdge[26] = MFloatVector(-0.04422204801f,0,0.05761122845f);
	shapeEdge[27] = MFloatVector(-0.04663517803f,0,-0.02215684176f);
	shapeEdge[28] = MFloatVector(-0.01737260233f,0,-0.08399245976f);
	
	rachisCurve.setLength(6);
	rachisCurve[0] = MFloatVector(-0.128880097f,0,0.01455387974f);
	rachisCurve[1] = MFloatVector(0.100273891f,0,-0.01721309851f);
	rachisCurve[2] = MFloatVector(0.3599005382f,0,-0.03093490816f);
	rachisCurve[3] = MFloatVector(0.6629378069f,0,-0.02277486506f);
	rachisCurve[4] = MFloatVector(0.8938211686f,0,0.01256085559f);
	rachisCurve[5] = MFloatVector(0.9930458239f,0,0.0544280313f);
}

void * jcPreController::creator()
{
    return new jcPreController();
}

MStatus jcPreController::initialize()
{
	MStatus status=MS::kSuccess;

	if( !featherTools::checkLicense() )
		return MS::kFailure;
	
	MFnEnumAttribute enumAttr;

	displayShape = enumAttr.create("displayShape","dispsh",0);
	enumAttr.addField("Sphere",0);
	enumAttr.addField("Feather",1);
	MAKE_INPUT(enumAttr,true);

	MFnNumericAttribute nAttr;

	controllerColorHue= nAttr.create("controllerColorHue", "concolhue",MFnNumericData::kInt,0);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	nAttr.setMax(360);
	locatorSize= nAttr.create("controllerSize", "consize",MFnNumericData::kFloat,1);
	MAKE_INPUT(nAttr,true);
	nAttr.setMin(0);
	controllerRotate= nAttr.create("controllerRotate", "conrot",MFnNumericData::k3Float,0);
	MAKE_INPUT(nAttr,true);
	bendCurvature= nAttr.create("bendCurvature", "bndcur",MFnNumericData::kFloat,0);
	MAKE_INPUT(nAttr,true);

	controllerWeightMap = nAttr.createColor("controllerWeightMap", "conweimap");
	MAKE_INPUT(nAttr,false);
	MFnTypedAttribute typAttr;
	MFnStringData strData;
	MObject tempObj = strData.create("");
	outMapName =typAttr.create("outMapName", "outmn",MFnData::kString,tempObj);
	MAKE_OUTPUT(typAttr);

	CHECK_MSTATUS( addAttribute(locatorSize) );
	CHECK_MSTATUS( addAttribute(displayShape) );
	CHECK_MSTATUS( addAttribute(controllerRotate) );
	CHECK_MSTATUS( addAttribute(controllerColorHue) );
	CHECK_MSTATUS( addAttribute(bendCurvature) );
	CHECK_MSTATUS( addAttribute(controllerWeightMap) );
	CHECK_MSTATUS( addAttribute(outMapName) );

	CHECK_MSTATUS( attributeAffects( controllerWeightMap, outMapName ) );
	return status;
}

MStatus jcPreController::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;

	if( plug == outMapName )
	{
		MObject me = thisMObject();
		MPlug texPlug(me,controllerWeightMap);
		MPlugArray texConnect;
		MString outStr="";

		if(texPlug.connectedTo(texConnect,true,false,&status))
			outStr = texConnect[0].name();
		MDataHandle outMapName_Hnd   = block.outputValue(outMapName);
		outMapName_Hnd.set(outStr);
		outMapName_Hnd.setClean();
		return MS::kSuccess;
	}
	else
		return MS::kUnknownParameter;
}

void    jcPreController::draw(M3dView &view,
						 const MDagPath &path,
						 M3dView::DisplayStyle style,
						 M3dView::DisplayStatus dispStatus)
{

	MObject thisNode = thisMObject();
	MPlug mePlug(thisNode,controllerColorHue);

	MColor drawCol(MColor::kHSV,(float)mePlug.asInt(),1,1);
	mePlug.setAttribute(locatorSize);
	float locsize=mePlug.asFloat();
	
	mePlug.setAttribute(controllerRotate);
	double rotDeg[3];
	rotDeg[0] = mePlug.child(0).asFloat()*jerryC::radians_per_degree;
	rotDeg[1] = mePlug.child(1).asFloat()*jerryC::radians_per_degree;
	rotDeg[2] = mePlug.child(2).asFloat()*jerryC::radians_per_degree;
	MTransformationMatrix rotMatrix;
	rotMatrix.setRotation(rotDeg,MTransformationMatrix::kXYZ,MSpace::kWorld);

	double rot[4][4];
	view.beginGL();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
				rotMatrix.asMatrix().get(rot);
				glMultMatrixd(&(rot[0][0]));
				if( dispStatus == M3dView::kLead )
					drawController(drawCol,locsize,1);
				else if(dispStatus == M3dView::kDormant)
					drawController(drawCol,locsize,0.6f);
				else if(dispStatus != M3dView::kLead &&dispStatus != M3dView::kDormant)
					drawController(colorRGB(dispStatus),locsize,1);
			glPopMatrix();
		glPopAttrib();
	view.endGL();
}

void inline jcPreController::drawSphereTriangle(float size)
{
	float angle=0;
	int segment=30;
	float degPerSeg=360.0f/(float)segment;

	glBegin(GL_LINE_LOOP);
	        for(int i=0; i<segment; i++)
			{
			  angle= jerryC::radians_per_degree*i*degPerSeg;
			  glVertex3f( (size * cos(angle)) ,
				          (size * sin(angle)) ,
						  0
						);
             }
	glEnd();

	glBegin(GL_LINE_LOOP);
	        for(int i=0; i<segment; i++)
			{
			  angle = jerryC::radians_per_degree*i*degPerSeg;
			  glVertex3f( 0,
						  (size * cos(angle)) ,
				          (size * sin(angle))
						);
             }
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(0,0,size/2);
		glVertex3f(size,0,0);
		glVertex3f(0,0,-size/2);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0);
		glVertex3f(0,size,0);
	glEnd();
}
void jcPreController::drawController(const MColor &col,float size,float colorScale)
{
	glColor3f((float)col.r*colorScale,(float)col.g*colorScale,(float)col.b*colorScale);

	MObject me = thisMObject();
	MPlug shapeType(me,displayShape);

	if(shapeType.asShort()==1)
	{
		glBegin(GL_LINE_LOOP);
		for(unsigned int ii=0;ii<shapeEdge.length();++ii)
			glVertex3f(shapeEdge[ii].x*size,shapeEdge[ii].y*size,shapeEdge[ii].z*size);
		glEnd();

		glBegin(GL_LINE_STRIP);
		for(unsigned int ii=0;ii<rachisCurve.length();++ii)
			glVertex3f(rachisCurve[ii].x*size,rachisCurve[ii].y*size,rachisCurve[ii].z*size);
		glEnd();
	}
	else
		drawSphereTriangle(size);
}
