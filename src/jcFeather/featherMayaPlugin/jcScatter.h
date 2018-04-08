#ifndef _jcScatter_
#define _jcScatter_

#include <maya/MPxLocatorNode.h>
#include <maya/MFloatPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MColorArray.h>
#include <vector>
#include "scatterPoints.h"
#include "scatterPointData.h"

class jcScatter : public MPxLocatorNode
{
	friend class jcFeatherToCurves;
	public:
						jcScatter();
		virtual         ~jcScatter();
		virtual void	postConstructor(); 

		virtual MStatus compute( const MPlug&, MDataBlock& );
		MStatus			getDataCompute( MDataBlock& block );

		void			drawScatterPoints(const MColor &col,float scale);
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		bool			isBounded() const{ return false;}
		bool excludeAsLocator() const{ return false;}

		static  void *  creator();
		static  MStatus initialize();
		static  void    nodeRemoved(MObject& node, void *clientData);
		void updateJCFeather();

		static  MTypeId id;
		static  MString typeName;

	public:

		// Input attributes
		//---------------------------------------------------------------------------shape attr
		static MObject  displayLocator;
		static MObject  locatorColorHue;

		static MObject  scatterMode;
		static MObject  numPerArea;

		static MObject  inPosition;
		static MObject  maxDistance;

		static MObject  uvSegment;
		static MObject  uvOffset;

		static MObject  jitterDistance;
		static MObject  jitterFrequency;

		static MObject  baldnessTex;

		static MObject  randomSeed;
		static MObject  uvSetName;
		
		static MObject  customerUVData;
		static MObject  customerUArrayData;
		static MObject  customerVArrayData;

		static MObject  inComponentList;
		static MObject  inputGrowMesh;

		static MObject  outGrowData;
		static MObject  pointSize;
	private:
		MFloatPointArray _displayPts;
		MMatrix           m_worldMatrix;
};
#endif