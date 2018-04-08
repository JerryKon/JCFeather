#include <jcFeatherMayaSdk.h>

#include <maya/MSimple.h>
#include <maya/MGlobal.h>
#include <maya/MObjectArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFloatArray.h>
DeclareSimpleCommand( feasdk, "Jerry Kon", "any");

MStatus feasdk::doIt( const MArgList& args )
//
{
	MStatus stat = MS::kSuccess;

	MObjectArray feaObjs;
	jcFeatherMaya::Init();
	jcFeatherMaya::GetActiveJCFeatherNode(feaObjs,true);
	
	MDoubleArray result;
	MFnDependencyNode dgNodeFn;
	for(int ii=0;ii<feaObjs.length();++ii)
	{
		int feaNum=0;
		dgNodeFn.setObject(feaObjs[ii]);
		MString str = dgNodeFn.name();
		MGlobal::displayInfo(str);

		featherCurves* curveData=jcFeatherMaya::GetFeatherCurveData(feaObjs[ii],feaNum);
		if( !curveData || feaNum==0 )continue;

		int pointNum=curveData->_rachisPointNum + (curveData->_leftBarbuleNum + curveData->_rightBarbuleNum) *curveData->_barbulePointNum;
		
		result.append(pointNum);
		result.append(curveData->_rachisPointNum);
		result.append(curveData->_barbulePointNum);
		result.append(curveData->_leftBarbuleNum);
		result.append(curveData->_rightBarbuleNum);
		for(int kk=0;kk<pointNum;++kk)
		{
			result.append(curveData->_featherPointPosition[kk].x);
			result.append(curveData->_featherPointPosition[kk].y);
			result.append(curveData->_featherPointPosition[kk].z);
		}
		jcFeatherMaya::ClearFeatherPointers();
	}

	setResult( result );
	jcFeatherMaya::ClearAll();

	return stat;
}