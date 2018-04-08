#ifndef _jcPreController_
#define _jcPreController_

#include <maya/MPxLocatorNode.h>
#include <maya/MFloatVectorArray.h>
class jcPreController : public MPxLocatorNode
{
	public:
						jcPreController();
		virtual         ~jcPreController();
		void			postConstructor();
		virtual MStatus compute( const MPlug&, MDataBlock& );		
		void            draw(M3dView &view, 
							 const MDagPath &path, 
							 M3dView::DisplayStyle style, 
							 M3dView::DisplayStatus);
		void drawController(const MColor &col,float size,float colorScale);
		bool isBounded() const{ return false;}
		bool excludeAsLocator() const{ return false;}
		void inline drawSphereTriangle(float size);

		static  void *  creator();
		static  MStatus initialize();

		static  MTypeId id;
		static  MString typeName;
		
	public:
		static MObject locatorSize;
		static MObject displayShape;
		static MObject controllerRotate;
		static MObject controllerColorHue;//mesh
		static MObject bendCurvature;
		static MObject controllerWeightMap;
		static MObject outMapName;
	public:
		MFloatVectorArray shapeEdge;
		MFloatVectorArray rachisCurve;
};
#endif