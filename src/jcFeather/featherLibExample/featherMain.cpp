#include "jcFeatherSDK.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;


void printJCFeather(singleFeatherData *sgfdata)
{
	if(!sgfdata)return;

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
	for(int ii=0;ii<posPtNum*3;++ii)//points data
	{
		cout<<sgfdata->_featherPointPosition[ii]<<" ";
		if((ii+1)%3==0) cout<<"   "<<ii/3<<endl;
	}

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
	for(int ii=0;ii<widthNum;++ii)
		cout<<sgfdata->_featherPointWidth[ii]<<" ";
	cout<<endl<<endl;



	cout<<"feabaru : "<<endl;
	for(int ii=0;ii<uniformCurveVarNum;++ii)
		cout<<sgfdata->_barbulePosAtRachis[ii]<<" ";
	cout<<endl<<endl;



	cout<<"feather bar len per : "<<endl;
	for(int ii=0;ii<uniformCurveVarNum;++ii)
		cout<<sgfdata->_barbuleLenghtPer[ii]<<" ";
	cout<<endl<<endl;



	cout<<"texture : "<<sgfdata->_texture<<endl;

	cout<<endl<<"----------------Feather print end."<<endl<<endl;
}

int main(int argc, char *argv[])
{
	char paramstr[256];

	if(argc==2)
		strcpy(paramstr,argv[1]);
	else
	{
		cerr<<"Need a xml file path."<<endl;
		return 0;
	}
	
	//------set the jcFeather infomation verbose
	jcf_SetInfoVerbose(JCFINFO_DETAIL);
	
	//------open a xml feather cahce data
	if(!jcf_OpenXmlFile(paramstr))
		return 0;
		
	//------get xml global data
	xmlCacheFileAttr *cacheGlobalData = jcf_GetGlobalData();
	if(!cacheGlobalData)return 0;
	
	cout<<endl<<endl<<"----------------Begin Print JCFeather Info"<<endl<<endl<<endl;

	//-----print the bounding box of each feather element, the bbox includes all the motion blur leaves for each feather
	//-----so you can use this bbox as the procedual box for each feather
	for(int ii =0;ii<cacheGlobalData->_featherNum;++ii)
		cout<<"bounding box "<<ii<<"  "<<cacheGlobalData->_allBBox[ii].minX<<" "
			<<cacheGlobalData->_allBBox[ii].maxX<<" "
			<<cacheGlobalData->_allBBox[ii].minY<<" "
			<<cacheGlobalData->_allBBox[ii].maxY<<" "
			<<cacheGlobalData->_allBBox[ii].minZ<<" "
			<<cacheGlobalData->_allBBox[ii].maxZ<<endl;
	cout<<endl;

	for(int ii =0;ii<cacheGlobalData->_featherNum;++ii)//for each feather element
	{		
		//----get the feather node at blur 0, if motion blur is active , specify the blur index,if not,use 0
		jcFeatherNode *sgfNode = jcf_GetFeatherNode(ii,0);
		if(!sgfNode) continue;
		
		//-----the feather leaf num of the feather element ii
		int feaLeafNum = sgfNode->_meshFeaNum+sgfNode->_surfaceFeaNum;

		if(!cacheGlobalData->_motionBlurEnable)//----no motion blur
		{
			//----get the feather curves data at blur 0. If motion blur is active , specify the blur index, if not, use 0
			singleFeatherData* sgfData = jcf_GetFeathersData(ii,0);
			if(!sgfData) continue;
			
			cout<<"Feather name : "<<cacheGlobalData->_featherShape[ii]<<"  "<<ii<<endl<<endl;
			for(int kk=0;kk<cacheGlobalData->_blurNum;++kk)
				printJCFeather(&sgfData[kk]);
		}

		jcf_ClearEleFeathersData(ii);
	}

	return 1;
}