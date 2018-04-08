#ifndef _jcFeatherLibTools_
#define _jcFeatherLibTools_

#include <iostream>
#include "jcFeatherSDKDataType.h"

using namespace std;

#define JCFEATHER_ERROR(value,info)							\
{														\
	if ( !value ) cerr<< "JCFeather Error : "<<info<< endl;		\
}

class jcFeatherLibTools
{
public:
	jcFeatherLibTools(){};
	~jcFeatherLibTools(){};

public:
	static void initBBox(float *box)
	{
		box[0]=std::numeric_limits<float>::max();
		box[1]=std::numeric_limits<float>::min();
		box[2]=box[4]=box[0];
		box[3]=box[5]=box[1];
	}

	static void expandBBox(float *box1,float *box2)
	{
		box1[0]= min(box1[0],box2[0]);
		box1[1]= max(box1[1],box2[1]);
		box1[2]= min(box1[2],box2[2]);
		box1[3]= max(box1[3],box2[3]);
		box1[4]= min(box1[4],box2[4]);
		box1[5]= max(box1[5],box2[5]);
	}

	static void initJCBBox(jcBoundingBox &box)
	{
		box.minX=std::numeric_limits<float>::max();
		box.maxX=std::numeric_limits<float>::min();
		box.minY = box.minZ=box.minX;
		box.maxY = box.maxZ=box.maxX;
	}

	static void expandJCBBox(jcBoundingBox &box,const jcBoundingBox &boxNew)
	{
		box.minX=min(boxNew.minX,box.minX);
		box.minY=min(boxNew.minY,box.minY);
		box.minZ=min(boxNew.minZ,box.minZ);

		box.maxX=max(boxNew.maxX,box.maxX);
		box.maxY=max(boxNew.maxY,box.maxY);
		box.maxZ=max(boxNew.maxZ,box.maxZ);
	}

	static void expandJCBBoxF(jcBoundingBox &box,float *boxNew)
	{
		box.minX=min(box.minX,boxNew[0]);
		box.minY=min(box.minY,boxNew[2]);
		box.minZ=min(box.minZ,boxNew[3]);

		box.maxX=max(box.maxX,boxNew[1]);
		box.maxY=max(box.maxY,boxNew[3]);
		box.maxZ=max(box.maxZ,boxNew[5]);
	}

	static void inline printBBox(float *box)
	{
		cout<<"box : "<<box[0]<<" "<<box[1]<<" "<<box[2]<<" "<<box[3]<<" "<<box[4]<<" "<<box[5]<<endl;
	}

	static void inline printJCBBox(const jcBoundingBox &boxNew)
	{
		cout<<"box : "<<boxNew.minX<<" "<<boxNew.maxX<<" "
			<<boxNew.minY<<" "<<boxNew.maxY<<" "
			<<boxNew.minZ<<" "<<boxNew.maxZ
			<<endl;
	}

	static bool checkLicense()
	{
		return true;
	}

	static void printJCFeather(singleFeatherData *sgfdata)
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
};

#endif