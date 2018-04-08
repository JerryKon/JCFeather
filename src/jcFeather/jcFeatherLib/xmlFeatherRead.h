#ifndef _xmlFeatherRead_
#define _xmlFeatherRead_

#include <vector>
#include <string>
#include <stdarg.h>
#include <map>

#include "tinyxml.h"
#include "featherInfo.h"
#include "jcFeatherNodeData.h"
#include "jcFeatherLibTools.h"

using namespace std;

class xmlFeatherRead
{
	public:
		xmlFeatherRead();
		~xmlFeatherRead();
		//---import xml data to memory
		bool xmlImport(const char* fileName);
		bool getFeatherAttributeAtStep(  featherInfo *feaNodeAttr,
										jcFeatherNodeData &feaNodeData,//get bbox and render attr for jcFeather
										const char* featherShape,
										int step);

		bool getFeathersBBox(map<string,jcBoundingBox> &nameBox);

		bool getFCPath(char* outPath,const char* feaName,int step);
	protected:
		void clearXmlGlobalData();
		bool getCacheFileAttr(TiXmlElement *cacheAttrTix);
		bool getRenderAttr(TiXmlElement *FeaAttrTix,jcFeatherNodeData &feaEleAttr);
		void getSimpleAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr);
		void getKeyBarbuleAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr);
		void getRampsAttrFromJCFea(TiXmlElement *FeaAttrTix,featherInfo *feaNodeAttr);
		void getSingleRamp(TiXmlElement *rampEntryTix,jcRamp &jcr);

	private:
		TiXmlDocument _feaXMLDoc;//xml file doc
		TiXmlElement *_baseEle;//jcFeather_CacheFile element
		vector<TiXmlElement*> _blurStep;//blur element
		int _maxJCFeatherNodes;
	public:
		xmlCacheFileAttr *_globalAttrPt;
		int _overrideCurveType;//-1 时用xml文件中的curveType
};
#endif