
#include "jcFeatherToCurves.h"
#include "jcFeather.h"
#include "jcFeatherSystem.h"
#include "boostTools.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNumericAttribute.h>
jcFeatherToCurves::jcFeatherToCurves()
{
	addAttr = false;
}

jcFeatherToCurves::~jcFeatherToCurves()
{
}

void *jcFeatherToCurves::creator()
{
   return new jcFeatherToCurves;
}

bool jcFeatherToCurves::isUndoable() const
{
   return false;
}

MSyntax jcFeatherToCurves::newSyntax()
{
   MSyntax syntax;
   CHECK_MSTATUS( syntax.addFlag("-n", "-nodes", MSyntax::kString) );
   CHECK_MSTATUS( syntax.addFlag("-aa", "-addAttr", MSyntax::kBoolean) );
   return syntax;
}

bool jcFeatherToCurves::getFlagData(const MArgList& args)
{
	MArgDatabase argData(syntax(), args);

	//-----------------------------------------get cmd infos
	MString objs;
	if( argData.isFlagSet("-nodes") )
	{
		argData.getFlagArgument("-nodes", 0, objs);
		objs.split(' ',objects);
	}
	else
		objs = "";

	if( argData.isFlagSet("-addAttr") )
		argData.getFlagArgument("-addAttr", 0, addAttr);

	return true;
}

void jcFeatherToCurves::getJCFeatherArrayNodesData(MStringArray &resultCurves)
{
	MFnDependencyNode dgNodeFn;
	MStatus status = MS::kFailure;

	MPlug jcSysInFeaPlug;
	MPlugArray jcSysInFeaPlugConArray;
	
	MSelectionList selList;
	MDagPath dagP;
	resultCurves.clear();

	if(objects.length()>0)
	{
	    for(int ii=0;ii<objects.length();++ii)
			selList.add(objects[ii],false);
	}
	else
		MGlobal::getActiveSelectionList(selList);
	
	MString curvesGrp;

	if(selList.length()>0)
	{
		for(unsigned int ii=0;ii<selList.length();ii++)
		{
			curvesGrp.clear();
			if( selList.getDagPath(ii,dagP) != MS::kSuccess) continue;
			if( dagP.apiType()==MFn::kTransform && 
				dagP.extendToShape()!=MS::kSuccess)
				continue;

			dgNodeFn.setObject(dagP.node());
			if( dgNodeFn.typeId() == jcFeather::id && 
				dgNodeFn.findPlug(jcFeather::active,false).asBool())
			{
				unsigned int curDeg = 1;
				if( (dgNodeFn.findPlug(jcFeather::curveType,false).asShort())==1)
					curDeg = 3;
				convertToCurves ( curDeg,
								  static_cast<jcFeather*>(dgNodeFn.userNode())->featherDraw,
								  dgNodeFn.name(),
								  curvesGrp);
			}
			else if(dgNodeFn.typeId() == jcFeatherSystem::id && dgNodeFn.findPlug(jcFeatherSystem::active,false).asBool())
			{
				jcSysInFeaPlug = dgNodeFn.findPlug(jcFeatherSystem::inRenderFeather,false,&status);
				if( !jcSysInFeaPlug.connectedTo(jcSysInFeaPlugConArray,true,false,&status) )
					continue;
				
				MFnDependencyNode dgNodeFn1;
				dgNodeFn1.setObject(jcSysInFeaPlugConArray[0].node());
				if(dgNodeFn1.typeId()==jcFeather::id && dgNodeFn1.findPlug(jcFeather::active,false).asBool())
				{
					unsigned int curDeg = 1;
					if( (dgNodeFn.findPlug(jcFeather::curveType,false).asShort())==1)
						curDeg = 3;
					convertToCurves ( curDeg,
									  static_cast<jcFeatherSystem*>(dgNodeFn.userNode())->featherDraw,
									  dgNodeFn.name(),
									  curvesGrp);
				}
			}
			resultCurves.append(curvesGrp);
		}
	}
}

MStatus jcFeatherToCurves::pointArrayToCurve(int deg,const MFloatPointArray &fpts,MObject &curveObj)
{
	MFnNurbsCurve curveFn;
	MPointArray pts;
	MDoubleArray knotSequences;
	MStatus stat;
    unsigned  int ncvs    = 0;                   // Number of CVs
    unsigned  int spans   = 0;   // Number of spans
    unsigned  int nknots  = 0;// Number of knots


	floatPAryToPAry(fpts,pts);
	ncvs    = pts.length();
	spans   = ncvs - deg;
	nknots  = spans+2*deg-1;

	knotSequences.clear();
	if( deg==3 )
	{
		nknots -= 4 ;
		knotSequences.append( 0 );
		knotSequences.append( 0 );
	}
	unsigned int jj=0;
	for(;jj<nknots;++jj)
		knotSequences.append( (double)jj );
	if( deg==3 )
	{
		knotSequences.append( jj-1 );
		knotSequences.append( jj-1 );
	}
	
	curveObj= curveFn.create( pts,
						knotSequences, deg, 
						MFnNurbsCurve::kOpen, 
						false, false, 
						MObject::kNullObj, 
						&stat );
	return stat;

}

void jcFeatherToCurves::addBarbCurveAttr( MObject &obj ,const barbAttr &value)
{
	MFnNumericAttribute numFn;
	MObject dynamicAttr = numFn.create("feaBarbU", "fbu", MFnNumericData::kFloat,0);
	MFnDependencyNode dgFn(obj);
	dgFn.addAttribute(dynamicAttr);
	
	dgFn.findPlug("feaBarbU").setFloat(value._barbU);
}

void jcFeatherToCurves::convertToCurves(unsigned int deg,
					const vector<singleFeather> &curveData,
					MString nodeName,
					MString &resultGroup)
{
	MStatus stat;
	MFnDagNode dagFn,dagFn1,dagFn2;
	MObject topRootGrp = dagFn.create("transform",(nodeName+"_CurveRoot#"),MObject::kNullObj,&stat);
	MFnDependencyNode dgNodeFn(topRootGrp);
	resultGroup = dgNodeFn.name();
	barbAttr tempBarAttr;

	MObject tempObj;
	for(int ii=0;ii<curveData.size();++ii)
	{
		MObject topGrp = dagFn.create("transform",(nodeName+"_"+ii+"_Curve#"),topRootGrp,&stat);
		//---------------------------rachis curve
		if( pointArrayToCurve(deg,curveData[ii].rachis,tempObj)==MS::kSuccess)
		{
			dgNodeFn.setObject(tempObj);
			dgNodeFn.setName((nodeName+"_Rachis#"));
			dagFn.addChild(tempObj);
		}

		//--------------------------------------
		MObject leftGrp = dagFn1.create("transform",(nodeName+"_LeftBarb#"),topGrp,&stat);
		MObject rightGrp = dagFn2.create("transform",(nodeName+"_RightBarb#"),topGrp,&stat);
		int barbSize = curveData[ii].leftBarbules.size();
		for(int kk=0;kk<barbSize;++kk)
			if( pointArrayToCurve(deg,curveData[ii].leftBarbules[kk],tempObj)==MS::kSuccess)
			{
				dgNodeFn.setObject(tempObj);
				dgNodeFn.setName((nodeName+"_LBarb#"));

				if(addAttr){
					tempBarAttr._barbU = curveData[ii].leftBarbuleRachisPos[kk];
					addBarbCurveAttr( tempObj ,tempBarAttr);
				}
				dagFn1.addChild(tempObj);
			}

		//--------------------------------------
		barbSize = curveData[ii].rightBarbules.size();
		for(int kk=0;kk<barbSize;++kk)
			if( pointArrayToCurve(deg,curveData[ii].rightBarbules[kk],tempObj)==MS::kSuccess)
			{
				dgNodeFn.setObject(tempObj);
				dgNodeFn.setName((nodeName+"_RBarb#"));

				if(addAttr) {
					tempBarAttr._barbU =curveData[ii].rightBarbuleRachisPos[kk];
					addBarbCurveAttr( tempObj ,tempBarAttr);
				}
				dagFn2.addChild(tempObj);
			}
	}
}

MStatus jcFeatherToCurves::doIt(const MArgList& args)
{
	if( !featherTools::checkLicense() )	return MS::kFailure;
	if( !this->getFlagData(args) ) return MS::kFailure;

	MStringArray returnResult;
	getJCFeatherArrayNodesData(returnResult);

	//----------set result
	setResult(returnResult);

	return MS::kSuccess;
}
