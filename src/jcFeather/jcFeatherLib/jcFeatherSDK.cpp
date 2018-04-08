#include "jcFeatherSDK.h"
#include "featherGenDso.h"

#if defined(_OPENMP)
	#include <omp.h>
#endif

bool jcFea_UseMultiThread = true; 
int jcFea_ProcessorNum = omp_get_num_procs();
int jcFea_MaxThreads = jcFea_ProcessorNum;

featherGenDso jcFeatherGen;
jcfInfoVerbose jcFeaSDKInfoVerbose = JCFINFO_WARNING;

using namespace std;

//------------------------------------------native
void inline           jcf_ne_PrintInfo(const char *info,jcfInfoVerbose verboseNeed)
{
	if(jcFeaSDKInfoVerbose >= verboseNeed)
	{
		if( verboseNeed == JCFINFO_ERROR )
			cerr<<"JCFeather Error : "<<info<<endl;
		else if( verboseNeed == JCFINFO_WARNING )
			cout<<"JCFeather Warning : "<<info<<endl;
		else
			cout<<"JCFeather : "<<info<<endl;
	}
}

bool inline           jcf_ne_inRange(int index,int start,int end)//for vector array index check
{
	if( index>=start && index<end ) 
		return true;
	else 
		return false;
}


//------------------------------------------export

int                   jcf_GetSDKVersion()
{
	return jcFea_SDKVersion;
}

void                  jcf_SetInfoVerbose(jcfInfoVerbose verbose)
{
	jcFeaSDKInfoVerbose = verbose;
}

bool                  jcf_OpenXmlFile(const char* file)
{
	if(!jcFeatherLibTools::checkLicense()) return false;

	if( !jcFeatherGen.openXmlData(file) ) 
	{
		string info("Cannot load xml ");
		info.append(file);
		jcf_ne_PrintInfo(info.c_str(),JCFINFO_ERROR);

		return false;
	}

	return true;
}

xmlCacheFileAttr     *jcf_GetGlobalData()
{
	if(!jcFeatherLibTools::checkLicense()) return NULL;
	return &jcFeatherGen._globalAttr;
}


jcFeatherNode        *jcf_GetFeatherNode(int eleIndex,int blurIndex)
{
	return &jcFeatherGen._allFeaElement[eleIndex]._feaNodedata[blurIndex]._node;
}

singleFeatherData    *jcf_GetFeathersData(  int eleIndex,int blurIndex )
{
	return jcFeatherGen.getFeatherCurveData(eleIndex,blurIndex);
}

void                  jcf_ClearAllFeathersData()
{
	jcFeatherGen.clearAllFeatherCurves();
}

void                  jcf_ClearEleFeathersData(int eleIndex)
{
	jcFeatherGen.clearEleFeathers(eleIndex);
}

void                  jcf_OverrideCurveType(int type)
{
	jcFeatherGen.setOverrideCurveType(type);
}

