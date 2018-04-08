#ifndef _jcPreGuid_
#define _jcPreGuid_

#include "vertexDirUVFn.h"
#include "vertexDirectionData.h"
#include <maya/MPxLocatorNode.h>
class jcPreGuid : public MPxLocatorNode
{
	public:
						jcPreGuid();
		virtual         ~jcPreGuid();
		virtual MStatus compute( const MPlug&, MDataBlock& );		
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);

		bool			isBounded() const{ return false;}
		bool excludeAsLocator() const{ return false;}
		void assignToOutputAttr(MDataBlock& block);
		MStatus storeDataAndCompute(MDataBlock& block);

		void drawGuidVertexLine(const MColor &col,float scale);

		static void  nodeRemoved(MObject& node, void *clientData);
		static  void *  creator();
		static  MStatus initialize();

		static  MTypeId id;
		static  MString typeName;

	public:
		static MObject displayLocator;
		static MObject locatorColorHue;
		static MObject inGrowMesh;//mesh
		static MObject inComponentList;//componentlist
		static MObject inGuidCurves;//array

		static MObject  guidsPerVertex;
		static MObject  searchCurveRadius;
		static MObject  curvePower;
		static MObject  maxAngle;

		static MObject  sampleTolerance;
		static MObject  outVertexDir;

	private:
		std::vector<pointRay> guidResult;
		MMatrix           m_worldMatrix;
};
#endif