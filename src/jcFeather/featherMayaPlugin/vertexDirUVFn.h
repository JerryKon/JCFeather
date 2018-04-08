
#ifndef _VertexDirUVFn_
#define _VertexDirUVFn_

#include "mathFunctions.h"
#include "featherInfo.h"
#include "randomc.h"
#include "noise.h"

#include <vector>
#include <map>
#include <limits>

//----used in jcPreGuid node
class vertexDirUVFn
{
	public:
		vertexDirUVFn();
		~vertexDirUVFn();
		void init();
		void getVertexDir( verDirVector &outVertexGuid ,
						   vector<pointRay> &displayLine);
	protected:
		bool getClosestDirection(const MPoint &pt,MVector &dir);
		bool getClosestConnectedVertex( const MIntArray &verIdAry,
										const MVector &curveDir,
										const MPointArray &allPts,
										const MPoint &pt,
										int &dirVerid);
	public:
		unsigned int _guidsPerVertex;
		float _searchCurveRadius;
		float _curvePower;
		float _tolerance;
		float _maxAngle;

		MObjectArray _inGuidCurves;
		MObject _inGrownMesh;
		MObject _growComponent;
};

#endif