#include "jcFeatherNodeData.h"

jcFeatherNodeData::jcFeatherNodeData()
{	
	jcFeatherLibTools::initJCBBox( _node._bbox);
	strcpy(_node._fcPath,"");
	_node._surfaceFeaNum = 0;
	_node._meshFeaNum = 0;
	_node._feaBBox=NULL;
}

jcFeatherNodeData::~jcFeatherNodeData(){
}

void jcFeatherNodeData::setFeaInfo(featherInfo* feainfo)
{
	_shapeInfo = feainfo;

	_node._shader._rachisRootColor[0]=_shapeInfo->_shader._rachisRootColor.r;
	_node._shader._rachisRootColor[1]=_shapeInfo->_shader._rachisRootColor.g;
	_node._shader._rachisRootColor[2]=_shapeInfo->_shader._rachisRootColor.b;

	_node._shader._rachisTipColor[0]=_shapeInfo->_shader._rachisTipColor.r;
	_node._shader._rachisTipColor[1]=_shapeInfo->_shader._rachisTipColor.g;
	_node._shader._rachisTipColor[2]=_shapeInfo->_shader._rachisTipColor.b;

	_node._shader._barbuleRootColor[0]=_shapeInfo->_shader._rootColor.r;
	_node._shader._barbuleRootColor[1]=_shapeInfo->_shader._rootColor.g;
	_node._shader._barbuleRootColor[2]=_shapeInfo->_shader._rootColor.b;

	_node._shader._barbuleTipColor[0]=_shapeInfo->_shader._tipColor.r;
	_node._shader._barbuleTipColor[1]=_shapeInfo->_shader._tipColor.g;
	_node._shader._barbuleTipColor[2]=_shapeInfo->_shader._tipColor.b;

	_node._shader._baseOpacity = _shapeInfo->_shader._baseOpacity;
	_node._shader._fadeOpacity = _shapeInfo->_shader._fadeOpacity;
	_node._shader._fadeStart = _shapeInfo->_shader._fadeStart;
	_node._shader._uvProject = _shapeInfo->_shader._uvProject;
	_node._shader._uvProjectScale = _shapeInfo->_shader._uvProjectScale;
	_node._shader._barbuleDiffuse = _shapeInfo->_shader._barbuleDiffuse;

	_node._shader._barbuleSpecular = _shapeInfo->_shader._barbuleSpecular;
	_node._shader._barbuleSpecularColor[0]=_shapeInfo->_shader._barbuleSpecularColor.r;
	_node._shader._barbuleSpecularColor[1]=_shapeInfo->_shader._barbuleSpecularColor.g;
	_node._shader._barbuleSpecularColor[2]=_shapeInfo->_shader._barbuleSpecularColor.b;
	_node._shader._barbuleGloss = _shapeInfo->_shader._barbuleGloss;
	_node._shader._selfShadow = _shapeInfo->_shader._selfShadow;

	_node._shader._hueVar = _shapeInfo->_shader._hueVar;
	_node._shader._satVar = _shapeInfo->_shader._satVar;
	_node._shader._valVar = _shapeInfo->_shader._valVar;
	_node._shader._varFreq = _shapeInfo->_shader._varFreq;

	strcpy(_node._shader._texture,_shapeInfo->_shader._barbuleTexture.asChar());
	_node._shader._useOutShader = _shapeInfo->_useOutShader;
	
	if(_node._shader._useOutShader)
		strcpy(_node._shader._shaderName,_shapeInfo->_shaderName.asChar());
	else
		strcpy(_node._shader._shaderName,"jcFeather");

	_node._shader._curveType = _shapeInfo->_curveType;
	if(_shapeInfo->_rachisRenderAs==0)
		_node._shader._hasRachis = true;
	else
		_node._shader._hasRachis = false;
}

bool  jcFeatherNodeData::getDataFromFCFile()
{
	//initFeatherNode();
	bool openFC = surfaceTurtleIO::readInfoFrom(_fcFileData,_node._fcPath);
	if(!openFC) return false;

	jcFeatherLibTools::initJCBBox( _node._bbox );

	jcBoundingBox tempBBox;
	MPoint pt1,pt2;

	_node._surfaceFeaNum=(int)_fcFileData._surfaces._surfaceD.size();
	_node._meshFeaNum=(int)_fcFileData._turtlesG._turtlesD.size();
	_node._feaBBox = (jcBoundingBox*)malloc( sizeof(jcBoundingBox)* (_node._surfaceFeaNum+_node._meshFeaNum) );

	for(int ii=0;ii<_node._surfaceFeaNum;++ii)
	{
		pt1 = _fcFileData._surfaces._surfaceFeaInfo[ii]._bbox.min();
		pt2 = _fcFileData._surfaces._surfaceFeaInfo[ii]._bbox.max();

		tempBBox.minX = (float)pt1.x;
		tempBBox.minY = (float)pt1.y;
		tempBBox.minZ = (float)pt1.z;

		tempBBox.maxX = (float)pt2.x;
		tempBBox.maxY = (float)pt2.y;
		tempBBox.maxZ = (float)pt2.z;

		_node._feaBBox[ ii ]=tempBBox;
		jcFeatherLibTools::expandJCBBox(_node._bbox,tempBBox);
	}

	for(int ii=0;ii<_node._meshFeaNum;++ii)
	{
		pt1 = _fcFileData._turtlesG._turtlesFeaInfo[ii]._bbox.min();
		pt2 = _fcFileData._turtlesG._turtlesFeaInfo[ii]._bbox.max();

		tempBBox.minX = (float)pt1.x;
		tempBBox.minY = (float)pt1.y;
		tempBBox.minZ = (float)pt1.z;

		tempBBox.maxX = (float)pt2.x;
		tempBBox.maxY = (float)pt2.y;
		tempBBox.maxZ = (float)pt2.z;

		_node._feaBBox[ ii+_node._surfaceFeaNum ]=tempBBox;
		jcFeatherLibTools::expandJCBBox(_node._bbox,tempBBox);
	}
	return true;
}