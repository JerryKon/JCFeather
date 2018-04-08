#ifndef _jcFeatherNodeData_
#define _jcFeatherNodeData_

#include "jcFeatherSDKDataType.h"
#include "featherGenBase.h"
#include "jcFeatherLibTools.h"

#include <iostream>

using namespace std;

//store each jcFeather Element node attribute data
class  jcFeatherNodeData
{
	friend class featherGenDso;
	friend class xmlFeatherRead;

	public:
		jcFeatherNodeData();
		~jcFeatherNodeData();
	private:
		/*
		set 
			_node._surfaceFeaNum
			_node._meshFeaNum
			_node._feaBBox
			_node._bbox
		*/
		bool  getDataFromFCFile();
		/*set 
			_node._shader 
			featherInfo pointer
		*/
		void  setFeaInfo(featherInfo* feainfo);

	private:
		featherInfo* _shapeInfo;
		surfaceTurtleIO _fcFileData;
	public:
		/* set 2 attribute in xmlreader
			_node._fcPath
			_node._renderAttr
		*/
		jcFeatherNode _node;
};

typedef struct 
{
	int _feaIndex;//feather node index in xml file
	vector<jcFeatherNodeData> _feaNodedata;//for each blur step
}featherElement;

#endif