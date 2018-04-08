#ifndef _jcWriteGrowData_
#define _jcWriteGrowData_

#include <fstream>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <vector>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>

#include "jcPreGuid.h"
#include "jcScatter.h"
#include "jcPreFeather.h"
#include "jcFeatherComponentArray.h"
#include "surfaceTurtleArray.h"

using namespace std;
//write jcPreGuid ,jcScatter result into a  file
class jcWriteGrowData : public MPxCommand

{
   public:
      jcWriteGrowData();
      ~jcWriteGrowData();

      static void *creator();
      bool isUndoable() const;
      MStatus doIt(const MArgList& args);
	  void getVertexDirectionData(MDagPath &pGuid,verDirVector &vdData);
	  void getScatterData(MDagPath &scatter,jcScatterPointData &stData);
	  void getComponentArrayData(MDagPath &comArray,MObject &nodePlug,std::vector<turtles> &outData);
};
#endif