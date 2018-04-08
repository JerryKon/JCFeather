#ifndef _deligthTools_
#define _deligthTools_

#include "jcFeatherSDK.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "ri.h"

#if defined(_WINDOWS)
#define DELIGHT_DLLEXPORT __declspec(dllexport)
#else
#define DELIGHT_DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

//3delight procedual functions
RtPointer DELIGHT_DLLEXPORT ConvertParameters(RtString paramstr);
RtVoid    DELIGHT_DLLEXPORT Subdivide(RtPointer data, float detail);
RtVoid    DELIGHT_DLLEXPORT Free(RtPointer data);

RtVoid    Subdivide_JCFInElement(RtPointer data, float detail);
RtVoid    Free_JCFInElement(RtPointer data);

xmlCacheFileAttr *cacheGlobalData=NULL;

void printJCFeather(singleFeatherData *sgfdata)
	{
		if(!sgfdata)return;
		cout <<"Element id "<<sgfdata->_nodeIndex<<endl;
		//---get the total point num of feather curves
		int posPtNum=  sgfdata->_rachisPointNum*sgfdata->_shader->_hasRachis + 
						sgfdata->_leftBarbuleNum*sgfdata->_barbulePointNum +
						sgfdata->_rightBarbuleNum*sgfdata->_barbulePointNum;
		


		//----get the feather curve num of this leaf of feather
		int cuveNum = sgfdata->_shader->_hasRachis  + sgfdata->_leftBarbuleNum + sgfdata->_rightBarbuleNum;
		cout<<"curve num : "<<cuveNum<<
			"  left barb num : "<<sgfdata->_leftBarbuleNum<<
			"  right barb num : "<<sgfdata->_rightBarbuleNum<<endl;
		


		//print curve segments for each curve in this feather
		cout<<endl<<"curve segments : ";
		cout<<"rachis segments "<<sgfdata->_rachisPointNum<<endl;
		cout<<"barbule segments for each barbule "<<sgfdata->_barbulePointNum<<endl;
		cout<<endl<<endl;



		cout<<"point position num : "<<posPtNum<<endl;
		//for(int ii=0;ii<posPtNum*3;++ii)//points data
		//{
		//	cout<<sgfdata->_featherPointPosition[ii]<<" ";
		//	if((ii+1)%3==0) cout<<"   "<<ii/3<<endl;
		//}

		int uniformCurveVarNum= sgfdata->_shader->_hasRachis + 
								sgfdata->_leftBarbuleNum +
								sgfdata->_rightBarbuleNum;
	
		int widthNum=0;
		if(sgfdata->_useUniformWidth)
			widthNum = uniformCurveVarNum;
		else
		{
			int varyingWidthNum=0;
			if(sgfdata->_shader->_curveType==0)
				//linear curve
				varyingWidthNum =	sgfdata->_rachisPointNum  + 
							( sgfdata->_leftBarbuleNum + sgfdata->_rightBarbuleNum )*sgfdata->_barbulePointNum;
			else
				//cubic curve
				varyingWidthNum =	sgfdata->_rachisPointNum - 2  + 
									( sgfdata->_leftBarbuleNum + sgfdata->_rightBarbuleNum )*(sgfdata->_barbulePointNum-2);
			widthNum = varyingWidthNum;
		}



		cout<<endl<<"uniform width : "<<sgfdata->_useUniformWidth<<
			  " width num : "<<widthNum<<endl;
		//for(int ii=0;ii<widthNum;++ii)
		//	cout<<sgfdata->_featherPointWidth[ii]<<" ";
		//cout<<endl<<endl;



		//cout<<"feabaru : "<<endl;
		//for(int ii=0;ii<uniformCurveVarNum;++ii)
		//	cout<<sgfdata->_barbulePosAtRachis[ii]<<" ";
		//cout<<endl<<endl;



		//cout<<"feather bar len per : "<<endl;
		//for(int ii=0;ii<uniformCurveVarNum;++ii)
		//	cout<<sgfdata->_barbuleLenghtPer[ii]<<" ";
		//cout<<endl<<endl;



		cout<<"texture : "<<sgfdata->_texture<<endl;

		cout<<endl<<"----------------Feather print end."<<endl<<endl;
	}

void        printJCBBox(const jcBoundingBox &bbox)
{
	cout<<bbox.minX<<" "<<bbox.maxX<<" "
		<<bbox.minY<<" "<<bbox.maxY<<" "
		<<bbox.minZ<<" "<<bbox.maxZ<<endl;
}

void inline addRenderStates(const mayaRenderAttr &rdAttr)
{
		RtString visRefl[1]={"+_3dfm_not_visible_in_reflections"};
		if(!rdAttr._visibleInReflections)
			RiAttribute("grouping","string membership",visRefl,RI_NULL);
		
		RtString visRefr[1]={"+_3dfm_not_visible_in_refractions"};
		if(!rdAttr._visibleInRefractions)
			RiAttribute("grouping","string membership",visRefr,RI_NULL);
		
		int visibCam[1]={0};
		if(!rdAttr._primaryVisibility)
		{
			RiIfBegin("!defined(ShadowMapRendering)");
			RiAttribute("visibility","camera",visibCam,RI_NULL);
			RiIfEnd();
		}
		if(!rdAttr._castsShadow)
		{
			RiIfBegin("defined(ShadowMapRendering)");
			RiAttribute("visibility","camera",visibCam,RI_NULL);
			RiIfEnd();
		}

		float closeIt[1]={0};
		if(!rdAttr._receiveShadow)
			RiAttribute("user","float delight_receive_shadows",closeIt,RI_NULL);
}

void inline jcFeatherRiDclare(const char* shader,const char* feaName,const mayaRenderAttr &rdAttr)
{
	if(strlen(shader)==0)
	{
		cout<<"JCFeather : Empty shader name. Use default jcFeather shader."<<endl;
		RiSurface("jcFeather",RI_NULL);
	}
	else
		RiSurface(shader,RI_NULL);

	RtString name[1] = { feaName };
	RiAttribute("identifier","name",name,RI_NULL);

	RtInt dice[1]={cacheGlobalData->_diceHair};
	RiAttribute("dice","int hair",dice,RI_NULL);

	addRenderStates(rdAttr);
	RiBasis(RiCatmullRomBasis,1,RiCatmullRomBasis,1);

	RiDeclare("jcFeaRachisRootC","constant color");
	RiDeclare("jcFeaRachisTipC","constant color");

	RiDeclare("jcFeaSurU","constant float");
	RiDeclare("jcFeaSurV","constant float");

	RiDeclare("jcFeaBarU","uniform float");
	RiDeclare("jcFeaBarLenPer","uniform float");
	RiDeclare("jcFeaRootC","constant color");
	RiDeclare("jcFeaTipC","constant color");
	RiDeclare("jcFeaMainC","constant color");
	RiDeclare("jcFeaBaseOpacity","constant float");
	RiDeclare("jcFeaFadeOpacity","constant float");
	RiDeclare("jcFeaFadeStart","constant float");
	RiDeclare("jcFeaUVProjectScale","constant float");
	RiDeclare("jcFeaColorTex","constant string");
	
	RiDeclare("jcFeadiff","constant float");
	RiDeclare("jcFeagloss","constant float");
	RiDeclare("jcFeaselfshad","constant float");
	RiDeclare("jcFeaspec","constant float");
	RiDeclare("jcFeaspecColor","constant color");

	RiDeclare("jcFeaHueVar","constant float");
	RiDeclare("jcFeaSatVar","constant float");
	RiDeclare("jcFeaValVar","constant float");
	RiDeclare("jcFeaVarFreq","constant float");
	RiDeclare("jcFeaRandSeed","constant float");

	RiShadingInterpolation("smooth");
}

void        renderSingleFeather(singleFeatherData *curveData)
{
	//printJCFeather(curveData);
	if(curveData->_useUniformWidth)
		RiDeclare("width","uniform float");
	else
		RiDeclare("width","varying float");
	
	RtToken curveType;
	if(curveData->_shader->_curveType==0)
		curveType = "linear";
	else
		curveType="cubic";

	int curveNum = (int)((unsigned int)curveData->_shader->_hasRachis + curveData->_leftBarbuleNum + curveData->_rightBarbuleNum);
	
	float rndSeed[1]={(float)curveData->_randSeed};
	float uvProj[1]={(float)curveData->_shader->_uvProject};
	float hueVar[1]={(float)curveData->_shader->_hueVar};
	
	RtString tex[1];
	tex[0]=curveData->_shader->_texture;
	if(string(curveData->_texture).compare("None"))	tex[0]=curveData->_texture;

	//get curve segments num
	int *segments=(int*)malloc( sizeof(int)*
								((int)curveData->_shader->_hasRachis+
								curveData->_leftBarbuleNum+
								curveData->_rightBarbuleNum));
	int segIndex=0;
	if(curveData->_shader->_hasRachis )
		segments[segIndex++] = curveData->_rachisPointNum;
	for(int ii=0;
		ii<(curveData->_leftBarbuleNum + curveData->_rightBarbuleNum);
		++ii)
		segments[segIndex++] = curveData->_barbulePointNum;

	//Ìí¼ÓtextureÊôÐÔ
	if(!cacheGlobalData->_hasBarbuleNormal)
		RiCurves(curveType,	 (RtInt)curveNum, (RtInt*)segments,
				 "nonperiodic",RI_P,(RtPointer)curveData->_featherPointPosition,
				 "width",curveData->_featherPointWidth,
				 "jcFeaBarU",curveData->_barbulePosAtRachis,
				 "jcFeaBarLenPer",curveData->_barbuleLenghtPer,
				 "jcFeaRandSeed",rndSeed,
				 "jcFeaColorTex",tex,

				 "jcFeaUVProjectScale",uvProj,
				 "jcFeaMainC",curveData->_mainColor,
				 "jcFeaRootC",curveData->_shader->_barbuleRootColor,
				 "jcFeaTipC",curveData->_shader->_barbuleTipColor,
				 "jcFeaRachisRootC",curveData->_shader->_rachisRootColor,
				 "jcFeaRachisTipC",curveData->_shader->_rachisTipColor,
				 "jcFeaBaseOpacity",&curveData->_shader->_baseOpacity,
				 
				 "jcFeaFadeOpacity",&curveData->_shader->_fadeOpacity,
				 "jcFeaFadeStart",&curveData->_shader->_fadeStart,
				 "jcFeadiff",&curveData->_shader->_barbuleDiffuse,			 
				 
				 "jcFeagloss",&curveData->_shader->_barbuleGloss,
				 "jcFeaHueVar",hueVar,
				 "jcFeaSatVar",&curveData->_shader->_satVar,				 
				 "jcFeaValVar",&curveData->_shader->_valVar,

				 "jcFeaVarFreq",&curveData->_shader->_varFreq,
				 "jcFeaspec",&curveData->_shader->_barbuleSpecular,		 
				 "jcFeaspecColor",curveData->_shader->_barbuleSpecularColor,
				 "jcFeaselfshad",&curveData->_shader->_selfShadow,
				 
				 "jcFeaSurU",&curveData->_surfaceUV[0],
				 "jcFeaSurV",&curveData->_surfaceUV[1],
				 RI_NULL
		);
	else
		RiCurves(curveType,	 (RtInt)curveNum, (RtInt*)segments,
				 "nonperiodic",RI_P,(RtPointer)curveData->_featherPointPosition,
				 "N",curveData->_featherCurveNormal,
				 "width",curveData->_featherPointWidth,
				 "jcFeaBarU",curveData->_barbulePosAtRachis,
				 "jcFeaBarLenPer",curveData->_barbuleLenghtPer,
				 "jcFeaRandSeed",rndSeed,
				 "jcFeaColorTex",tex,

				 "jcFeaUVProjectScale",uvProj,
				 "jcFeaMainC",curveData->_mainColor,
				 "jcFeaRootC",curveData->_shader->_barbuleRootColor,
				 "jcFeaTipC",curveData->_shader->_barbuleTipColor,
				 "jcFeaRachisRootC",curveData->_shader->_rachisRootColor,
				 "jcFeaRachisTipC",curveData->_shader->_rachisTipColor,
				 "jcFeaBaseOpacity",&curveData->_shader->_baseOpacity,
				 
				 "jcFeaFadeOpacity",&curveData->_shader->_fadeOpacity,
				 "jcFeaFadeStart",&curveData->_shader->_fadeStart,
				 "jcFeadiff",&curveData->_shader->_barbuleDiffuse,			 
				 
				 "jcFeagloss",&curveData->_shader->_barbuleGloss,
				 "jcFeaHueVar",hueVar,
				 "jcFeaSatVar",&curveData->_shader->_satVar,				 
				 "jcFeaValVar",&curveData->_shader->_valVar,

				 "jcFeaVarFreq",&curveData->_shader->_varFreq,
				 "jcFeaspec",&curveData->_shader->_barbuleSpecular,		 
				 "jcFeaspecColor",curveData->_shader->_barbuleSpecularColor,
				 "jcFeaselfshad",&curveData->_shader->_selfShadow,
				 
				 "jcFeaSurU",&curveData->_surfaceUV[0],
				 "jcFeaSurV",&curveData->_surfaceUV[1],
				 RI_NULL
		);		
	free(segments);
}

void        renderFeatherEle(int index)
{
	jcFeatherNode *sgfNode = jcf_GetFeatherNode(index,0);
	if(!sgfNode) 
	{
		cerr<<"Cannot find node for feather element "<<index<<" named "
		<<cacheGlobalData->_featherShape[index]<<endl;
		return;
	}
	int feaLeafNum = sgfNode->_meshFeaNum+sgfNode->_surfaceFeaNum;
	
	RiTransformBegin();
		RiAttributeBegin();
			if(!cacheGlobalData->_blurSteps || !sgfNode->_renderAttr._motionBlur)//----no motion blur
			{
				singleFeatherData* sgfData = jcf_GetFeathersData(index,0);
				if(!sgfData) 
				{
					cerr<<"Cannot find feather curve data for feather element "<<index<<" named "
					<<cacheGlobalData->_featherShape[index]<<endl;
					return;
				}

				//if(!sgfData->_shader->_useOutShader)
					jcFeatherRiDclare(cacheGlobalData->_globalShaderName,cacheGlobalData->_featherShape[index],sgfNode->_renderAttr);
				//else
				//	jcFeatherRiDclare(sgfData->_shader->_shaderName,cacheGlobalData->_featherShape[index],sgfNode->_renderAttr);
                
				for(int kk=0;kk<feaLeafNum;++kk)
					renderSingleFeather(&sgfData[kk]);
			}
			else//-------------motion blur enable
			{
				vector<singleFeatherData*> feathersPtr;
				for(int jj=0;jj<cacheGlobalData->_blurNum;++jj)
				{
					singleFeatherData* tempsgf= jcf_GetFeathersData(index,jj);
					feathersPtr.push_back(tempsgf );
				}
				
				//if(!feathersPtr[0]->_shader->_useOutShader)
					jcFeatherRiDclare(cacheGlobalData->_globalShaderName,cacheGlobalData->_featherShape[index],sgfNode->_renderAttr);
				//else
					//jcFeatherRiDclare(feathersPtr[0]->_shader->_shaderName,cacheGlobalData->_featherShape[index],sgfNode->_renderAttr);

				for(int kk=0;kk<feaLeafNum;++kk)
				{
					RiMotionBeginV(cacheGlobalData->_blurNum,cacheGlobalData->_blurSteps);
						for(int jj=0;jj<cacheGlobalData->_blurNum;++jj)
							renderSingleFeather(&feathersPtr[jj][kk]);					
					RiMotionEnd();
				}
			}
		RiAttributeEnd();
	RiTransformEnd();
	jcf_ClearEleFeathersData(index);
}

//----------render out all feathers once, called in 3delight entry piont
void        renderAll()
{
	//-----render feather element one by one
	for(int ii =0;ii<cacheGlobalData->_featherNum;++ii)//for each feather element
		renderFeatherEle(ii);
}

//----------render feather element one by one, called in 3delight endtry point
void        renderInElement()
{
	for(int ii =0;ii<cacheGlobalData->_featherNum;++ii)
	{
		RtBound feaBox={cacheGlobalData->_allBBox[ii].minX , cacheGlobalData->_allBBox[ii].maxX,
						cacheGlobalData->_allBBox[ii].minY , cacheGlobalData->_allBBox[ii].maxY,
						cacheGlobalData->_allBBox[ii].minZ , cacheGlobalData->_allBBox[ii].maxZ};
		int *eleId= new int;
		*eleId = ii;
		RiProcedural(eleId,feaBox,Subdivide_JCFInElement,Free_JCFInElement);
	}
}


#ifdef __cplusplus
}

#endif

#endif