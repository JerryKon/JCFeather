
#ifndef _ScatterPoints_
#define _ScatterPoints_

#include "mathFunctions.h"
#include "featherInfo.h"
#include "randomc.h"
#include "noise.h"

#include <vector>
#include <map>
#include <limits>
#include <maya/MFnNurbsSurface.h>
#include <maya/MItMeshPolygon.h>

using namespace jerryC;
using namespace std;

class jcScatterPointData
{
	public:
		jcScatterPointData(){}
		~jcScatterPointData(){}
		void init()
		{
			_u.clear();
			_v.clear();
			
			_localPos.clear();
			_triangleId.clear();
			_pointFaceIndex.clear();

			_faceId.clear();
			_pointsOnFace.clear();
		}
		jcScatterPointData operator= ( const jcScatterPointData &other);
		static bool writeDataTo(const MString &file, jcScatterPointData &sctData);
		static bool readDataFrom(const MString &file, jcScatterPointData &sctData);
	public:
		MFloatArray _u;
		MFloatArray _v;

		vector<jcPoint2D> _localPos;//local position of point in triangle
		vector<jcTri3Int> _triangleId;//triangle vertices Id on the polygon face for each _u _v point, x y z means the vertex id for each point of the triangle
		MIntArray _pointFaceIndex;//the face index in _pointsOnFace for each point in _u

		MIntArray _faceId;//all faces those have points on them
		intArrayVector _pointsOnFace;//uv ids in each face ,index in _faceId
};

class scatterPoints
{
	public:
		scatterPoints(){init();}
		~scatterPoints(){}
		void init();

		MStatus getFinalPosition( MFloatPointArray &outPos);
	protected:
		MStatus getUVPositionsFromMesh( MFloatPointArray &outPositions);
		MStatus getAreaPositionsFromMesh( MFloatPointArray &outPositions);
		MStatus getUserInPosition( MFloatPointArray &outPositions );
		MStatus addUsrUVData(MFloatPointArray &outPositions);
		void getComponentFace(MIntArray &faceIds);
		void computeBaldTexture(MFloatPointArray &outPos);
	public:
		short _scatterMode;
		double _ptNumPerArea;
		int _randSeed;
		double _maxDistance;

		MObject _inMesh;
		MObject _inComponentList;
		MString _uv;
		MString _baldTex;

		MFloatArray _usrU;
		MFloatArray _usrV;
		MPointArray _usrInPosition;

		jcScatterPointData _outScatterData;
	private:
		bool _faceFromComponent;
};

#endif
