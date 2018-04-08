
#include "jcWriteFeather.h"
#include "jcFeather.h"
#include "jcFeatherSystem.h"
#include "jcFeatherGlobals.h"
#include "boostTools.h"

#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MItDependencyNodes.h>
#include <maya/M3dView.h>

extern const char* jcFeatherVersion;
extern int jcFea_SDKVersion;

jcWriteFeather::jcWriteFeather()
{
	ascii=true;
	fileName="";
	localDir="";
	localShortName="";
	ftype=0;
	selected=false;
	frameDouble=0;
	cmdFeatherPt = new featherInfo;
}

jcWriteFeather::~jcWriteFeather()
{
	delete cmdFeatherPt;
}

void *jcWriteFeather::creator()
{
   return new jcWriteFeather;
}

bool jcWriteFeather::isUndoable() const
{
   return false;
}

MSyntax jcWriteFeather::newSyntax()
{
   MSyntax syntax;

   CHECK_MSTATUS( syntax.addFlag("-ft", "-fileType", MSyntax::kLong) );
   CHECK_MSTATUS( syntax.addFlag("-a", "-ascii", MSyntax::kBoolean) );
   CHECK_MSTATUS( syntax.addFlag("-p", "-path", MSyntax::kString) );
   CHECK_MSTATUS( syntax.addFlag("-s", "-selected", MSyntax::kBoolean) );
   CHECK_MSTATUS( syntax.addFlag("-n", "-nodes", MSyntax::kString) );
   return syntax;
}

void jcWriteFeather::timeStart()
{
	startTime=0;
	finishTime=0;
	startTime = clock();
}

void jcWriteFeather::timeEnd()
{
	finishTime = clock();
	float duration = (float)(finishTime - startTime) / CLOCKS_PER_SEC;
	MString timeStr="Export jcFeather in ";
	timeStr += duration;
	timeStr +="s";
	MGlobal::displayInfo(timeStr);
}

bool jcWriteFeather::getFlagData(const MArgList& args)
{
	MArgDatabase argData(syntax(), args);

	//-----------------------------------------get cmd infos
	//
	//
	if(argData.isFlagSet("-fileType") )
		argData.getFlagArgument("-fileType", 0, ftype);
	else
		ftype =featherExportInfo::RIB;
	if( ftype != featherExportInfo::RIB &&
		ftype != featherExportInfo::CACHE&& 
		ftype != featherExportInfo::CACHE_RM&&
		ftype != featherExportInfo::CACHE_MR)
	{
		MGlobal::displayError("-fileType should be 0(rib) , 1(onlyCache), 2(cacheRM) , or 3(chacheMR).");
		return false;
	}

	if( argData.isFlagSet("-ascii") )
		argData.getFlagArgument("-ascii", 0, ascii);
	else
		ascii = true;

	if( argData.isFlagSet("-selected") )
		argData.getFlagArgument("-selected", 0, selected);
	else
		selected = false;

	MString objs;
	if( argData.isFlagSet("-nodes") )
		argData.getFlagArgument("-nodes", 0, objs);
	else
		objs = "";
	objs.split(' ',objects);

	if( argData.isFlagSet( "-path" ) )
		argData.getFlagArgument("-path", 0, fileName) ;
	else
	{
		MGlobal::displayError("-path need a valid file path.");
		return false;
	}

	if( fileName.length()==0 )
	{
		MGlobal::displayError("-path need a valid file path.");
		return false;
	}

	std::string fileP,fileN;
	if(! jerryC::getDir_Name(fileName.asChar(),true,fileP,fileN))
	{
		MGlobal::displayError("jcFeather error: Not invalid file path.");
		return false;
	}
	fileP.append("/");
	localDir.set(fileP.c_str());
	localShortName.set(fileN.c_str());

	return true;
}

void jcWriteFeather::saveXML()
{
	MString xmlFile;
	xmlFile +=cmd_feaG._feaExpInf._localDir;
	xmlFile +=featherTools::getPadding(cmd_feaG._feaExpInf._frame);
	xmlFile +="/";
	xmlFile +=localShortName;
	xmlFile +=".xml";
	cmd_feaG._feaExpInf._featherAttrXmlIO._feaXMLDoc->SaveFile(xmlFile.asChar());
}

MString jcWriteFeather::writeCacheRib()
{
	ofstream os;

	MString newName = cmd_feaG._feaExpInf._localDir + localShortName+".";
	newName += featherTools::getPadding(cmd_feaG._feaExpInf._frame) + ".rib";
	os.open( newName.asChar(),ios::out );
	if( !os )
	{
		MGlobal::displayError("jcFeather error: cannot open file "+newName+".");
		return "";
	}
	MString xmlFile;
	xmlFile +=cmd_feaG._feaExpInf._localDir;
	xmlFile +=featherTools::getPadding(cmd_feaG._feaExpInf._frame);
	xmlFile +="/";
	xmlFile +=localShortName;
	xmlFile +=".xml";

	MPoint pt1=cmd_feaG._feaExpInf._xmlFileBoundingbox.min();
	MPoint pt2=cmd_feaG._feaExpInf._xmlFileBoundingbox.max();
	MString con="Procedural \"DynamicLoad\" [ \"";
	con+=cmd_feaG._feaExpInf.dsoName;	con+="\" \"";	con+=xmlFile;	con+="\" ] [";
	con+=pt1.x;	con+=" ";	con+=pt2.x;	con+=" ";	con+=pt1.y;	con+=" ";	con+=pt2.y;
	con+=" ";	con+=pt1.z;	con+=" ";	con+=pt2.z;	con+="]";
	os<<con.asChar()<<endl;
	os.close();

	return newName;
}

void jcWriteFeather::getJCFeatherArrayNodes()
{
	MFnDependencyNode dgNodeFn;
	MStatus status = MS::kFailure;

	MPlug jcSysInFeaPlug;
	MPlugArray jcSysInFeaPlugConArray;
	
	MSelectionList selList;
	MDagPath dagP;

	if(selected)
		MGlobal::getActiveSelectionList(selList);
	if(objects.length()>0)
	{
		selList.clear();
	    for(int ii=0;ii<objects.length();++ii)
			selList.add(objects[ii],false);
	}

	if(selList.length()>0)
	{
		for(unsigned int ii=0;ii<selList.length();ii++)
		{
			if( selList.getDagPath(ii,dagP) != MS::kSuccess) continue;
			if( dagP.apiType()==MFn::kTransform && 
				dagP.extendToShape()!=MS::kSuccess)
				continue;

			dgNodeFn.setObject(dagP.node());
			if( dgNodeFn.typeId() == jcFeather::id )
			{
				if(!(dgNodeFn.findPlug(jcFeather::active,false).asBool())) continue;
				cmd_feaG._feaExpInf._jcFeatherNodes.append( dagP.node() );
			}
			else if(dgNodeFn.typeId() == jcFeatherSystem::id && dgNodeFn.findPlug(jcFeatherSystem::active,false).asBool())
			{
				jcSysInFeaPlug = dgNodeFn.findPlug(jcFeatherSystem::inRenderFeather,false,&status);
				if( !jcSysInFeaPlug.connectedTo(jcSysInFeaPlugConArray,true,false,&status) )
					continue;

				dgNodeFn.setObject(jcSysInFeaPlugConArray[0].node());
				if(cmd_feaG._feaExpInf._fileType==featherExportInfo::RIB&& 
					dgNodeFn.typeId()==jcFeather::id && 
					dgNodeFn.findPlug(jcFeather::active,false).asBool())
				{
					cmd_feaG._feaExpInf._jcFeatherNodes.append( jcSysInFeaPlugConArray[0].node() );
					cmd_feaG._feaExpInf._jcFeatherNotIn.append( cmd_feaG._feaExpInf._jcFeatherNodes.length()-1 );
					cmd_feaG._feaExpInf._jcFeatherSysNodes.append( dagP.node());
				}
			}
		}
	}
	else
	{
		MItDependencyNodes itNode(MFn::kPluginDependNode,&status);
		CHECK_MSTATUS(status);
		for( itNode.reset(MFn::kPluginDependNode);!itNode.isDone();itNode.next() )
		{
			dgNodeFn.setObject(itNode.thisNode());
			if( dgNodeFn.typeId() != jcFeather::id ) continue;
			if(!(dgNodeFn.findPlug(jcFeather::active,false).asBool())) continue;
			cmd_feaG._feaExpInf._jcFeatherNodes.append( itNode.thisNode() );
		}
	}
}

bool jcWriteFeather::initFeatherGen()
{
	cmd_feaG.init();
	cmd_feaG._featherAttrs = cmdFeatherPt;

	cmd_feaG._feaExpInf.jcFeatherVersion=MString( jcFeatherVersion );
	cmd_feaG._feaExpInf.mayaVersion = MGlobal::mayaVersion();
	cmd_feaG._feaExpInf.needSdkVersion = featherGen::jcFea_SDKVersion;

	cmd_feaG._feaExpInf._fileType = ftype;
	cmd_feaG._feaExpInf._renderCurveFuncion=jcSpline::kCatmullRom;//catmull rom
	if(cmd_feaG._feaExpInf._fileType==featherExportInfo::RIB)
		cmd_feaG._feaExpInf._filePostfix=".rib";
	else
	{
		cmd_feaG._feaExpInf._onlyGetBBox=true;//only need bounding box of each feather
		if(cmd_feaG._feaExpInf._fileType==featherExportInfo::CACHE_MR)
		{
			cmd_feaG._feaExpInf._filePostfix=".mi";//feather cache
			//cmd_feaG._feaExpInf._renderCurveFuncion=jcSpline::kBezier;//bezier
		}
	}
	
	cmd_feaG._feaExpInf._useSelected=selected;
	getJCFeatherArrayNodes();
	if(cmd_feaG._feaExpInf._jcFeatherNodes.length()==0)
		return false;

	//get global setting node info and prepare file type info
	if(cmd_feaG.prepareForRender()!=MS::kSuccess )
	{
		MGlobal::displayError("jcFeather error: cannot find jcFeatherGlobals node.");
		return false;
	}
	MGlobal::executeCommand("currentTime -q",frameDouble,false,false);
	cmd_feaG._feaExpInf._frame = (int)(frameDouble+0.5);
	cmd_feaG._feaExpInf._localDir = localDir;
	cmd_feaG._feaExpInf._startTime=MTime(cmd_feaG._feaExpInf._frame,MTime::uiUnit());

	MString framePath ;
	framePath=cmd_feaG._feaExpInf._localDir + featherTools::getPadding(cmd_feaG._feaExpInf._frame)+"/"; //   c:/ + 0001 + /
	jerryC::createDirs(framePath.asChar());

	return true;
}

bool jcWriteFeather::writeFeatherFile()
{
	if( !featherTools::checkLicense() )
		return false;

	if(cmd_feaG._feaExpInf._fileType!=featherExportInfo::RIB )
	{
		bool xmlok=cmd_feaG._feaExpInf._featherAttrXmlIO.xmlCreate(cmd_feaG._feaExpInf);
		if(!xmlok)return false;
	}

	float blurFrame=0;
	if(cmd_feaG._feaExpInf.enableMotionBlur)
	{
		for(;
			cmd_feaG._feaExpInf._mbCurrentSample<cmd_feaG._feaExpInf._mbSample;
			cmd_feaG._feaExpInf._mbCurrentSample++)
		{
			cmd_feaG.initForEachFrame();
			blurFrame = (float)frameDouble + 
						cmd_feaG._feaExpInf._mbStep*
						(float)cmd_feaG._feaExpInf._mbCurrentSample/(float)(cmd_feaG._feaExpInf._mbSample-1);
			MGlobal::viewFrame(blurFrame);
			if( cmd_feaG.doWriteFeather()!=MS::kSuccess)
			{
				MGlobal::displayError("jcFeather error: cannot write jcFeather file.");
				return false;
			}
		}
	}
	else
	{
		MGlobal::viewFrame((float)cmd_feaG._feaExpInf._frame);
		if( cmd_feaG.doWriteFeather()!=MS::kSuccess)
		{
			MGlobal::displayError("jcFeather error: cannot write jcFeather file.");
			return false;
		}
	}
	
	if(cmd_feaG._feaExpInf._fileType!=featherExportInfo::RIB)
		saveXML();
	return true;
}

bool jcWriteFeather::writeResultFile()
{
	//-----------
	if(cmd_feaG._feaExpInf._fileType==featherExportInfo::RIB)
	{	
		unsigned int flen=cmd_feaG._feaExpInf._allFiles.length();
		if(flen==0) return true;

		ofstream os;
		MString newName = cmd_feaG._feaExpInf._localDir + localShortName+".";
		newName += featherTools::getPadding(cmd_feaG._feaExpInf._frame) + cmd_feaG._feaExpInf._filePostfix;
		os.open( newName.asChar(),ios::out );
		if( !os )
		{
			MGlobal::displayError("jcFeather error: cannot open file "+newName+".");
			return false;
		}

		for(unsigned int jj=0;jj<flen;++jj)
			os<<"ReadArchive \""<<cmd_feaG._feaExpInf._allFiles[jj].asChar()<<"\""<<endl;
		os.close();

		MStringArray files;
		files.append(newName);
		setResult(files);
	}
	else if(cmd_feaG._feaExpInf._fileType!=featherExportInfo::RIB)
	{
		if(cmd_feaG._feaExpInf._fileType==featherExportInfo::CACHE_RM )
			cmd_feaG._feaExpInf._allFiles.append(writeCacheRib());
		setResult(cmd_feaG._feaExpInf._allFiles);
	}

	return true;
}

MStatus jcWriteFeather::doIt(const MArgList& args)
{
	if( !featherTools::checkLicense() )
		return MS::kFailure;

	if( !this->getFlagData(args) ) return MS::kFailure;

	this->timeStart();
		if(!this->initFeatherGen()) return MS::kFailure;

		if(!this->writeFeatherFile()) return MS::kFailure;

		if(!this->writeResultFile()) return MS::kFailure;
	this->timeEnd();

	MGlobal::viewFrame(cmd_feaG._feaExpInf._frame);//go back to start frame

	return MS::kSuccess;
}
