#ifndef _xmlFeatherAttr_
#define _xmlFeatherAttr_

#include "tinyxml.h"
#include "featherInfo.h"

#include <string>
#include <stdarg.h>

struct featherExportInfo;

class xmlFeatherAttr
{
	public:
		xmlFeatherAttr();
		~xmlFeatherAttr();
		void init();

		//---export feather info and global settings info
		bool xmlCreate( featherExportInfo &expInfo);
		bool xmlAddData(TiXmlElement *xmlEle,const char *eleName,...);
		inline void xmlAddAttribute(TiXmlElement* ele,const char *n,int v);
		void xmlAddAttribute(TiXmlElement* ele,const char *n,float v);
		void xmlAddAttribute(TiXmlElement* ele,const char *n,MRampAttribute &rv);
		void xmlAddAttribute(TiXmlElement* ele,const char *n,jcRamp &jcr);

		//bool alreadyHasFeather(const MString &n);
		//void updateJCFeaBBox(TiXmlElement *feaTix,const MBoundingBox &bbox,bool featherNativeBBox);//add extend bbox from jcFeatherSystem

		//---add jcFeather Data
		bool xmlAddFeather(featherInfo *xmlFeaAttr,
							const char* feaName,
							int currentSample,
							const MBoundingBox &bbox,
							const featherRenderAttr	&renderAttr);
		void addTotalBoundingBox(const MBoundingBox &bbox);

	public:
		TiXmlDocument *_feaXMLDoc;//xml file doc
		TiXmlElement *_baseEle;//jcFeather_CacheFile element
		vector<TiXmlElement*> _blurStep;//blur element
		MStringArray _featherShape;//all the featherShape name
		MString _xmlFilePath;
};

#endif