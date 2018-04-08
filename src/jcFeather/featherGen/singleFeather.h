#ifndef _singleFeather_
#define _singleFeather_

#include <vector>
#include <maya/MColor.h>
#include <maya/MColorArray.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MBoundingBox.h>
#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MStatus.h>
#include <maya/MFnMeshData.h>
#include <maya/MFloatMatrix.h>
#include <fstream>
#if defined(_OPENMP)
	#include <omp.h>
#endif
using namespace std;

extern bool jcFea_UseMultiThread; 
extern int jcFea_MaxThreads;
extern int jcFea_ProcessorNum;

typedef std::vector< MFloatPointArray > fpa;
typedef std::vector< MFloatVectorArray > fva;

class polyObj
{
	public:
		polyObj();
		~polyObj();
		void init();

		void initWith(const polyObj &other);
		bool initWithMObject(const MObject &meshData, const MString *uvset);
		bool convertToMObject(MObject &meshData,const MString *uvset);
		bool convertToDagMObject(MObject &meshData,const MString *uvset);
		void appendMesh(const polyObj &mesh);
		void transform(const MFloatMatrix &matrix);
	public:
		MIntArray faceCounts;
		MIntArray faceConnects;
		MFloatPointArray pa;
		MFloatArray ua;
		MFloatArray va;
		MIntArray uvids;
};  //定义传输的多边形数据格式

class singleFeather
{
	public:
		singleFeather();
		~singleFeather();
		void clear();

	public:
		MFloatPointArray rachis;  //rachis的点的位置
		MFloatArray rachisWidth;  //rachis的粗细，个数与rachis的段数和曲率有关
		MFloatVectorArray rachisNormal;
		float rachisLength;//rachis的总长度

		fpa leftBarbules;//记录着每根leftbarbule的每个点的位置数据
		MFloatArray leftBarbuleRachisPos;//每一段barbule在rachis上的位置的百分比，模拟u值，供以后贴图采样使用
		MFloatArray leftBarbuleLenPer;//记录着每个leftbarbule相对于最长的那根barbule所占的百分比,用来在shader中进行贴图采样uv
		fva leftBarbuleNormal;//记录着每根leftbarbule的每个点的法线数据，如果是surface feather，则存储的是最近点面的法线，如果是turtlefeather,则由turtle走位决定
		MFloatArray leftBarbuleWidth;//记录着每一根leftbarbule的粗细缩放，即一根barbule一个float数值
		
		fpa rightBarbules;
		MFloatArray rightBarbuleRachisPos;
		MFloatArray rightBarbuleLenPer;
		fva rightBarbuleNormal;
		MFloatArray rightBarbuleWidth;//记录着每一根rightbarbule的粗细缩放，即一根barbule一个float数值
		
		MFloatArray barbuleWidth;//假设每一段barbule的粗细都相同,个数与barbule的段数以及barbule的曲率有关，记录着左右两边barbule曲线每个点的粗细变化
		
		float surfaceUV[2];//如果此feather长在某个物体上，则记录在该物体的uv位置
		MColor mainColor;//用于采样颜色来使每一根羽毛有变化
		MString colorTexOverride;//每根羽毛都可以有一个贴图来覆盖掉jcFeather节点里的贴图,目前采用在jcFeather和jcFeatherSystem节点中运行mel命令来得到不同的tex
		unsigned int randSeed;//随机种子

		MBoundingBox proxyBBox;
		MBoundingBox rachisMeshBBox;
		polyObj rachisCylinder;
		polyObj barbuleFace;
};

#endif