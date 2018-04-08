#ifndef _jcFeatherComponent_
#define _jcFeatherComponent_

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include "featherTurtleData.h"

class jcFeatherComponent : public MPxLocatorNode
{
	public:
						jcFeatherComponent();
		virtual         ~jcFeatherComponent();
		virtual void	postConstructor(); 
		virtual MStatus compute( const MPlug&, MDataBlock& );

		inline  void drawComponent(const MColor &drawColor);
		//----locator draw
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		bool			isBounded() const{return false;}
		//MBoundingBox    boundingBox() const; 
		bool excludeAsLocator() const{return false;}
		static  void  nodeRemoved(MObject& node, void *clientData);
		static  void *creator();
		static  MStatus initialize();

		static  MTypeId id;
		static  MString typeName;

	public:
		static MObject displayLocator;
		static MObject inMesh;
		static MObject vertexGroup;
		static MObject featherUV;
		static MObject locatorSize;

		static MObject leftVertexGroup;
		static MObject rightVertexGroup;
		static MObject exMeshFeather;

		static MObject outVertexGroup;
		static MObject outFeatherTurtleData;
	private:
		MMatrix           m_worldMatrix;
		MPointArray		  m_drawPoints;
};
#endif