#ifndef _jcFeatherSDKDataType_
#define _jcFeatherSDKDataType_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH_LEN 256
#define MAX_FEA_NUM 1024
#define MAX_FEA_NAME_LEN 128
#define jcFea_SDKVersion 1000

typedef char* charPtr;
/*

xml file structure:

globalData<>
	blur<0>
		jcFeather1<>
		jcFeather2<>
		...
	blur<1>
		jcFeather1<>
		jcFeather2<>
		...

The global data will be stored in xmlCacheFileAttr struct.

blur<.> means the motion blur sample num. Each blur<.> stores all the jcFeather node in maya scene.
When motion blur is off, there is only one blur<.>.
When motion blur is on, you may need to iterate all the feather in each blur to get motion blur feathers.

In xml file, we call each jcFeather as an element, for example, jcFeather1 is an element.
And jcFeather in each blur<.> is called a feather node, stored in jcFeatherNode struct.
The curves data for each leaf of feather is stored in singleFeatherData struct.
*/

//feather render attributes
typedef struct 
{
	bool _castsShadow;
	bool _receiveShadow;
	bool _motionBlur;
	bool _primaryVisibility;
	bool _visibleInReflections;
	bool _visibleInRefractions;
}mayaRenderAttr;

//-------------------------------------------------------------------------------------------------------------------------------
typedef struct 
{
	float minX;
	float minY;
	float minZ;

	float maxX;
	float maxY;
	float maxZ;
}jcBoundingBox;

//-------------------------------------------------------------------------------------------------------------------------------
//store the root xml global data
typedef struct 
{
	char            _xmlFeatherVersion[32];
	char            _xmlMayaVersion[32];

	bool			_motionBlurEnable;//motion blur or not
	int				_blurNum;//motion blur samples num
	float			*_blurSteps;//motion blur by _blurStep frame for each sample, motion segments[0,0.1,0.2,0.3 ...]

	jcBoundingBox	_bbox;//the total bounding box for this xml file.
	jcBoundingBox*	_allBBox;//bounding box for each feather element, the num is equal to _featherNum, 

	charPtr			_featherShape[MAX_FEA_NUM];//feather element name ,only the first _featherNum string is useful
	int				_featherNum;//the num of feather element
	char			_xmlFilePath[MAX_PATH_LEN];//xml file name
	char			_currentFrame[5];//frame num "0001" "0002" "0003" ...
	char			_globalShaderName[MAX_FEA_NAME_LEN];//delight shader name ,default  is jcFeather

	bool            _hasBarbuleNormal;//need feather curve normal or not
	bool            _diceHair;//dice hair in renderman or not
	bool            _delayRead;
}xmlCacheFileAttr;

//-------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
	float			_rachisRootColor[3];// rachis root color
	float			_rachisTipColor[3];// rachis tip color
	float			_barbuleRootColor[3];// barbule root color
	float			_barbuleTipColor[3];// barbule tip color

	float			_baseOpacity;// treat this as the root opacity
	float			_fadeOpacity;// treat this as the tip opaticy
	float			_fadeStart;//start to fade
	bool			_uvProject;
	float			_uvProjectScale;// this value is used to scale the barbule u value
	float			_barbuleDiffuse;//barbule diffuse for renderman shader
	 
	float			_barbuleSpecular;//barbule specular for renderman shader
	float			_barbuleSpecularColor[3];//barbule specular color for renderman shader
	float			_barbuleGloss;//barbule gloss for renderman shader
	float			_selfShadow;// self shadow parameters for renderman shader

	int				_hueVar;//hue variation
	float			_satVar;//saturation variation
	float			_valVar;// value variation
	float			_varFreq;// variation frequency

	char			_texture[MAX_PATH_LEN];//will be overrided

	short			_curveType;//linear or cubic
	bool			_hasRachis;//whether this feather has rachis curve

	bool			_useOutShader;//use out shader or not ,currently useless
	char			_shaderName[MAX_FEA_NAME_LEN];//the out shader name ,currently useless
}jcFeatherShaderInfo;

//-------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
	int						_surfaceFeaNum;
	int						_meshFeaNum;

	jcFeatherShaderInfo		_shader;
	mayaRenderAttr			_renderAttr;
	char					_fcPath[MAX_PATH_LEN];

	jcBoundingBox*			_feaBBox;//the size is equal to (_surfaceFeaNum+_meshFeaNum)
	jcBoundingBox			_bbox;
}jcFeatherNode;

//-------------------------------------------------------------------------------------------------------------------------------
typedef struct  
{
	//all the data int the pointers follows the order [rachis,leftBarbule,rightBarbule]

	//the curve num of the feather = _hasRachis + _leftBarbuleNum + _rightBarbuleNum
	unsigned int				_leftBarbuleNum;//leftBarbule curve Num 
	unsigned int				_rightBarbuleNum;//rightBarbule curve Num

	unsigned int				_rachisPointNum;//the pt num of rachis
	unsigned int				_barbulePointNum;//the pt num of each barbule, all the barbules have the same num of points

	/*
	All the curves' point position of this feather
	_featherPointPosition num = _hasRachis*_rachisPointNum * 3 + (_leftBarbuleNum + _rightBarbuleNum) * _barbulePointNum * 3
	*/
	float						*_featherPointPosition;

	/*
	uniform width mode	or not
	*/
	bool						_useUniformWidth;

	/*
	The point width of all the feather curves.

	uniform width mode
	width for each feather curves , data size =_hasRachis + _leftBarbuleNum + _rightBarbuleNum
	
	varying width mode
	for linear curves,data size = _rachisPointNum + _barbulePointNum * (_leftBarbuleNum + _rightBarbuleNum)
	for cubic curves, data size = _rachisPointNum-2 + (_barbulePointNum-2) * (_leftBarbuleNum + _rightBarbuleNum)
	*/
	float						*_featherPointWidth;

	/*
	if xmlCacheFileAttr._hasBarbuleNormal is true, this data will store all the points' normal,
	if false ,it is NULL
	varying data
	data size equals to the (varying _featherPointWidth size * 3)
	*/
	float						*_featherCurveNormal;//

	/*
	each barbule's position at rachis , from 0 to 1,from root to tip of the rachis
	uniform data
	data size = _hasRachis + _leftBarbuleNum + _rightBarbuleNum
	*/
	float						*_barbulePosAtRachis;

	/*
	each barbule's length percent compared with the longest barbule
	uniform data
	data size = _hasRachis + _leftBarbuleNum + _rightBarbuleNum
	*/
	float						*_barbuleLenghtPer;
	
	/*
	const data
	*/
	float						_surfaceUV[2];// the uv value of each feather ,you can project a texture to all the feathers
	float						_mainColor[3];// this is used to make each feather's color different
	char						_texture[MAX_PATH_LEN];//texture path
	int							_randSeed;//random seed for each feather
	
	float						_proxyBBox[6];//proxy boundingbox for each feather	
	int							_nodeIndex;//element index
	int							_nodeBlurIndex;//blur index
	int							_nodeLeafIndex;//feather leaf id in this feather node. 

	jcFeatherShaderInfo			*_shader;

}singleFeatherData;

//-------------------------------------------------------------------------------------------------------------------------------
enum jcfInfoVerbose
{
	JCFINFO_NO=0,// no message
	JCFINFO_ERROR=1,//only error message
	JCFINFO_WARNING=2,//warning message
	JCFINFO_DETAIL=3//all the detail message
};

#ifdef __cplusplus
}
#endif

#endif