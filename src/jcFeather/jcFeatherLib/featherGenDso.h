#ifndef _featherGenDso_
#define _featherGenDso_

#include "jcFeatherSDKDataType.h"
#include "jcFeatherNodeData.h"
#include "featherGenBase.h"
#include "xmlFeatherRead.h"
using namespace std;

typedef vector< vector<featherInfo*> > FeaInfoVV;
typedef pair<int,int> EleBlurPair;

typedef struct 
{
	singleFeatherData* _feaCurves;
	int _feaNum;
}FeatherIndex;

//-------------------------------------------------------------------------------------------------------------------------------
class featherGenDso
{
	public:
		featherGenDso();
		~featherGenDso();
		

		//----xml data
		bool openXmlData(const char * file);// open xml file

		singleFeatherData *getFeatherCurveData( int eleIndex,int blurIndex );

		void clearAllFeatherCurves();
		void clearEleFeathers(int eleIndex);
		void clearMe();

		void setOverrideCurveType(int type);
	private:
		//----get feather node attr data
		bool getFeatherElementData(int eleIndex);
		void convertFeatherData(singleFeather* inData,
								singleFeatherData *outData,
								featherInfo* featherAttr);
		void clearFeatherElement();
		void clearFeatherInfo();
		void clearXmlGlobalData();

		void clearSingleFeatherData(singleFeatherData *feaData);
		void initSingleFeatherData(singleFeatherData *feaData);
		singleFeatherData* createSingleFeatherData(int num);

	public:
		vector<featherElement> _allFeaElement;
		xmlCacheFileAttr _globalAttr;

	private:
		xmlFeatherRead _xmlReader;//xml read class

		FeaInfoVV _feaAttrInfo;//featherInfo[element][blur]
		map<EleBlurPair,FeatherIndex> _allFeaCurvesIndices;		//store all the singleFeather data

		
};

#endif
