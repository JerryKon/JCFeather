#ifndef _featherGenBase_
#define _featherGenBase_

#include "singleFeather.h"
#include "noise.h"
#include "pointInPolygon.h"
#include "cell.h"
#include "kdtree.h"
#include "zlib.h"
#include "featherInfo.h"
#include "boostTools.h"
#include "xmlFeatherAttr.h"
#include "surfaceTurtleIO.h"

#include <string>
#include <algorithm>
#include <assert.h>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <cmath>

using namespace std;
class turtles;

struct featherExportInfo
{
	enum currentRender{
		DELIGHT=0,
		PRMAN=1,
		MENTALRAY=2,
	};
	enum outputFileType{
		RIB=0,
		CACHE=1,
		CACHE_RM=2,
		CACHE_MR=3,	
		SDK=4,
	};
	currentRender _userRender;

	MString jcFeatherVersion;
	MString mayaVersion;
	int     needSdkVersion;

	//jcFeather Global settings info
	MString comment;
	MString dsoName;
	MString featherRib;
	MString rachisMeshInsert;
	MString constantParam;
	bool exportNormal;
	bool diceHair;
	compressionInfo _compress;//How to compress rib file

	bool enableMotionBlur;
	int _mbSample;//motion blur samples
	float _mbStep;//motion blur step
	float _mbFactor;

	bool _relativePath;//use relative path in rib file or not
	bool _delayArchive;//use delayReadArchive in rib file or not

	int _mbCurrentSample;
	int _frame;//start frame when exporting feather data.存储文件写出的当前起始帧
	MTime _startTime;

	map<string,MBoundingBox> _proxyBB;//store the proxy bouding box with the key jcfeather node name and index
	MString _name;//store the root folder for the cache files hierarchy
	MString _combineCacheContent;
	MStringArray _allFiles;
	MString _localDir;

	bool _useSelected;
	int _fileType;
	int _renderCurveFuncion;//catmullrom or bezier
	MString _filePostfix;
	featherRenderAttr _renderAttr;

	//-------when export jcFeather with selected nodes
	MObjectArray _jcFeatherNodes;
	MObjectArray _jcFeatherSysNodes;
	MIntArray _jcFeatherNotIn;

	bool _onlyGetBBox;//only get the proxy boundingbox,not computing all feather barbules
	MBoundingBox _xmlFileBoundingbox;

	//--export feather cache *.fc and *.xml
	surfaceTurtleIO _featherCacheIO;
	xmlFeatherAttr _featherAttrXmlIO;
};

class featherGenBase
{
	public:
		 featherGenBase();
		 ~featherGenBase();
		 void init();
		 void initForEachFrame();
		 void setGlobalInfo(const MString &rRib,
								  const MString &meshRib,
								  const MString &constantBContent,
								  const MString &constRContent);

		 void setCompressInfo(bool docomp,int level);
		 void assignFeatherInfo(featherInfo *feaInfo);

		 void getJCFeather( const MObject &inputSurface, singleFeather& oneFeather ,unsigned int seed);
		 bool getJCFeather(turtles& featherTurtle,singleFeather &realFeather,unsigned int seed);
		 bool getTurtleJCFeather( turtles& featherTurtle,singleFeather &realFeather,unsigned int seed);
		 bool getTurtleExJCFeather(turtles& featherTurtle,singleFeather &realFeather,unsigned int seed);
		 void genMeshFromFeather(singleFeather &realFeather);
		 
		 void getKeyBarbule(const MObject &inputSurface, singleFeather& oneFeather);
		 void getKeyBarbule(turtles& featerhTurtle,singleFeather &realFeather );
		 void getExKeyBarbule( turtles& featherTurtle,singleFeather &realFeather );

		 void getTurtles( const MObject &inputSurface,turtles * result);
		 void getFeatherRachis(turtles& featherTurtle,singleFeather &realFeather);
	public:
		 float chooseLR(float u,float v,int seed);//-1,1
		 float noiseRandom(int x,int y,int z,int seed);//0,1
		 
		 void computeGapData(barbuleGapData &gapData,int left_right,int seed);
		 void rotateForGap(const barbuleGapData &gapData,int left_right,int pos,lsysTurtle &curTurtle);		 
		 
		 void rotate(float degree,lsysTurtle &curTurtle);
		 void rotate(MPoint pt,lsysTurtle &curTurtle);
		 void move(float length,lsysTurtle &curTurtle);
		 void moveInNurbs(float length,lsysTurtle &curTurtle);
		 void rotateAlongRight(float degree,lsysTurtle &curTurtle);//rotate turtle up and down

		 void getExProxyBB(const turtles& featherTurtle,singleFeather &feather);
		 void getProxyBBFromRachis(singleFeather &feather,float scale,float barLenScale);//get proxyboundingbox based on the rachis
		 bool getBBFromMap(const string &str,MBoundingBox &bb);
		 bool memMapWriteFile(const char* writeData,const char* path);//write files with memory map
		 bool gzip( const char *inData, jerryC::jcString& outData);//compress data with gzip
		 
		 void addCylinder(singleFeather& oneFeather);
		 void adjCrossSec( const MPoint& pt,const MVector& fwd ,float thickScale,MFloatPointArray& pta);
		 void makeBarbuleFace(polyObj& mesh);
		 void addBarbuleFacePoint(   polyObj& mesh,float uValue,float vValue,
									 float uvVWidth,float width,
									 int left_right,lsysTurtle &curTurtle);
		 
	public:
		  featherInfo *_featherAttrs; //feather shape attributes,you need malloc a featherInfo pointer outside this class
		  sysControlData _jcSysD;//some featherSystem attributes
		  featherExportInfo _feaExpInf;//temp data when exporting feather data

		  int _multiThreadBarbNum;
};

#endif
