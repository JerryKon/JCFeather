#ifndef _jcFeatherComponentArray_
#define _jcFeatherComponentArray_

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include "featherTurtleData.h"

class jcFeatherComponentArray : public MPxLocatorNode
{
	public:
						jcFeatherComponentArray();
		virtual         ~jcFeatherComponentArray();
		virtual void	postConstructor(); 
		virtual MStatus compute( const MPlug&, MDataBlock& );

		inline  void drawComponent(const MColor &drawColor);
		//----locator draw
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		bool			isBounded() const{return false;}
		bool excludeAsLocator() const{return false;}
		static  void  nodeRemoved(MObject& node, void *clientData);
		static  void *creator();
		static  MStatus initialize();
		static  void addAndAffect();

		static  MTypeId id;
		static  MString typeName;

	public:
		static MObject displayLocator;
		static MObject walkStep;
		static MObject walkTimes;
		static MObject vertexGroup;

		static MObject leftVertexGroup;
		static MObject rightVertexGroup;
		static MObject exMeshFeather;

		static MObject useGrowVertexID;
		static MObject uvType;
		static MObject locatorSize;
		static MObject uvSetName;
		static MObject uArrayData;
		static MObject vArrayData;

		static MObject inMesh;
		static MObject inGrowMesh;
		static MObject outFeatherTurtleDataArray;
		
		static MObject outFeatherNumArray;
		static MObject outFeatherUArray;
		static MObject outFeatherVArray;
		static MObject outRootIdArray;
		static MObject outRootPositionArray;
	private:
		MMatrix           m_worldMatrix;
		MPointArray		  m_drawPoints;
};
#endif