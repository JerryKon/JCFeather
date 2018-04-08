#ifndef _jcWriteFeather_
#define _jcWriteFeather_

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>

#include <fstream>
#include <vector>
#include <time.h>
#include "singleFeather.h"
#include "featherGen.h"

using namespace std;
//-----write jcFeather file out for current frame
class jcWriteFeather : public MPxCommand
{
   public:
      jcWriteFeather();
      ~jcWriteFeather();

      static void *creator();
	  static MSyntax newSyntax();
      bool isUndoable() const;
      MStatus doIt(const MArgList& args);

	  bool getFlagData(const MArgList& args);//--get flag data
	  void timeStart();//--start record
	  void timeEnd();//--end record and print the elapsed time 
	  bool writeResultFile();//--write the result rib
	  bool initFeatherGen();//--prepare cmd_feaG for writing data
	  bool writeFeatherFile();//--write feather data
	  void getJCFeatherArrayNodes();
	  void saveXML();
	  MString writeCacheRib();

	private:
	  bool ascii;
	  int ftype;
	  MString fileName; // "C:/hello/world/feather.rib"
	  MString localDir; // "C:/hello/world/"
	  MString localShortName; // feather"
	  bool selected;
	  double frameDouble;
	  MStringArray objects;

	  clock_t startTime;
	  clock_t finishTime;

	  featherGen  cmd_feaG;
	  featherInfo *cmdFeatherPt;
};
#endif