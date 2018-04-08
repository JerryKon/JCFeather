#ifndef _featherInfo_
#define _featherInfo_

#include <vector>
#include <list>
#include <map>
#include <sstream>

#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>
#include <maya/MPointArray.h>
#include <maya/MRampAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MStringArray.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MFloatMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MRenderUtil.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MDagPath.h>
#include <maya/M3dView.h>
#include <maya/MObjectArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MTime.h>
#include <boost/serialization/vector.hpp>

#include "singleFeather.h"
#include "noise.h"
#include "mathFunctions.h"
#include "boostTools.h"
#include "splineLib.h"
#include "randomc.h"
#include "cell.h"
#include "pointInPolygon.h"

using namespace std;
using namespace jerryC;


class jcRamp:public jcRampBase
{
	public:
		jcRamp();
		jcRamp(MRampAttribute &rampAttr);
		~jcRamp();
		void assign( MRampAttribute &other);
		jcRamp& operator=( const MRampAttribute &other);
};

class lsysTurtle
{
	public:
		MFloatPoint currentPos;
		MFloatVector currentDir;
		MFloatVector currentUp;
	public:
		lsysTurtle();
		lsysTurtle(const MFloatPoint &pos,const MFloatVector &forward,const MFloatVector &up);
		~lsysTurtle();
		void lsysInit(float pos);
		void lsysInit(float fowardPos,float lrPos);
		void assign(const lsysTurtle &turtle);
		MFloatMatrix getMatrix();//����������µ���ǰ����µľ���任
		lsysTurtle getTurtleInThis(lsysTurtle lturtle);//���������ת��Ϊ��turtle�µ����
		lsysTurtle getTurtleInWorld(lsysTurtle lturtle);//��getTurtleInThis()�õ��Ľ��ת��Ϊ�������
};

class keyTurtle
{
	public:
		MFloatPoint currentPos;
		MFloatVector currentUp;
	public:
		keyTurtle();
		~keyTurtle();
		void setValue(const MFloatPoint &pos,const MFloatVector &up);
		void assign(const keyTurtle &turtle);

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			float temp=0;
			temp = currentPos.x;ar<<temp;
			temp = currentPos.y;ar<<temp;
			temp = currentPos.z;ar<<temp;

			temp = currentUp.x;ar<<temp;
			temp = currentUp.y;ar<<temp;
			temp = currentUp.z;ar<<temp;
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			float temp=0;
			ar>>temp; currentPos.x=temp;
			ar>>temp; currentPos.y=temp;
			ar>>temp; currentPos.z=temp;
			ar>>temp; currentUp.x=temp;
			ar>>temp; currentUp.y=temp;
			ar>>temp; currentUp.z=temp;
		}

		template<class Archive>
		void serialize(	Archive & ar,	const unsigned int file_version )
		{	
			boost::serialization::split_member(ar, *this, file_version);
		}

};

class turtles
{
	public:
		void addKeyTurtle( const keyTurtle &kt );
		void setExMeshPoints(const MFloatPointArray &lpts,const MFloatPointArray &rpts);
		void clearKeyTurtle();
		turtles();
		~turtles();

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			float tempF=_scale;				ar << tempF;
			int tempI=_exMeshFea;			ar << tempI;
			tempI=_turtles.size();			ar << tempI;
			
			for(int ii=0;ii<tempI;++ii)
			{
				ar<<_turtles[ii].currentPos.x;
				ar<<_turtles[ii].currentPos.y;
				ar<<_turtles[ii].currentPos.z;

				ar<<_turtles[ii].currentUp.x;
				ar<<_turtles[ii].currentUp.y;
				ar<<_turtles[ii].currentUp.z;
			}

			if(_exMeshFea)
			{
				tempI=_leftPoints.length();			
				ar << tempI;
				for(int ii=0;ii<tempI;++ii)
				{
					ar<<_leftPoints[ii].x;
					ar<<_leftPoints[ii].y;
					ar<<_leftPoints[ii].z;
				}

				tempI=_rightPoints.length();			
				ar << tempI;
				for(int ii=0;ii<tempI;++ii)
				{
					ar<<_rightPoints[ii].x;
					ar<<_rightPoints[ii].y;
					ar<<_rightPoints[ii].z;
				}
			}
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			int tempI=0;
			ar >> _scale;	
			ar >> tempI;	_exMeshFea=tempI;

			int tsz=0;
			ar >> tsz;
			float tempf=0;
			_turtles.resize(tsz);
			for(int ii=0;ii<tsz;++ii)
			{
				ar>>tempf;_turtles[ii].currentPos.x=tempf;
				ar>>tempf;_turtles[ii].currentPos.y=tempf;
				ar>>tempf;_turtles[ii].currentPos.z=tempf;

				ar>>tempf;_turtles[ii].currentUp.x=tempf;
				ar>>tempf;_turtles[ii].currentUp.y=tempf;
				ar>>tempf;_turtles[ii].currentUp.z=tempf;
			}
			if(_exMeshFea)
			{	
				ar >> tempI;
				_leftPoints.setLength(tempI);
				
				for(int ii=0;ii<tempI;++ii)
				{
					ar>>tempf; _leftPoints[ii].x=tempf;
					ar>>tempf; _leftPoints[ii].y=tempf;
					ar>>tempf; _leftPoints[ii].z=tempf;
				}

				ar >> tempI;
				_rightPoints.setLength(tempI);
				for(int ii=0;ii<tempI;++ii)
				{
					ar>>tempf; _rightPoints[ii].x=tempf;
					ar>>tempf; _rightPoints[ii].y=tempf;
					ar>>tempf; _rightPoints[ii].z=tempf;
				}
			}
		}

		template<class Archive>
		void serialize(	Archive & ar,	const unsigned int file_version )
		{	
			boost::serialization::split_member(ar, *this, file_version);
		}

		void copyToTurtles(turtles &newTurtle);
		void getRachis(singleFeather &fea);
			//�õ�rachis��ʵ�ʳ��ȣ���start��end��0��1֮�����·���ȷֳ���
			//���պ���0,1�������ȡ��ֱ�ӷ��ص���ֵ�������Ӧ�ֲ�,linear
		void getPosition(lsysTurtle &lsyst,float givePercent);//�õ���ǰLϵͳ��turtle��λ�úͷ��򣬲����ظ�barbuleλ�õ���ë�Ŀ��
	public:
		 vector<keyTurtle> _turtles;//һ����ë�����������
		
		 MFloatPointArray _leftPoints;
		 MFloatPointArray _rightPoints;
		 
		 float _scale;
		 bool _exMeshFea;
};

class sysControlData
{
public:
	long _featherID;//��ϵͳ��ʹ�õ�feather ID��
	bool _outputFeatherMesh;
	int _dispPer;//��ʾ�ٷֱ�
	int _renderQuality;//��Ⱦ�ٷֱ�
	int _guidPF;//ÿһ����ɳ�������ë�ж��ٸ����Բ�ֵ���
	float _globalScale;//������ë����������

	bool _useUVI;
	MString _uviFile;
	MString _uvSet;
	float2 _translate;

	float _randScale;//������ſ���
	float _radius;//������ë���Ʒ�Χ
	float _power;
	int _seed;//�������

	MStringArray _textures;//0 scale texture ��ë������ͼ
						   //1 baldness texture ��ë�Ƿ�����ͼ
						   //2 color texture ��ë��ɫƫ����ͼ
						   //3 radius texture ��ë�����뾶��ͼ
public:
	sysControlData();
	~sysControlData();
	void init();
};

class keyBarbule
{
	public:
		float stepAngle; //basic degree
		std::vector<float> startAngle;//���ĸ�
		std::vector<float> barbule;//���ĸ� barbule position
		std::vector<jcRamp> ramp;//���ĸ�
	public:
		keyBarbule();
		~keyBarbule();
		void copy(const keyBarbule &other);
		void pushRamp(const jcRamp &rmp1,const jcRamp &rmp2,const jcRamp &rmp3,const jcRamp &rmp4);
		void pushRamp(MRampAttribute &rmp1,MRampAttribute &rmp2,MRampAttribute &rmp3,MRampAttribute &rmp4);
		void clear();
		float getStartAngleAt(float uPos);
		float getAngleAt(float uPos,float vPos);
};

class shaderInfo
{
	public:
		shaderInfo();
		~shaderInfo();
		void copy(const shaderInfo &other);
	public:
		MColor _rachisRootColor;
		MColor _rachisTipColor;

		MColor _rootColor;
		MColor _tipColor;
		float _baseOpacity;
		float _fadeOpacity;
		float _fadeStart;
		bool _uvProject;
		float _uvProjectScale;
		MString _barbuleTexture;//texture path to sample color
		MString _texProcedure;

		float _barbuleDiffuse;//barbule diffuse for renderman shader
		float _barbuleSpecular;//barbule specular for renderman shader
		MColor _barbuleSpecularColor;//barbule specular color for renderman shader
		float _barbuleGloss;//barbule gloss for renderman shader
		float _selfShadow;

		int _hueVar;
		float _satVar;
		float _valVar;
		float _varFreq;
};

struct featherRenderAttr
{
	bool _castsShadow;
	bool _receiveShadow;
	bool _motionBlur;
	bool _primaryVisibility;
	bool _visibleInReflections;
	bool _visibleInRefractions;
	void init()
	{
		_castsShadow=true;
		_receiveShadow=true;
		_motionBlur=true;
		_primaryVisibility=true;
		_visibleInReflections=true;
		_visibleInRefractions=true;
	}
};

struct barbuleGapData
{
	vector<int> _gapPosition;
	vector<float> _gapForce;
	void clear()
	{
		_gapPosition.clear();
		_gapForce.clear();
	}
};

class featherInfo
{
	public:
		featherInfo();
		~featherInfo();
		void init();
		bool copyData(featherInfo *other);
		void clear();

	public:
		 std::list<long> _turtleChildrenID;//�洢ʹ�����feather��ݵ�nurbs��Ƭ��id��,Ϊ������ʱȥ������id�ţ��ʲ���list,��vector

		 short _curveType;//0 linear 1 cubic
		 bool _exactFeather;
		 short _direction; //0 u 1 v grow direction
		 bool _renderable;

		 float _rachisPos; //rachis position,default in the middle line
		 short _rachisRenderAs;//render rachis as curve or tube 0,1
		 int _rachisSegment;//how many segments a rachis has ,����ֵΪ5ʱ�������4�Σ�5����
		 int _rachisSides;//how many sides for nurbs tube render
		 float _rachisThick;//thickness of the rachis
		 float _rachisStart;//rachis start position
		 float _rachisEnd;//rachis end position

		 int _barbuleNum;//how many barbules a feather has each side,in display mode, this data will be (_renderBarbuleNum*_displayPercent)
		 int _renderBarbuleNum;//the real data from jcFeather node
		 float _displayPercent;
		 int _barbuleSegments;//how many barbule segments a barbule has,����ֵΪ5ʱ�������4�Σ�5����
		 float _barbuleLength;//barbule length
		 float _barbuleLengthRandScale;//barbule length rand scale
		 bool _shapeSymmetry;

		 float _forcePerSegment;//each segment a random force will be added to the current force
		 float _turnForce;//when the current force is bigger than this force ,the turtle will rotate a random degree
		 float _rotateLamda;//multiply this num to the random degree to get the real rotate angle
		 float _upDownLamda;
		 bool  _upDownNoise;//whether add noise to barbule updown

		 float _rachisNoiseFrequency;//use this to control the rotate dir and rotate force
		 float _barbuleNoiseFrequency;
		 float _noisePhase[2];//noise phase

		 //----------barbule gap attr
		 float _gapForce[2];
		 float _gapMaxForce[2];
		 float _gapSize[2];

		 float _barbuleThick;//barbule thickness
		 float _barbuleRandThick;
		 bool _uniformWidth;

		 float _proxyBoundingBoxScale[3];

		 shaderInfo _shader;//��Ⱦ��������

		 int _randSeed;
		 bool _outputFeatherMesh;
		 float _meshUVScale[2];

		 bool _useOutShader;
		 MObject _shadingEngineNode;
		 MObject _shaderNode;
		 MString _shaderName;

		 keyBarbule _keyBarAttrs;
		 jcRamp _rachisThickScale;

		 jcRamp _barbuleDensity;
		 jcRamp _leftBarbuleLengthScale; //count num is _barbuleNum, work with _barbuleLength and _barbuleLengthRandScale
		 jcRamp _rightBarbuleLengthScale;//count num is _barbuleNum, work with _barbuleLength and _barbuleLengthRandScale

		 jcRamp _barbuleUpDownScale;
		 jcRamp _barbuleUpDownRachis;
		 jcRamp _forceRotateScale;
		 jcRamp _forceScale;//forceScale perSegment

		 jcRamp _gapDensity[2];
		 
		 jcRamp _barbuleThickScale;//barbule thick scale ,work with _barbuleThick
		 jcRamp _barbuleThickAdjust;

		 MString _preFeatherRib;//feather renderman shader
		 MString _preRachisMeshRib;
		 MString _postFeatherRib;
		 MString _postRachisMeshRib;
};

struct compressionInfo
{
	bool doCompress;
	int level;
};

//MPxData featherTurtleData
class turtleDataInfo
{
public:
	MVectorArray _pos;
	MVectorArray _nor;

	MFloatPointArray _leftPt;
	MFloatPointArray _rightPt;

	float _featherUV[2];
	bool _exMeshFea;

	turtleDataInfo();
	~turtleDataInfo();
	void clear();
	void assign(const turtleDataInfo &other);
	void transform(const MMatrix &matrix,turtleDataInfo &result);
};

//MPxData featherTurtleArrayData;
typedef std::vector<turtleDataInfo> turtleArrayData;

//---------------------MPxData vertexDirectionData
struct vertexDirectionInfo
{
	int _vertexId;
	int _dirVertexId;
};

typedef std::vector<vertexDirectionInfo> verDirVector;

//-----------------------vertexDirUVFn
struct pointRay
{
	MPoint _startPt;
	MPoint _endPt;
};
//---------------------MPxData vertexDirectionData

typedef std::vector<MIntArray> intArrayVector;

//----��feather �������õ���һЩ�����
class featherTools
{
	public:
		static bool checkLicense();//���֤��

		//---------------------------------------------preguid prefeather�ڵ����õ����㷨
		//---------------------------------------------
		//---------------------------------------------
		static bool writePreGuideFile( const MString &file,verDirVector &vdData);
		static bool getPreGuideFromFile(const MString &file,verDirVector &vdData);
		static bool getTurtles(const turtleDataInfo &tdInfo,turtles *result);
		
		static MStatus computeDataForUVI(MDagPath &particle,MDagPath &mesh,
									MFloatArray &ua,
									MFloatArray &va,
									MIntArray &faceIndexAry,
									MIntArray &triangleVertexList);
		static MStatus computeDataForPointDir(const MVectorArray &pos,
											MObject &mesh,
											MString &uvSetName,
											MFloatArray &ua,
											MFloatArray &va,
											MIntArray &faceIndexAry,
											MPointArray &closestPt,
											float toler);
		static bool getUVIFromFile(MString &uvFile,MFloatArray &ua,MFloatArray &va,MIntArray &faceIndexAry);


		//--------------------------------------------------jcComponent jcComponentArray�ڵ����õ���һϵ���㷨
		//--------------------------------------------------
		//--------------------------------------------------
		//--------------------------------------------------
		//���ַ��л�ȡ����id��
		static void computeStringIDArray(const MString &inStr,MIntArray &outVertex,unsigned int maxVerid);
		//���ַ��а���һ���ĵ�����ʽ��ȡ����id��
		static void computeStringMultiArray(int step,int num,const MString &inStr,intArrayVector &outVertex,int maxVerid);


		//��id�ţ��õ�λ�����
		static void getPFromMesh(const MFnMesh& meshFn,	const MIntArray& verts,	MFloatPointArray &outPoint);
		//��id�ţ��õ�λ�ã��������
		static void getPNFromMesh(const MFnMesh& meshFn,const MIntArray& verts,	MVectorArray &outPoint,	MVectorArray &outNormal,MString &outStr);
		//�õ���ë���ߵı�Եλ����ݣ�����exmesh feather
		static void getMultiSidePFromMesh(const MFnMesh& meshFn,  const intArrayVector &inVertex, turtleArrayData &outTurtles, bool left);
		//componentArray�ڵ��еõ�λ�ã��������
		static void getMultiPNFromMesh(const MFnMesh& meshFn, const intArrayVector &inVertex, turtleArrayData &outTurtles );
		//componentArray�ڵ��л�ȡλ�ã����ߣ���������uv���
		static void getMultiPNFromMesh(const MFnMesh& meshFn,
							   const intArrayVector &inVertex,
							   turtleArrayData &outTurtles,
							   const MFloatArray &uValue,
							   const MFloatArray &vValue,
							   const MIntArray &verValue
							   );

		static void getTurtle_UVData( turtleArrayData &inTurtles,
								MFnMesh& meshFn,
								MString *uvSetName );
		static void getFeaComponentDrawPoints(	const MVectorArray &outPoint,
										const MVectorArray &outNormal,
										float locs,
										MPointArray &drawPts);
		static void getComponentList_verId(MObject &mesh,
							MObject &componentList,
							MIntArray &outVertexId);
		static void getComponentIds(const MFnMesh &growMeshFn,
									MObject &componentList,
									MIntArray &outId,
									int componentType);



		//-------------------------------------Maya С����
		//-------------------------------------
		//-------------------------------------
		static bool isObjectVisible(MObject &obj);
		static MStatus nodeFromName(MString name, MObject & obj);//�����ֵõ�MObject
		static MStatus dagPathFromName(MString name, MDagPath & obj);//�����ֵõ�dagPath
		static MString checkNodeName(const MString &name);//��maya�ڵ������ȥ���㼶�ṹ�Լ�����ռ�
		static void getMinMax(const MFloatArray &fa,int &minID,int &maxID);//�����������С��id��
		static float getColorHSV(const MColor &col,int hsvIndex);//colorתhsv
		static std::string numtostr(const double value);//doubleתstring
		static MString getPadding(int f);//������ת��һ����ȵ��ַ�,1 --> 0001
		static MStatus sampleTexture(MString nodeName,
									 MFloatArray *uCoord,
									 MFloatArray *vCoord,
									 MFloatVectorArray &resultColors,
									 MFloatVectorArray &resultTransparencies);//������ͼ��color alpha
		static float noiseRandom(int x,int y,int z,int seed);

		//�������Ƿ��ĳ��ֵ
		template <typename TARRAY,typename T>
		static bool arrayContain(const TARRAY &sourceA,const T &test)
		{
			unsigned int num=sourceA.length();
			for(unsigned int ii = 0; ii < num; ++ii )
				if(test==sourceA[ii]) 
					return true;
			return false;
		}
	
		//���������ת�����ַ�
		template <typename T>
		static MString convertPointArrayToString(const T &dataArray,int &countEndl,bool useBothEnds)
		{
			unsigned int len=dataArray.length();

			MString tempStr;
			if(len==0) return tempStr;

			if(useBothEnds==1)
			{
				tempStr+=" ";
				tempStr+=dataArray[0].x;
				tempStr+=" ";
				tempStr+=dataArray[0].y;
				tempStr+=" ";
				tempStr+=dataArray[0].z;
			}

			unsigned int ii=0;
			for(;ii<len;++ii)
			{
				tempStr+=" ";
				tempStr+=dataArray[ii].x;
				tempStr+=" ";
				tempStr+=dataArray[ii].y;
				tempStr+=" ";
				tempStr+=dataArray[ii].z;
				if( (countEndl++)%7==0) tempStr+="\n";
			}

			if(useBothEnds==1)
			{
				tempStr+=" ";
				tempStr+=dataArray[len-1].x;
				tempStr+=" ";
				tempStr+=dataArray[len-1].y;
				tempStr+=" ";
				tempStr+=dataArray[len-1].z;
			}
			return tempStr;
		}
		//����ά���ת���ɱ�׼���ַ�
		template <typename T>
		std::string vectorToStr(T dataXYZ)
		{
			string result;
			result.assign(numtostr(dataXYZ.x));
			result.append(" ");
			result.append(numtostr(dataXYZ.y));
			result.append(" ");
			result.append(numtostr(dataXYZ.z));
			return result;
		}
		


		//--------------------------------------spline��ֵ��ϵ���㷨
		//--------------------------------------
		//--------------------------------------
		static void getPosNormFromTurtles(const turtles &feaTurtle,int sampleNum,int curveType,MFloatPointArray &fpa, MFloatVectorArray &fva);
		static void getJCSplineTurtlesSide(const MFloatPointArray &currentArray,jcSpline3D &curve);
		static void getJCSplineTurtlesPos(const turtles &feaTurtle,jcSpline3D &curve);
		static void getJCSplineTurtlesNor(const turtles &feaTurtle,jcSpline3D &curve);
		static void getlsysTurtleFromSpline(int curveType,int step,int stepNum,
											jcSpline3D &posSp,jcSpline3D &norSp,
											lsysTurtle &lst);



		//--------------------------------------�����ɢ�����ϵ���㷨
		//--------------------------------------
		//--------------------------------------
		static void getTrianleVec(int num,int seed,vector<jcPoint2D> &ptVec);//�õ�һ�����������jcPoint2D [0,1)�Ķ�άʸ����Ϊ�������������׼��
		static void ptVecInTriangle( const MPoint &pt1,const MPoint &pt2,const MPoint &pt3,
									 const jcPoint2D  &ptVec,MPoint &outPt);		
		static void ptVecInTriangle( const MPoint &pt1,const MPoint &pt2,const MPoint &pt3,
									 const vector<jcPoint2D> &ptVec,MPointArray &outPts);//������ε�����㣬��ȡһ�����������㣬���λ����jcPoint2D�Ķ�άʸ�����Ʒ���
		static void getPointWeightInTri(const MPoint &pt,const MPointArray &triangPts,double weights[3]);//������ڲ��ĵ��ܵ���������Ȩ�ز�ֵ
		static bool isPointInTriangle(const MPoint &pt,const MPointArray &trianglePt,int &ptIndex);//���Ƿ���������ڲ�
		static bool isPointInTriangle(  const MPoint &pt,
										const MPoint &pt0,
										const MPoint &pt1,
										const MPoint &pt2);
		static void getLocalPosInTriangle(  const MPoint &pt,
										const MPoint &pt0,
										const MPoint &pt1,
										const MPoint &pt2,
										jcPoint2D &outLocalPos);
		static double triangleArea(const MPoint &pt1,const MPoint &pt2,const MPoint &pt3);//�õ���������

		static void randPointInTriangle(const MPoint &pt1,const MPoint &pt2,const MPoint &pt3,
										int ptnum,int seed,
										MPointArray &outPts);//������ε�����㣬��ȡһ������������
};

#endif
