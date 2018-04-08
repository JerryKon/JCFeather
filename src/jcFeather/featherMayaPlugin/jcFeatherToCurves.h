#ifndef _jcFeatherToCurves_
#define _jcFeatherToCurves_

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>
#include <maya/MPointArray.h>
#include <vector>
#include "singleFeather.h"

using namespace std;
struct barbAttr
{
	float _barbU;
};
class jcFeatherToCurves : public MPxCommand
{
   public:
      jcFeatherToCurves();
      ~jcFeatherToCurves();

      static void *creator();
	  static MSyntax newSyntax();
      bool isUndoable() const;
	  bool getFlagData(const MArgList& args);//--get flag data
      MStatus doIt(const MArgList& args);
	  
	  inline void floatPAryToPAry(const MFloatPointArray pta1,MPointArray &pta2)
	  {
		pta2.clear();
		unsigned int ii=0;
		for(;ii<pta1.length();++ii)
			pta2.append(pta1[ii].x,pta1[ii].y,pta1[ii].z);
	  }
	  MStatus pointArrayToCurve(int deg,const MFloatPointArray &pts,MObject &curveObj);//point array to nurbscurve
	  void getJCFeatherArrayNodesData(MStringArray &resultCurves);//get feather data and gen nurbscurve
	  void convertToCurves(unsigned int degree,
						  const vector<singleFeather> &curveData,
						  MString nodeName,
						  MString &resultGroup);//convert singlefeather data to curve
	  void addBarbCurveAttr( MObject &obj ,const barbAttr &value);
	private:
	  MStringArray objects;
	  bool addAttr;
};
#endif