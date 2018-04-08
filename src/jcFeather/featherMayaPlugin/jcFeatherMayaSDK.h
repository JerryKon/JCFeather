#ifndef _jcFeatherMayaSDK_
#define _jcFeatherMayaSDK_

#if defined(JCFEATHER_LIB_EXPORTS)
#define JCFea_DLLEXPORT __declspec(dllexport)
#else
#define JCFea_DLLEXPORT
#endif

using namespace std;

class MObject;
class MObjectArray;

typedef struct
{
    float x,y,z;
} jcVertex;

/*
	Do not modify the returned pointer data in this class.
	Notice that the data structure in this maya sdk is a little different from the one in jcFeatherLib.
	The data size of *_featherPointPosition, *_featherPointWidth, *_featherCurveNormal are not properly converted for renders.
*/
class JCFea_DLLEXPORT featherCurves
{
public:
	featherCurves();
	~featherCurves();
public:
	/*
	curves num = 1 (one rachis)+_leftBarbuleNum+_rightBarbuleNum
	*/
	unsigned int				_leftBarbuleNum;//leftBarbule curve Num 
	unsigned int				_rightBarbuleNum;//rightBarbule curve Num

	unsigned int				_rachisPointNum;//the pt num of rachis
	unsigned int				_barbulePointNum;//the pt num of each barbule, all the barbules have the same num of points

	/*
	All the curves' points position of this feather.
	[rachis points, leftBarbule points, rightBarbule points]
	_featherPointPosition num = _rachisPointNum + (_leftBarbuleNum + _rightBarbuleNum) * _barbulePointNum
	*/
	jcVertex					*_featherPointPosition;

	/*
	The point width of all the feather curves.
	type ( uniform , varying)

	uniform width mode
	width for each feather curves , data size =1 + _leftBarbuleNum + _rightBarbuleNum
	
	varying width mode
	data size = _rachisPointNum + _barbulePointNum * (_leftBarbuleNum + _rightBarbuleNum)
	*/
	bool                         _useUniformWidth;
	float						*_featherPointWidth;
	jcVertex					*_featherCurveNormal;//data size equals to the (varying _featherPointWidth size ), varying data

	/*
	each barbule's position at rachis, from the root to tip of the rachis
	type ( uniform  )
	data size = 1 + _leftBarbuleNum + _rightBarbuleNum
	*/
	float						*_barbulePosAtRachis;

	/*
	each barbule's length percent compared with the longest barbule
	type ( uniform  )
	data size = 1 + _leftBarbuleNum + _rightBarbuleNum
	*/
	float						*_barbuleLenghtPer;
	
	//constant data
	float						_surfaceUV[2];// the uv value of each feather 
	float						_mainColor[3];// this is used to make each feather's color different
	char						_texture[256];//texture path

	int							_randSeed;//random seed for each feather
	float						_proxyBBox[6];//proxy boundingbox for each feather

	int							_nodeLeafIndex;//the leaf index
};

class JCFea_DLLEXPORT jcFeatherMaya
{
	public:
		/*
			Init feather sdk, all computing needs start with this funciton.
		*/
		static bool              Init();

		//Get current version of jcFeatherMayaSdk
		static int               GetVersion();

		/*
			Get all active jcfeather node in the scene.
		*/
		static void              GetActiveJCFeatherNode(MObjectArray &featherAry,bool visibleOnly);

		/*
			Compute the jcFeather curves data in the jcFeather node "featherNode".
			Return feather curve pointer, store the feather num in "feaNum".
			Return NULL if error occurs.
		*/
		static featherCurves*    GetFeatherCurveData(MObject &featherNode,int &feaNum);

		/*
			Every time running the "GetFeatherCurveData" function, new pointer data will be added to jcFeather engine.
			So if you do not need the "featherCurves*" pointer, please clear the memory with this function.
		*/
		static void              ClearFeatherPointers();
		
		/*
			At the end of your computing, ClearAll will clear all the memory in jcFeather engine.
			Make sure your code ends with this function.
		*/
		static void              ClearAll();
};

#endif