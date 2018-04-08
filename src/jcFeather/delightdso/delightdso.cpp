#include "delightTools.h"


using namespace std;

#ifdef __cplusplus

extern "C" {

#endif

//------------------3delight dso entry point
RtPointer DELIGHT_DLLEXPORT ConvertParameters(RtString paramstr)
{
	jcf_SetInfoVerbose(JCFINFO_DETAIL);
	
	string *xmlPath=new string;
	*xmlPath = string(paramstr);
	return (RtPointer)xmlPath;
}

RtVoid DELIGHT_DLLEXPORT Subdivide(RtPointer blinddata, RtFloat detailsize)
{
	//system("pause");

	string *file=static_cast<string*>(blinddata);
	if(  !file || !jcf_OpenXmlFile( file->c_str() )  ) return;

	cacheGlobalData = jcf_GetGlobalData();
	if(!cacheGlobalData)return;

	if(!cacheGlobalData->_delayRead)
		renderAll();
	else
		renderInElement();
}

RtVoid DELIGHT_DLLEXPORT Free(RtPointer blinddata)
{
	string *path = static_cast<string*>(blinddata);
	if(path) delete path;
	cout<<"JCFeather : Delight Dso Render For Frame "<<cacheGlobalData->_currentFrame<<endl;
}


//------------------render feather in element
RtVoid Subdivide_JCFInElement(RtPointer blinddata, RtFloat detailsize)//对整个feather的boundingbox计算
{
	int *eleId=(int*) blinddata;
	if(eleId)
		renderFeatherEle(*eleId);
}

RtVoid Free_JCFInElement(RtPointer blinddata)
{
	int *eleId=(int*) blinddata;
	if(eleId)		
		delete eleId;
}


#ifdef __cplusplus
}

#endif