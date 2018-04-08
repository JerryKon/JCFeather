#ifndef _jcInstancerToMeshCmd_
#define _jcInstancerToMeshCmd_

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
/*
use this cmd to convert paritcle instancer to mesh.
the instancer needs to be mesh instanced.
*/
class jcInstancerToMeshCmd : public MPxCommand
{
   public:
      jcInstancerToMeshCmd();
      ~jcInstancerToMeshCmd();

      static void *creator();
	  static MSyntax newSyntax();
      bool isUndoable() const;
	  bool getFlagData(const MArgList& args);//--get flag data
      MStatus doIt(const MArgList& args);
	  bool convertInstancerToMesh(MDagPath &instancer,MObject &outMeshData);
	  void getInstancerNodes(MStringArray &resultObjs);

	private:
	  MStringArray objects;
};
#endif