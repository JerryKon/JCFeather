#include "xmlFeatherAttr.h"
#include "boostTools.h"
#include "featherGenBase.h"

xmlFeatherAttr::xmlFeatherAttr(){init();}

xmlFeatherAttr::~xmlFeatherAttr()
{
	init();
}

void xmlFeatherAttr::init()
{
	_feaXMLDoc=NULL;
	_baseEle=NULL;
	_featherShape.clear();
	_blurStep.clear();
	_xmlFilePath="";
}

bool xmlFeatherAttr::xmlCreate(featherExportInfo &expInfo)
{
	_feaXMLDoc = new TiXmlDocument;
    if (!_feaXMLDoc)  return false;

    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0","","");
    if (!pDeclaration)  return false;
	_feaXMLDoc->LinkEndChild(pDeclaration);

 	_baseEle = new TiXmlElement("jcFeather_CacheFile");
	if (!_baseEle)		return false;
	_feaXMLDoc->LinkEndChild(_baseEle);

	_baseEle->SetAttribute("jcFeatherVersion",expInfo.jcFeatherVersion.asChar());
	xmlAddAttribute(_baseEle,"sdkVersion",expInfo.needSdkVersion);
	_baseEle->SetAttribute("mayaVersion",expInfo.mayaVersion.asChar());

	_baseEle->SetAttribute("currentFrame",featherTools::getPadding(expInfo._frame).asChar());
	_baseEle->SetAttribute("shaderName",expInfo.featherRib.asChar());
	xmlAddAttribute(_baseEle,"delayRead",(int)expInfo._delayArchive);

	xmlAddAttribute(_baseEle,"exportBarbuleNormal",(int)expInfo.exportNormal);
	xmlAddAttribute(_baseEle,"diceHair",(int)expInfo.diceHair);
	if(expInfo.enableMotionBlur)
	{
		xmlAddAttribute(_baseEle,"motionBlur",(int)expInfo.enableMotionBlur);
		xmlAddAttribute(_baseEle,"blurNum",expInfo._mbSample);
		xmlAddAttribute(_baseEle,"blurStep",expInfo._mbFactor * expInfo._mbStep );
	}

	_blurStep.resize(expInfo._mbSample);
	for(unsigned int ii=0;ii<expInfo._mbSample;++ii)
	{
 		_blurStep[ii] = new TiXmlElement("blur");
		if (!_blurStep[ii])	return false;

		_baseEle->LinkEndChild(_blurStep[ii]);
		_blurStep[ii]->SetAttribute("num",(int)ii);
	}
	return true;
}

void xmlFeatherAttr::addTotalBoundingBox(const MBoundingBox &bbox)
{
	xmlAddAttribute(_baseEle,"minX",(float)bbox.min().x);
	xmlAddAttribute(_baseEle,"maxX",(float)bbox.max().x);
	xmlAddAttribute(_baseEle,"minY",(float)bbox.min().y);
	xmlAddAttribute(_baseEle,"maxY",(float)bbox.max().y);
	xmlAddAttribute(_baseEle,"minZ",(float)bbox.min().z);
	xmlAddAttribute(_baseEle,"maxZ",(float)bbox.max().z);
}

bool xmlFeatherAttr::xmlAddData(TiXmlElement *xmlEle,const char *eleName,...)//(eleName,name1,value1,name2,value2,"\0")���һ��һ�����
{
	va_list argp;
	int argno=0;
	char *para0,*para1;
	va_start(argp,eleName);

	TiXmlElement *pAttr = new TiXmlElement(eleName);
	if (NULL==pAttr)	return false;
	xmlEle->LinkEndChild(pAttr);

	while(1)
	{
		para0 = va_arg(argp,char*);
		if(strcmp(para0,"\0")==0) break;
		para1 = va_arg(argp,char*);
		if(strcmp(para1,"\0")==0) break;
		pAttr->SetAttribute(para0,para1);
		argno+=2;
	}

    if(strcmp(para0,"\0")==0) return true;
	else return false;
}

void xmlFeatherAttr::xmlAddAttribute(TiXmlElement* ele,const char *n,int v)
{
	ele->SetAttribute(n,v);
}

void xmlFeatherAttr::xmlAddAttribute(TiXmlElement* ele,const char *n,float v)
{
	const int chSize=50;
	char ch[chSize];
	snprintf(ch,chSize,"%f",v);
	ele->SetAttribute(n,ch);
}

void xmlFeatherAttr::xmlAddAttribute(TiXmlElement* ele,const char *n,MRampAttribute &rv)
{
	MFloatArray pos,value;
	MIntArray index,interps;
	rv.getEntries(index,pos,value,interps);

	TiXmlElement *pfeaRamp = new TiXmlElement(n);
	if (!pfeaRamp)	return;
	ele->LinkEndChild(pfeaRamp);

	for(unsigned int ii=0;ii<rv.getNumEntries();ii++)
	{
		TiXmlElement *prampAttr = new TiXmlElement("rampEntry");
		if (!prampAttr)	continue;
		pfeaRamp->LinkEndChild(prampAttr);

		xmlAddAttribute(prampAttr,"position",pos[ii]);
		xmlAddAttribute(prampAttr,"value",value[ii]);
		xmlAddAttribute(prampAttr,"interpolate",interps[ii]);
	}
}

void xmlFeatherAttr::xmlAddAttribute(TiXmlElement* ele,const char *n,jcRamp &jcr)
{
	TiXmlElement *pfeaRamp = new TiXmlElement(n);
	if (!pfeaRamp)	return;
	ele->LinkEndChild(pfeaRamp);

	jcInterpolation rv;
	jcr.getEntries(rv);
	for(unsigned int ii=0;ii<rv._positions.size();ii++)
	{
		TiXmlElement *prampAttr = new TiXmlElement("rampEntry");
		if (!prampAttr)	continue;
		pfeaRamp->LinkEndChild(prampAttr);

		xmlAddAttribute(prampAttr,"position",rv._positions[ii]);
		xmlAddAttribute(prampAttr,"value",rv._values[ii]);
		xmlAddAttribute(prampAttr,"interpolate",rv._interps[ii]);
	}
}

bool xmlFeatherAttr::xmlAddFeather(featherInfo* xmlFeaAttr,
									const char* feaName,
									int currentSample,
									const MBoundingBox &bbox,
									const featherRenderAttr	&renderAttr
									)//Ϊÿ��feather������
{
	if(!_baseEle || !xmlFeaAttr||!_blurStep[currentSample]) return false;

	MString markFeather(feaName);
	markFeather+=".";
	markFeather+=currentSample;

	_featherShape.append(markFeather);


	TiXmlElement *pjcFeather = new TiXmlElement("jcFeather");
	if (!pjcFeather)	return false;

	_blurStep[currentSample]->LinkEndChild(pjcFeather);

	pjcFeather->SetAttribute("nodeName",feaName);

	xmlAddAttribute(pjcFeather,"caShad",renderAttr._castsShadow);
	xmlAddAttribute(pjcFeather,"recSh",renderAttr._receiveShadow);
	xmlAddAttribute(pjcFeather,"mb",renderAttr._motionBlur);
	xmlAddAttribute(pjcFeather,"primVis",renderAttr._primaryVisibility);
	xmlAddAttribute(pjcFeather,"visRefl",renderAttr._visibleInReflections);
	xmlAddAttribute(pjcFeather,"visRefr",renderAttr._visibleInRefractions);

	xmlAddAttribute(pjcFeather,"minX",(float)bbox.min().x);
	xmlAddAttribute(pjcFeather,"maxX",(float)bbox.max().x);
	xmlAddAttribute(pjcFeather,"minY",(float)bbox.min().y);
	xmlAddAttribute(pjcFeather,"maxY",(float)bbox.max().y);
	xmlAddAttribute(pjcFeather,"minZ",(float)bbox.min().z);
	xmlAddAttribute(pjcFeather,"maxZ",(float)bbox.max().z);

	TiXmlElement *pfeaAttr = new TiXmlElement("simpleAttributes");
	if (!pfeaAttr)	return false;
	pjcFeather->LinkEndChild(pfeaAttr);

	//--general properties
	xmlAddAttribute(pfeaAttr,"curveType",(int)xmlFeaAttr->_curveType);
	xmlAddAttribute(pfeaAttr,"direction",(int)xmlFeaAttr->_direction);
	xmlAddAttribute(pfeaAttr,"surfaceFeather",(int)xmlFeaAttr->_exactFeather);
	xmlAddAttribute(pfeaAttr,"renderable",(int)xmlFeaAttr->_renderable);

	//---rachis
	xmlAddAttribute(pfeaAttr,"rachisRenderAs",(int)xmlFeaAttr->_rachisRenderAs);
	xmlAddAttribute(pfeaAttr,"rachisSegment",xmlFeaAttr->_rachisSegment);
	xmlAddAttribute(pfeaAttr,"rachisPos",xmlFeaAttr->_rachisPos);
	xmlAddAttribute(pfeaAttr,"rachisStart",xmlFeaAttr->_rachisStart);
	xmlAddAttribute(pfeaAttr,"rachisEnd",xmlFeaAttr->_rachisEnd);
	xmlAddAttribute(pfeaAttr,"rachisSides",xmlFeaAttr->_rachisSides);
	xmlAddAttribute(pfeaAttr,"rachisThick",xmlFeaAttr->_rachisThick);
	//rachis material
	xmlAddAttribute(pfeaAttr,"rachisRootColorR",xmlFeaAttr->_shader._rachisRootColor.r);
	xmlAddAttribute(pfeaAttr,"rachisRootColorG",xmlFeaAttr->_shader._rachisRootColor.g);
	xmlAddAttribute(pfeaAttr,"rachisRootColorB",xmlFeaAttr->_shader._rachisRootColor.b);
	xmlAddAttribute(pfeaAttr,"rachisTipColorR",xmlFeaAttr->_shader._rachisTipColor.r);
	xmlAddAttribute(pfeaAttr,"rachisTipColorG",xmlFeaAttr->_shader._rachisTipColor.g);
	xmlAddAttribute(pfeaAttr,"rachisTipColorB",xmlFeaAttr->_shader._rachisTipColor.b);
	//barbule
	xmlAddAttribute(pfeaAttr,"barbuleNum",xmlFeaAttr->_barbuleNum);
	xmlAddAttribute(pfeaAttr,"barbuleSegments",xmlFeaAttr->_barbuleSegments);
	xmlAddAttribute(pfeaAttr,"shapeSymmetry",xmlFeaAttr->_shapeSymmetry);
	xmlAddAttribute(pfeaAttr,"barbuleLength",xmlFeaAttr->_barbuleLength);
	xmlAddAttribute(pfeaAttr,"barbuleLengthRandScale",xmlFeaAttr->_barbuleLengthRandScale);

	xmlAddAttribute(pfeaAttr,"LGapForce",xmlFeaAttr->_gapForce[0]);
	xmlAddAttribute(pfeaAttr,"RGapForce",xmlFeaAttr->_gapForce[1]);
	xmlAddAttribute(pfeaAttr,"LGapMaxForce",xmlFeaAttr->_gapMaxForce[0]);
	xmlAddAttribute(pfeaAttr,"RGapMaxForce",xmlFeaAttr->_gapMaxForce[1]);
	xmlAddAttribute(pfeaAttr,"LGapSize",xmlFeaAttr->_gapSize[0]);
	xmlAddAttribute(pfeaAttr,"RGapSize",xmlFeaAttr->_gapSize[1]);

	xmlAddAttribute(pfeaAttr,"forcePerSegment",xmlFeaAttr->_forcePerSegment);
	xmlAddAttribute(pfeaAttr,"turnForce",xmlFeaAttr->_turnForce);
	xmlAddAttribute(pfeaAttr,"rotateLamda",xmlFeaAttr->_rotateLamda);
	xmlAddAttribute(pfeaAttr,"upDownLamda",xmlFeaAttr->_upDownLamda);
	xmlAddAttribute(pfeaAttr,"upDownNoise",(int)xmlFeaAttr->_upDownNoise);

	xmlAddAttribute(pfeaAttr,"rachisNoiseFrequency",xmlFeaAttr->_rachisNoiseFrequency);
	xmlAddAttribute(pfeaAttr,"barbuleNoiseFrequency",xmlFeaAttr->_barbuleNoiseFrequency);
	xmlAddAttribute(pfeaAttr,"noisePhaseX",xmlFeaAttr->_noisePhase[0]);
	xmlAddAttribute(pfeaAttr,"noisePhaseY",xmlFeaAttr->_noisePhase[1]);

	xmlAddAttribute(pfeaAttr,"barbuleThick",xmlFeaAttr->_barbuleThick);
	xmlAddAttribute(pfeaAttr,"barbuleRandThick",xmlFeaAttr->_barbuleRandThick);
	//barbule material
	xmlAddAttribute(pfeaAttr,"useOutShader",xmlFeaAttr->_useOutShader);
	if(xmlFeaAttr->_useOutShader)
		pfeaAttr->SetAttribute("outShaderName",xmlFeaAttr->_shaderName.asChar());

	xmlAddAttribute(pfeaAttr,"rootColorR",xmlFeaAttr->_shader._rootColor.r);
	xmlAddAttribute(pfeaAttr,"rootColorG",xmlFeaAttr->_shader._rootColor.g);
	xmlAddAttribute(pfeaAttr,"rootColorB",xmlFeaAttr->_shader._rootColor.b);
	xmlAddAttribute(pfeaAttr,"tipColorR",xmlFeaAttr->_shader._tipColor.r);
	xmlAddAttribute(pfeaAttr,"tipColorG",xmlFeaAttr->_shader._tipColor.g);
	xmlAddAttribute(pfeaAttr,"tipColorB",xmlFeaAttr->_shader._tipColor.b);

	if(xmlFeaAttr->_shader._barbuleTexture.length()!=0)
		pfeaAttr->SetAttribute("barbuleTexture",xmlFeaAttr->_shader._barbuleTexture.asChar());
	else
		pfeaAttr->SetAttribute("barbuleTexture","None");

	xmlAddAttribute(pfeaAttr,"uvProject",(int)xmlFeaAttr->_shader._uvProject);
	xmlAddAttribute(pfeaAttr,"uvProjectScale",xmlFeaAttr->_shader._uvProjectScale);
	xmlAddAttribute(pfeaAttr,"rootOpacity",xmlFeaAttr->_shader._baseOpacity);
	xmlAddAttribute(pfeaAttr,"tipOpacity",xmlFeaAttr->_shader._fadeOpacity);
	xmlAddAttribute(pfeaAttr,"fadeStart",xmlFeaAttr->_shader._fadeStart);


	xmlAddAttribute(pfeaAttr,"barbuleDiffuse",xmlFeaAttr->_shader._barbuleDiffuse);
	xmlAddAttribute(pfeaAttr,"barbuleSpecularColorR",xmlFeaAttr->_shader._barbuleSpecularColor.r);
	xmlAddAttribute(pfeaAttr,"barbuleSpecularColorG",xmlFeaAttr->_shader._barbuleSpecularColor.g);
	xmlAddAttribute(pfeaAttr,"barbuleSpecularColorB",xmlFeaAttr->_shader._barbuleSpecularColor.b);
	xmlAddAttribute(pfeaAttr,"barbuleSpecular",xmlFeaAttr->_shader._barbuleSpecular);

	xmlAddAttribute(pfeaAttr,"barbuleGloss",xmlFeaAttr->_shader._barbuleGloss);
	xmlAddAttribute(pfeaAttr,"selfShadow",xmlFeaAttr->_shader._selfShadow);

	xmlAddAttribute(pfeaAttr,"hueVar",xmlFeaAttr->_shader._hueVar);
	xmlAddAttribute(pfeaAttr,"satVar",xmlFeaAttr->_shader._satVar);
	xmlAddAttribute(pfeaAttr,"valVar",xmlFeaAttr->_shader._valVar);
	xmlAddAttribute(pfeaAttr,"varFreq",xmlFeaAttr->_shader._varFreq);

	//render properties
	xmlAddAttribute(pfeaAttr,"uniformWidth",(int)xmlFeaAttr->_uniformWidth);
	xmlAddAttribute(pfeaAttr,"randSeed",xmlFeaAttr->_randSeed);
	//output mesh
	xmlAddAttribute(pfeaAttr,"outputFeatherMesh",(int)xmlFeaAttr->_outputFeatherMesh);//�Ƿ��������Σ���dso��ȷ��
	xmlAddAttribute(pfeaAttr,"meshUScale",xmlFeaAttr->_meshUVScale[0]);
	xmlAddAttribute(pfeaAttr,"meshVScale",xmlFeaAttr->_meshUVScale[1]);

	TiXmlElement *pfeaKeyBar = new TiXmlElement("keyBarbule");
	if (!pfeaKeyBar)	return false;
	pjcFeather->LinkEndChild(pfeaKeyBar);

	xmlAddAttribute(pfeaKeyBar,"stepAngle",xmlFeaAttr->_keyBarAttrs.stepAngle);
	xmlAddAttribute(pfeaKeyBar,"startAngle1",xmlFeaAttr->_keyBarAttrs.startAngle[0]);
	xmlAddAttribute(pfeaKeyBar,"startAngle2",xmlFeaAttr->_keyBarAttrs.startAngle[1]);
	xmlAddAttribute(pfeaKeyBar,"startAngle3",xmlFeaAttr->_keyBarAttrs.startAngle[2]);
	xmlAddAttribute(pfeaKeyBar,"startAngle4",xmlFeaAttr->_keyBarAttrs.startAngle[3]);

	xmlAddAttribute(pfeaKeyBar,"barbulePos1",xmlFeaAttr->_keyBarAttrs.barbule[0]);
	xmlAddAttribute(pfeaKeyBar,"barbulePos2",xmlFeaAttr->_keyBarAttrs.barbule[1]);
	xmlAddAttribute(pfeaKeyBar,"barbulePos3",xmlFeaAttr->_keyBarAttrs.barbule[2]);
	xmlAddAttribute(pfeaKeyBar,"barbulePos4",xmlFeaAttr->_keyBarAttrs.barbule[3]);

	TiXmlElement *pfeaRamps = new TiXmlElement("ramps");
	if (!pfeaRamps)	return false;
	pjcFeather->LinkEndChild(pfeaRamps);

	xmlAddAttribute(pfeaRamps,"keyBarbuleRamp1",xmlFeaAttr->_keyBarAttrs.ramp[0]);
	xmlAddAttribute(pfeaRamps,"keyBarbuleRamp2",xmlFeaAttr->_keyBarAttrs.ramp[1]);
	xmlAddAttribute(pfeaRamps,"keyBarbuleRamp3",xmlFeaAttr->_keyBarAttrs.ramp[2]);
	xmlAddAttribute(pfeaRamps,"keyBarbuleRamp4",xmlFeaAttr->_keyBarAttrs.ramp[3]);

	xmlAddAttribute(pfeaRamps,"rachisThickScale",xmlFeaAttr->_rachisThickScale);

	xmlAddAttribute(pfeaRamps,"barbuleDensity",xmlFeaAttr->_barbuleDensity);
	xmlAddAttribute(pfeaRamps,"leftBarbuleLengthScale",xmlFeaAttr->_leftBarbuleLengthScale);
	xmlAddAttribute(pfeaRamps,"rightBarbuleLengthScale",xmlFeaAttr->_rightBarbuleLengthScale);

	xmlAddAttribute(pfeaRamps,"barbuleUpDownScale",xmlFeaAttr->_barbuleUpDownScale);
	xmlAddAttribute(pfeaRamps,"barbuleUpDownRachis",xmlFeaAttr->_barbuleUpDownRachis);
	xmlAddAttribute(pfeaRamps,"forceRotateScale",xmlFeaAttr->_forceRotateScale);
	xmlAddAttribute(pfeaRamps,"forceScale",xmlFeaAttr->_forceScale);

	xmlAddAttribute(pfeaRamps,"barbuleThickScale",xmlFeaAttr->_barbuleThickScale);
	xmlAddAttribute(pfeaRamps,"barbuleThickAdjust",xmlFeaAttr->_barbuleThickAdjust);

	xmlAddAttribute(pfeaRamps,"leftBarbuleGapDensity",xmlFeaAttr->_gapDensity[0]);
	xmlAddAttribute(pfeaRamps,"rightBarbuleGapDensity",xmlFeaAttr->_gapDensity[1]);
	return true;
}
