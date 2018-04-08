#ifndef _jcFeatherSystem_
#define _jcFeatherSystem_

#include <maya/MPxLocatorNode.h>
#include <maya/MFloatPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MColorArray.h>
#include <vector>
#include "featherGen.h"
#include "jcFeather.h"

class jcFeatherSystem : public MPxLocatorNode
{
	friend class jcFeatherToCurves;
	public:
						jcFeatherSystem();
		virtual         ~jcFeatherSystem();
		virtual void	postConstructor(); 

		virtual MStatus compute( const MPlug&, MDataBlock& );
		void            makeFeathers(MDataBlock&);
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		bool			isBounded() const;
		MBoundingBox    boundingBox() const;
		static  void *  creator();
		static  MStatus initialize();
		static  void    nodeRemoved(MObject& node, void *clientData);
		bool excludeAsLocator() const;
		void updateJCFeather();

		static  MTypeId id;
		static  MString typeName;

	public:

		// Input attributes
		//---------------------------------------------------------------------------shape attr
		static MObject  displayPercent;  //opengl显示百分比  int
		static MObject  displayBarbulePercent;
		static MObject  renderQuality;

		static MObject  active;//是否进行计算 bool
		static MObject  outputFeatherMesh;

		static MObject  useUVIFile;
		static MObject  uviFile;

		static MObject  uSegment;
		static MObject  vSegment;

		static MObject  uvOffset;
		static MObject  uvTranslate;

		static MObject  jitterDistance;
		static MObject  jitterFrequency;

		static MObject  guidsPerFeather;
		static MObject  gloalScale;
		static MObject  randScale;
		static MObject  radius;
		static MObject  power;

		static MObject  colorTex;
		static MObject  scaleTex;
		static MObject  baldnessTex;
		static MObject  radiusTex;

		static MObject  randomSeed;
		static MObject  uvSetName;

		static MObject  growthSet;
		static MObject  inputGrowSurface;

		static MObject  inputGrowMesh;
		static MObject  inputGrowMeshID;

		static MObject  inRenderFeather;
		static MObject  inFeatherMesh;
		static MObject  outFeatherMesh;

		static MObject  outRachisMesh;
		static MObject  outBarbuleMesh;

		static MObject  displayProxyBoundingBox;

	private:
		std::vector< singleFeather > featherDraw;
		MMatrix           m_worldMatrix;
		MColorArray store4Color;
};
#endif