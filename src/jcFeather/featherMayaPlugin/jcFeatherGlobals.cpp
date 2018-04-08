
#include "jcFeatherGlobals.h"
#include "featherInfo.h"
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>

#if defined(_OPENMP)
	#include <omp.h>
#endif


MTypeId     jcFeatherGlobals::id( 0x81031 );
MString		jcFeatherGlobals::typeName("jcFeatherGlobals");    
MObject		jcFeatherGlobals::diceHair;
MObject		jcFeatherGlobals::featherRibText;
MObject     jcFeatherGlobals::rachisMeshRib;
MObject		jcFeatherGlobals::exportNormal;
MObject		jcFeatherGlobals::relativePath;
MObject		jcFeatherGlobals::gzipCompression;
MObject		jcFeatherGlobals::delayReadArchive;
MObject		jcFeatherGlobals::deleteTempFile;
MObject		jcFeatherGlobals::useDelightDso;
MObject		jcFeatherGlobals::dsoName;
MObject		jcFeatherGlobals::tempFileDir;

MObject		jcFeatherGlobals::enableMotionBlur;
MObject		jcFeatherGlobals::motionBlurSamples;
MObject		jcFeatherGlobals::motionBlurStep;
MObject		jcFeatherGlobals::motionBlurFactor;
MObject     jcFeatherGlobals::useMultiThread;
MObject     jcFeatherGlobals::maxThreads;

#define MAKE_INPUT(attr,keyable)	\
    CHECK_MSTATUS( attr.setKeyable(keyable) );		\
    CHECK_MSTATUS( attr.setStorable(true) );	\
    CHECK_MSTATUS( attr.setReadable(true) );	\
    CHECK_MSTATUS( attr.setWritable(true) );

extern bool jcFea_UseMultiThread;
extern  int jcFea_MaxThreads;

void featherGlobalCC( MNodeMessage::AttributeMessage msg, MPlug & plug,  MPlug & otherPlug,void* )
{
	MFnDependencyNode fnNode(plug.node());
	if(fnNode.name()!="jcFeatherGlobals") return;
	if ( (msg & MNodeMessage::kAttributeSet) )
	{
		if(plug.partialName() == "umtd")
		{
			jcFea_UseMultiThread = plug.asBool();
			jcFea_MaxThreads = fnNode.findPlug("mtds").asInt();
		}
		if(plug.partialName() == "mtds") 
		{
			jcFea_MaxThreads = plug.asInt();
			jcFea_UseMultiThread = fnNode.findPlug("umtd").asBool();
		}
		if(jcFea_MaxThreads==0) jcFea_MaxThreads = jcFea_ProcessorNum;
		omp_set_num_threads(jcFea_MaxThreads);
	}
}

jcFeatherGlobals::jcFeatherGlobals() {}
jcFeatherGlobals::~jcFeatherGlobals() {}

void  jcFeatherGlobals::postConstructor ()
{
	MStatus stat;
	MObject obj=thisMObject();
	feaGlobalCI = MNodeMessage::addAttributeChangedCallback( obj,
                                            featherGlobalCC,
                                            NULL,
                                            &stat);
}

void  jcFeatherGlobals::nodeRemoved(MObject& node, void *clientData)
{
	MFnDependencyNode fnNode(node);
	MMessage::removeCallback( static_cast<jcFeatherGlobals*>( fnNode.userNode())->feaGlobalCI );
}

MStatus jcFeatherGlobals::compute( const MPlug& plug, MDataBlock& data )
{    return MS::kSuccess; }

void* jcFeatherGlobals::creator()
{	return new jcFeatherGlobals();  }

MStatus jcFeatherGlobals::initialize()
{	
	MStatus	stat;
	
	if( !featherTools::checkLicense() )
		return MS::kFailure;

	MFnNumericAttribute nAttr;
	useMultiThread = nAttr.create( "useMultiThread", "umtd", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,false);
	maxThreads = nAttr.create( "maxThreads", "mtds", MFnNumericData::kInt,0);
	nAttr.setMin(0);

	exportNormal = nAttr.create( "exportNormal", "expnor", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	relativePath = nAttr.create( "relativePath", "rp", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	gzipCompression = nAttr.create( "gzipCompression", "gzip", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	useDelightDso = nAttr.create( "dso", "dso", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	diceHair = nAttr.create( "diceHair", "dh", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	delayReadArchive = nAttr.create( "delayReadArchive", "dera", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	deleteTempFile = nAttr.create( "deleteTempFile", "deltf", MFnNumericData::kBoolean,1);
	MAKE_INPUT(nAttr,false);
	enableMotionBlur = nAttr.create( "enableMotionBlur", "enmobl", MFnNumericData::kBoolean,0);
	MAKE_INPUT(nAttr,false);
	motionBlurSamples = nAttr.create( "motionBlurSamples", "mbsamp", MFnNumericData::kInt,2);
	nAttr.setMin(2);
	nAttr.setMax(10);
	motionBlurStep = nAttr.create( "motionBlurStep", "mbs", MFnNumericData::kFloat,1);
	nAttr.setMin(0);
	motionBlurFactor = nAttr.create( "motionBlurFactor", "mbf", MFnNumericData::kFloat,1);
	nAttr.setMin(0);
	
	MFnTypedAttribute typAttr;

	MFnStringData strData;
	MObject tempObj = strData.create("jcFeather");
	featherRibText = typAttr.create( "featherRibText", "feart", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	tempObj = strData.create("jcRachisMesh");
	rachisMeshRib = typAttr.create( "rachisMeshRibText", "rchmeribt", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);

	tempObj = strData.create("%temp%");
	tempFileDir = typAttr.create( "tempFileDir", "tempfd", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);
	
	tempObj = strData.create("jcFeatherDelight");
	dsoName = typAttr.create( "dsoName", "dsoname", MFnData::kString,tempObj);
	MAKE_INPUT(typAttr,false);	
	
	CHECK_MSTATUS( addAttribute(featherRibText) );
	CHECK_MSTATUS( addAttribute(rachisMeshRib) );
	CHECK_MSTATUS( addAttribute(exportNormal) );
	CHECK_MSTATUS( addAttribute(relativePath) );
	CHECK_MSTATUS( addAttribute(gzipCompression) );
	CHECK_MSTATUS( addAttribute(diceHair) );
	CHECK_MSTATUS( addAttribute(delayReadArchive) );
	CHECK_MSTATUS( addAttribute(useDelightDso) );
	CHECK_MSTATUS( addAttribute(dsoName) );	
	CHECK_MSTATUS( addAttribute(deleteTempFile) );
	CHECK_MSTATUS( addAttribute(tempFileDir) );
	CHECK_MSTATUS( addAttribute(enableMotionBlur) );
	CHECK_MSTATUS( addAttribute(motionBlurSamples) );
	CHECK_MSTATUS( addAttribute(motionBlurStep) );
	CHECK_MSTATUS( addAttribute(motionBlurFactor) );
	CHECK_MSTATUS( addAttribute(useMultiThread) );
	CHECK_MSTATUS( addAttribute(maxThreads) );
	return MS::kSuccess;
}
