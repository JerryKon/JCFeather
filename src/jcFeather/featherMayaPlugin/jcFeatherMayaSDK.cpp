
#include <vector>
#include <maya/MItDependencyNodes.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>

#include "featherGen.h"
#include "jcFeather.h"
#include "jcFeatherMayaSDK.h"

typedef struct 
{
	featherCurves* _sdkFeatherData;
	int _sdkFeatherNum;
}FeatherIndex;

vector<FeatherIndex> sdkFeather;
bool sdkInit=false;

featherCurves::featherCurves()
{
	_featherPointPosition = NULL;
	_featherPointWidth = NULL;
	_featherCurveNormal = NULL;
	_barbulePosAtRachis=NULL;
	_barbuleLenghtPer=NULL;

	_leftBarbuleNum = 0;
	_rightBarbuleNum = 0;
	_rachisPointNum = 0;
	_barbulePointNum = 0;

	_randSeed=0;
	_surfaceUV[0]=_surfaceUV[1]=0;
	_mainColor[0]=_mainColor[1]=_mainColor[2]=1;
	strcpy(_texture,"None");
}

featherCurves::~featherCurves()
{
	if(_featherPointPosition)
		delete _featherPointPosition;

	if(_featherPointWidth)
		delete _featherPointWidth;

	if(_featherCurveNormal)
		delete _featherCurveNormal;
	if(_barbulePosAtRachis)
		delete _barbulePosAtRachis;
	if(_barbuleLenghtPer)
		delete _barbuleLenghtPer;
}

void    initFeatherGen( featherGen &genFeather)
{
	genFeather.init();
	genFeather._feaExpInf._fileType = featherExportInfo::SDK;
	genFeather._feaExpInf._onlyGetBBox =false;
	genFeather._feaExpInf._renderCurveFuncion=jcSpline::kCatmullRom;
	genFeather._featherAttrs = new featherInfo;
}

void    convertFeatherToSdkMaya(singleFeather* inData,
								featherCurves* outData,
								featherInfo* featherAttr)
{
	if(!inData||!outData) return;

	outData->_leftBarbuleNum=inData->leftBarbules.size();
	outData->_rightBarbuleNum=inData->rightBarbules.size();

	outData->_rachisPointNum = inData->rachis.length();
	outData->_barbulePointNum=featherAttr->_barbuleSegments;

	//-------------------------------------malloc curve pos point
	int posPtNum=   outData->_rachisPointNum + 
					outData->_leftBarbuleNum*outData->_barbulePointNum +
					outData->_rightBarbuleNum*outData->_barbulePointNum;

	outData->_featherPointPosition = (jcVertex*)malloc( sizeof(jcVertex) * posPtNum );
	

	//--------------------------------------get rachis
	//-----------------------------------
	//-----------------------------------
	int feaPtIndex=0;
	int curveNumIndex=0;
	for(int ii=0;ii<outData->_rachisPointNum;++ii)
	{
		outData->_featherPointPosition[feaPtIndex].x = inData->rachis[ii].x;
		outData->_featherPointPosition[feaPtIndex].y = inData->rachis[ii].y;
		outData->_featherPointPosition[feaPtIndex].z = inData->rachis[ii].z;
		feaPtIndex++;
	}


	//-----------------------------------get left barbules
	//-----------------------------------
	//-----------------------------------
	int barPt=0;
	for(int ii=0;ii<outData->_leftBarbuleNum;++ii)
	{	
		barPt = inData->leftBarbules[ii].length();
		for(int jj=0;jj<barPt;++jj)
		{
			outData->_featherPointPosition[feaPtIndex].x = inData->leftBarbules[ii][jj].x;
			outData->_featherPointPosition[feaPtIndex].y = inData->leftBarbules[ii][jj].y;
			outData->_featherPointPosition[feaPtIndex].z = inData->leftBarbules[ii][jj].z;
			feaPtIndex++;
		}
	}


	//-----------------------------------get right barbules
	for(int ii=0;ii<outData->_rightBarbuleNum;++ii)
	{
		barPt = inData->rightBarbules[ii].length();
		for(int jj=0;jj<barPt;++jj)
		{
			outData->_featherPointPosition[feaPtIndex].x = inData->rightBarbules[ii][jj].x;
			outData->_featherPointPosition[feaPtIndex].y = inData->rightBarbules[ii][jj].y;
			outData->_featherPointPosition[feaPtIndex].z = inData->rightBarbules[ii][jj].z;
			feaPtIndex++;
		}
	}


	//-----------------------------------get width data
	//-----------------------------------
	//-----------------------------------
	outData->_useUniformWidth = featherAttr->_uniformWidth;

	int uniformCurveVarNum= 1 + 
							outData->_leftBarbuleNum +
							outData->_rightBarbuleNum;
	int varyingCurveVarNum= outData->_rachisPointNum + 
							outData->_leftBarbuleNum * outData->_barbulePointNum +
							outData->_rightBarbuleNum * outData->_barbulePointNum;

	int widthIndex=0;

	if(outData->_useUniformWidth)
	{
		outData->_featherPointWidth = (float*)malloc( sizeof(float) * uniformCurveVarNum  );

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

		for( int jj=0;jj<inData->rachis.length();++jj )
			outData->_featherPointWidth[widthIndex++] = inData->rachisWidth[jj]* inData->rachisLength*0.01f;

		for( int ii=0;ii<outData->_leftBarbuleNum;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
				outData->_featherPointWidth[widthIndex++] =  inData->barbuleWidth[jj]*
															 inData->leftBarbuleWidth[ii]* 
															 inData->rachisLength*0.01f;

		for( int ii=0;ii<outData->_rightBarbuleNum;++ii )
			for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
				outData->_featherPointWidth[widthIndex++] = inData->barbuleWidth[jj]*
															inData->rightBarbuleWidth[ii]* 
															inData->rachisLength*0.01f;
	}



	//-----------------------------------------normal
	//----------------------------------------
	int rachisSize= inData->rachisNormal.length();
	int leftSize=inData->leftBarbuleNormal.size();
	int rightSize=inData->rightBarbuleNormal.size();

	outData->_featherCurveNormal = (jcVertex*)malloc( sizeof(jcVertex) * varyingCurveVarNum );
		
	int varIndex=0;
	for( int jj=0;jj<rachisSize;++jj )
	{
		outData->_featherCurveNormal[varIndex].x = inData->rachisNormal[jj].x;
		outData->_featherCurveNormal[varIndex].y = inData->rachisNormal[jj].y;
		outData->_featherCurveNormal[varIndex].z = inData->rachisNormal[jj].z;
		varIndex++;
	}
	for( int ii=0;ii<leftSize;++ii )
		for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
		{
			outData->_featherCurveNormal[varIndex].x =  inData->leftBarbuleNormal[ii][jj].x;
			outData->_featherCurveNormal[varIndex].y =  inData->leftBarbuleNormal[ii][jj].y;
			outData->_featherCurveNormal[varIndex].z =  inData->leftBarbuleNormal[ii][jj].z;
			varIndex++;
		}

	for( int ii=0;ii<rightSize;++ii )
		for(int jj=0;jj<featherAttr->_barbuleSegments;++jj)
		{
			outData->_featherCurveNormal[varIndex].x =  inData->rightBarbuleNormal[ii][jj].x;
			outData->_featherCurveNormal[varIndex].y =  inData->rightBarbuleNormal[ii][jj].y;
			outData->_featherCurveNormal[varIndex].z =  inData->rightBarbuleNormal[ii][jj].z;	
			varIndex++;
		}

	//--------------------------------------barbule u
	//-----------------------------------
	//-----------------------------------
	outData->_barbulePosAtRachis = (float*)malloc( sizeof(float) * uniformCurveVarNum  );

	int barbUIndex=0;
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
	if(inData->colorTexOverride.length()!=0)
		strcpy( outData->_texture,inData->colorTexOverride.asChar() );
	else
		strcpy(outData->_texture,"None");

	MPoint minPt,maxPt;
	minPt=inData->proxyBBox.min();
	maxPt=inData->proxyBBox.max();

	outData->_proxyBBox[0]=minPt.x;	outData->_proxyBBox[1]=maxPt.x;
	outData->_proxyBBox[2]=minPt.y;	outData->_proxyBBox[3]=maxPt.y;
	outData->_proxyBBox[4]=minPt.z;	outData->_proxyBBox[5]=maxPt.z;
}


int     jcFeatherMaya::GetVersion()
{
	return featherGen::jcFea_SDKVersion;
}

void    jcFeatherMaya::ClearFeatherPointers()
{
	for(int ii=0;ii<sdkFeather.size();++ii)
		if(sdkFeather[ii]._sdkFeatherData) 
			free(sdkFeather[ii]._sdkFeatherData);

	sdkFeather.clear();
}

bool    jcFeatherMaya::Init()
{
	if( !featherTools::checkLicense() )
		return false;	

	ClearFeatherPointers();
	sdkInit = true;

	return true;
}

void    jcFeatherMaya::GetActiveJCFeatherNode(MObjectArray &featherAry,bool visibleOnly)
{
	jcFeather::getActiveJCFeatherNode(featherAry,visibleOnly);
}

featherCurves* jcFeatherMaya::GetFeatherCurveData(MObject &featherNode,int &feaNum)
{
	if(!sdkInit) 
	{
		MGlobal::displayError("jcFeather : Init sdk first.");
		return NULL;
	}
	MFnDependencyNode dgNodeFn;
	dgNodeFn.setObject(featherNode);
	MString nodeName=dgNodeFn.name();
	if( dgNodeFn.typeId() != jcFeather::id )
	{
		MGlobal::displayError("jcFeather : "+nodeName+" is not a jcFeather node.");
		return NULL;
	}

	featherGen featherSdkGen;
	initFeatherGen(featherSdkGen);

	if( ! featherSdkGen.initWithJCFeather( featherNode ) ) 
	{
		MGlobal::displayError("jcFeather : "+nodeName+" is not a valid jcFeather node.");
		if(featherSdkGen._featherAttrs) delete featherSdkGen._featherAttrs;
		return NULL;
	}	

	vector<singleFeather> renderFeather;
	if(!dgNodeFn.findPlug(jcFeather::render).asBool() ||
	   !featherSdkGen.computeJCFeatherNode(featherNode,renderFeather))
	{
		MGlobal::displayError("jcFeather : "+nodeName+" is not renderable.");
		if(featherSdkGen._featherAttrs) delete featherSdkGen._featherAttrs;
		return NULL;
	}

	feaNum = renderFeather.size();
	if(feaNum==0)
	{
		MGlobal::displayWarning("jcFeather : "+nodeName+" has no feathers.");
		if(featherSdkGen._featherAttrs) delete featherSdkGen._featherAttrs;
		return NULL;
	}

	featherCurves* feaData = (featherCurves*)malloc(sizeof(featherCurves)*feaNum);
	
	for(int ii=0;ii<feaNum;++ii)
	{
		feaData[ii]._nodeLeafIndex = ii;
		convertFeatherToSdkMaya(&renderFeather[ii],&feaData[ii],featherSdkGen._featherAttrs);
	}

	FeatherIndex tempIndex;
	tempIndex._sdkFeatherData = feaData;
	tempIndex._sdkFeatherNum=feaNum;
	sdkFeather.push_back(tempIndex);

	if(featherSdkGen._featherAttrs) delete featherSdkGen._featherAttrs;

	return feaData;
}

void    jcFeatherMaya::ClearAll()
{
	ClearFeatherPointers();
}
