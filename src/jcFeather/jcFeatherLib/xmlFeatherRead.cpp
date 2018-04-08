#include "xmlFeatherRead.h"

xmlFeatherRead::xmlFeatherRead()
{
	_globalAttrPt=NULL;
	_baseEle = NULL;
	_overrideCurveType=-1;
	_maxJCFeatherNodes=3;
}

xmlFeatherRead::~xmlFeatherRead()
{}

bool xmlFeatherRead::getFCPath( char* outPath,const char* feaName,int step )
{
	if( !_globalAttrPt) return false;
	string xmlDir,xmlName;
	if( !jerryC::getDir_Name( _globalAttrPt->_xmlFilePath,false,xmlDir,xmlName ))
		return false;
	string path="";
	path.append(xmlDir);
	path.append("/");
	path.append(feaName);
	path.append(".");
	path.append( boost::lexical_cast<std::string>(step));
	path.append(".fc");
	strcpy(outPath,path.c_str());
	return true;
}

bool xmlFeatherRead::xmlImport(const char* fileName)
{
	if(!_globalAttrPt) return false;
	if(!_feaXMLDoc.LoadFile(fileName)) return false;

	strcpy(_globalAttrPt->_xmlFilePath,fileName);//load file

	_baseEle=_feaXMLDoc.RootElement();
	if(string(_baseEle->Value()).compare(string("jcFeather_CacheFile"))) return false; //check file data is ok

	if(!getCacheFileAttr(_baseEle))return false;//get xml global data
	TiXmlElement *blurTix=NULL,*jcFeaShapeTix=NULL;
	
	blurTix = _baseEle->FirstChildElement();
	if(!blurTix)return false;
	
	//------------store feather element name
	vector<string> tempstr;
	for(jcFeaShapeTix = blurTix->FirstChildElement(); jcFeaShapeTix; jcFeaShapeTix = jcFeaShapeTix->NextSiblingElement())
	{
		if(string(jcFeaShapeTix->Value()).compare(string("jcFeather"))) continue;
		tempstr.push_back(jcFeaShapeTix->Attribute("nodeName"));
	}

	_globalAttrPt->_featherNum=tempstr.size();

	if(_globalAttrPt->_featherNum>_maxJCFeatherNodes)
	{
		cerr<<"Free version jcFeather only support "<<_maxJCFeatherNodes<<" jcFeather nodes."<<endl;
		return false;
	}
	for(int ii=0;ii<_globalAttrPt->_featherNum;++ii)
	{
		_globalAttrPt->_featherShape[ii] = (charPtr)malloc( sizeof(char)* MAX_FEA_NAME_LEN );
		strcpy( _globalAttrPt->_featherShape[ii],tempstr[ii].c_str() );
	}

	if(_globalAttrPt->_featherNum==0)
	{
		cerr<<"JCFeather Error : No jcFeather node found in "<<_globalAttrPt->_xmlFilePath<<" file."<<endl;
		return false;
	}
	
	//--------------------store blur xml pointer
	_blurStep.clear();
	for( blurTix = _baseEle->FirstChildElement(); blurTix; blurTix = blurTix->NextSiblingElement() )
	{
		if(string(blurTix->Value()).compare(string("blur"))) continue;
		_blurStep.push_back(blurTix);
	}
	if( _blurStep.size() != _globalAttrPt->_blurNum )
	{
		cerr<<"JCFeather Error : Motion blur num is not correct, file corrupted."<<endl;
		return false;
	}

	//---------------store the whole bounding box for each feather
	map<string,jcBoundingBox> feaBBox;
	if( !getFeathersBBox(feaBBox) || feaBBox.size()!=_globalAttrPt->_featherNum ) return false;
	_globalAttrPt->_allBBox = (jcBoundingBox*)malloc(sizeof(jcBoundingBox) * _globalAttrPt->_featherNum);

	for(int ii=0;ii<_globalAttrPt->_featherNum;++ii)
		_globalAttrPt->_allBBox[ii]=feaBBox[tempstr[ii]];

	return true;
}

bool xmlFeatherRead::getFeathersBBox(map<string,jcBoundingBox> &nameBox)
{
	if(!_baseEle  || !_globalAttrPt) return false;
	
	TiXmlElement *jcFeaShapeTix=NULL;
	TiXmlElement *jcFeaAttrTix=NULL;
	
	double box[6];
	jcBoundingBox tempBox;
	nameBox.clear();
	map<string,jcBoundingBox>::iterator nameBBoxIter;

	for(int ii=0;ii<_globalAttrPt->_blurNum;++ii)
		for(jcFeaShapeTix = _blurStep[ii]->FirstChildElement(); jcFeaShapeTix; jcFeaShapeTix = jcFeaShapeTix->NextSiblingElement())
		{
			string shapeTixValue(jcFeaShapeTix->Value());
			if(shapeTixValue.compare(string("jcFeather"))) continue;
			string feaName = jcFeaShapeTix->Attribute("nodeName");

			jcFeaShapeTix->Attribute("minX",&box[0]); 
			jcFeaShapeTix->Attribute("maxX",&box[1]);
			jcFeaShapeTix->Attribute("minY",&box[2]); 
			jcFeaShapeTix->Attribute("maxY",&box[3]); 
			jcFeaShapeTix->Attribute("minZ",&box[4]); 
			jcFeaShapeTix->Attribute("maxZ",&box[5]);

			tempBox.minX=(float)box[0]; tempBox.maxX=(float)box[1];
			tempBox.minY=(float)box[2]; tempBox.maxY=(float)box[3];
			tempBox.minZ=(float)box[4]; tempBox.maxZ=(float)box[5];

			nameBBoxIter = nameBox.find(feaName);
			if(nameBBoxIter==nameBox.end())
				nameBox.insert( pair<string,jcBoundingBox>(feaName,tempBox) );
			else
				jcFeatherLibTools::expandJCBBox(nameBox[feaName],tempBox);
		}
	return true;
}

bool xmlFeatherRead::getFeatherAttributeAtStep( featherInfo *feaNodeAttr,
										jcFeatherNodeData &feaNodeData,
										const char* featherShape,
	  	  	  	  	  	  	  	  	  	int step)
{
	if(!_baseEle || !feaNodeAttr || !_globalAttrPt) return false;

	TiXmlElement *jcFeaShapeTix=NULL;
	TiXmlElement *jcFeaAttrTix=NULL;
	
	for(jcFeaShapeTix = _blurStep[step]->FirstChildElement(); jcFeaShapeTix; jcFeaShapeTix = jcFeaShapeTix->NextSiblingElement())
	{
		string shapeTixValue(jcFeaShapeTix->Value());
		if(shapeTixValue.compare(string("jcFeather"))) continue;
		if(MString(jcFeaShapeTix->Attribute("nodeName")) != MString(featherShape)) continue;
		
		getRenderAttr(jcFeaShapeTix,feaNodeData);
		if( !getFCPath(feaNodeData._node._fcPath,featherShape,step) )
		{
			string info("Cannot find *.fc file ");
			info.append(feaNodeData._node._fcPath);
			JCFEATHER_ERROR(false,info);
			return false;
		}
		
		for(jcFeaAttrTix= jcFeaShapeTix->FirstChildElement(); jcFeaAttrTix; jcFeaAttrTix = jcFeaAttrTix->NextSiblingElement())
		{
			string tixValue=string(jcFeaAttrTix->Value());
			if(!tixValue.compare(string("simpleAttributes")))
				getSimpleAttrFromJCFea(jcFeaAttrTix,feaNodeAttr);
			if(!tixValue.compare(string("keyBarbule")))
				getKeyBarbuleAttrFromJCFea(jcFeaAttrTix,feaNodeAttr);
			if(!tixValue.compare(string("ramps")))
				getRampsAttrFromJCFea(jcFeaAttrTix,feaNodeAttr);
		}

		feaNodeData.setFeaInfo(feaNodeAttr);
	}
	return true;
}

bool xmlFeatherRead::getCacheFileAttr(TiXmlElement *cacheAttrTix)
{
	int tempI=0;
	cacheAttrTix->Attribute("sdkVersion",&tempI);
	if( tempI>jcFea_SDKVersion )
		cout<<"JCFeather Warning : Cache need sdk version "<<tempI<<", current sdk version is an older one "<<jcFea_SDKVersion<<endl;

	strcpy( _globalAttrPt->_xmlFeatherVersion ,cacheAttrTix->Attribute("jcFeatherVersion") );
	strcpy( _globalAttrPt->_xmlMayaVersion ,cacheAttrTix->Attribute("mayaVersion") );
	
	strcpy( _globalAttrPt->_currentFrame ,cacheAttrTix->Attribute("currentFrame") );
	double box[6];
	cacheAttrTix->Attribute("minX",&box[0]);
	cacheAttrTix->Attribute("minY",&box[1]);
	cacheAttrTix->Attribute("minZ",&box[2]);
	cacheAttrTix->Attribute("maxX",&box[3]);
	cacheAttrTix->Attribute("maxY",&box[4]);
	cacheAttrTix->Attribute("maxZ",&box[5]);
	_globalAttrPt->_bbox.minX=(float)box[0];
	_globalAttrPt->_bbox.minY=(float)box[1];
	_globalAttrPt->_bbox.minZ=(float)box[2];
	_globalAttrPt->_bbox.maxX=(float)box[3];
	_globalAttrPt->_bbox.maxY=(float)box[4];
	_globalAttrPt->_bbox.maxZ=(float)box[5];

	strcpy( _globalAttrPt->_globalShaderName ,cacheAttrTix->Attribute("shaderName") );

	
	double tempD=0;
	cacheAttrTix->Attribute("exportBarbuleNormal",&tempI);
	_globalAttrPt->_hasBarbuleNormal = tempI;

	cacheAttrTix->Attribute("delayRead",&tempI);
	_globalAttrPt->_delayRead = tempI;

	cacheAttrTix->Attribute("diceHair",&tempI);
	_globalAttrPt->_diceHair = tempI;
	_globalAttrPt->_blurSteps= NULL;

	if(cacheAttrTix->Attribute("motionBlur",&tempI))
	{
		_globalAttrPt->_motionBlurEnable=(bool)tempI;
		if(_globalAttrPt->_motionBlurEnable)
		{
			cacheAttrTix->Attribute("blurNum",&_globalAttrPt->_blurNum);
			cacheAttrTix->Attribute("blurStep",&tempD);

			_globalAttrPt->_blurSteps = (float*)malloc(sizeof(float) * _globalAttrPt->_blurNum);
			for(int kk=0;kk<_globalAttrPt->_blurNum;++kk)
				_globalAttrPt->_blurSteps[kk] = tempD * kk;
		}
		else
			_globalAttrPt->_blurNum= 1;
	}
	else
	{
		_globalAttrPt->_motionBlurEnable=false;
		_globalAttrPt->_blurNum= 1;
	}

	return true;
}

bool xmlFeatherRead::getRenderAttr(TiXmlElement *FeaAttrTix,jcFeatherNodeData &feaEleAttr)
{
	int tempI=0;
	double tempD=0;
	double box[6];

	FeaAttrTix->Attribute("caShad",&tempI); feaEleAttr._node._renderAttr._castsShadow = (bool)tempI;
	FeaAttrTix->Attribute("recSh",&tempI); feaEleAttr._node._renderAttr._receiveShadow = (bool)tempI;
	FeaAttrTix->Attribute("mb",&tempI); feaEleAttr._node._renderAttr._motionBlur = (bool)tempI;
	FeaAttrTix->Attribute("primVis",&tempI); feaEleAttr._node._renderAttr._primaryVisibility = (bool)tempI;
	FeaAttrTix->Attribute("visRefl",&tempI); feaEleAttr._node._renderAttr._visibleInReflections = (bool)tempI;
	FeaAttrTix->Attribute("visRefr",&tempI); feaEleAttr._node._renderAttr._visibleInRefractions = (bool)tempI;

	FeaAttrTix->Attribute("minX",&box[0]); 
	FeaAttrTix->Attribute("maxX",&box[1]);
	FeaAttrTix->Attribute("minY",&box[2]); 
	FeaAttrTix->Attribute("maxY",&box[3]); 
	FeaAttrTix->Attribute("minZ",&box[4]); 
	FeaAttrTix->Attribute("maxZ",&box[5]);
	feaEleAttr._node._bbox.minX=(float)box[0]; feaEleAttr._node._bbox.maxX=(float)box[1];
	feaEleAttr._node._bbox.minY=(float)box[2]; feaEleAttr._node._bbox.maxY=(float)box[3];
	feaEleAttr._node._bbox.minZ=(float)box[4]; feaEleAttr._node._bbox.maxZ=(float)box[5];

	return true;
}

void xmlFeatherRead::getSimpleAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr)
{
	int tempI=0;double tempD=0;
	
	if(_overrideCurveType!=-1)
		feaNodeAttr->_curveType=(short)_overrideCurveType;
	else
	{
		FeaAttrTix->Attribute("curveType",&tempI);  
		feaNodeAttr->_curveType=(short)tempI;
	}

	FeaAttrTix->Attribute("direction",&tempI);  feaNodeAttr->_direction=(short)tempI;
	FeaAttrTix->Attribute("surfaceFeather",&tempI);  feaNodeAttr->_exactFeather=(bool)tempI;
	FeaAttrTix->Attribute("renderable",&tempI);  feaNodeAttr->_renderable=(bool)tempI;

	FeaAttrTix->Attribute("rachisRenderAs",&tempI);  feaNodeAttr->_rachisRenderAs=(short)tempI;
	FeaAttrTix->Attribute("rachisSegment",&tempI);  feaNodeAttr->_rachisSegment=tempI;
	FeaAttrTix->Attribute("rachisPos",&tempD);  feaNodeAttr->_rachisPos=(float)tempD;
	FeaAttrTix->Attribute("rachisStart",&tempD);  feaNodeAttr->_rachisStart=(float)tempD;
	FeaAttrTix->Attribute("rachisEnd",&tempD);  feaNodeAttr->_rachisEnd=(float)tempD;
	FeaAttrTix->Attribute("rachisSides",&tempI);  feaNodeAttr->_rachisSides=tempI;
	FeaAttrTix->Attribute("rachisThick",&tempD);  feaNodeAttr->_rachisThick=(float)tempD;

	FeaAttrTix->Attribute("rachisRootColorR",&tempD);  feaNodeAttr->_shader._rachisRootColor.r=(float)tempD;
	FeaAttrTix->Attribute("rachisRootColorG",&tempD);  feaNodeAttr->_shader._rachisRootColor.g=(float)tempD;
	FeaAttrTix->Attribute("rachisRootColorB",&tempD);  feaNodeAttr->_shader._rachisRootColor.b=(float)tempD;
	FeaAttrTix->Attribute("rachisTipColorR",&tempD);  feaNodeAttr->_shader._rachisTipColor.r=(float)tempD;
	FeaAttrTix->Attribute("rachisTipColorG",&tempD);  feaNodeAttr->_shader._rachisTipColor.g=(float)tempD;
	FeaAttrTix->Attribute("rachisTipColorB",&tempD);  feaNodeAttr->_shader._rachisTipColor.b=(float)tempD;

	FeaAttrTix->Attribute("barbuleNum",&tempI);  feaNodeAttr->_barbuleNum=tempI;
	FeaAttrTix->Attribute("barbuleSegments",&tempI);  feaNodeAttr->_barbuleSegments=tempI;
	FeaAttrTix->Attribute("shapeSymmetry",&tempI);  feaNodeAttr->_shapeSymmetry=(bool)tempI;
	FeaAttrTix->Attribute("barbuleLength",&tempD);  feaNodeAttr->_barbuleLength=(float)tempD;
	FeaAttrTix->Attribute("barbuleLengthRandScale",&tempD);  feaNodeAttr->_barbuleLengthRandScale=(float)tempD;

	FeaAttrTix->Attribute("LGapForce",&tempD);  feaNodeAttr->_gapForce[0]=(float)tempD;
	FeaAttrTix->Attribute("RGapForce",&tempD);  feaNodeAttr->_gapForce[1]=(float)tempD;
	FeaAttrTix->Attribute("LGapMaxForce",&tempD);  feaNodeAttr->_gapMaxForce[0]=(float)tempD;
	FeaAttrTix->Attribute("RGapMaxForce",&tempD);  feaNodeAttr->_gapMaxForce[1]=(float)tempD;
	FeaAttrTix->Attribute("LGapSize",&tempD);  feaNodeAttr->_gapSize[0]=(float)tempD;
	FeaAttrTix->Attribute("RGapSize",&tempD);  feaNodeAttr->_gapSize[1]=(float)tempD;

	FeaAttrTix->Attribute("forcePerSegment",&tempD);  feaNodeAttr->_forcePerSegment=(float)tempD;
	FeaAttrTix->Attribute("turnForce",&tempD);  feaNodeAttr->_turnForce=(float)tempD;
	FeaAttrTix->Attribute("rotateLamda",&tempD);  feaNodeAttr->_rotateLamda=(float)tempD;
	FeaAttrTix->Attribute("upDownLamda",&tempD);  feaNodeAttr->_upDownLamda=(float)tempD;
	FeaAttrTix->Attribute("upDownNoise",&tempI);  feaNodeAttr->_upDownNoise=(bool)tempI;

	FeaAttrTix->Attribute("rachisNoiseFrequency",&tempD);  feaNodeAttr->_rachisNoiseFrequency=(float)tempD;
	FeaAttrTix->Attribute("barbuleNoiseFrequency",&tempD);  feaNodeAttr->_barbuleNoiseFrequency=(float)tempD;
	FeaAttrTix->Attribute("noisePhaseX",&tempD);  feaNodeAttr->_noisePhase[0]=(float)tempD;
	FeaAttrTix->Attribute("noisePhaseY",&tempD);  feaNodeAttr->_noisePhase[1]=(float)tempD;

	FeaAttrTix->Attribute("barbuleThick",&tempD);  feaNodeAttr->_barbuleThick=(float)tempD;
	FeaAttrTix->Attribute("barbuleRandThick",&tempD);  feaNodeAttr->_barbuleRandThick=(float)tempD;

	
	FeaAttrTix->Attribute("useOutShader",&tempI); feaNodeAttr->_useOutShader = (bool)tempI;
	if( feaNodeAttr->_useOutShader && !FeaAttrTix->Attribute("outShaderName")) 
		feaNodeAttr->_shaderName=MString( FeaAttrTix->Attribute("outShaderName") );
	
	feaNodeAttr->_shader._barbuleTexture=MString( FeaAttrTix->Attribute("barbuleTexture") );

	FeaAttrTix->Attribute("rootColorR",&tempD);  feaNodeAttr->_shader._rootColor.r=(float)tempD;
	FeaAttrTix->Attribute("rootColorG",&tempD);  feaNodeAttr->_shader._rootColor.g=(float)tempD;
	FeaAttrTix->Attribute("rootColorB",&tempD);  feaNodeAttr->_shader._rootColor.b=(float)tempD;
	FeaAttrTix->Attribute("tipColorR",&tempD);  feaNodeAttr->_shader._tipColor.r=(float)tempD;
	FeaAttrTix->Attribute("tipColorG",&tempD);  feaNodeAttr->_shader._tipColor.g=(float)tempD;
	FeaAttrTix->Attribute("tipColorB",&tempD);  feaNodeAttr->_shader._tipColor.b=(float)tempD;

	FeaAttrTix->Attribute("uvProject",&tempI);  feaNodeAttr->_shader._uvProject=(bool)tempI;
	FeaAttrTix->Attribute("uvProjectScale",&tempD);  feaNodeAttr->_shader._uvProjectScale=(float)tempD;
	FeaAttrTix->Attribute("rootOpacity",&tempD);  feaNodeAttr->_shader._baseOpacity=(float)tempD;
	FeaAttrTix->Attribute("tipOpacity",&tempD);  feaNodeAttr->_shader._fadeOpacity=(float)tempD;
	FeaAttrTix->Attribute("fadeStart",&tempD);  feaNodeAttr->_shader._fadeStart=(float)tempD;

	FeaAttrTix->Attribute("barbuleDiffuse",&tempD);  feaNodeAttr->_shader._barbuleDiffuse=(float)tempD;
	FeaAttrTix->Attribute("barbuleSpecularColorR",&tempD);  feaNodeAttr->_shader._barbuleSpecularColor.r=(float)tempD;
	FeaAttrTix->Attribute("barbuleSpecularColorG",&tempD);  feaNodeAttr->_shader._barbuleSpecularColor.g=(float)tempD;
	FeaAttrTix->Attribute("barbuleSpecularColorB",&tempD);  feaNodeAttr->_shader._barbuleSpecularColor.b=(float)tempD;
	FeaAttrTix->Attribute("barbuleSpecular",&tempD);  feaNodeAttr->_shader._barbuleSpecular=(float)tempD;

	FeaAttrTix->Attribute("barbuleGloss",&tempD);  feaNodeAttr->_shader._barbuleGloss=(float)tempD;
	FeaAttrTix->Attribute("selfShadow",&tempD);  feaNodeAttr->_shader._selfShadow=(float)tempD;
	FeaAttrTix->Attribute("hueVar",&tempI);  feaNodeAttr->_shader._hueVar=tempI;
	FeaAttrTix->Attribute("satVar",&tempD);  feaNodeAttr->_shader._satVar=(float)tempD;
	FeaAttrTix->Attribute("valVar",&tempD);  feaNodeAttr->_shader._valVar=(float)tempD;
	FeaAttrTix->Attribute("varFreq",&tempD);  feaNodeAttr->_shader._varFreq=(float)tempD;

	FeaAttrTix->Attribute("uniformWidth",&tempI);  feaNodeAttr->_uniformWidth=(bool)tempI;
	FeaAttrTix->Attribute("randSeed",&tempI);  feaNodeAttr->_randSeed=tempI;

	FeaAttrTix->Attribute("outputFeatherMesh",&tempI);  feaNodeAttr->_outputFeatherMesh=(bool)tempI;
	FeaAttrTix->Attribute("meshUScale",&tempD);  feaNodeAttr->_meshUVScale[0]=(float)tempD;
	FeaAttrTix->Attribute("meshVScale",&tempD);  feaNodeAttr->_meshUVScale[1]=(float)tempD;
}

void xmlFeatherRead::getKeyBarbuleAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr)
{
	int tempI=0;double tempD=0;
	feaNodeAttr->_keyBarAttrs.startAngle.resize(4);
	FeaAttrTix->Attribute("stepAngle",&tempD);  feaNodeAttr->_keyBarAttrs.stepAngle=(float)tempD;
	FeaAttrTix->Attribute("startAngle1",&tempD);  feaNodeAttr->_keyBarAttrs.startAngle[0]=(float)tempD;
	FeaAttrTix->Attribute("startAngle2",&tempD);  feaNodeAttr->_keyBarAttrs.startAngle[1]=(float)tempD;
	FeaAttrTix->Attribute("startAngle3",&tempD);  feaNodeAttr->_keyBarAttrs.startAngle[2]=(float)tempD;
	FeaAttrTix->Attribute("startAngle4",&tempD);  feaNodeAttr->_keyBarAttrs.startAngle[3]=(float)tempD;
	
	feaNodeAttr->_keyBarAttrs.barbule.resize(4);
	FeaAttrTix->Attribute("barbulePos1",&tempD);  feaNodeAttr->_keyBarAttrs.barbule[0]=(float)tempD;
	FeaAttrTix->Attribute("barbulePos2",&tempD);  feaNodeAttr->_keyBarAttrs.barbule[1]=(float)tempD;
	FeaAttrTix->Attribute("barbulePos3",&tempD);  feaNodeAttr->_keyBarAttrs.barbule[2]=(float)tempD;
	FeaAttrTix->Attribute("barbulePos4",&tempD);  feaNodeAttr->_keyBarAttrs.barbule[3]=(float)tempD;
	}

void xmlFeatherRead::getRampsAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr)
{
	TiXmlElement *prampAttr=NULL;
	MString rampName;
	feaNodeAttr->_keyBarAttrs.ramp.resize(4);
	for(prampAttr = FeaAttrTix->FirstChildElement(); prampAttr; prampAttr = prampAttr->NextSiblingElement())
	{
		rampName = MString(prampAttr->Value());
		if(rampName == MString("keyBarbuleRamp1"))	getSingleRamp(prampAttr,feaNodeAttr->_keyBarAttrs.ramp[0]);
		if(rampName == MString("keyBarbuleRamp2"))	getSingleRamp(prampAttr,feaNodeAttr->_keyBarAttrs.ramp[1]);
		if(rampName == MString("keyBarbuleRamp3"))	getSingleRamp(prampAttr,feaNodeAttr->_keyBarAttrs.ramp[2]);
		if(rampName == MString("keyBarbuleRamp4"))	getSingleRamp(prampAttr,feaNodeAttr->_keyBarAttrs.ramp[3]);

		if(rampName == MString("rachisThickScale"))	getSingleRamp(prampAttr,feaNodeAttr->_rachisThickScale);
		if(rampName == MString("barbuleDensity"))	getSingleRamp(prampAttr,feaNodeAttr->_barbuleDensity);
		if(rampName == MString("leftBarbuleLengthScale")) getSingleRamp(prampAttr,feaNodeAttr->_leftBarbuleLengthScale);
		if(rampName == MString("rightBarbuleLengthScale"))	getSingleRamp(prampAttr,feaNodeAttr->_rightBarbuleLengthScale);
		if(rampName == MString("barbuleUpDownScale"))	getSingleRamp(prampAttr,feaNodeAttr->_barbuleUpDownScale);
		if(rampName == MString("barbuleUpDownRachis"))	getSingleRamp(prampAttr,feaNodeAttr->_barbuleUpDownRachis);
		if(rampName == MString("forceRotateScale"))	getSingleRamp(prampAttr,feaNodeAttr->_forceRotateScale);
		if(rampName == MString("forceScale"))	getSingleRamp(prampAttr,feaNodeAttr->_forceScale);

		if(rampName == MString("barbuleThickScale"))	getSingleRamp(prampAttr,feaNodeAttr->_barbuleThickScale);
		if(rampName == MString("barbuleThickAdjust"))	getSingleRamp(prampAttr,feaNodeAttr->_barbuleThickAdjust);
		
		if(rampName == MString("leftBarbuleGapDensity"))	getSingleRamp(prampAttr,feaNodeAttr->_gapDensity[0]);
		if(rampName == MString("rightBarbuleGapDensity"))	getSingleRamp(prampAttr,feaNodeAttr->_gapDensity[1]);

	}
}

void xmlFeatherRead::getSingleRamp(TiXmlElement *rampEntryTix,jcRamp &jcr)
{
	int tempI=0;
	double tempD=0;
	jcInterpolation rv;
	TiXmlElement *prampAttr=NULL;
	for(prampAttr = rampEntryTix->FirstChildElement(); prampAttr; prampAttr = prampAttr->NextSiblingElement())
	{
		if(MString(prampAttr->Value())!= MString("rampEntry"))continue;
		prampAttr->Attribute("position",&tempD);  rv._positions.push_back((float)tempD);
		prampAttr->Attribute("value",&tempD);  rv._values.push_back((float)tempD);
		prampAttr->Attribute("interpolate",&tempI);  rv._interps.push_back(tempI);
	}
	jcr.setSort(false);
	jcr.setEntries(rv);
}