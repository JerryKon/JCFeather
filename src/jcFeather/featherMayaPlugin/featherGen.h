#ifndef _featherGen_
#define _featherGen_

#include "featherGenBase.h"
#include "HashTable.h"
#include <fstream>
#include <maya/MPlugArray.h>
#include <maya/MFnSet.h>
using namespace std;

class featherGen: public featherGenBase
{
	friend class featherRib;
	friend class jcFeatherSystem;

	public:
	 static int jcFea_SDKVersion;
	public:
		featherGen();
		~featherGen();

		 MStatus initWithJCFeather( MObject &featherObj );
		 MStatus initWithJCSystem(MObject &sysObj);
		 MStatus prepareForRender();
		 void setRenderManDeclare();
		 MString addRManRibAttr(const char *nodeName);
		 bool jcFeaIsValide(MObject &featherObj,MObject &displayMeshNode);
		 bool jcFeaSysIsValide(MObject &feaSysObj,MObject &displayMeshNode);
		 MString getCurrentFCPath(const MString &nodeName);
		 bool getNodeShadingEngine(MObject &node, MObject &shadingengine,MObject &shader);
		 
		 bool useThisSysNode(const MObject &obj);
		 bool computeJCFeatherNode(MObject &featherObj,vector<singleFeather> &renderFeather);
		 bool computeJCFeatherSystemNode(MObject &featherSysObj,vector<singleFeather> &renderFeather);
		 void doWriteRibFiles(const MString &nodeName,const vector<singleFeather> &renderFeather);
		 MStatus doWriteFeather();
		 bool outputFeather(MString &tempStr,const singleFeather &feather);//����ë��ݵ�����rib
		 void outputRachisCylinder(MString &tempStr,const singleFeather &feathers);//��rachisת���ɶ����
		 bool writeAsciiRib( const MString &nodeName ,const vector<singleFeather> &feathers);//��֯дrib�ļ��Ľṹ

		 bool xmlAddData(const MString &nodeName);
		 void assignMRampToJCRamp(MPlug &rampPlug,jcRamp &ramp,MDGContext &ctx);
		 void getGrowTexPosUV(	MFloatPointArray &posArray0,
											MFloatArray &featherPU0,
											MFloatArray &featherPV0,
											MFloatPointArray &outposArray,
											MFloatArray &outfeatherPU,
											MFloatArray &outfeatherPV);
		 void getTexData(MFloatArray &featherPU,
							MFloatArray &featherPV,
							MColorArray &sampleMainColor,
							MFloatArray &sampleScale,
							MFloatArray &sampleRadius);//----multi thread
		 void getFeatherFromSys(  MFloatPointArray &posArray0,
										MFloatArray &featherPU0,
								        MFloatArray &featherPV0,
										vector<singleFeather> &feathers
										);
		 bool interpolateFeather( const MFloatPoint &growPos, //----���λ��
										const vector<jerryC::idDist> &idData,//---����ӽ�Զ������
										turtles& featherTurtle,
										float texScale,float rascale);//---�����ɵ���ëturtle

		 MStatus getPositions(MItMeshPolygon* meshFaceIter,
									MFloatPointArray &outPositions,
									MFloatArray &ua,
									MFloatArray &va);
		 void transformUVPosition( MFloatArray &ua,MFloatArray &va ); //--multi thread
		 MStatus getPositions( MDagPath &surfaceObj,
											MFloatPointArray &outPositions,
											MFloatArray &ua,
											MFloatArray &va);
		 void getDisplayPositions(	MFloatPointArray &outPositions,
											MFloatArray &ua,
											MFloatArray &va,
											float displayQuality);//�����е�λ����ݸ����ʾ�ٷֱ��ӵ�һЩ�Լӿ���ʾ
};

#endif
