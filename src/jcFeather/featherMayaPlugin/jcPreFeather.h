#ifndef _jcPreFeather_
#define _jcPreFeather_

#include "preFeatherMesh.h"
#include "vertexDirectionData.h"
#include "scatterPointData.h"
#include "featherTurtleArrayData.h"
#include "featherTurtleData.h"
#include <maya/MPxLocatorNode.h>
#include <maya/MPlugArray.h>

class jcPreFeather : public MPxLocatorNode
{
	public:
						jcPreFeather();
		virtual         ~jcPreFeather();
		virtual void	postConstructor(); 
		virtual MStatus compute( const MPlug&, MDataBlock& );		
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		bool			isBounded() const{ return false;}
		bool excludeAsLocator() const{ return false;}
		static void addMayaAttributes();
		MStatus initPreFeatherInfo(MDataBlock& block);
		MStatus getInGuideScatter(MDataBlock& block,
								  vertexDirectionData* verDirPoint,
								  scatterPointData*  sctPoint);
		static  void  nodeRemoved(MObject& node, void *clientData);
		static  void *  creator();
		static  MStatus initialize();

		static  MTypeId id;
		static  MString typeName;

	public:
		static MObject  active;

		static MObject  guideFileName;
		static MObject  scatterFileName;

		static MObject inScatterData;
		static MObject inVertexDir;

		static MObject  globalScale;
		static MObject  randScale;
		static MObject  rotateOrder;
		static MObject  rotateOffset;
		static MObject  useRotateNoise;
		static MObject  rotateNoiseFrequency;
		static MObject  rotateNoiseAmplitude;
		static MObject  rotateNoisePhase;
		
		static MObject  bendActive;
		static MObject  bendPosition;
		static MObject  bendCurvature;
		static MObject  bendRandCurvature;

		static MObject  randSeed;
		static MObject  uvSetName;
		static MObject  interactiveScale;
		static MObject  scaleTexture;
		
		static MObject  paintRotateX;
		static MObject  paintRotateY;
		static MObject  paintRotateZ;
		static MObject  paintScaleX;
		static MObject  paintScaleY;
		static MObject  paintScaleZ;
		static MObject  paintBendCurvature;
		//-----
		static MObject controlInfo;//--compound array
		static MObject controlRotate;
		static MObject controlRotateX;
		static MObject controlRotateY;
		static MObject controlRotateZ;
		static MObject controlScale;
		static MObject controlScaleX;
		static MObject controlScaleY;
		static MObject controlScaleZ;
		static MObject controlBend;
		static MObject controlWeightMap;

		static MObject inGrowMesh;//mesh
		static MObject instanceMesh;//feather template,mesh
		static MObject instanceFeatherTurtle;

		//----output attributes
		static MObject  outFeatherMesh;
		static MObject  outFeatherTurtleArray;
	public:
		preFeatherAttr preFeatherInfo;
	private:
		bool useVerDirFile;
		bool useScatterFile;
};
#endif