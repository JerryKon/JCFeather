#ifndef _preFeatherMesh_
#define _preFeatherMesh_

#include "mathFunctions.h"
#include "featherInfo.h"
#include "randomc.h"
#include "noise.h"
#include "scatterPoints.h"

#include <vector>
#include <map>
#include <limits>
#include <maya/MFnNurbsSurface.h>
extern bool jcFea_UseMultiThread; 
extern int jcFea_ProcessorNum;
extern int jcFea_MaxThreads;

//---------------------�������߲�ֵ����ë�����򣬲����ģ��
//---------------------------------------------

typedef struct 
{
	MPointArray _pts;
	MFloatArray _u;
	MFloatArray _v;

	MVectorArray _dir;
	MVectorArray _nor;

	MVectorArray _rotate;
	MVectorArray _scale;

	MDoubleArray _bendCur;

	void clear()
	{
		_pts.clear();
		_u.clear();
		_v.clear();

		_dir.clear();
		_nor.clear();

		_rotate.clear();
		_scale.clear();

		_bendCur.clear();
	}
}preFeatherGrowPosInfo;

class preFeatherAttr
{
	public:
		preFeatherAttr();
		~preFeatherAttr();

		void init();
		void initVerDirTable(const verDirVector  &verDirPoint);//map direction data to a std::map<int,int>, used in jcPreFeather
		void initScatterData(jcScatterPointData *scatData);//get the scatter data ,used in jcPreFeather
		void prepareData();
		bool GenMesh( MObject &meshData);
		void GenFeatherTurtleArray(turtleArrayData &outTurtleArray);
		void getScaleTexture();
	private:
		void computeVerDirDatas( map<int,MVector> &idDir,MVectorArray &verNormals);//store the vertex direction to a map and get all vertex normals
		bool getVerDirData(int id, map<int,MVector> &idDir,MVector &dir); //get direction of the specified id 

		bool getTriangleIntpDir(    const jcTri3Int &currentTriId,
									map<int,MVector> &mapVector,
									double weight[],
									MVector &outDir);//get the interpolated direction of the point using the input weight.
		bool GenMeshBase( polyObj &finalPoly);
		bool GenTurtleArrayBase(turtleArrayData &outTurtleArray);

		void getGrowPosData();
		bool getVertexDir(int verId,MVector &dir);

		inline double getRotNoise(const MPoint &pos,int seed)
		{
			return jerryC::ValueCoherentNoise3D( (pos.x+_rotNoisePha[0])*_rotNoiseFre,(pos.y+_rotNoisePha[1])*_rotNoiseFre,(pos.z+_rotNoisePha[2])*_rotNoiseFre,seed,1);
		}
		void getTriangleIntpRotScal(const jcTri3Int &triIds,double triWeight[],MVector &rot,MVector &scal,double &curvature);
		
		//-----------control method
		void getVertexWeightMapValues();
		void getInterpolateWeightData(unsigned int index,MVector &outRot,MVector &outScale,double &curvature);

		void bendPoint(const MFloatPointArray &inPts,float xpos,float curvature,MFloatPointArray &outPts);
	public:
		float			_globalScale;
		float			_randScale;
		float			_rotateOffset[3];
		bool			_useRotNoise;
		float			_rotNoiseFre;
		float			_rotNoiseAmp[3];
		float			_rotNoisePha[3];
		MTransformationMatrix::RotationOrder _rotOrder;

		int				_randSeed;
		MString			_uvSetName;
		MString			_scaleTexture;//scale texture for each interpolated feather mesh
		MFloatVectorArray _texScaleData;


		MVectorArray	_vertexRotate;//[-3.1416,3.1416]
		MVectorArray	_vertexScale;//[0,+infinity]

		MStringArray	_controlColorMaps;//controller weight maps
		MVectorArray	_rotateArray; //contoller rotate
		MVectorArray	_scaleArray;//controller scale
		MFloatArray		_controlBend;
		std::vector<MFloatArray> _mapIlluminaceValues;
		MIntArray		_mapIlluminaceIndex;

		MObject			_instanceMesh;
		MObject			_inGrownMesh;
		turtleDataInfo	_instanceTurtleData;

		bool			_bendActive;
		float			_bendPosition;//[0,1]
		float			_bendCurvature;//[0,+infinity]
		float			_bendRandomCurvature;//[0,+infinity]
		MDoubleArray	_vertexBendCurvature;
		MDoubleArray	_vertexBendPosition;

	private:
		jcScatterPointData*		_scatterData; //scatter point position data.
		std::map<int,int>		_verDirTable;//each vertex id has a target vertex id to get the direction.
		preFeatherGrowPosInfo	_pointGrowInfo;
};

#endif
