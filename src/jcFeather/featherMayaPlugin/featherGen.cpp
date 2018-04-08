#include "featherGen.h"
#include "jcFeatherGlobals.h"
#include "jcFeather.h"
#include "jcFeatherSystem.h"

extern HashTable*    feaInfoDataBase;
extern HashTable*    turtlesDataBase;
int featherGen::jcFea_SDKVersion=1000;


//-----------init for writing files
featherGen::featherGen():featherGenBase()
{}

featherGen::~featherGen()
{
	//do not do anything
}

MStatus			featherGen::prepareForRender()
{
	MStatus status;
	MFnDependencyNode dgNodeFn;
	MSelectionList sellist;
	MObject globalObj;

	sellist.clear();
	if( sellist.add("jcFeatherGlobals")==MS::kSuccess )
	{
		CHECK_MSTATUS( sellist.getDependNode(0,globalObj) );
		CHECK_MSTATUS( dgNodeFn.setObject(globalObj) );

		if(dgNodeFn.typeId() == jcFeatherGlobals::id)
		{
			MPlug plug= dgNodeFn.findPlug(jcFeatherGlobals::featherRibText,false);
			plug.getValue( _feaExpInf.featherRib );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::rachisMeshRib,false);
			plug.getValue( _feaExpInf.rachisMeshInsert );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::dsoName,false);
			plug.getValue( _feaExpInf.dsoName );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::exportNormal,false);
			plug.getValue( _feaExpInf.exportNormal );

			plug= dgNodeFn.findPlug(jcFeatherGlobals::enableMotionBlur,false);
			plug.getValue( _feaExpInf.enableMotionBlur );

			plug= dgNodeFn.findPlug(jcFeatherGlobals::motionBlurSamples,false);
			plug.getValue( _feaExpInf._mbSample );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::motionBlurStep,false);
			plug.getValue( _feaExpInf._mbStep );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::motionBlurFactor,false);
			plug.getValue( _feaExpInf._mbFactor );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::relativePath,false);
			plug.getValue( _feaExpInf._relativePath );
			plug= dgNodeFn.findPlug(jcFeatherGlobals::delayReadArchive,false);
			plug.getValue( _feaExpInf._delayArchive );
			_feaExpInf._mbCurrentSample = 0;

			if(_feaExpInf._mbSample<=1)_feaExpInf._mbSample=2;
			if(!_feaExpInf.enableMotionBlur) _feaExpInf._mbSample=1;

			plug= dgNodeFn.findPlug(jcFeatherGlobals::gzipCompression,false);
			bool compress=false;
			plug.getValue( compress );
			setCompressInfo(compress,-1);

			plug= dgNodeFn.findPlug(jcFeatherGlobals::diceHair,false);
			plug.getValue( _feaExpInf.diceHair );
			if(plug.asBool()&&_feaExpInf._fileType==featherExportInfo::RIB)//--- means export rib file
				_feaExpInf.constantParam+="Attribute \"dice\" \"int hair\" [1]\n";

			if(_feaExpInf._fileType==featherExportInfo::RIB)//--- means export rib file
				_feaExpInf.constantParam ="Basis \"catmull-rom\" 1 \"catmull-rom\" 1\n";

			if(_feaExpInf._fileType==featherExportInfo::RIB) setRenderManDeclare();//--- means export rib file
		}
		else
		{
			MGlobal::displayError("jcFeather : Cannot find jcFeatherGlobals node.");
			return MS::kFailure;
		}
	}
	else
	{
		MGlobal::displayError("jcFeather : Cannot find jcFeatherGlobals node.");
		return MS::kFailure;
	}

	return MS::kSuccess;
}

void			featherGen::setRenderManDeclare()
{		
		std::string tempConstantP="";
		tempConstantP.append("Declare \"jcFeaRachisRootC\" \"constant color\"\n");
		tempConstantP.append("Declare \"jcFeaRachisTipC\" \"constant color\"\n");

		tempConstantP.append("Declare \"jcFeaBarU\" \"uniform float\"\n");
		tempConstantP.append("Declare \"jcFeaBarLenPer\" \"uniform float\"\n");
		tempConstantP.append("Declare \"jcFeaSurU\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaSurV\" \"constant float\"\n");

		tempConstantP.append("Declare \"jcFeaRootC\" \"constant color\"\n");
		tempConstantP.append("Declare \"jcFeaTipC\" \"constant color\"\n");
		tempConstantP.append("Declare \"jcFeaMainC\" \"constant color\"\n");
		tempConstantP.append("Declare \"jcFeaBaseOpacity\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaFadeOpacity\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaFadeStart\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaUVProjectScale\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaColorTex\" \"constant string\"\n");
		tempConstantP.append("Declare \"jcFeadiff\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeagloss\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaselfshad\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaspec\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaspecColor\" \"constant color\"\n");
		tempConstantP.append("Declare \"jcFeaHueVar\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaSatVar\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaValVar\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaVarFreq\" \"constant float\"\n");
		tempConstantP.append("Declare \"jcFeaRandSeed\" \"constant float\"\n");

		tempConstantP.append("ShadingInterpolation \"smooth\"");
		_feaExpInf.constantParam += MString(tempConstantP.c_str());
}

MString			featherGen::addRManRibAttr(const char *nodeName)
{
	std::string renderAttr="";
	renderAttr.append("Attribute \"identifier\" \"string name\" [ \"");
	renderAttr.append(nodeName);
	renderAttr.append("\" ]\n" );
	if(_feaExpInf._userRender == featherExportInfo::DELIGHT)
	{
		if(!_feaExpInf._renderAttr._visibleInReflections)
			renderAttr.append("Attribute \"grouping\" \"string membership\" [ \"+_3dfm_not_visible_in_reflections\" ]\n");

		if(!_feaExpInf._renderAttr._visibleInRefractions)
			renderAttr.append("Attribute \"grouping\" \"string membership\" [ \"+_3dfm_not_visible_in_refractions\" ]\n");

		if(!_feaExpInf._renderAttr._primaryVisibility)
		{
			renderAttr.append("IfBegin \"!defined(ShadowMapRendering)\"\n" );
			renderAttr.append("	Attribute \"visibility\" \"camera\" [ 0 ] \n" );
			renderAttr.append("IfEnd\n" );
		}
		if(!_feaExpInf._renderAttr._castsShadow)
		{
			renderAttr.append("IfBegin \"defined(ShadowMapRendering)\"\n" );
			renderAttr.append("	Attribute \"visibility\" \"camera\" [ 0 ] \n" );
			renderAttr.append("IfEnd\n" );
		}
		if(!_feaExpInf._renderAttr._receiveShadow)
			renderAttr.append("Attribute \"user\" \"float delight_receive_shadows\" [ 0 ]\n");
	}
	return MString(renderAttr.c_str());
}

bool			featherGen::jcFeaIsValide(MObject &featherObj,MObject &displayMeshNode)
{
	MStatus status = MS::kFailure;

	MFnDependencyNode dgNodeFn(featherObj,&status);

	MPlug jcOutDisplayMesh = dgNodeFn.findPlug(jcFeather::outputMesh,false,&status);
	MPlugArray outDisplayMesh;
	displayMeshNode=MObject::kNullObj;
	if( !jcOutDisplayMesh.connectedTo(outDisplayMesh,false,true,&status)) return false;
	for(unsigned int ii=0;ii<outDisplayMesh.length();++ii)
		if(outDisplayMesh[ii].node().hasFn(MFn::kMesh)) 
			displayMeshNode = outDisplayMesh[ii].node();
	if(displayMeshNode.isNull())return false;

	if(!dgNodeFn.findPlug(jcFeather::active).asBool()) return false;
	if(!dgNodeFn.findPlug(jcFeather::render).asBool())
	{
		MPlug outConnectPlug = dgNodeFn.findPlug(jcFeather::outRenderFeather,false,&status);
		MPlugArray outFeaArray;
		if( !outConnectPlug.connectedTo(outFeaArray,false,true,&status) )
			return false;
		else
		{
			int sysCount=0;
			MFnDependencyNode sysFn;
			for(unsigned int ii=0;ii<outFeaArray.length();++ii)
			{
				sysFn.setObject(outFeaArray[ii]);
				if( sysFn.typeId() == jcFeatherSystem::id && sysFn.findPlug(jcFeatherSystem::active).asBool())
					return true;
			}
			return false;
		}
	}
	else
		return true;
}

bool			featherGen::jcFeaSysIsValide(MObject &feaSysObj,MObject &displayMeshNode)
{
	MStatus status = MS::kFailure;
	MFnDependencyNode dgNodeFn(feaSysObj);
	if( dgNodeFn.typeId() != jcFeatherSystem::id ||
		!dgNodeFn.findPlug(jcFeatherSystem::active).asBool())
		return false;

	//----find the node's outdisplay mesh node.
	MPlug jcOutDisplayMesh = dgNodeFn.findPlug(jcFeatherSystem::outFeatherMesh,false,&status);
	CHECK_MSTATUS( status );
	MPlugArray outDisplayMesh;
	displayMeshNode=MObject::kNullObj;
	if( !jcOutDisplayMesh.connectedTo(outDisplayMesh,false,true,&status)) return false;
	for(unsigned int ii=0;ii<outDisplayMesh.length();++ii)
		if(outDisplayMesh[ii].node().hasFn(MFn::kMesh)) 
			displayMeshNode = outDisplayMesh[ii].node();
	if(displayMeshNode.isNull())return false;
	return true;
}

bool			featherGen::getNodeShadingEngine(MObject &node, MObject &shadingengine,MObject &shader)
{
	MStatus status = MS::kFailure;

	MFnDependencyNode dgNodeFn(node,&status);
	MPlug instPlg=dgNodeFn.findPlug("instObjGroups",&status);
	MPlug instPlg0=instPlg.elementByLogicalIndex(0,&status);
	MPlugArray plgary;
	if( !instPlg0.connectedTo(plgary,false,true,&status) ) return false;
	shadingengine = plgary[0].node();

	dgNodeFn.setObject(shadingengine);
	MPlug surplg=dgNodeFn.findPlug("surfaceShader",&status);
	if(!surplg.connectedTo(plgary,true,false,&status)) return false;
	shader = plgary[0].node();

	dgNodeFn.setObject(shader);
	//_featherAttrs->_shaderName = featherTools::checkNodeName(dgNodeFn.name());
	_featherAttrs->_shaderName = dgNodeFn.name();
	return true;
}

MStatus			featherGen::initWithJCFeather( MObject &featherObj )
{
	MStatus status = MS::kFailure;
	MObject outDisplayNode;
	if(!jcFeaIsValide(featherObj,outDisplayNode))return MS::kFailure;
	if(!_featherAttrs) return MS::kFailure;

	MFnDependencyNode dgNodeFn(featherObj,&status);
	Node* tempFeaPt = feaInfoDataBase->Find( static_cast<jcFeather*>(dgNodeFn.userNode())->featherID );
	if( !tempFeaPt ) return MS::kFailure;

	if( !dgNodeFn.findPlug(jcFeather::interactiveTexture).asBool() )
	{
		static_cast<jcFeather*>(dgNodeFn.userNode())->updateJCFTexture();
		MObject updateObj;
		dgNodeFn.findPlug(jcFeather::outputMesh).getValue(updateObj);
	}

	_featherAttrs->copyData( reinterpret_cast<featherInfo*>( tempFeaPt->Value));

	_featherAttrs->_barbuleNum = dgNodeFn.findPlug(jcFeather::barbuleNum).asInt();
	_featherAttrs->_renderable = dgNodeFn.findPlug(jcFeather::render).asBool();

	_featherAttrs->_rachisRenderAs = dgNodeFn.findPlug(jcFeather::rachisRenderAs).asShort();
	_featherAttrs->_outputFeatherMesh = false;//���ribʱ����Ҫ��������
	_featherAttrs->_preFeatherRib = dgNodeFn.findPlug(jcFeather::preFeatherRib).asString();
	_featherAttrs->_postFeatherRib = dgNodeFn.findPlug(jcFeather::postFeatherRib).asString();
	_featherAttrs->_preRachisMeshRib = dgNodeFn.findPlug(jcFeather::preFeatherMeshRib).asString();
	_featherAttrs->_postRachisMeshRib = dgNodeFn.findPlug(jcFeather::postFeatherMeshRib).asString();
	_featherAttrs->_shader._barbuleDiffuse = dgNodeFn.findPlug(jcFeather::barbuleDiffuse).asFloat();
	_featherAttrs->_shader._barbuleGloss = dgNodeFn.findPlug(jcFeather::barbuleGloss).asFloat();
	_featherAttrs->_shader._barbuleSpecular = dgNodeFn.findPlug(jcFeather::barbuleSpecular).asFloat();
	_featherAttrs->_shader._barbuleSpecularColor.r = dgNodeFn.findPlug(jcFeather::barbuleSpecularColor).child(0).asFloat();
	_featherAttrs->_shader._barbuleSpecularColor.g = dgNodeFn.findPlug(jcFeather::barbuleSpecularColor).child(1).asFloat();
	_featherAttrs->_shader._barbuleSpecularColor.b = dgNodeFn.findPlug(jcFeather::barbuleSpecularColor).child(2).asFloat();
	_featherAttrs->_shader._uvProject = dgNodeFn.findPlug(jcFeather::projectTexUV).asBool();
	_featherAttrs->_shader._uvProjectScale = dgNodeFn.findPlug(jcFeather::uvProjectScale).asFloat();

	_featherAttrs->_shader._barbuleTexture = dgNodeFn.findPlug(jcFeather::barbuleColorTex).asString();
	_featherAttrs->_shader._texProcedure = dgNodeFn.findPlug(jcFeather::textureProcedure).asString();

	_featherAttrs->_shader._selfShadow = dgNodeFn.findPlug(jcFeather::selfShadow).asFloat();
	_featherAttrs->_shader._baseOpacity = dgNodeFn.findPlug(jcFeather::baseOpacity).asFloat();
	_featherAttrs->_shader._fadeOpacity = dgNodeFn.findPlug(jcFeather::fadeOpacity).asFloat();
	_featherAttrs->_shader._fadeStart = dgNodeFn.findPlug(jcFeather::fadeStart).asFloat();

	_featherAttrs->_shader._hueVar = dgNodeFn.findPlug(jcFeather::barbuleHueVar).asInt();
	_featherAttrs->_shader._satVar = dgNodeFn.findPlug(jcFeather::barbuleSatVar).asFloat();
	_featherAttrs->_shader._valVar = dgNodeFn.findPlug(jcFeather::barbuleValueVar).asFloat();
	_featherAttrs->_shader._varFreq = dgNodeFn.findPlug(jcFeather::varFrequency).asFloat();

	_featherAttrs->_barbuleThick = dgNodeFn.findPlug(jcFeather::barbuleThick).asFloat();
	_featherAttrs->_barbuleRandThick = dgNodeFn.findPlug(jcFeather::barbuleRandThick).asFloat();
	_featherAttrs->_barbuleThickAdjust = MRampAttribute(dgNodeFn.findPlug(jcFeather::barbuleThickAdjust));

	_featherAttrs->_rachisThick = dgNodeFn.findPlug(jcFeather::rachisThick).asFloat();
	_featherAttrs->_rachisThickScale   = MRampAttribute(dgNodeFn.findPlug(jcFeather::rachisThickScale));
	_featherAttrs->_barbuleThickScale  = MRampAttribute(dgNodeFn.findPlug(jcFeather::barbuleThickScale));



	if(_feaExpInf._fileType==featherExportInfo::SDK) return MS::kSuccess;

	_featherAttrs->_useOutShader = dgNodeFn.findPlug(jcFeather::useOutShader).asBool();
	if( _featherAttrs->_useOutShader )
		if(!getNodeShadingEngine(featherObj,_featherAttrs->_shadingEngineNode,_featherAttrs->_shaderNode))
		{
			MGlobal::displayWarning("JCFeather : No shader linked to node "+dgNodeFn.name()+", use default shader");
			_featherAttrs->_useOutShader=false;
		}
	_feaExpInf._renderAttr._castsShadow = dgNodeFn.findPlug("castsShadows").asBool();
	_feaExpInf._renderAttr._receiveShadow = dgNodeFn.findPlug("receiveShadows").asBool();
	_feaExpInf._renderAttr._primaryVisibility = dgNodeFn.findPlug("primaryVisibility").asBool();
	_feaExpInf._renderAttr._motionBlur = dgNodeFn.findPlug("motionBlur").asBool();
	_feaExpInf._renderAttr._visibleInReflections = dgNodeFn.findPlug("visibleInReflections").asBool();
	_feaExpInf._renderAttr._visibleInRefractions = dgNodeFn.findPlug("visibleInRefractions").asBool();


	//---------the following attribute will affect the feather curves num,so it needs recompute for motion blur
	if( _feaExpInf.enableMotionBlur && _feaExpInf._mbCurrentSample!=0 )
	{
		MDGContext startCtx(_feaExpInf._startTime);

		_featherAttrs->_rachisSegment = dgNodeFn.findPlug(jcFeather::rachisSegment).asInt(startCtx) + 1;
		_featherAttrs->_rachisSides = dgNodeFn.findPlug(jcFeather::rachisCrossSection).asInt(startCtx);
		_featherAttrs->_barbuleNum = dgNodeFn.findPlug(jcFeather::barbuleNum).asInt(startCtx);
		_featherAttrs->_barbuleSegments = dgNodeFn.findPlug(jcFeather::barbuleSegments).asInt(startCtx) + 1;
		
		MPlug temp_asgramp_plug=dgNodeFn.findPlug(jcFeather::barbuleDensity);
		assignMRampToJCRamp(temp_asgramp_plug,_featherAttrs->_barbuleDensity,startCtx);
		temp_asgramp_plug=dgNodeFn.findPlug(jcFeather::leftBarbuleLengthScale);
		assignMRampToJCRamp(temp_asgramp_plug,_featherAttrs->_leftBarbuleLengthScale,startCtx);
		if( dgNodeFn.findPlug(jcFeather::shapeSymmetry).asBool(startCtx) )
			_featherAttrs->_rightBarbuleLengthScale=_featherAttrs->_leftBarbuleLengthScale;
		else
		{
			temp_asgramp_plug=dgNodeFn.findPlug(jcFeather::rightBarbuleLengthScale);
			assignMRampToJCRamp(temp_asgramp_plug,_featherAttrs->_rightBarbuleLengthScale,startCtx);

		}
	}

	return MS::kSuccess;
}

MStatus			featherGen::initWithJCSystem(MObject &sysObj)
{
	MFnDependencyNode dgNodeFn(sysObj);
	MStatus status = MS::kFailure;
	MObject outDisplayNode;
	if( !jcFeaSysIsValide(sysObj,outDisplayNode) ) 
	  return MS::kFailure;

	float2 uvtran;
	if(_feaExpInf.enableMotionBlur && _feaExpInf._mbCurrentSample!=0)
	{
		MDGContext startCtx(_feaExpInf._startTime);
		jerryC::jCell::m_xSegment = dgNodeFn.findPlug(jcFeatherSystem::uSegment).asInt(startCtx);
		jerryC::jCell::m_zSegment = dgNodeFn.findPlug(jcFeatherSystem::vSegment).asInt(startCtx);
		jerryC::jCell::m_seed     = dgNodeFn.findPlug(jcFeatherSystem::randomSeed).asInt(startCtx);
		jerryC::jCell::m_jitter   = dgNodeFn.findPlug(jcFeatherSystem::jitterDistance).asFloat(startCtx);
		jerryC::jCell::m_frequency = dgNodeFn.findPlug(jcFeatherSystem::jitterFrequency).asFloat(startCtx);
		jerryC::jCell::m_xOffset= dgNodeFn.findPlug(jcFeatherSystem::uvOffset).child(0).asFloat(startCtx);
		jerryC::jCell::m_zOffset= dgNodeFn.findPlug(jcFeatherSystem::uvOffset).child(1).asFloat(startCtx);

		uvtran[0] = dgNodeFn.findPlug(jcFeatherSystem::uvTranslate).child(0).asFloat(startCtx);
		uvtran[1] = dgNodeFn.findPlug(jcFeatherSystem::uvTranslate).child(1).asFloat(startCtx);

		_jcSysD._renderQuality     = dgNodeFn.findPlug(jcFeatherSystem::renderQuality).asInt(startCtx);
		_jcSysD._seed              = dgNodeFn.findPlug(jcFeatherSystem::randomSeed).asInt(startCtx);
	}
	else
	{
		jerryC::jCell::m_xSegment = dgNodeFn.findPlug(jcFeatherSystem::uSegment).asInt();
		jerryC::jCell::m_zSegment = dgNodeFn.findPlug(jcFeatherSystem::vSegment).asInt();
		jerryC::jCell::m_seed     = dgNodeFn.findPlug(jcFeatherSystem::randomSeed).asInt();
		jerryC::jCell::m_jitter   = dgNodeFn.findPlug(jcFeatherSystem::jitterDistance).asFloat();
		jerryC::jCell::m_frequency = dgNodeFn.findPlug(jcFeatherSystem::jitterFrequency).asFloat();
		jerryC::jCell::m_xOffset = dgNodeFn.findPlug(jcFeatherSystem::uvOffset).child(0).asFloat();
		jerryC::jCell::m_zOffset = dgNodeFn.findPlug(jcFeatherSystem::uvOffset).child(1).asFloat();

		uvtran[0] = dgNodeFn.findPlug(jcFeatherSystem::uvTranslate).child(0).asFloat();
		uvtran[1] = dgNodeFn.findPlug(jcFeatherSystem::uvTranslate).child(1).asFloat();

		_jcSysD._renderQuality     = dgNodeFn.findPlug(jcFeatherSystem::renderQuality).asInt();
		_jcSysD._seed              = dgNodeFn.findPlug(jcFeatherSystem::randomSeed).asInt();
	}

	MPlugArray texConnect;
	MStringArray texs(4,MString(""));

	if(dgNodeFn.findPlug(jcFeatherSystem::scaleTex).connectedTo(texConnect,true,false))
		texs[0]=texConnect[0].name();
	if(dgNodeFn.findPlug(jcFeatherSystem::baldnessTex).connectedTo(texConnect,true,false))
		texs[1]=texConnect[0].name();
	if(dgNodeFn.findPlug(jcFeatherSystem::colorTex).connectedTo(texConnect,true,false))
		texs[2]=texConnect[0].name();
	if(dgNodeFn.findPlug(jcFeatherSystem::radiusTex).connectedTo(texConnect,true,false))
		texs[3]=texConnect[0].name();

	_jcSysD._featherID         = dgNodeFn.findPlug(jcFeatherSystem::inRenderFeather).asInt();
	_jcSysD._outputFeatherMesh = false;
	_jcSysD._guidPF            = dgNodeFn.findPlug(jcFeatherSystem::guidsPerFeather).asInt();
	_jcSysD._globalScale       = dgNodeFn.findPlug(jcFeatherSystem::gloalScale).asFloat();
	_jcSysD._randScale         = dgNodeFn.findPlug(jcFeatherSystem::randScale).asFloat();
	_jcSysD._radius            = dgNodeFn.findPlug(jcFeatherSystem::radius).asFloat()+0.00000001f;
	_jcSysD._power             = dgNodeFn.findPlug(jcFeatherSystem::power).asFloat();
	_jcSysD._translate[0]      = uvtran[0];
	_jcSysD._translate[1]      = uvtran[1];
	_jcSysD._textures          = texs;
	_jcSysD._useUVI            = dgNodeFn.findPlug(jcFeatherSystem::useUVIFile).asBool();
	_jcSysD._uviFile           = dgNodeFn.findPlug(jcFeatherSystem::uviFile).asString();
	_jcSysD._uvSet             = dgNodeFn.findPlug(jcFeatherSystem::uvSetName).asString();
	if(_jcSysD._uvSet.length()==0) _jcSysD._uvSet = "map1";
	
	//----find the render attributes
	_feaExpInf._renderAttr._castsShadow = dgNodeFn.findPlug("castsShadows").asBool();
	_feaExpInf._renderAttr._receiveShadow = dgNodeFn.findPlug("receiveShadows").asBool();
	_feaExpInf._renderAttr._primaryVisibility = dgNodeFn.findPlug("primaryVisibility").asBool();
	_feaExpInf._renderAttr._motionBlur = dgNodeFn.findPlug("motionBlur").asBool();
	_feaExpInf._renderAttr._visibleInReflections = dgNodeFn.findPlug("visibleInReflections").asBool();
	_feaExpInf._renderAttr._visibleInRefractions = dgNodeFn.findPlug("visibleInRefractions").asBool();
	return MS::kSuccess;
}

//------------compute feather data and write them out
bool			featherGen::computeJCFeatherNode(MObject &featherObj,vector<singleFeather> &renderFeather)
{
	MStatus status = MS::kFailure;
	
	MFnDependencyNode dgNodeFn(featherObj,&status);
	CHECK_MSTATUS(status);

	MObject newObj;
	MPlug newPlug,currentGuidPlug;
	Node* tempNodePt=NULL;
	turtles *tempTurtle=NULL;
	MString texCmd="";

	bool exactFeather = dgNodeFn.findPlug(jcFeather::exactFeather).asBool();
	//find jcFeather
	MPlug surfacePlug = dgNodeFn.findPlug(jcFeather::guidSurfaceInfo,true,&status);
	CHECK_MSTATUS(status);
	std::map<int,long> indexMap = static_cast<jcFeather*>(dgNodeFn.userNode())->surfaceIndexIDMap;
	std::map<int,long> meshIndexMap = static_cast<jcFeather*>(dgNodeFn.userNode())->inTurtleIDMap;
	std::map<std::pair<int,int>,long> inTurtleArrayMap = static_cast<jcFeather*>(dgNodeFn.userNode())->inTurtleArrayIDMap;
	
	//std::vector< singleFeather > featherInNode = static_cast<jcFeather*>(dgNodeFn.userNode())->featherDraw;
	//size_t finLen=featherInNode.size();

	MColorArray colAryPt=static_cast<jcFeather*>(dgNodeFn.userNode())->mainColorArray;
	MFloatArray ufAryPt=static_cast<jcFeather*>(dgNodeFn.userNode())->jcU;
	MFloatArray vfAryPt=static_cast<jcFeather*>(dgNodeFn.userNode())->jcV;
	unsigned int finLen=colAryPt.length();

	std::map<int,long>::iterator iter;
	std::map<std::pair<int,int>,long>::iterator pairiter;
	renderFeather.resize(indexMap.size() + meshIndexMap.size() + inTurtleArrayMap.size() );

	//-------------------surface feather
	unsigned int ii=0;
	for( iter = indexMap.begin();iter!=indexMap.end();++iter,++ii)
	{
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;

		currentGuidPlug = surfacePlug.elementByLogicalIndex(iter->first);
		//���ڽڵ���������ɫ��Ϣ��uv��Ϣ��������ë��
		if(ii<finLen)
		{
			renderFeather[ii].mainColor = colAryPt[ii];
			renderFeather[ii].surfaceUV[0] = ufAryPt[ii];
			renderFeather[ii].surfaceUV[1] = vfAryPt[ii];
			if(_featherAttrs->_shader._texProcedure.length()!=0)
				{
					texCmd = _featherAttrs->_shader._texProcedure+"("+renderFeather[ii].surfaceUV[0]+","+renderFeather[ii].surfaceUV[1]+","+ii+")";
					if( MGlobal::executeCommand(texCmd,renderFeather[ii].colorTexOverride)!=MS::kSuccess )
						renderFeather[ii].colorTexOverride="";
				}
		}

		if( exactFeather )
		{
			newPlug = currentGuidPlug.child(jcFeather::guidSurface);
			newPlug.getValue(newObj);
			if( !newObj.hasFn(MFn::kNurbsSurface) )
				continue;
			getJCFeather( newObj,renderFeather[ii] ,ii);
		}
		else
		{
			tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);
			getTurtleJCFeather( *tempTurtle,renderFeather[ii] ,ii);
		}
	}

	//---------------mesh feather
	for( iter = meshIndexMap.begin();iter!=meshIndexMap.end();++iter,++ii)
	{
		tempNodePt = turtlesDataBase->Find( &iter->second );
		if(!tempNodePt) continue;

		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);
		//���ڽڵ���������ɫ��Ϣ��uv��Ϣ��������ë��
		if(ii<finLen)
		{
			renderFeather[ii].mainColor = colAryPt[ii];
			renderFeather[ii].surfaceUV[0] = ufAryPt[ii];
			renderFeather[ii].surfaceUV[1] = vfAryPt[ii];

			//��ȡuv
			if(_featherAttrs->_shader._texProcedure!="")
			{
					texCmd = _featherAttrs->_shader._texProcedure+"("+renderFeather[ii].surfaceUV[0]+","+renderFeather[ii].surfaceUV[1]+","+ii+")";
					if( MGlobal::executeCommand(texCmd,renderFeather[ii].colorTexOverride)!=MS::kSuccess )
						renderFeather[ii].colorTexOverride="";
			}
		}
		getJCFeather( *tempTurtle,renderFeather[ii] ,ii);
	}

	//-------------------featherInturtleArray
	for( pairiter = inTurtleArrayMap.begin();pairiter!=inTurtleArrayMap.end();++pairiter,++ii)
	{
		tempNodePt = turtlesDataBase->Find( &pairiter->second );
		if(!tempNodePt) continue;
		tempTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);

		//���ڽڵ���������ɫ��Ϣ��uv��Ϣ��������ë��
		if(ii<finLen)
		{
			renderFeather[ii].mainColor = colAryPt[ii];
			renderFeather[ii].surfaceUV[0] = ufAryPt[ii];
			renderFeather[ii].surfaceUV[1] = vfAryPt[ii];

			//��ȡuv
			if(_featherAttrs->_shader._texProcedure!="")
			{
					texCmd = _featherAttrs->_shader._texProcedure+"("+renderFeather[ii].surfaceUV[0]+","+renderFeather[ii].surfaceUV[1]+","+ii+")";
					if( MGlobal::executeCommand(texCmd,renderFeather[ii].colorTexOverride)!=MS::kSuccess )
						renderFeather[ii].colorTexOverride="";
			}
		}
		getJCFeather( *tempTurtle,renderFeather[ii] ,ii );
	}
	return true;
}

bool			featherGen::computeJCFeatherSystemNode(MObject &featherSystemObj,vector<singleFeather> &renderFeather)
{
	MStatus status = MS::kFailure;

	MFnDependencyNode dgNodeFn(featherSystemObj);

	//------------
	MDagPath growObjPath;
	MObject  growObjComp;
	MFloatPointArray positions;
	MFloatArray u,v;
	MPlugArray setConnect;
	MSelectionList selList;
	MString texCmd="";

	if(dgNodeFn.findPlug(jcFeatherSystem::growthSet).connectedTo(setConnect,true,false,&status))
	{
		MFnSet set(setConnect[0].node());
		if(set.getMembers(selList,false)==MS::kSuccess)
			selList.getDagPath(0,growObjPath,growObjComp);
		else
			return false;
	}
	else
		return false;

	if(growObjPath.extendToShape()!=MS::kSuccess) return false;

	//-------------------
	if( dgNodeFn.findPlug(jcFeatherSystem::inputGrowSurface).isConnected() )
	{
		if( getPositions( growObjPath,positions,u,v)!=MS::kSuccess )
			return false;
		getDisplayPositions(positions,u,v,(float)_jcSysD._renderQuality/100.0f);
		getFeatherFromSys( positions,u,v,renderFeather);

	}
	if( dgNodeFn.findPlug(jcFeatherSystem::inputGrowMesh).isConnected() )
	{
		MItMeshPolygon itMeshPoly(growObjPath,growObjComp);
		if( getPositions(&itMeshPoly,positions,u,v)!=MS::kSuccess)
			return false;
		getDisplayPositions(positions,u,v,(float)_jcSysD._renderQuality/100.0f);
		getFeatherFromSys(positions,u,v,renderFeather);
	}
	if(_featherAttrs->_shader._texProcedure!="")
	{
		for(unsigned int jj=0;jj<renderFeather.size();++jj)
		{
			texCmd = _featherAttrs->_shader._texProcedure+"("+renderFeather[jj].surfaceUV[0]+","+renderFeather[jj].surfaceUV[1]+","+jj+")";
			if( MGlobal::executeCommand(texCmd,renderFeather[jj].colorTexOverride)!=MS::kSuccess )
				renderFeather[jj].colorTexOverride="";
		}
	}
	return true;
}

void			featherGen::doWriteRibFiles(const MString &nodeName,const vector<singleFeather> &renderFeather)
{
	if( writeAsciiRib( nodeName,renderFeather) && (_feaExpInf._mbCurrentSample ==(_feaExpInf._mbSample-1)))
	{
		if(!_feaExpInf._relativePath)
			_feaExpInf._allFiles.append( _feaExpInf._localDir + featherTools::getPadding(_feaExpInf._frame) +"/"+nodeName + _feaExpInf._filePostfix);
		else
			_feaExpInf._allFiles.append( "./"+ featherTools::getPadding(_feaExpInf._frame) +"/"+nodeName + _feaExpInf._filePostfix);
	}
}

bool			featherGen::useThisSysNode(const MObject &obj)
{
	if(_feaExpInf._useSelected)
		return featherTools::arrayContain(_feaExpInf._jcFeatherSysNodes,obj);
	else
		return true;
}

MStatus			featherGen::doWriteFeather()
{
	MStatus status = MS::kFailure;

	MFnDependencyNode dgNodeFn;	
	vector<singleFeather> renderFeather;
	MPlugArray inFeatherArray;
	MPlug jcOutFea;
	MObject sysNode;
	MString jcFeatherNodeName;
	MString jcFeatherSysNodeName;
	MObjectArray jcFeatherSysUsed;
	
	bool fcok=false;
	for(unsigned int jj=0;jj<_feaExpInf._jcFeatherNodes.length();++jj)
	{
		if(!featherTools::isObjectVisible(_feaExpInf._jcFeatherNodes[jj]))continue;
		CHECK_MSTATUS( dgNodeFn.setObject( _feaExpInf._jcFeatherNodes[jj] ) );
		if( initWithJCFeather( _feaExpInf._jcFeatherNodes[jj] ) != MS::kSuccess) continue;
		jcFeatherNodeName = featherTools::checkNodeName( dgNodeFn.name() );
		renderFeather.clear();

		if( !featherTools::arrayContain(_feaExpInf._jcFeatherNotIn,jj) ) //if this jcFeather node is directly selected
		{
			MObject dg_object=dgNodeFn.object();
			if(dgNodeFn.findPlug(jcFeather::render).asBool() &&
			   computeJCFeatherNode(dg_object,renderFeather))
			{
				//---------------write jcFeather
				if(_feaExpInf._fileType==featherExportInfo::RIB)
					doWriteRibFiles(jcFeatherNodeName,renderFeather);
				else
				{
					xmlAddData(jcFeatherNodeName);
					surfaceTurtleIO::writeInfoTo(_feaExpInf._featherCacheIO,getCurrentFCPath(jcFeatherNodeName).asChar());
					_feaExpInf._featherCacheIO.init();
				}
			}
		}

		//------jcFeatherSystem is only used in rib mode
		if(_feaExpInf._fileType!=featherExportInfo::RIB) continue;

		//---------------find connected jcFeatherSystem
		jcOutFea = dgNodeFn.findPlug(jcFeather::outRenderFeather,false,&status);
		inFeatherArray.clear();
		if( !jcOutFea.connectedTo(inFeatherArray,false,true,&status) )
			continue;

		unsigned int inFeaLen=inFeatherArray.length();

		for(unsigned int ii=0;ii<inFeaLen;++ii)
		{
			if( !useThisSysNode( inFeatherArray[ii].node() ) ||
				featherTools::arrayContain( jcFeatherSysUsed,inFeatherArray[ii].node() )
				) 
				continue;//if this jcFeatherSystem node is not in selection list or it's been exported already
			MObject node_ii=inFeatherArray[ii].node();
			if(!featherTools::isObjectVisible(node_ii))continue;
			if( initWithJCSystem( node_ii )!= MS::kSuccess) continue;
			
			dgNodeFn.setObject( inFeatherArray[ii].node());
			jcFeatherSysNodeName = featherTools::checkNodeName( dgNodeFn.name() );//replace the : with _ if the name includes namespace
			
			//---------------write feather
			renderFeather.clear();
			if(!computeJCFeatherSystemNode(node_ii,renderFeather)) continue;

			doWriteRibFiles(jcFeatherSysNodeName,renderFeather);

			jcFeatherSysUsed.append( inFeatherArray[ii].node() );
		}
	}

	if(_feaExpInf._fileType!=featherExportInfo::RIB) 
		_feaExpInf._featherAttrXmlIO.addTotalBoundingBox(_feaExpInf._xmlFileBoundingbox);
	return MS::kSuccess;
}

bool			featherGen::outputFeather(MString &tempStr,const singleFeather &feather)
{
	tempStr.clear();
	if( feather.rachis.length()<=0)
		return false;
	int outForSize=0;

	int endlCount=1;
	int vertexNum = _featherAttrs->_barbuleSegments;
	int vertexNum1 = feather.rachis.length();

	if(_featherAttrs->_uniformWidth)
		tempStr+="Declare \"width\" \"uniform float\"\n";
	else
		tempStr+="Declare \"width\" \"varying float\"\n";

	if( _featherAttrs->_curveType ==1 )
	{
		tempStr+="Curves \"cubic\" [ ";
		vertexNum += 2;
		vertexNum1 += 2;
	}
	else
		tempStr+="Curves \"linear\" [ ";

	//------------------------
	//------------------------
	if( _featherAttrs->_rachisRenderAs==0)
		tempStr += vertexNum1;//���rachis vertex num
	//------------------------
	//------------------------
	outForSize=feather.leftBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
	{
		tempStr+=" ";
		tempStr+=vertexNum ;
		if( ((endlCount++)%23)==0) 	tempStr+="\n";
	}
	outForSize=feather.rightBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
	{
		tempStr+=" ";
		tempStr+= vertexNum ;
		if(((endlCount++)%23)==0) tempStr+="\n";
	}

	tempStr+="]\n";
	//-----------------------------------------------------

	//----------------------------------------------------------------------
	tempStr+="\"nonperiodic\" \"P\" [";
	endlCount=1;

	//-------------------------rachis vertex position
	if( _featherAttrs->_rachisRenderAs==0  )
		tempStr += featherTools::convertPointArrayToString(feather.rachis,endlCount,(bool)_featherAttrs->_curveType);

	//-------------------------barbule vertex position
	outForSize=feather.leftBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
		tempStr += featherTools::convertPointArrayToString(feather.leftBarbules[jj],endlCount,(bool)_featherAttrs->_curveType);
	outForSize=feather.rightBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
		tempStr += featherTools::convertPointArrayToString(feather.rightBarbules[jj],endlCount,(bool)_featherAttrs->_curveType);

	tempStr+="]\n";
	//-------------------------------------------------------------


	//---------------------------------------------------------------
	//----------------------------------------------------------------------
	tempStr+="\"width\" [";
	endlCount=1;
	//Ϊ��ʹrachis ��barbule�Ĵ�ϸ������ë�Ĵ�Сͬ��ȱȱ仯�����Զ�����һ�� feather.rachisLength*0.01f ϵ��
	if(!_featherAttrs->_uniformWidth)
	{
		//----------rachis width
		if( _featherAttrs->_rachisRenderAs==0 )
		{
			outForSize=feather.rachisWidth.length();
			for(int kk=0;kk<outForSize;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.rachisWidth[kk]* feather.rachisLength*0.01f;
			}
		}
		//----------barbule width
		outForSize = feather.leftBarbules.size();
		int outForSize1=feather.barbuleWidth.length();
		for( int jj=0;jj<outForSize;++jj )
			for(int kk=0;kk<outForSize1;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.barbuleWidth[kk]*feather.leftBarbuleWidth[jj]* feather.rachisLength*0.01f;
				if( (endlCount++)%23==0) tempStr+="\n";
			}
		outForSize =feather.rightBarbules.size();
		outForSize1=feather.barbuleWidth.length();
		for( int jj=0;jj<outForSize;++jj )
			for(int kk=0;kk<outForSize1;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.barbuleWidth[kk]*feather.rightBarbuleWidth[jj]* feather.rachisLength*0.01f;
				if( (endlCount++)%23==0) tempStr+="\n";
			}
	}
	else
	{
		//----------rachis width
		if( _featherAttrs->_rachisRenderAs==0 )
		{
				tempStr+=" ";
				tempStr+=_featherAttrs->_rachisThick * feather.rachisLength*0.01f;
		}
		//----------barbule width
		outForSize=feather.leftBarbules.size();
		for( int jj=0;jj<outForSize;++jj )
		{
				tempStr+=" ";
				tempStr+=_featherAttrs->_barbuleThick*feather.leftBarbuleWidth[jj]* feather.rachisLength*0.01f;
				if( (endlCount++)%23==0) tempStr+="\n";
		}
		outForSize=feather.rightBarbules.size();
		for( int jj=0;jj<outForSize;++jj )
		{
				tempStr+=" ";
				tempStr+=_featherAttrs->_barbuleThick*feather.rightBarbuleWidth[jj]* feather.rachisLength*0.01f;
				if( (endlCount++)%23==0) tempStr+="\n";
		}
	}
	tempStr+="]\n";
	//-------------------------------------------------------

	//---------------------------------------------------------------
	//----------------------------------------------------------------------
	if(_feaExpInf.exportNormal )
	{
		tempStr+="\"N\" [";
		endlCount=1;
		//----------rachis normal
		if( _featherAttrs->_rachisRenderAs==0 )
		{
			outForSize=feather.rachisNormal.length();
			for(int kk=0;kk<outForSize;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.rachisNormal[kk].x;
				tempStr+=" ";
				tempStr+=feather.rachisNormal[kk].y;
				tempStr+=" ";
				tempStr+=feather.rachisNormal[kk].z;
				if( (endlCount++)%7==0) tempStr+="\n";
			}
		}
		//----------barbule normal
		outForSize = feather.leftBarbuleNormal.size();
		int outForSize1=0;
		for( int jj=0;jj<outForSize;++jj )
		{
			outForSize1 = feather.leftBarbuleNormal[jj].length();
			for(int kk=0;kk<outForSize1;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.leftBarbuleNormal[jj][kk].x;
				tempStr+=" ";
				tempStr+=feather.leftBarbuleNormal[jj][kk].y;
				tempStr+=" ";
				tempStr+=feather.leftBarbuleNormal[jj][kk].z;
				if( (endlCount++)%7==0) tempStr+="\n";
			}
		}
		outForSize =feather.rightBarbuleNormal.size();
		for( int jj=0;jj<outForSize;++jj )
		{
			outForSize1=feather.rightBarbuleNormal[jj].length();
			for(int kk=0;kk<outForSize1;++kk)
			{
				tempStr+=" ";
				tempStr+=feather.rightBarbuleNormal[jj][kk].x;
				tempStr+=" ";
				tempStr+=feather.rightBarbuleNormal[jj][kk].y;
				tempStr+=" ";
				tempStr+=feather.rightBarbuleNormal[jj][kk].z;
				if( (endlCount++)%7==0) tempStr+="\n";
			}
		}
		tempStr+="]\n";
	}
	//-------------------------------------------------------

	endlCount=1;
	tempStr+="\"jcFeaBarU\" [";

	//-----------featherBarbule u of rachis
	//------------------------------------
	if( _featherAttrs->_rachisRenderAs==0 )
	{
		tempStr+=" ";
		tempStr+= -1;
	}
	//---------------------------
	//---------------------------
	outForSize=feather.leftBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
	{
		tempStr+=" ";
		tempStr+=feather.leftBarbuleRachisPos[jj];
		if( ((endlCount++)%23)==0) tempStr+="\n";
	}
	outForSize=feather.rightBarbules.size();
	for( int jj=0;jj<outForSize;++jj )
	{
		tempStr+=" ";
		tempStr+=feather.rightBarbuleRachisPos[jj];
		if( ((endlCount++)%23)==0) tempStr+="\n";
	}
	tempStr+="]\n";

	if(_featherAttrs->_shader._uvProject)
	{
		//-----------------------------------------------
		tempStr+="\"jcFeaBarLenPer\" [";
		endlCount=1;
		//------------------------------------
		if( _featherAttrs->_rachisRenderAs==0 )
		{
			tempStr+=" ";
			tempStr+=1;
		}
		//---------------------------
		outForSize=feather.leftBarbuleLenPer.length();
		for( int jj=0;jj<outForSize;++jj )
		{
			tempStr+=" ";
			tempStr+=feather.leftBarbuleLenPer[jj];
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		outForSize=feather.rightBarbuleLenPer.length();
		for( int jj=0;jj<outForSize;++jj )
		{
			tempStr+=" ";
			tempStr+=feather.rightBarbuleLenPer[jj];
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		tempStr+="]\n";
		//-----------------------------------------------

		tempStr+="\"jcFeaUVProjectScale\" [";
		tempStr+=_featherAttrs->_shader._uvProjectScale;
		tempStr+="]\n";
	}


	//-------------------------------------------------------
	tempStr+="\"jcFeaRootC\" [";
	tempStr+=_featherAttrs->_shader._rootColor.r;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._rootColor.g;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._rootColor.b;
	tempStr+="] ";
	tempStr+="\"jcFeaTipC\" [";
	tempStr+=_featherAttrs->_shader._tipColor.r;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._tipColor.g;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._tipColor.b;
	tempStr+="] ";
	tempStr+="\"jcFeaMainC\" [";
	tempStr+=feather.mainColor.r;
	tempStr+=" ";
	tempStr+=feather.mainColor.g;
	tempStr+=" ";
	tempStr+=feather.mainColor.b;
	tempStr+="] ";

	//-------------------------
	//-------------------------
	//-------------------------
	if( _featherAttrs->_rachisRenderAs==0 )
	{
		tempStr+="\"jcFeaRachisRootC\" [";
		tempStr+=_featherAttrs->_shader._rachisRootColor.r;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisRootColor.g;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisRootColor.b;
		tempStr+="] ";
		tempStr+="\"jcFeaRachisTipC\" [";
		tempStr+=_featherAttrs->_shader._rachisTipColor.r;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisTipColor.g;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisTipColor.b;
		tempStr+="] ";
	}
	//-------------------------
	//-------------------------
	//-------------------------

	tempStr+="\"jcFeaBaseOpacity\" [";
	tempStr+=_featherAttrs->_shader._baseOpacity;
	tempStr+="] ";
	tempStr+="\"jcFeaFadeOpacity\" [";
	tempStr+=_featherAttrs->_shader._fadeOpacity;
	tempStr+="] ";
	tempStr+="\"jcFeaFadeStart\" [";
	tempStr+=_featherAttrs->_shader._fadeStart;
	tempStr+="] ";
	tempStr+="\"jcFeadiff\" [";
	tempStr+=_featherAttrs->_shader._barbuleDiffuse;
	tempStr+="] ";
	tempStr+="\"jcFeagloss\" [";
	tempStr+=_featherAttrs->_shader._barbuleGloss;
	tempStr+="]\n";
	tempStr+="\"jcFeaHueVar\" [";
	tempStr+=_featherAttrs->_shader._hueVar;
	tempStr+="] ";
	tempStr+="\"jcFeaSatVar\" [";
	tempStr+=_featherAttrs->_shader._satVar;
	tempStr+="] ";
	tempStr+="\"jcFeaValVar\" [";
	tempStr+=_featherAttrs->_shader._valVar;
	tempStr+="] ";
	tempStr+="\"jcFeaVarFreq\" [";
	tempStr+=_featherAttrs->_shader._varFreq;
	tempStr+="] ";

	tempStr+="\"jcFeaspec\" [";
	tempStr+=_featherAttrs->_shader._barbuleSpecular;
	tempStr+="] ";
	tempStr+="\"jcFeaspecColor\" [";
	tempStr+=_featherAttrs->_shader._barbuleSpecularColor.r;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._barbuleSpecularColor.g;
	tempStr+=" ";
	tempStr+=_featherAttrs->_shader._barbuleSpecularColor.b;
	tempStr+="] ";
	tempStr+="\"jcFeaselfshad\" [";
	tempStr+=_featherAttrs->_shader._selfShadow;
	tempStr+="] ";

	tempStr+="\"jcFeaSurU\" [";
	tempStr+=feather.surfaceUV[0];
	tempStr+="] ";
	tempStr+="\"jcFeaSurV\" [";
	tempStr+=feather.surfaceUV[1];
	tempStr+="] ";

	if(_featherAttrs->_shader._barbuleTexture!=""||feather.colorTexOverride!="")
	{
		tempStr+="\"jcFeaColorTex\" [\"";
		if(feather.colorTexOverride!="")
			tempStr+=feather.colorTexOverride;
		else
			tempStr+=_featherAttrs->_shader._barbuleTexture;
		tempStr+="\"] ";
	}
	tempStr+="\"jcFeaRandSeed\" [";
	tempStr+=feather.randSeed;
	tempStr+="]";
	tempStr+="\n";
	tempStr+=_featherAttrs->_postFeatherRib;
	return true;
}

void			featherGen::outputRachisCylinder(MString &tempStr,const singleFeather &feathers)
{
		int endlCount =1;
		unsigned int forLen=0;
		tempStr.clear();
		if(feathers.rachisCylinder.faceCounts.length()<=0)
			return;
		tempStr += "\nHierarchicalSubdivisionMesh \"catmull-clark\"\n";

		//-------face counts
		tempStr += "[";
		forLen=feathers.rachisCylinder.faceCounts.length();
		for(unsigned int ii=0;ii<forLen;++ii)
		{
			tempStr += feathers.rachisCylinder.faceCounts[ii];
			tempStr +=" ";
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		tempStr +="]\n";

		//-------face connects
		tempStr += "[";
		forLen=feathers.rachisCylinder.faceConnects.length();
		for(unsigned int ii=0,endlCount=1;ii<forLen;++ii)
		{
			tempStr += feathers.rachisCylinder.faceConnects[ii];
			tempStr += " ";
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		tempStr +="]\n";

		tempStr +=" [\"creasemethod\" \"facevaryingpropagatecorners\" \"interpolateboundary\"]";
		tempStr +=" [0 0 1 1 0 0 1 0 0]";
		tempStr +=" [1 1]";
		tempStr +=" []";
		tempStr +=" [\"chaikin\"]\n";


		//---------------point position
		tempStr +=" \"P\" [";
		forLen=feathers.rachisCylinder.pa.length();
		for(unsigned int ii=0,endlCount=1;ii<forLen;++ii)
		{
			tempStr += feathers.rachisCylinder.pa[ii].x;
			tempStr += " ";
			tempStr += feathers.rachisCylinder.pa[ii].y;
			tempStr += " ";
			tempStr += feathers.rachisCylinder.pa[ii].z;
			tempStr += " ";
			if( ((endlCount++)%7)==0) tempStr+="\n";
		}
		tempStr +="]\n";

		tempStr +="\"constant string primtype\" [\"subdiv\"]\n";

		tempStr +=" \"facevarying float s\" [";
		forLen = feathers.rachisCylinder.uvids.length();
		for(unsigned int ii=0,endlCount=1;ii<forLen;++ii)
		{
			tempStr += feathers.rachisCylinder.ua[feathers.rachisCylinder.uvids[ii]];
			tempStr += " ";
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		tempStr +="]\n";

		tempStr +=" \"facevarying float t\" [";
		forLen = feathers.rachisCylinder.uvids.length();
		for(unsigned int ii=0,endlCount=1;ii<forLen;++ii)
		{
			tempStr += feathers.rachisCylinder.va[feathers.rachisCylinder.uvids[ii]];
			tempStr += " ";
			if( ((endlCount++)%23)==0) tempStr+="\n";
		}
		tempStr +="]\n";

		tempStr+="\"jcFeaRachisRootC\" [";
		tempStr+=_featherAttrs->_shader._rachisRootColor.r*feathers.mainColor.r;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisRootColor.g*feathers.mainColor.g;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisRootColor.b*feathers.mainColor.b;
		tempStr+="] ";
		tempStr+="\"jcFeaRachisTipC\" [";
		tempStr+=_featherAttrs->_shader._rachisTipColor.r*feathers.mainColor.r;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisTipColor.g*feathers.mainColor.g;
		tempStr+=" ";
		tempStr+=_featherAttrs->_shader._rachisTipColor.b*feathers.mainColor.b;
		tempStr+="]\n";
		tempStr += _featherAttrs->_postRachisMeshRib;

}

bool			featherGen::writeAsciiRib( const MString &nodeName,const vector<singleFeather> &feathers)
{
	int forSize=feathers.size();
	if(forSize==0) return false;

	MString currentName = featherTools::getPadding(_feaExpInf._frame) + "/" + nodeName;//0001/jcFeatherShape1
	MString combineName =  _feaExpInf._localDir + currentName;// C:/jcFeather/  + 0001/jcFeatherShape1


	//---------------write feather curves to a file
	//
	//
	//
	//clock_t t1,t2;
	//t1=clock();
	MString subFile="",bbID="",content="",tempData="";
	string bbIDStr;
	for(int ii=0;ii<forSize;++ii)
	{
		subFile=combineName+"."+ii+"."+_feaExpInf._mbCurrentSample+_feaExpInf._filePostfix;
		// c:/feather/0001/jcFeatherShape1  (combineName)·���ӽڵ���
		//.0                                ("."+ii)feather�е���ëid��
		//.0								(_feaExpInf._mbCurrentSample)��ǰmotionblur�Ĳ����
		//.rib 
		outputFeather(content,feathers[ii]);
		memMapWriteFile( content.asChar(),subFile.asChar() );

		bbID= nodeName+"."+ii+"."+_feaExpInf._mbCurrentSample;//boungingbox��key��ʽ
		bbIDStr.assign(bbID.asChar());
		_feaExpInf._proxyBB.insert(pair<string,MBoundingBox>(bbIDStr,feathers[ii].proxyBBox));
	}
	//t2=clock();
	//MString info;
	//info +=jcFea_UseMultiThread;
	//info +=" ";
	//info +=jcFea_MaxThreads;
	//info +=" ";
	//info +=(double)(t2-t1);
	//MGlobal::displayInfo(info);


	//------------------���������
	//
	//
	//
	if(_featherAttrs->_rachisRenderAs==1)//render rachis as mesh
	{
		for(unsigned int ii=0;ii<forSize;++ii)
		{
			subFile=combineName+"."+ii+"."+_feaExpInf._mbCurrentSample+".mesh.rib";
			content.clear();

			outputRachisCylinder(tempData,feathers[ii]);
			content += tempData;
			memMapWriteFile( content.asChar(),subFile.asChar() );

			bbID= nodeName+".m."+ii+"."+_feaExpInf._mbCurrentSample;//boungingbox��key��ʽ
			bbIDStr.assign(bbID.asChar());
			_feaExpInf._proxyBB.insert(pair<string,MBoundingBox>(bbIDStr,feathers[ii].rachisMeshBBox));
		}
	}
	if(_feaExpInf._mbCurrentSample!=(_feaExpInf._mbSample-1))
		return true;//ֻ�е�Motionblur���һ�β����ʱ������½���

	//-----------------read the files above
	//-----------------
	//-----------------
	//
	//
	MString fileData="";
	fileData += _feaExpInf.comment;

	fileData+="\n";
	fileData+="\nAttributeBegin\n";
	fileData+="Surface \"";
	fileData+=_feaExpInf.featherRib;
	fileData+="\"\n";
	fileData+=addRManRibAttr(nodeName.asChar());
	fileData+=_feaExpInf.constantParam+"\n";

	fileData+=_featherAttrs->_preFeatherRib;

	MBoundingBox tempBB,multiBB;
	MString mblurFileContent,blurPath;
	for(unsigned int ii=0;ii<forSize;++ii)
	{
		multiBB.clear();
		mblurFileContent="";
		if(_feaExpInf.enableMotionBlur)
		{
			//----------------------------------------------------------------׼��blur�ļ��е�����
			/*
				MotionBegin [ 0 0.5 1.0 ]
					ReadArchive "*.rib"
					ReadArchive "*.rib"
					...
				MotionEnd
			*/
			mblurFileContent+="\nMotionBegin [";
			for(int jj=0;jj<_feaExpInf._mbSample;++jj)
			{
				mblurFileContent+=" ";
				//mblurFileContent+=(float)jj/((float)_feaExpInf._mbSample-1.0f)*_feaExpInf._mbFactor;
				mblurFileContent += (float)jj * _feaExpInf._mbStep * _feaExpInf._mbFactor;
			}
			mblurFileContent +=" ]";
			for(int jj=0;jj<_feaExpInf._mbSample;++jj)
			{
				bbID= nodeName+"."+ii+"."+jj;//boungingbox��key��ʽ
				bbIDStr.assign(bbID.asChar());
				if(getBBFromMap(bbIDStr,tempBB))
					multiBB.expand(tempBB);

				mblurFileContent += "\nReadArchive \"";
				if(!_feaExpInf._relativePath)
					mblurFileContent += combineName+"."+ii+"."+jj+".rib\"";
				else
					mblurFileContent += "./"+currentName+"."+ii+"."+jj+".rib\"";
			}
			mblurFileContent += "\nMotionEnd";
			//----------------------------------------------------------------
			if(_feaExpInf._delayArchive)
			{
				blurPath = combineName+"."+ii+".blur.rib";
				if( memMapWriteFile(mblurFileContent.asChar(),blurPath.asChar()))//��blur�ļ�д��ȥ
				{
					fileData+="\nProcedural \"DelayedReadArchive\" [ \"";

					if(!_feaExpInf._relativePath)
						fileData+=combineName+"."+ii+".blur.rib\"";
					else
						fileData+="./"+currentName+"."+ii+".blur.rib\"";

					fileData +=" ] [";
					fileData += (float)multiBB.min().x;fileData += " ";
					fileData += (float)multiBB.max().x ;fileData += " ";
					fileData += (float)multiBB.min().y ;fileData += " ";
					fileData += (float)multiBB.max().y ;fileData += " ";
					fileData += (float)multiBB.min().z ;fileData += " ";
					fileData += (float)multiBB.max().z ;fileData += " ]";
				}
			}
			else
			{
				fileData += mblurFileContent;
				fileData +="\n";
			}
		}
		else//û��motion blur����һֻ֡��һ���ļ�
		{
			if(_feaExpInf._delayArchive)
				fileData+="\nProcedural \"DelayedReadArchive\" [ \"";
			else
				fileData+="\nReadArchive \"";

			if(!_feaExpInf._relativePath)
				fileData+=combineName+"."+ii+".0.rib\"";
			else
				fileData+="./"+currentName+"."+ii+".0.rib\"";

			if(_feaExpInf._delayArchive)
			{
				bbID= nodeName+"."+ii+".0";//boungingbox��key��ʽ
				bbIDStr.assign(bbID.asChar());
				fileData += " ] [ ";
				if(getBBFromMap(bbIDStr,tempBB))
				{
					fileData += (float)tempBB.min().x;fileData += " ";
					fileData += (float)tempBB.max().x ;fileData += " ";
					fileData += (float)tempBB.min().y ;fileData += " ";
					fileData += (float)tempBB.max().y ;fileData += " ";
					fileData += (float)tempBB.min().z ;fileData += " ";
					fileData += (float)tempBB.max().z ;fileData += " ]";
				}
			}
		}
	}

	fileData+="\nAttributeEnd\n\n";

	if(_featherAttrs->_rachisRenderAs==1)//render rachis as mesh
	{
		fileData+="AttributeBegin\n";
		fileData += "Surface \"";
		fileData += _feaExpInf.rachisMeshInsert ;
		fileData += "\"\n";
		fileData += _featherAttrs->_preRachisMeshRib;


		for(unsigned int ii=0;ii<forSize;++ii)
		{
			multiBB.clear();
			mblurFileContent="";
			if(_feaExpInf.enableMotionBlur)
			{
				//�洢motion blur ����
				mblurFileContent+="\nMotionBegin [";
				for(int jj=0;jj<_feaExpInf._mbSample;++jj)
				{
					mblurFileContent+=" ";
					//mblurFileContent+=(float)jj/((float)_feaExpInf._mbSample-1.0f)*_feaExpInf._mbFactor;
					mblurFileContent += ( (float)jj * _feaExpInf._mbStep * _feaExpInf._mbFactor );
				}
				mblurFileContent +=" ]";
				for(int jj=0;jj<_feaExpInf._mbSample;++jj)
				{
					bbID= nodeName+".m."+ii+"."+jj;//boungingbox��key��ʽ
					bbIDStr.assign(bbID.asChar());
					if(getBBFromMap(bbIDStr,tempBB))
						multiBB.expand(tempBB);

					mblurFileContent += "\nReadArchive \"";
					if(!_feaExpInf._relativePath)
						mblurFileContent += combineName+"."+ii+"."+jj+".mesh.rib\"";
					else
						mblurFileContent += "./"+currentName+"."+ii+"."+jj+".mesh.rib\"";
				}
				mblurFileContent += "\nMotionEnd";

				//if use delay read archive
				if(_feaExpInf._delayArchive)
				{
					blurPath = combineName+"."+ii+".blurMesh.rib";
					if( memMapWriteFile(mblurFileContent.asChar(),blurPath.asChar()))//��blur�ļ�д��ȥ
					{
						fileData+="\nProcedural \"DelayedReadArchive\" [ \"";

						if(!_feaExpInf._relativePath)
							fileData+=combineName+"."+ii+".blurMesh.rib\"";
						else
							fileData+="./"+currentName+"."+ii+".blurMesh.rib\"";

						fileData +=" ] [";
						fileData += (float)multiBB.min().x;fileData += " ";
						fileData += (float)multiBB.max().x ;fileData += " ";
						fileData += (float)multiBB.min().y ;fileData += " ";
						fileData += (float)multiBB.max().y ;fileData += " ";
						fileData += (float)multiBB.min().z ;fileData += " ";
						fileData += (float)multiBB.max().z ;fileData += " ]";
					}
				}
				else
				{
					fileData += mblurFileContent;
					fileData +="\n";
				}
			}
			else//û��motionblur��ֻд��һ���ļ�
			{
				if(_feaExpInf._delayArchive)
					fileData+="\nProcedural \"DelayedReadArchive\" [ \"";
				else
					fileData+="\nReadArchive \"";

				if(!_feaExpInf._relativePath)
					fileData+=combineName+"."+ii+".0.mesh.rib\"";
				else
					fileData+="./"+currentName+"."+ii+".0.mesh.rib\"";

				if(_feaExpInf._delayArchive)
				{
					bbID= nodeName+".m."+ii+".0";//boungingbox��key��ʽ
					bbIDStr.assign(bbID.asChar());
					fileData += " ] [ ";
					if(getBBFromMap(bbIDStr,tempBB))
					{
						fileData += (float)tempBB.min().x;fileData += " ";
						fileData += (float)tempBB.max().x ;fileData += " ";
						fileData += (float)tempBB.min().y ;fileData += " ";
						fileData += (float)tempBB.max().y ;fileData += " ";
						fileData += (float)tempBB.min().z ;fileData += " ";
						fileData += (float)tempBB.max().z ;fileData += " ]";
					}
				}
			}
		}
		fileData+="\nAttributeEnd\n\n";
	}

	MString realFile = combineName;
	realFile+=_feaExpInf._filePostfix;
	return memMapWriteFile( fileData.asChar(),realFile.asChar() );
}


MString			featherGen::getCurrentFCPath(const MString &nodeName)
{
	MString localCacheFile=featherTools::getPadding(_feaExpInf._frame);
	localCacheFile+="/";
	localCacheFile+=nodeName;
	localCacheFile+=".";
	localCacheFile+=_feaExpInf._mbCurrentSample;
	localCacheFile+=".fc";

	return (_feaExpInf._localDir+localCacheFile);
}

bool			featherGen::xmlAddData(const MString &nodeName)
{
	MBoundingBox bbox;

	for(unsigned int ii=0;
		ii<_feaExpInf._featherCacheIO._turtlesG._turtlesFeaInfo.size();
		++ii)
		bbox.expand(_feaExpInf._featherCacheIO._turtlesG._turtlesFeaInfo[ii]._bbox);
	for(unsigned int ii=0;
		ii<_feaExpInf._featherCacheIO._surfaces._surfaceFeaInfo.size();
		++ii)
		bbox.expand(_feaExpInf._featherCacheIO._surfaces._surfaceFeaInfo[ii]._bbox);

	_feaExpInf._xmlFileBoundingbox.expand(bbox);//�洢���xml�ļ������е���ݵ�boundingbox�������ڵ���xml�ļ�ʱʹ�á�


	return _feaExpInf._featherAttrXmlIO.xmlAddFeather(  _featherAttrs,
														nodeName.asChar(),
														_feaExpInf._mbCurrentSample,
														bbox,
														_feaExpInf._renderAttr
														);
}

void			featherGen::assignMRampToJCRamp(MPlug &rampPlug,jcRamp &ramp,MDGContext &ctx)
{
	unsigned int numEle = rampPlug.numElements();
	MPlug currentEle;
	jcInterpolation jci;
	for(unsigned int ii=0;ii<numEle;++ii)
	{
		currentEle = rampPlug.elementByPhysicalIndex(ii);
		if(currentEle.numChildren()==3)
		{
			jci._positions.push_back( currentEle.child(0).asFloat(ctx));
			jci._values.push_back( currentEle.child(1).asFloat(ctx));
			jci._interps.push_back(currentEle.child(2).asInt(ctx));
		}
	}
	ramp.setSort(true);
	ramp.setEntries(jci);
}


//------------compute data for jcFeatherSystem node
void			featherGen::getGrowTexPosUV(	MFloatPointArray &posArray0,
								MFloatArray &featherPU0,
								MFloatArray &featherPV0,
								MFloatPointArray &outposArray,
								MFloatArray &outfeatherPU,
								MFloatArray &outfeatherPV)
{
	//----------------------�Ը���ͼ����---------------------------------//
	MFloatVectorArray sampleColors,sampleTransparencies;
	//-----------------------�����Ƿ���
	if(_jcSysD._textures[1].length()!=0 && featherTools::sampleTexture(_jcSysD._textures[1],&featherPU0,&featherPV0,sampleColors,sampleTransparencies)==MS::kSuccess)
	{
		unsigned int posAlen=posArray0.length();
		for(unsigned int ii=0;ii<posAlen;++ii)
			if( featherTools::getColorHSV(MColor(sampleColors[ii].x,sampleColors[ii].y,sampleColors[ii].z),2) >= noiseRandom(ii,10,0,_jcSysD._seed) )
			{
				outposArray.append(posArray0[ii]);
				outfeatherPU.append(featherPU0[ii]);
				outfeatherPV.append(featherPV0[ii]);
			}
	}
	else
	{
		outposArray = posArray0;
		outfeatherPU = featherPU0;
		outfeatherPV = featherPV0;
	}
}
void			featherGen::getTexData(MFloatArray &featherPU,
							MFloatArray &featherPV,
							MColorArray &sampleMainColor,
							MFloatArray &sampleScale,
							MFloatArray &sampleRadius)
{
	#pragma omp parallel sections if( jcFea_UseMultiThread ) num_threads(3)
	{
		#pragma omp  section
		{
			MFloatVectorArray sampleColors,sampleTransparencies;
			//----------------------������ɫƫ��
			if(_jcSysD._textures[2]!="" && featherTools::sampleTexture(_jcSysD._textures[2],&featherPU,&featherPV,sampleColors,sampleTransparencies)==MS::kSuccess)
				for(unsigned int ii=0;ii<featherPU.length();++ii)
					sampleMainColor.append( MColor(sampleColors[ii].x,sampleColors[ii].y,sampleColors[ii].z) );
		}
		#pragma omp  section
		{
			MFloatVectorArray sampleColors,sampleTransparencies;
			//-----------------------�����С����
			if(_jcSysD._textures[0]!="" && featherTools::sampleTexture(_jcSysD._textures[0],&featherPU,&featherPV,sampleColors,sampleTransparencies)==MS::kSuccess)
				for(unsigned int ii=0;ii<featherPU.length();++ii)
					sampleScale.append( featherTools::getColorHSV(MColor(sampleColors[ii].x,sampleColors[ii].y,sampleColors[ii].z),2));
		}
		#pragma omp  section
		{
			MFloatVectorArray sampleColors,sampleTransparencies;
			//------------------------����kdtree�������뾶������
			if(_jcSysD._textures[3]!="" && featherTools::sampleTexture(_jcSysD._textures[3],&featherPU,&featherPV,sampleColors,sampleTransparencies)==MS::kSuccess)
				for(unsigned int ii=0;ii<featherPU.length();++ii)
					sampleRadius.append( featherTools::getColorHSV(MColor(sampleColors[ii].x,sampleColors[ii].y,sampleColors[ii].z),2) );
		}
	}
}
void			featherGen::getFeatherFromSys(MFloatPointArray &posArray0,
								   MFloatArray &featherPU0,
								   MFloatArray &featherPV0,
								   vector<singleFeather> &feathers)
{
	MStatus status;

	if(_jcSysD._radius<=0|| _jcSysD._globalScale<=0) return;
	Node *tempNodePt=NULL;

	////---------------------�����е���ë������Ƭ��id��ò��洢��turtleID
	if( _featherAttrs->_turtleChildrenID.size()<=0 ) return;
	_featherAttrs->_outputFeatherMesh = _jcSysD._outputFeatherMesh;
	//---------------------//

	//---------------------�ڸ�������ϼ����Ӧ�ó���ë�ĵ��λ�ã�������Щλ�ô洢��posArray
	MFloatPointArray posArray;
	MFloatArray featherPU,featherPV;
	getGrowTexPosUV(posArray0,featherPU0,featherPV0,posArray,featherPU,featherPV);

	MColorArray sampleMainColor;
	MFloatArray sampleScale,sampleRadius;
	getTexData(featherPU,featherPV,sampleMainColor,sampleScale,sampleRadius);

	//-------------------׼��kdtree����ݱ���,�����е�
	kdtree *ptree=NULL;
	struct kdres *presults=NULL;
	ptree = kd_create( 3 );

	turtles* tempTurtles=NULL;
	std::list<long>::iterator liter;
	jcFeaNearID idInKdTree((int)_featherAttrs->_turtleChildrenID.size());
	unsigned int indexInKT=0;
	for( liter=_featherAttrs->_turtleChildrenID.begin();liter!=_featherAttrs->_turtleChildrenID.end();liter++,indexInKT++)
	{
		idInKdTree.setValue( indexInKT,*liter );
		tempNodePt = turtlesDataBase->Find(idInKdTree.getValue(indexInKT));
		if(!tempNodePt) continue;

		tempTurtles = reinterpret_cast<turtles*>( tempNodePt->Value );

		//����������е�rootposition�����ǵ�id��¼��kdtree
		if( tempTurtles->_turtles.size()>0 )
		{
			int addStatus =kd_insert3f(ptree,
						tempTurtles->_turtles[0].currentPos.x,
						tempTurtles->_turtles[0].currentPos.y,
						tempTurtles->_turtles[0].currentPos.z,
						idInKdTree.getValue(indexInKT));
			assert(addStatus==0);
		}
	}
	  //---------------------

	//---------------------���kdtree�е���ݣ������ÿ��λ�ö�Ӧ����ë
	std::vector<idDist> nearIDs;

	float currentPos[3]={0,0,0};
	void* tempresItem=NULL;
	idDist currentIDDist;

	turtles newTurtle;
	singleFeather sgf;
	float randScaleValue=0;
	
	unsigned int posAlen1=posArray.length();
	unsigned int sampRadLen=sampleRadius.length();
	for(unsigned int ii=0;ii<posAlen1;++ii)//�����������ϲ�����õ���ÿ���������õ�Ӧ�ó�����ë�����Բ�ֵturtle���
	{
		//����ڴ����
		nearIDs.clear();
		sgf.clear();
		newTurtle.clearKeyTurtle();

		//----�Ƿ���������ͼ�õ������
		float ltexraScale=1;
		if( posAlen1 == sampRadLen )
			ltexraScale =  sampleRadius[ii];

		presults = kd_nearest_range3f( ptree,posArray[ii].x,posArray[ii].y,posArray[ii].z,_jcSysD._radius * ltexraScale);
		if(!presults) continue;

		while( !kd_res_end( presults ) ) {
			/* get the data and position of the current result item */
			tempresItem = kd_res_itemf( presults, currentPos );
			if( !tempresItem ) continue;

			currentIDDist._id = *reinterpret_cast<long*>(tempresItem);
			currentIDDist._distance = distSquare(posArray[ii].x,posArray[ii].y,posArray[ii].z,currentPos[0],currentPos[1],currentPos[2]);
			nearIDs.push_back(currentIDDist);

			/* go to the next entry */
			kd_res_next( presults );
		 }
		//�ӽ�Զ���о���
		int idSize=(int)nearIDs.size() ;
		if( idSize>0 )
			jerryC::idDistSort(nearIDs,0,idSize-1);
		else
		{
			presults = kd_nearest3f( ptree,posArray[ii].x,posArray[ii].y,posArray[ii].z );
			if( !presults ) continue;
			tempresItem = kd_res_itemf( presults, currentPos );
			if( !tempresItem ) continue;

			idDist currentIDDist;
			currentIDDist._id = *reinterpret_cast<long*>(tempresItem);
			currentIDDist._distance = distSquare(posArray[ii].x,posArray[ii].y,posArray[ii].z,currentPos[0],currentPos[1],currentPos[2]);
			nearIDs.push_back( currentIDDist );
		}

		//----�Ƿ���������ͼ�õ������
		float ltexsizescale =1;
		if( sampleScale.length() == posArray.length() )
				ltexsizescale = sampleScale[ii];

		randScaleValue = noiseRandom(0,ii,23,_jcSysD._seed)*_jcSysD._randScale;
		if( interpolateFeather( posArray[ii],nearIDs,newTurtle,ltexsizescale * ( 1 - randScaleValue ), ltexraScale*ltexraScale))//���Բ�ֵ������õ��feather turtle
		{
			sgf.surfaceUV[0] = featherPU[ii];
			sgf.surfaceUV[1] = featherPV[ii];

			//----�Ƿ���������ͼ�õ������
			if( sampleMainColor.length() == posArray.length() )
				sgf.mainColor = sampleMainColor[ii];
			else
				sgf.mainColor = MColor(1,1,1);

			if( getJCFeather(newTurtle,sgf,_jcSysD._seed + ii) )//���feather turtle������������ë
				feathers.push_back( sgf );

		}
	}

	//---------------------���kdtree�е����
	idInKdTree.clear();
	if(presults)
		kd_res_free( presults );
	if(ptree)
	{
		kd_clear(ptree);
		kd_free( ptree );
	}

	posArray.clear();
	featherPU.clear();
	featherPV.clear();
	sampleMainColor.clear();
	sampleScale.clear();
	sampleRadius.clear();
	///--------
}
bool			featherGen::interpolateFeather(const MFloatPoint &growPos,
									const std::vector<idDist> &idData,
									turtles& featherTurtle,
									float texScale,
									float rascale
									)
{
	//�õ�������Ҫ�����������
	size_t realGuidNum=0;
	realGuidNum=idData.size();
	if( _jcSysD._guidPF < (int)realGuidNum )
		realGuidNum = _jcSysD._guidPF;
	if(realGuidNum<=0) return false;

	keyTurtle tempKeyTurtle;
	long tempFid = idData[0]._id;
	Node *tempNodePt=turtlesDataBase->Find(&tempFid);
	if(!tempNodePt) return false;

	float gaussExp=1,totalGauss=0,segLen=0;//tempW=0;
	MFloatVector tempPosDir,posDir;
	turtles* tTurtle = reinterpret_cast<turtles*>(tempNodePt->Value);
	size_t ttLen=tTurtle->_turtles.size();
	for(size_t ii=0;ii<ttLen;++ii)
	{
		tempKeyTurtle.setValue(MFloatPoint(0,0,0),MFloatVector(0,0,0));
		totalGauss = 0;segLen=0;
		tempPosDir.x = tempPosDir.y =tempPosDir.z =0;
		//tempW=0;
		for(size_t jj=0;jj<realGuidNum;++jj)
		{
			tempFid = idData[jj]._id;
			tempNodePt = turtlesDataBase->Find(&tempFid);
			if(!tempNodePt) return false;
			tTurtle = reinterpret_cast<turtles*>( tempNodePt->Value );

			//�ø�˹��ֵ�㷨���ÿ�������湱�׵�Ȩ��
			gaussExp =  exp( -idData[jj]._distance* _jcSysD._power*_jcSysD._power);
			if(gaussExp<0.0000001f) gaussExp=0.0000001f;

			//��Ȩ���������
			tempKeyTurtle.currentUp += tTurtle->_turtles[ii].currentUp * gaussExp;

			//��Ȩ����������Լ�����ĳ���
			if(ii!=0)
			{
				posDir = ( tTurtle->_turtles[ii].currentPos - tTurtle->_turtles[ii-1].currentPos );
				tempPosDir += posDir * gaussExp;
				segLen += posDir.length()*gaussExp;
			}

			totalGauss += gaussExp;
		}

		if(ii!=0)
		{
			tempPosDir.normalize();
			tempPosDir *= segLen/totalGauss;//�����򳤶��ܺͳ��Ը�˹��ֵ��Ȩ���ܺ͵õ�ʵ�ʵ����򳤶�
			tempKeyTurtle.currentPos = featherTurtle._turtles.back().currentPos + tempPosDir * _jcSysD._globalScale * texScale;//�����򳤶Ƚ����������ſ��Ƽ���ͼ����
		}
		else
			tempKeyTurtle.currentPos = growPos;

		tempKeyTurtle.currentUp.normalize();
		featherTurtle.addKeyTurtle(tempKeyTurtle);//���һ��keyTuretle
	}
	featherTurtle._scale = _jcSysD._globalScale * texScale;//�����е����Ŵ洢�����Ա����boundingbox����ʹ��
	return true;
}
void			featherGen::transformUVPosition( MFloatArray &ua,MFloatArray &va )
{
	int len=ua.length();

	#pragma omp parallel for if( jcFea_UseMultiThread && len>5) num_threads(jcFea_MaxThreads)
	for(int i=0;i<len;++i)
	{
		ua[i] += _jcSysD._translate[0];
		va[i] += _jcSysD._translate[1];
	}
}
MStatus			featherGen::getPositions( MItMeshPolygon* meshFaceIter,MFloatPointArray &outPositions,MFloatArray &ua,MFloatArray &va )
{
	MStatus status=MS::kSuccess;

	outPositions.clear();
	ua.clear();
	va.clear();

	double uur[2]={0,1},vvr[2]={0,1};
	MFloatArray tua,tva,ttua,ttva;
	MIntArray faceID;

	if(!_jcSysD._useUVI)
	{
		faceID.clear();
		int id1=0,id2=0;
		for( meshFaceIter->reset(); !meshFaceIter->isDone(); meshFaceIter->next() )
		{
			status = meshFaceIter->getUVs(ttua,ttva,&_jcSysD._uvSet);
			for(unsigned int i=0;i<ttua.length();++i)
			{
				tua.append(ttua[i]);
				tva.append(ttva[i]);
			}
		}
		if(status!=MS::kSuccess) return status;

		featherTools::getMinMax(tua,id1,id2);
		uur[0] = (double)tua[id1];		uur[1] = (double)tua[id2];
		featherTools::getMinMax(tva,id1,id2);
		vvr[0] = (double)tva[id1];		vvr[1] = (double)tva[id2];

		jerryC::jCell::setRange(uur,vvr);
		jerryC::jCell::GetValue(ttua,ttva);
		transformUVPosition(ttua,ttva);
	}
	else
	{
		if( !featherTools::getUVIFromFile(_jcSysD._uviFile,ttua,ttva,faceID))
			return MS::kFailure;
	}

	MFloatArray localua,localva;

	jerryC::tPolygoni polyPts;
	jerryC::tPointd queryPt;
	MPoint fpt;
	float2 storeUV;
	if( faceID.length() == 0 )
	{
		for(unsigned int ii=0;ii<ttua.length();++ii)
		{
			queryPt[0]=ttua[ii];
			queryPt[1]=ttva[ii];

			for( meshFaceIter->reset(); !meshFaceIter->isDone(); meshFaceIter->next() )
			{
				meshFaceIter->getUVs(localua,localva,&_jcSysD._uvSet);
				for(unsigned int jj=0;jj<localua.length();++jj)
				{
					polyPts[jj][0]=localua[jj];
					polyPts[jj][1]=localva[jj];
				}
				char pip = jerryC::InPoly( queryPt,polyPts,localua.length() );
				if( pip != 'o' )
				{
					storeUV[0] = (float)queryPt[0];
					storeUV[1] = (float)queryPt[1];

					if( meshFaceIter->getPointAtUV(fpt,storeUV,MSpace::kWorld,&_jcSysD._uvSet)==MS::kSuccess )
					{
						ua.append(storeUV[0]);
						va.append(storeUV[1]);
						outPositions.append( MFloatPoint(fpt) );
					}
					break;
				}
			}
		}
	}
	else
	{
		//����Ӧ���Ӧuv���λ����Ϣ�洢����
		meshFaceIter->reset();
		int dummyId=0;
		for(unsigned int ii=0;ii<faceID.length();++ii)
		{
			if( meshFaceIter->setIndex(faceID[ii],dummyId) != MS::kSuccess )
				continue;
			storeUV[0]= ttua[ii];
			storeUV[1]= ttva[ii];
			if( meshFaceIter->getPointAtUV(fpt,storeUV,MSpace::kWorld,&_jcSysD._uvSet)==MS::kSuccess )
			{
				ua.append(storeUV[0]);
				va.append(storeUV[1]);
				outPositions.append( MFloatPoint(fpt) );
			}
		}
	}

	return status;
}
MStatus			featherGen::getPositions( MDagPath &surfaceObj,MFloatPointArray &outPositions,MFloatArray &ua,MFloatArray &va)
{
	outPositions.clear();
	ua.clear();
	va.clear();

	//nurbsSurface���뽫uv��һ����0,1֮��
	MStatus status=MS::kSuccess;
	if(!surfaceObj.hasFn(MFn::kNurbsSurface)) return MS::kFailure;

	double uur[2]={0,1},vvr[2]={0,1};
	MFloatArray tua,tva,ttua,ttva;
	MPoint fpt;

	MFnNurbsSurface surfaceFn(surfaceObj);
	MIntArray faceID;

	if(!_jcSysD._useUVI)
	{
		jerryC::jCell::setRange(uur,vvr);
		jerryC::jCell::GetValue(ttua,ttva);
		transformUVPosition(ttua,ttva);
	}
	else
	{
		if( !featherTools::getUVIFromFile(_jcSysD._uviFile,ttua,ttva,faceID))
			return MS::kFailure;
	}

	for(unsigned int i=0;i<ttua.length();++i)
		if( surfaceFn.getPointAtParam( ttua[i],ttva[i],fpt,MSpace::kWorld )==MS::kSuccess )
		{
			outPositions.append(MFloatPoint(fpt));
			ua.append(ttua[i]);
			va.append(ttva[i]);
		}

	return status;
}
void			featherGen::getDisplayPositions(MFloatPointArray &outPositions,
									MFloatArray &ua,
									MFloatArray &va,
									float displayQuality)
{
	if(displayQuality>=1.0f) return;

	MFloatPointArray resultPositions;
	MFloatArray resultua,resultva;
	for(unsigned int ii=0;ii<outPositions.length();++ii)
		if(noiseRandom(0,0,ii+150,_jcSysD._seed)<displayQuality)
		{
			resultPositions.append(outPositions[ii]);
			resultua.append(ua[ii]);
			resultva.append(va[ii]);
		}
	outPositions.clear();
	ua.clear();va.clear();

	outPositions = resultPositions;
	ua = resultua;
	va = resultva;

	resultPositions.clear();
	resultua.clear();resultva.clear();
}
