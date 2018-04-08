#include "featherGenDso.h"

#include "featherGenBase.h"
#include "xmlFeatherAttr.h"
#include "xmlFeatherRead.h"

#include <fstream>
#include <string>
#include <iterator>


featherGenDso::featherGenDso()
{
	_globalAttr._featherNum=0;
	_xmlReader._globalAttrPt = &_globalAttr;
}

featherGenDso::~featherGenDso()
{
	_xmlReader._globalAttrPt = NULL;
	clearMe();
}


//---------------clear data
void featherGenDso::clearMe()
{	
	clearFeatherInfo();
	clearFeatherElement();
	clearXmlGlobalData();
	clearAllFeatherCurves();
}

void featherGenDso::clearFeatherInfo()
{
	int feaInfoSize=_feaAttrInfo.size();
	for(int ii=0;ii<feaInfoSize;++ii)//element
		for(int jj=0;jj<_globalAttr._blurNum;++jj)
			if(_feaAttrInfo[ii][jj])
			{
				delete _feaAttrInfo[ii][jj];
				_feaAttrInfo[ii][jj]=NULL;
			}
	_feaAttrInfo.clear();
}

void featherGenDso::clearFeatherElement()
{
	for(int jj=0;jj<_allFeaElement.size();++jj)
	{
		for(int ii=0;ii<_allFeaElement[jj]._feaNodedata.size();++ii)
			if(!_allFeaElement[jj]._feaNodedata[ii]._node._feaBBox)
				free(_allFeaElement[jj]._feaNodedata[ii]._node._feaBBox);
		_allFeaElement[jj]._feaNodedata.clear();
	}
	_allFeaElement.clear();
}

void featherGenDso::clearXmlGlobalData()
{
	if( _globalAttr._featherNum!=0 )
	{
		for(int ii=0;ii<_globalAttr._featherNum;++ii)
		{
			if( _globalAttr._featherShape[ii])
			{
				free( _globalAttr._featherShape[ii] );
				_globalAttr._featherShape[ii]=NULL;
			}
			
		}
		if(_globalAttr._allBBox)
		{
			free(_globalAttr._allBBox);
			_globalAttr._allBBox=NULL;
		}
		_globalAttr._featherNum=0;
	}

	if(_globalAttr._blurSteps) 
	{
		free(_globalAttr._blurSteps);
		_globalAttr._blurSteps=NULL;
	}

}

void featherGenDso::clearAllFeatherCurves()
{
	map<EleBlurPair,FeatherIndex>::iterator iter;
	for(iter=_allFeaCurvesIndices.begin();iter!=_allFeaCurvesIndices.end();++iter)
	{
		if(iter->second._feaCurves)
		{
			for(int jj=0;jj<iter->second._feaNum;++jj)
				clearSingleFeatherData(&iter->second._feaCurves[jj]);
			free(iter->second._feaCurves);
			iter->second._feaCurves=NULL;
		}
	}
	_allFeaCurvesIndices.clear();
	
}

void featherGenDso::clearEleFeathers(int eleIndex)
{
	map<EleBlurPair,FeatherIndex>::iterator iter;
	for(iter=_allFeaCurvesIndices.begin();iter!=_allFeaCurvesIndices.end();++iter)
	{
		if(iter->first.first==eleIndex)
		{
			for(int jj=0;jj<iter->second._feaNum;++jj)
				clearSingleFeatherData(&iter->second._feaCurves[jj]);
			free(iter->second._feaCurves);
			iter->second._feaCurves=NULL;
		}
	}
}
//---------------clear data



//---------------operate singleFeather data
void featherGenDso::clearSingleFeatherData(singleFeatherData *feaData)
{
	if(!feaData)return;
	if(feaData->_featherPointPosition)
		delete feaData->_featherPointPosition;


	if(feaData->_featherPointWidth)
		delete feaData->_featherPointWidth;

	if(feaData->_featherCurveNormal)
		delete feaData->_featherCurveNormal;
	if(feaData->_barbulePosAtRachis)
		delete feaData->_barbulePosAtRachis;
	if(feaData->_barbuleLenghtPer)
		delete feaData->_barbuleLenghtPer;
}

void featherGenDso::initSingleFeatherData(singleFeatherData *feaData)
{
	if(!feaData)return;
	feaData->_shader = NULL;
	feaData->_featherPointPosition = NULL;
	feaData->_featherPointWidth = NULL;
	feaData->_featherCurveNormal = NULL;
	feaData->_barbulePosAtRachis=NULL;
	feaData->_barbuleLenghtPer=NULL;

	feaData->_leftBarbuleNum = 0;
	feaData->_rightBarbuleNum = 0;
	feaData->_rachisPointNum = 0;
	feaData->_barbulePointNum = 0;

	feaData->_useUniformWidth = true;

	feaData->_randSeed=0;
	jcFeatherLibTools::initBBox(feaData->_proxyBBox);
	feaData->_surfaceUV[0]=feaData->_surfaceUV[1]=0;
	feaData->_mainColor[0]=feaData->_mainColor[1]=feaData->_mainColor[2]=1;
	strcpy(feaData->_texture,"");

	feaData->_nodeIndex=-1;
	feaData->_nodeBlurIndex=-1;
	feaData->_nodeLeafIndex=-1;
}

//create single feather struct data
singleFeatherData* featherGenDso::createSingleFeatherData(int num)
{
		
	singleFeatherData *result = (singleFeatherData*)malloc(sizeof(singleFeatherData)* num );
	for(int ii=0;ii<num;++ii)
		initSingleFeatherData(&result[ii]);
	return result;
}

//convert feather data
void featherGenDso::convertFeatherData(singleFeather* inData,
									   singleFeatherData *outData,
									   featherInfo* featherAttr)
{
	if(!inData||!outData) return;
	int hasRachis = outData->_shader->_hasRachis;

	outData->_leftBarbuleNum=inData->leftBarbules.size();
	outData->_rightBarbuleNum=inData->rightBarbules.size();

	outData->_rachisPointNum = inData->rachis.length();
	outData->_barbulePointNum=featherAttr->_barbuleSegments;

	//--------------------------------------curve type linear or cubic
	if(featherAttr->_curveType==1)
	{
		outData->_barbulePointNum += 2;
		outData->_rachisPointNum += 2;
	}

	//-------------------------------------malloc each curve point num 
	//int cuveNum = hasRachis + outData->_leftBarbuleNum + outData->_rightBarbuleNum;

	//-------------------------------------malloc curve pos point
	int posPtNum=  outData->_rachisPointNum*hasRachis + 
					outData->_leftBarbuleNum*outData->_barbulePointNum +
					outData->_rightBarbuleNum*outData->_barbulePointNum;

	outData->_featherPointPosition = (float*)malloc( sizeof(float) * posPtNum *3 );
	

	//--------------------------------------get rachis
	//-----------------------------------
	//-----------------------------------
	int feaPtIndex=0;
	int curveNumIndex=0;
	if(hasRachis && outData->_rachisPointNum>0)
	{
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[0].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[0].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[0].z;
		}
		int ii=0;
		for(;ii<inData->rachis.length();++ii)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii].z;
		}
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii-1].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii-1].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rachis[ii-1].z;
		}
	}


	//-----------------------------------get left barbules
	//-----------------------------------
	//-----------------------------------
	int jj=0;
	for(int ii=0;ii<outData->_leftBarbuleNum;++ii)
	{
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][0].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][0].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][0].z;
		}
		jj=0;
		for(;jj<featherAttr->_barbuleSegments;++jj)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj].z;
		}
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj-1].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj-1].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->leftBarbules[ii][jj-1].z;
		}
	}


	//-----------------------------------get right barbules
	for(int ii=0;ii<outData->_rightBarbuleNum;++ii)
	{
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][0].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][0].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][0].z;
		}
		jj=0;
		for(;jj<featherAttr->_barbuleSegments;++jj)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj].z;
		}
		if(featherAttr->_curveType==1)
		{
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj-1].x;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj-1].y;
			outData->_featherPointPosition[feaPtIndex++] = inData->rightBarbules[ii][jj-1].z;
		}
	}


	//-----------------------------------get width data
	//-----------------------------------
	//-----------------------------------
	outData->_useUniformWidth = featherAttr->_uniformWidth;

	int uniformCurveVarNum=hasRachis + 
						outData->_leftBarbuleNum +
						outData->_rightBarbuleNum;
	int varyingCurveVarNum=hasRachis * inData->rachis.length() + 
							outData->_leftBarbuleNum * featherAttr->_barbuleSegments +
							outData->_rightBarbuleNum * featherAttr->_barbuleSegments;
	int widthIndex=0;
	if(outData->_useUniformWidth)
	{
		outData->_featherPointWidth = (float*)malloc( sizeof(float) * uniformCurveVarNum  );

		if(hasRachis)
			outData->_featherPointWidth[widthIndex++] = featherAttr->_rachisThick * inData->rachisLength*0.01f;
		for( int jj=0;jj<outData->_leftBarbuleNum;++jj )
		{
			outData->_featherPointWidth[widthIndex++] = featherAttr->_barbuleThick*
														inData->leftBarbuleWidth[jj]* 
														inData->rachisLength*0.01f;
		}
		for( int jj=0;jj<outData->_rightBarbuleNum;++jj )
		{
			outData->_featherPointWidth[widthIndex++] = featherAttr->_barbuleThick*
														inData->rightBarbuleWidth[jj]* 
														inData->rachisLength*0.01f;
		}
	}
	else//-------------use varying width
	{
		outData->_featherPointWidth = (float*)malloc( sizeof(float) * varyingCurveVarNum  );

		if(hasRachis)
			for( int jj=0;jj<inData->rachis.length();++jj )
				outData->_featherPointWidth[widthIndex++] = inData->rachisWidth[jj]* inData->rachisLength*0.01f;
		for( int ii=0;ii<outData->_leftBarbuleNum;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
				outData->_featherPointWidth[widthIndex++] =  inData->barbuleWidth[jj]*
															inData->leftBarbuleWidth[ii]* 
															inData->rachisLength*0.01f;

		for( int ii=0;ii<outData->_rightBarbuleNum;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
				outData->_featherPointWidth[widthIndex++] =  inData->barbuleWidth[jj]*
															inData->rightBarbuleWidth[ii]* 
															inData->rachisLength*0.01f;
	}

	if(_globalAttr._hasBarbuleNormal)
	{
		int rachisSize= inData->rachisNormal.length();
		int leftSize=inData->leftBarbuleNormal.size();
		int rightSize=inData->rightBarbuleNormal.size();

		outData->_featherCurveNormal = (float*)malloc( sizeof(float) * varyingCurveVarNum * 3 );
		
		int varIndex=0;
		if(hasRachis)
			for( int jj=0;jj<rachisSize;++jj )
			{
				outData->_featherCurveNormal[varIndex++] = inData->rachisNormal[jj].x;
				outData->_featherCurveNormal[varIndex++] = inData->rachisNormal[jj].y;
				outData->_featherCurveNormal[varIndex++] = inData->rachisNormal[jj].z;
			}
		for( int ii=0;ii<leftSize;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
			{
				outData->_featherCurveNormal[varIndex++] =  inData->leftBarbuleNormal[ii][jj].x;
				outData->_featherCurveNormal[varIndex++] =  inData->leftBarbuleNormal[ii][jj].y;
				outData->_featherCurveNormal[varIndex++] =  inData->leftBarbuleNormal[ii][jj].z;
			}

		for( int ii=0;ii<rightSize;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
			{
				outData->_featherCurveNormal[varIndex++] =  inData->rightBarbuleNormal[ii][jj].x;
				outData->_featherCurveNormal[varIndex++] =  inData->rightBarbuleNormal[ii][jj].y;
				outData->_featherCurveNormal[varIndex++] =  inData->rightBarbuleNormal[ii][jj].z;				
			}
	}

	//--------------------------------------barbule u
	//-----------------------------------
	//-----------------------------------
	outData->_barbulePosAtRachis = (float*)malloc( sizeof(float) * uniformCurveVarNum  );

	int barbUIndex=0;
	if(hasRachis)
		outData->_barbulePosAtRachis[barbUIndex++]= -1;
	for( int jj=0;jj<outData->_leftBarbuleNum;++jj )
		outData->_barbulePosAtRachis[barbUIndex++]= inData->leftBarbuleRachisPos[jj];
	for( int jj=0;jj<outData->_rightBarbuleNum;++jj )
		outData->_barbulePosAtRachis[barbUIndex++]= inData->rightBarbuleRachisPos[jj];	

	//--------------------------------------barbule lengthPer
	//-----------------------------------
	//-----------------------------------
	outData->_barbuleLenghtPer = (float*)malloc( sizeof(float) * uniformCurveVarNum  );
	int barbPerLenIndex=0;
	if(hasRachis)
		outData->_barbuleLenghtPer[barbPerLenIndex++]= 1;
	for( int jj=0;jj<outData->_leftBarbuleNum;++jj )
		outData->_barbuleLenghtPer[barbPerLenIndex++]= inData->leftBarbuleLenPer[jj];
	for( int jj=0;jj<outData->_rightBarbuleNum;++jj )
		outData->_barbuleLenghtPer[barbPerLenIndex++]= inData->rightBarbuleLenPer[jj];


	//---------------------------------------const variables
	outData->_surfaceUV[0]=inData->surfaceUV[0];
	outData->_surfaceUV[1]=inData->surfaceUV[1];
	outData->_mainColor[0]=inData->mainColor.r;
	outData->_mainColor[1]=inData->mainColor.g;
	outData->_mainColor[2]=inData->mainColor.b;
	outData->_randSeed = inData->randSeed;

	strcpy( outData->_texture,inData->colorTexOverride.asChar() );

	MPoint minPt,maxPt;
	minPt=inData->proxyBBox.min();
	maxPt=inData->proxyBBox.max();

	outData->_proxyBBox[0]=minPt.x;	outData->_proxyBBox[1]=maxPt.x;
	outData->_proxyBBox[2]=minPt.y;	outData->_proxyBBox[3]=maxPt.y;
	outData->_proxyBBox[4]=minPt.z;	outData->_proxyBBox[5]=maxPt.z;

}

void featherGenDso::setOverrideCurveType(int type)
{
	_xmlReader._overrideCurveType = type;
}
//---------------operate singleFeather data




bool featherGenDso::openXmlData(const char * file)
{
	clearMe();
	bool open=_xmlReader.xmlImport(file);
	if(!open) return false;
	
	for(int ii=0;ii<_globalAttr._featherNum;++ii)
		getFeatherElementData(ii);

	return true;
}

bool featherGenDso::getFeatherElementData(int eleIndex)
{
	featherElement currentEle;

	currentEle._feaIndex = eleIndex;
	vector<featherInfo*> tempInfoArray(_globalAttr._blurNum,NULL);
	
	for(int ii=0;ii<_globalAttr._blurNum;++ii)
	{
		jcFeatherNodeData tempNodeData;
		tempInfoArray[ii]=new featherInfo;
		FeatherIndex tempFeaIndex;

		if(!_xmlReader.getFeatherAttributeAtStep(tempInfoArray[ii],//feather name
									 tempNodeData,//feather data
									 _globalAttr._featherShape[eleIndex],
									 ii))
		{
			JCFEATHER_ERROR(false,"Cannot get feather node data.");
			return false;
		}
		if( !tempNodeData.getDataFromFCFile() ) 
		{
			string info("Cannot open ");
			info.append(tempNodeData._node._fcPath);
			JCFEATHER_ERROR(false,info);
			return NULL;
		}
		tempFeaIndex._feaNum = tempNodeData._node._surfaceFeaNum+tempNodeData._node._meshFeaNum;
		tempFeaIndex._feaCurves = createSingleFeatherData(tempFeaIndex._feaNum);

		_allFeaCurvesIndices.insert( make_pair( make_pair(eleIndex,ii),tempFeaIndex ) );
		currentEle._feaNodedata.push_back(tempNodeData);
	}
	
	_allFeaElement.push_back(currentEle);
	_feaAttrInfo.push_back(tempInfoArray);

	return true;
}

//set the featherInfo pointer to the specified one
singleFeatherData *featherGenDso::getFeatherCurveData( int eleIndex,int blurIndex )
{
	featherGenBase genFeather;
	genFeather._featherAttrs = _feaAttrInfo[eleIndex][blurIndex];
	
	jcFeatherNodeData *tempNode = &_allFeaElement[eleIndex]._feaNodedata[blurIndex];

	int surfaceFeaSize=tempNode->_node._surfaceFeaNum;
	int meshFeaSize=tempNode->_node._meshFeaNum;
	singleFeatherData *featherData =_allFeaCurvesIndices[make_pair(eleIndex,blurIndex)]._feaCurves;

	singleFeather sgf;
	MObject tempObj;
	MFnNurbsSurfaceData dataCreator;
	MFnNurbsSurface nurbsFn;

	for(int ii=0;ii<surfaceFeaSize;++ii)
	{
		sgf.clear();
		
		sgf.surfaceUV[0]=tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._surfaceUV[0];
		sgf.surfaceUV[1]=tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._surfaceUV[1];
		sgf.randSeed = tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._randSeed;
		sgf.mainColor = tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._mainColor;
		sgf.proxyBBox = tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._bbox;

		sgf.colorTexOverride = tempNode->_fcFileData._surfaces._surfaceFeaInfo[ii]._colorTexOverride;
		
        MObject newSurfData = dataCreator.create();
		tempObj = nurbsFn.create(   tempNode->_fcFileData._surfaces._surfaceD[ii]._controlVertices,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._uKnotSequences,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._vKnotSequences,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._degreeInU,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._degreeInV,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._formU,
									tempNode->_fcFileData._surfaces._surfaceD[ii]._formV,
									false,newSurfData);

		genFeather.getJCFeather(tempObj,sgf,ii);
		
		featherData[ii]._nodeIndex = _allFeaElement[eleIndex]._feaIndex;
		featherData[ii]._nodeBlurIndex = blurIndex;
		featherData[ii]._nodeLeafIndex = ii;
		featherData[ii]._shader = &tempNode->_node._shader;

		convertFeatherData(&sgf,&featherData[ii],genFeather._featherAttrs);
	}

	for(int ii=0;ii<meshFeaSize;++ii)
	{
		sgf.clear();
		sgf.surfaceUV[0]=tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._surfaceUV[0];
		sgf.surfaceUV[1]=tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._surfaceUV[1];
		sgf.randSeed = tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._randSeed;
		sgf.mainColor = tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._mainColor;
		sgf.proxyBBox = tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._bbox;

		sgf.colorTexOverride = tempNode->_fcFileData._turtlesG._turtlesFeaInfo[ii]._colorTexOverride;

		genFeather.getJCFeather(tempNode->_fcFileData._turtlesG._turtlesD[ii],sgf,ii);
		
		featherData[ii+surfaceFeaSize]._nodeIndex = _allFeaElement[eleIndex]._feaIndex;
		featherData[ii+surfaceFeaSize]._nodeBlurIndex = blurIndex;
		featherData[ii+surfaceFeaSize]._nodeLeafIndex = ii+surfaceFeaSize;
		featherData[ii+surfaceFeaSize]._shader = &tempNode->_node._shader;		
		
		convertFeatherData(&sgf,&featherData[ii+surfaceFeaSize],genFeather._featherAttrs);
	}

	return featherData;
}
