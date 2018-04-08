#ifndef _jcFeatherGlobals
#define _jcFeatherGlobals

#include <maya/MPxNode.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MNodeMessage.h>

class jcFeatherGlobals : public MPxNode
{
public:
						jcFeatherGlobals();
	virtual				~jcFeatherGlobals(); 
	virtual void  		postConstructor ();
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	static  void        nodeRemoved(MObject& node, void *clientData);

	static  void*		creator();
	static  MStatus		initialize();
public:
    static  MObject     featherRibText; 
	static  MObject     rachisMeshRib;
	static  MObject     diceHair;

	static  MObject     exportNormal;

	static  MObject     relativePath;
	static  MObject     gzipCompression;
	static  MObject     delayReadArchive;
	static  MObject     deleteTempFile;
	static  MObject     useDelightDso;
	static  MObject     dsoName;
	static  MObject     tempFileDir;

	static  MObject     useMultiThread;
	static  MObject     maxThreads;

	static  MObject     enableMotionBlur;
	static  MObject     motionBlurSamples;
	static  MObject     motionBlurStep;
	static  MObject     motionBlurFactor;
	static	MTypeId		id;
	static  MString		typeName;

	MCallbackId  feaGlobalCI;
};

#endif