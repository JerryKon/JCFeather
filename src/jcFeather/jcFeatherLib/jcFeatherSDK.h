#ifndef _jcFeatherSDK_
#define _jcFeatherSDK_

#include "jcFeatherSDKDataType.h"
#include <string.h>
#include <stdlib.h>

#if defined(JCFEATHER_LIB_EXPORTS)
#define JCFea_DLLEXPORT __declspec(dllexport)
#else
#define JCFea_DLLEXPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif

//Return the sdk version
JCFea_DLLEXPORT int                         jcf_GetSDKVersion();

//Set jcFeather information ouput level
JCFea_DLLEXPORT void                        jcf_SetInfoVerbose(jcfInfoVerbose verbose);

/*
  Open xml cache file data and read in the cache data, your code needs to start with this function.
  Return false, if error occurs
*/
JCFea_DLLEXPORT bool                        jcf_OpenXmlFile(const char* file);

/*
Get xml file global data for this feahter cache
Return NULL if error occurs
*/
JCFea_DLLEXPORT  xmlCacheFileAttr*			jcf_GetGlobalData();


/*
Get the feather node data at [eleIndex][blurIndex] in xml file, the eleIndex should be pushed in first
Return NULL if error occurs.
*/
JCFea_DLLEXPORT  jcFeatherNode*				jcf_GetFeatherNode(int eleIndex,int blurIndex);


/*
get [element,blur] feather's feather curves data. The returned pointer has more than one leaf of feather.
featherLeafNum = _surfaceFeaNum + _meshFeaNum ,  find more info int struct jcFeatherNode
You can only use the pointer returned by this function, but not edit the data in this pointer.
JCFeather Sdk will free the memory of this pointer.

Return NULL if error occurs.
*/
JCFea_DLLEXPORT  singleFeatherData*			jcf_GetFeathersData( int eleIndex,int blurIndex );

/*
Every time you call "jcf_GetSingleFeatherData" or "jcf_GetFeathersData" , new singleFeatherData* pointer will be added to jcFeather engine.
You can use "jcf_ClearFeathersData" to clear the feather curves memory once you are sure of that all the feather curves will not be used.
Or just leave it to jcFeather engine to free the memroy.
After clearing, you need to call "jcf_GetFeathersData" or "jcf_GetFeathersData" to get feather curves.
At the end of your application, jcFeather will also clear all the memory.
*/
JCFea_DLLEXPORT void                        jcf_ClearAllFeathersData();

/*
	Clear the singleFeatherData* whose element is eleIndex. You can call this function after rendering the specified element to free memory.
*/
JCFea_DLLEXPORT void                        jcf_ClearEleFeathersData(int eleIndex);

/*
	type 

	-1: use type in xml 
	0 : linear 
	1 : cubic
	default : use type in xml

	All the feathers will use this type. Override xml cache attributs.
	Call this function before " jcf_OpenXmlFile(const char* file) "
*/
JCFea_DLLEXPORT void                        jcf_OverrideCurveType(int type);

#ifdef __cplusplus
}
#endif


#endif