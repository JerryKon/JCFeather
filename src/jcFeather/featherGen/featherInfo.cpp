#include "featherInfo.h"
#include "singleFeather.h"
#include <iostream>
#include <maya/MFnNurbsSurface.h>
#include <maya/MPlug.h>

jcRamp::jcRamp():jcRampBase()
{}
jcRamp::jcRamp(MRampAttribute &rampAttr)
{
	assign(rampAttr);
}
jcRamp::~jcRamp()
{}

void jcRamp::assign( MRampAttribute &other)
{
	MIntArray intIndices,ints;
	MFloatArray poses,values;

	MStatus status=MS::kSuccess;
	other.getEntries(intIndices,poses,values,ints,&status);
	if(!status) return;

	this->_needSort = true;
	this->_data.init();
	for(unsigned int ii=0;ii<intIndices.length();++ii)
	{
		this->_data._positions.push_back(poses[ii]);
		this->_data._values.push_back(values[ii]);
		this->_data._interps.push_back(ints[ii]);
	}
	this->sortRamp();
}

jcRamp& jcRamp::operator=( const MRampAttribute &other)
{
	MRampAttribute rother=other;
	assign(rother);
	return *this;
}


//-------------------------lsysTurtle

lsysTurtle::lsysTurtle(){	lsysInit(0.5);}
lsysTurtle::lsysTurtle(const MFloatPoint &pos,const MFloatVector &forward,const MFloatVector &up)
{
	currentPos = pos;
	currentDir = forward;
	currentUp = up;
}
lsysTurtle::~lsysTurtle(){};
void lsysTurtle::lsysInit(float pos)
{
	currentPos = MFloatPoint(0,0,pos);
	currentDir = MFloatVector(1.0,0,0);
	currentUp = MFloatVector(0,1,0);
}
void lsysTurtle::lsysInit(float fowardPos,float lrPos)
{
	currentPos = MFloatPoint(fowardPos,0,lrPos);
	currentDir = MFloatVector(1.0,0,0);
	currentUp  = MFloatVector(0,1,0);
}
void lsysTurtle::assign(const lsysTurtle &turtle)
{
	this->currentPos=turtle.currentPos;
	this->currentDir=turtle.currentDir;
	this->currentUp=turtle.currentUp;
}
MFloatMatrix lsysTurtle::getMatrix()//����������µ���ǰ����µľ���任
{
	MTransformationMatrix tranMatrix;

	MQuaternion quat1 = MVector(1,0,0).rotateTo(currentDir);
	tranMatrix.addRotationQuaternion(quat1.x,quat1.y,quat1.z,quat1.w,MSpace::kTransform);

	MVector newup= MVector(0,1,0).rotateBy(quat1);
	MQuaternion quat2=newup.rotateTo(currentUp);
	tranMatrix.addRotationQuaternion(quat2.x,quat2.y,quat2.z,quat2.w,MSpace::kTransform);

	tranMatrix.addTranslation(currentPos,MSpace::kWorld);

	return MFloatMatrix(tranMatrix.asMatrix().matrix);
}
lsysTurtle lsysTurtle::getTurtleInThis(lsysTurtle lturtle)//���������ת��Ϊ��turtle�µ����
{
	MFloatMatrix lmatrix = lturtle.getMatrix();

	lsysTurtle resultTurtle;
	resultTurtle.currentPos = lturtle.currentPos * lmatrix.inverse();
	resultTurtle.currentDir = lturtle.currentDir * lmatrix.inverse();
	resultTurtle.currentUp  = lturtle.currentUp * lmatrix.inverse();
	return resultTurtle;
}
lsysTurtle lsysTurtle::getTurtleInWorld(lsysTurtle lturtle)//��getTurtleInThis()�õ��Ľ��ת��Ϊ�������
{
	MFloatMatrix lmatrix = lturtle.getMatrix();

	lsysTurtle resultTurtle;
	resultTurtle.currentPos = lturtle.currentPos * lmatrix;
	resultTurtle.currentDir = lturtle.currentDir * lmatrix;
	resultTurtle.currentUp  = lturtle.currentUp * lmatrix;
	return resultTurtle;
}
//-------------------------

//---------------------keyTurtle

keyTurtle::keyTurtle()
{
	setValue( MFloatPoint(0,0,0),MFloatVector(0,0,0));
}
keyTurtle::~keyTurtle(){};
void keyTurtle::setValue(const MFloatPoint &pos,const MFloatVector &up)
{
	currentPos = pos;
	currentUp = up;
}
void keyTurtle::assign(const keyTurtle &turtle)
{
	this->currentPos = turtle.currentPos;
	this->currentUp = turtle.currentUp;
}
//---------------------


//---------------------turtles

void turtles::addKeyTurtle( const keyTurtle &kt )
{
	_turtles.push_back(kt);
}
void turtles::setExMeshPoints(const MFloatPointArray &lpts,const MFloatPointArray &rpts)
{
	if(lpts.length()<=1 && rpts.length()<=1)
	{
		_exMeshFea = false;
		return;
	}

	_leftPoints = lpts;
	_rightPoints = rpts;
	_exMeshFea = true;
}

void turtles::clearKeyTurtle()
{
	_turtles.clear();

	_leftPoints.clear();
	_rightPoints.clear();

	_scale = 1.0f;
	_exMeshFea=false;
}
turtles::turtles(){
	_scale = 1.0f;
	_exMeshFea=false;
}
turtles::~turtles(){}
void turtles::copyToTurtles(turtles &newTurtle)
{
	newTurtle.clearKeyTurtle();
	newTurtle._scale=this->_scale;
	newTurtle._turtles = this->_turtles;
	newTurtle._exMeshFea =this->_exMeshFea;

	newTurtle._leftPoints = this->_leftPoints;
	newTurtle._rightPoints = this->_rightPoints;

}
void turtles::getRachis(singleFeather &fea)//��turtles�ϵĵ�ӵ�rachis��
{
	int len = (int)_turtles.size();
	for(int ii=0;ii<len;++ii)
	{
		fea.rachis.append( _turtles[ii].currentPos );
		fea.rachisNormal.append(_turtles[ii].currentUp);
	}
	for(unsigned int jj=0;jj<fea.rachis.length()-1;++jj)
		fea.rachisLength += (fea.rachis[jj+1]-fea.rachis[jj]).length();
}
void turtles::getPosition(lsysTurtle &lsyst,float givePercent)//�õ���ǰLϵͳ��turtle��λ�úͷ���
{
	float realPosition=0;

	realPosition = givePercent* (float)(_turtles.size()-1);

	int num = (int)realPosition;
	float percent = realPosition - (float)num;

	MFloatPoint pos;
	MFloatVector dir;
	MFloatVector up;

	if( num < ((int)_turtles.size()-1))
	{
		dir = _turtles[num+1].currentPos - _turtles[num].currentPos;
		pos = _turtles[num].currentPos + dir * percent;

		up.x = jerryC::LinearInterpF(_turtles[num].currentUp.x,_turtles[num+1].currentUp.x,percent);
		up.y = jerryC::LinearInterpF(_turtles[num].currentUp.y,_turtles[num+1].currentUp.y,percent);
		up.z = jerryC::LinearInterpF(_turtles[num].currentUp.z,_turtles[num+1].currentUp.z,percent);

		lsyst.currentDir = dir;
		lsyst.currentPos = pos;
		lsyst.currentUp = up;
	}
	else
	{
		lsyst.currentPos = _turtles.back().currentPos;
		lsyst.currentDir = _turtles.back().currentPos - _turtles[_turtles.size()-2].currentPos;
		lsyst.currentUp = _turtles.back().currentUp;

	}
	lsyst.currentDir.normalize();
	lsyst.currentUp.normalize();
}
//----------------------

//-------------sysControlData

sysControlData::sysControlData(){init();}
sysControlData::~sysControlData(){init();}
void sysControlData::init()
{
	_featherID = -1;
	_power=1;
	_useUVI=false;
	_renderQuality=1;
	_uviFile="";
	_uvSet="";
	_outputFeatherMesh = false;
	_dispPer = 100;
	_guidPF = 1;
	_globalScale = 1;
	_translate[0]=_translate[1]=0;
	_randScale = 0;
	_radius = 10;
	_seed = 0;
	_textures.clear();
}

//---------------------keyBarbule
keyBarbule::keyBarbule(){	clear(); }
keyBarbule::~keyBarbule(){clear();}
void keyBarbule::copy(const keyBarbule &other)
{
	this->stepAngle = other.stepAngle;
	this->startAngle.assign(other.startAngle.begin(),other.startAngle.end());
	this->barbule.assign(other.barbule.begin(),other.barbule.end());
	this->ramp.assign(other.ramp.begin(),other.ramp.end());
}
void keyBarbule::clear()
{
	stepAngle = 0;
	startAngle.clear();
	barbule.clear();
	ramp.clear();
}
void keyBarbule::pushRamp(const jcRamp &rmp1,const jcRamp &rmp2,const jcRamp &rmp3,const jcRamp &rmp4)
{
	ramp.clear();
	ramp.push_back(rmp1);
	ramp.push_back(rmp2);
	ramp.push_back(rmp3);
	ramp.push_back(rmp4);
}
void keyBarbule::pushRamp(MRampAttribute &rmp1,MRampAttribute &rmp2,MRampAttribute &rmp3,MRampAttribute &rmp4)
{
	ramp.clear();
	ramp.push_back(jcRamp(rmp1));
	ramp.push_back(jcRamp(rmp2));
	ramp.push_back(jcRamp(rmp3));
	ramp.push_back(jcRamp(rmp4));
}
float keyBarbule::getAngleAt(float uPos,float vPos)
{
	float result1=0,result2=0;
	if( uPos<=barbule[0] )
	{
		ramp[0].getValueAtPosition(vPos,result1);
		return (result1-0.5f)*stepAngle;
	}
	else if( uPos>barbule[0] && uPos<=barbule[1])
	{
		ramp[0].getValueAtPosition(vPos,result1);
		ramp[1].getValueAtPosition(vPos,result2);
		return stepAngle*jerryC::LinearInterpF( result1-0.5f , result2 -0.5f , (uPos-barbule[0])/(barbule[1]-barbule[0]) );
	}
	else if( uPos>barbule[1] && uPos<=barbule[2] )
	{
		ramp[1].getValueAtPosition(vPos,result1);
		ramp[2].getValueAtPosition(vPos,result2);
		return stepAngle*jerryC::LinearInterpF( result1-0.5f , result2 -0.5f , (uPos-barbule[1])/(barbule[2]-barbule[1]) );
	}
	else if( uPos>barbule[2] && uPos<=barbule[3] )
	{
		ramp[2].getValueAtPosition(vPos,result1);
		ramp[3].getValueAtPosition(vPos,result2);
		return stepAngle*jerryC::LinearInterpF( result1-0.5f , result2 -0.5f , (uPos-barbule[2])/(barbule[3]-barbule[2]) );
	}
	else if( uPos>barbule[3] )
	{
		ramp[3].getValueAtPosition(vPos,result1);
		return (result1-0.5f)*stepAngle;
	}
	else
		return 0;
}
float keyBarbule::getStartAngleAt(float uPos)
{
	if( uPos<=barbule[0] )
		return startAngle[0];
	else if( uPos>barbule[0] && uPos<=barbule[1])
		return (jerryC::LinearInterpF(startAngle[0],startAngle[1],(uPos-barbule[0])/(barbule[1]-barbule[0])));
	else if( uPos>barbule[1] && uPos<=barbule[2])
		return (jerryC::LinearInterpF(startAngle[1],startAngle[2],(uPos-barbule[1])/(barbule[2]-barbule[1])));
	else if( uPos>barbule[2] && uPos<=barbule[3])
		return (jerryC::LinearInterpF(startAngle[2],startAngle[3],(uPos-barbule[2])/(barbule[3]-barbule[2])));
	else if(uPos>barbule[3])
		return startAngle[3];
	else
		return 0;
}

//---------------------turtleDataInfo
turtleDataInfo::turtleDataInfo(){
	_featherUV[0]=_featherUV[1]=0;
	_exMeshFea=false;
}
turtleDataInfo::~turtleDataInfo(){}
void turtleDataInfo::clear()
{
	_pos.clear();
	_nor.clear();

	_leftPt.clear();
	_rightPt.clear();

	_featherUV[0]=_featherUV[1]=0;
	_exMeshFea=false;
}

void turtleDataInfo::assign(const turtleDataInfo &other)
{
	this->clear();
	this->_pos = other._pos;
	this->_nor = other._nor;

	this->_leftPt = other._leftPt;
	this->_rightPt = other._rightPt;
	this->_featherUV[0] = other._featherUV[0];
	this->_featherUV[1] = other._featherUV[1];

	this->_exMeshFea = other._exMeshFea;
}
void turtleDataInfo::transform(const MMatrix &matrix,turtleDataInfo &result)
{
	result.clear();

	float fmatrixV[4][4];
	matrix.get(fmatrixV);
	MFloatMatrix fmatrix(fmatrixV);
	MPoint tempPt;

	result._exMeshFea = this->_exMeshFea;
	result._featherUV[0] = this->_featherUV[0];
	result._featherUV[1] = this->_featherUV[1];

	for(int ii=0;ii<this->_leftPt.length();++ii)
		result._leftPt.append( this->_leftPt[ii]*fmatrix );
	for(int ii=0;ii<this->_rightPt.length();++ii)
		result._rightPt.append( this->_rightPt[ii]*fmatrix );
	for(int ii=0;ii<this->_pos.length();++ii)
	{
		tempPt = MPoint(this->_pos[ii])*matrix;
		result._pos.append( MVector(tempPt) );
	}
	for(int ii=0;ii<this->_nor.length();++ii)
		result._nor.append( this->_nor[ii]*matrix );

}
//--------shader info
shaderInfo::shaderInfo()
{}
shaderInfo::~shaderInfo()
{}
void shaderInfo::copy(const shaderInfo &other)
{
	_rachisRootColor = other._rachisRootColor;
	_rachisTipColor = other._rachisTipColor;

	_rootColor =other._rootColor;
	_tipColor =other._tipColor;
	_baseOpacity=other._baseOpacity;
	_fadeOpacity = other._fadeOpacity;
	_fadeStart = other._fadeStart;
	_uvProject=other._uvProject;
	_uvProjectScale=other._uvProjectScale;
	_barbuleTexture = other._barbuleTexture;

	_barbuleDiffuse=other._barbuleDiffuse;
	_barbuleSpecular=other._barbuleSpecular;
	_barbuleSpecularColor=other._barbuleSpecularColor;
	_barbuleGloss=other._barbuleGloss;
	_selfShadow =other._selfShadow;

	_hueVar=other._hueVar;
	_satVar=other._satVar;
	_valVar=other._valVar;
	_varFreq=other._varFreq;
}

//--------------------featherInfo
featherInfo::featherInfo()
{
init();
}
featherInfo::~featherInfo()
{}
void featherInfo::init()
{
	_renderable = true;
	_exactFeather = false;
	_direction=0;
	_rachisSegment=10;
	_rachisRenderAs =0;
	_rachisPos = 0.5;
	_rachisSides = 4;
	_rachisThick=1.0;
	_rachisStart = 0.0;
	_rachisEnd = 1.0;

	_shapeSymmetry = false;
	_barbuleNum=100;
	_renderBarbuleNum= 100;
	_displayPercent=1.0f;
	_forcePerSegment=2;
	_turnForce=10.0;

	_rachisNoiseFrequency = 10;
	_barbuleNoiseFrequency = 20;
	_noisePhase[0] = 0;
	_noisePhase[1] = 0;
	_proxyBoundingBoxScale[0]=1;
	_proxyBoundingBoxScale[1]=1;
	_proxyBoundingBoxScale[2]=1;

	_rotateLamda=1;
	_upDownLamda = 1;
	_upDownNoise=true;
	_barbuleSegments=10;

	_barbuleLength=1.0f;
	_barbuleLengthRandScale=0.1f;

	_barbuleThick=1;
	_uniformWidth = false;
	_barbuleRandThick=0;
	_useOutShader = false;

	_outputFeatherMesh = false;
	_preFeatherRib="";
	_preRachisMeshRib = "";
	_postFeatherRib= "";
	_postRachisMeshRib ="";
	_shaderName="";
	_meshUVScale[0]=1;
	_meshUVScale[1]=1;
	_shaderNode = MObject::kNullObj;
	_shadingEngineNode = MObject::kNullObj;

	 _gapForce[0] = _gapForce[1] = 0;
	 _gapMaxForce[0] = _gapMaxForce[1] = 1;
	 _gapSize[0] = _gapSize[1] = 3;



	clear();
}
void featherInfo::clear()
{
	_keyBarAttrs.clear();
	_turtleChildrenID.clear();
}
bool featherInfo::copyData(featherInfo *other)
{
	if(!other)return false;

	_turtleChildrenID.assign(other->_turtleChildrenID.begin(),other->_turtleChildrenID.end());

	_keyBarAttrs.copy(other->_keyBarAttrs);
	_curveType = other->_curveType;

	_direction = other->_direction;

	_rachisPos = other->_rachisPos;
	_rachisRenderAs = other->_rachisRenderAs;
	_rachisSegment = other->_rachisSegment;
	_rachisSides = other->_rachisSides;
	_rachisThick = other->_rachisThick;
	_rachisStart = other->_rachisStart;
	_rachisEnd = other->_rachisEnd;
	_rachisThickScale = other->_rachisThickScale;

	_barbuleNum= other->_barbuleNum;
	_renderBarbuleNum= other->_renderBarbuleNum;
	_displayPercent= other->_displayPercent;
	_barbuleDensity= other->_barbuleDensity;
	_barbuleSegments= other->_barbuleSegments;
	_barbuleLength= other->_barbuleLength;
	_barbuleLengthRandScale= other->_barbuleLengthRandScale;
	_leftBarbuleLengthScale= other->_leftBarbuleLengthScale;
	_rightBarbuleLengthScale= other->_rightBarbuleLengthScale;

	_forcePerSegment= other->_forcePerSegment;
	_turnForce= other->_turnForce;
	_rotateLamda= other->_rotateLamda;
	_upDownLamda= other->_upDownLamda;
	_upDownNoise= other->_upDownNoise;

	_barbuleUpDownScale= other->_barbuleUpDownScale;
	_barbuleUpDownRachis= other->_barbuleUpDownRachis;
	_forceRotateScale= other->_forceRotateScale;
	_forceScale= other->_forceScale;

	_rachisNoiseFrequency= other->_rachisNoiseFrequency;
	_barbuleNoiseFrequency= other->_barbuleNoiseFrequency;
	_noisePhase[0]= other->_noisePhase[0];
	_noisePhase[1]= other->_noisePhase[1];

	 _gapForce[0]= other->_gapForce[0];_gapForce[1]= other->_gapForce[1];
	 _gapMaxForce[0]= other->_gapMaxForce[0];_gapMaxForce[1]= other->_gapMaxForce[1];
	 _gapSize[0]= other->_gapSize[0];_gapSize[1]= other->_gapSize[1];

	_barbuleThick= other->_barbuleThick;
	_barbuleThickScale= other->_barbuleThickScale;
	_barbuleRandThick= other->_barbuleRandThick;
	_barbuleThickAdjust= other->_barbuleThickAdjust;
	_gapDensity[0]= other->_gapDensity[0];
	_gapDensity[1]= other->_gapDensity[1];

	_uniformWidth= other->_uniformWidth;
	_proxyBoundingBoxScale[0]= other->_proxyBoundingBoxScale[0];
	_proxyBoundingBoxScale[1]= other->_proxyBoundingBoxScale[1];
	_proxyBoundingBoxScale[2]= other->_proxyBoundingBoxScale[2];

	_preFeatherRib= other->_preFeatherRib;
	_preRachisMeshRib= other->_preRachisMeshRib;
	_postFeatherRib= other->_postFeatherRib;
	_postRachisMeshRib= other->_postRachisMeshRib;

	_shader.copy( other->_shader );

	_randSeed= other->_randSeed;
	_outputFeatherMesh= other->_outputFeatherMesh;
	_meshUVScale[0]= other->_meshUVScale[0];
	_meshUVScale[1]= other->_meshUVScale[1];
	return true;
}

//---------------------featherTools
bool featherTools::checkLicense()
{
	return true;
}
bool featherTools::getUVIFromFile(MString &uvFile,MFloatArray &ua,MFloatArray &va,MIntArray &faceIndexAry)
{
	if(uvFile == "") return false;
	std::ifstream uviStream(uvFile.asChar(),std::ios::binary|std::ios::in);
	if(!uviStream)
	{
		MGlobal::displayError("jcFeather:: Can't open file "+uvFile+".");
		return false;
	}

	int uvNum=0,fid=0;
	uviStream.read(reinterpret_cast<char *>(&uvNum),sizeof(uvNum));

	//--------------�ж��Ƿ��ȡfaceIndex��Ϣ
	uviStream.read(reinterpret_cast<char *>(&fid),sizeof(fid));
	if(fid==1)
	{
		int tempI=-1;
		for(int ii=0;ii<uvNum;++ii)
		{
			uviStream.read(reinterpret_cast<char *>(&tempI),sizeof(tempI));
			faceIndexAry.append(tempI);
		}
	}

	double tempF=0;
	MFloatArray tempuvA;
	for(int ii=0;ii<uvNum*2;++ii)
	{
		uviStream.read(reinterpret_cast<char *>(&tempF),sizeof(tempF));
		tempuvA.append((float)tempF);
	}

	for(int kk=0;kk<((uvNum*2)-1);kk+=2)
	{
		ua.append(tempuvA[kk]);
		va.append(tempuvA[kk+1]);
	}
	return true;
}

bool featherTools::getPreGuideFromFile(const MString &file,verDirVector &vdData)
{
	std::ifstream uviStream(file.asChar(),std::ios::binary|std::ios::in);
	if(!uviStream)
	{
		MGlobal::displayError("Can't open file "+file+".");
		return false;
	}

	unsigned int vNum=0;
	uviStream.read(reinterpret_cast<char *>(&vNum),sizeof(unsigned int));
	vertexDirectionInfo tempVDI;
	vdData.clear();
	for(unsigned int ii=0;ii<vNum;++ii)
	{
		uviStream.read(reinterpret_cast<char *>(&tempVDI._vertexId),sizeof( int));
		uviStream.read(reinterpret_cast<char *>(&tempVDI._dirVertexId),sizeof( int));
		vdData.push_back(tempVDI);
	}

	return true;
}
bool featherTools::writePreGuideFile( const MString &file,verDirVector &vdData)
{
	unsigned int pNum = (unsigned int)vdData.size();
	if( pNum==0 )return false;

	ofstream os;
	os.open( file.asChar(),ios::out|ios::binary);
	if( !os )
	{
		MGlobal::displayError("jcFeather: Can't open file "+file+".");
		return false;
	}

	os.write( (char*)(&pNum), sizeof( unsigned int));
	for(unsigned int ii=0;ii<pNum;++ii)
	{
		os.write( (char*)(&vdData[ii]._vertexId), sizeof( int));
		os.write( (char*)(&vdData[ii]._dirVertexId), sizeof( int));
	}

	return true;
}

bool featherTools::getTurtles(const turtleDataInfo &tdInfo,turtles *result)
{
	unsigned int versize = tdInfo._pos.length();
	if(versize<=1 || tdInfo._nor.length()<=1 || versize!=tdInfo._nor.length()) return false;

	keyTurtle tempTurtle;
	for(unsigned int ii=0;ii<versize;++ii)
	{
		tempTurtle.currentPos.x =(float)tdInfo._pos[ii].x;
		tempTurtle.currentPos.y =(float)tdInfo._pos[ii].y;
		tempTurtle.currentPos.z =(float)tdInfo._pos[ii].z;

		tempTurtle.currentUp.x =(float)tdInfo._nor[ii].x;
		tempTurtle.currentUp.y =(float)tdInfo._nor[ii].y;
		tempTurtle.currentUp.z =(float)tdInfo._nor[ii].z;

		result->addKeyTurtle(tempTurtle);
	}

	if(tdInfo._exMeshFea)
		result->setExMeshPoints(tdInfo._leftPt,tdInfo._rightPt);
	return true;
}
MStatus featherTools::sampleTexture(MString nodeName,
									 MFloatArray *uCoord,
									 MFloatArray *vCoord,
									 MFloatVectorArray &resultColors,
									 MFloatVectorArray &resultTransparencies)
{
	MStatus status=MS::kSuccess;
	MDagPath cameraPath;
	CHECK_MSTATUS( M3dView::active3dView().getCamera( cameraPath ) );
	MMatrix mat = cameraPath.inclusiveMatrix(&status);
	CHECK_MSTATUS(status);
	MFloatMatrix cameraMat( mat.matrix );

	status = MRenderUtil::sampleShadingNetwork( nodeName,
												uCoord->length(),
												false,
												false,
												cameraMat,
												NULL,
												uCoord,
												vCoord,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												resultColors,
												resultTransparencies);
	return status;
}
void featherTools::computeStringIDArray(const MString &inStr,MIntArray &outVertex,unsigned int maxVerid)
{
		MStringArray splitA,secondSplitA;
		outVertex.clear();
		if(maxVerid==0||inStr.length()==0) return;
		MIntArray rawVertex;
		inStr.split(' ',splitA);
		unsigned int sasize=splitA.length();
		unsigned int first=0,last=0;
		for(unsigned int ii=0;ii<sasize;++ii)
		{
			secondSplitA.clear();
			splitA[ii].split(':',secondSplitA);

			if(secondSplitA.length()==1 && secondSplitA[0].isUnsigned())
			{
				unsigned int tempId=secondSplitA[0].asUnsigned();
				if(tempId < maxVerid)
					outVertex.append( tempId );
			}
			else if(secondSplitA.length()==2
					&&secondSplitA[0].isUnsigned()
					&&secondSplitA[1].isUnsigned())
			{
				first = secondSplitA[0].asUnsigned();
				last = secondSplitA[1].asUnsigned();
				if(last>=first)
				{
					for(unsigned int jj= first;jj<=last;jj++)
						if(jj<maxVerid)
							outVertex.append(jj);
				}
				else
				{
					for(unsigned int jj= first;jj>=last;jj--)
					{
						outVertex.append(jj);
						if(jj==0)break;
					}
				}
			}
		}

		if(outVertex.length()<=1) outVertex.clear();
}
void featherTools::computeStringMultiArray(int step,
									 int num,
									 const MString &inStr,
									 intArrayVector &outVertex,
									 int maxVerid)
{
	MIntArray initA;
	outVertex.clear();
	if(maxVerid==0||inStr.length()==0) return;
	computeStringIDArray(inStr,initA,maxVerid);
	int iniASize = initA.length();

	MIntArray tempIA;
	if(num<0)
		num = std::numeric_limits<int>::max();
	for(int ii=0;ii<num;++ii)
	{
		tempIA.clear();
		for(int jj=0;jj<iniASize;++jj)
		{
			int currentId = initA[jj] + step * ii;
			if(currentId < maxVerid )
				tempIA.append( currentId );
			else
				return;
		}
		if(tempIA.length()>0)
			outVertex.push_back(tempIA);
	}
}

void featherTools::getPFromMesh(const MFnMesh& meshFn,
							const MIntArray& verts,
							MFloatPointArray &outPoint)
{
	int versize = verts.length();
	MPoint pos;
	for(int ii=0;ii<versize;++ii)
		if( meshFn.getPoint(verts[ii],pos,MSpace::kWorld)==MS::kSuccess)
			outPoint.append(MFloatPoint(pos));
}
void featherTools::getPNFromMesh(const MFnMesh& meshFn,
							const MIntArray& verts,
							MVectorArray &outPoint,
							MVectorArray &outNormal,
							MString &outStr)
{
	unsigned int versize = verts.length();

	MPoint pos;
	MVector normal;
	for(unsigned int ii=0;ii<versize;++ii)
		if( meshFn.getPoint(verts[ii],pos,MSpace::kWorld)==MS::kSuccess&&
			meshFn.getVertexNormal(verts[ii],false,normal,MSpace::kWorld)==MS::kSuccess)
		{
			outPoint.append(pos);
			outNormal.append(normal);
			outStr += verts[ii];
			outStr += " ";
		}
	if(outPoint.length()<=1)
	{
		outPoint.clear();
		outNormal.clear();
		outStr = "";
	}
}
void featherTools::getMultiSidePFromMesh(const MFnMesh& meshFn,
										   const intArrayVector &inVertex,
										   turtleArrayData &outTurtles,
										   bool left)
{
	int  inVS = inVertex.size();
	inVS=min(inVS,(int)outTurtles.size());
	if(left)
		for(int ii=0;ii<inVS;++ii)
			getPFromMesh(meshFn,inVertex[ii],outTurtles[ii]._leftPt);
	else
		for(int ii=0;ii<inVS;++ii)
		{
			getPFromMesh(meshFn,inVertex[ii],outTurtles[ii]._rightPt);
			outTurtles[ii]._exMeshFea=true;
		}
}

void featherTools::getMultiPNFromMesh(const MFnMesh& meshFn,
							   const intArrayVector &inVertex,
							   turtleArrayData &outTurtles
							   )
{
	int  inVS = inVertex.size();
	turtleDataInfo currentTDI;
	MString updaStr;
	for(int ii=0;ii<inVS;++ii)
	{
		currentTDI.clear();
		getPNFromMesh( meshFn,inVertex[ii],currentTDI._pos,currentTDI._nor,updaStr );
		if( currentTDI._pos.length()>1 )
			outTurtles.push_back(currentTDI);
	}
}
void featherTools::getMultiPNFromMesh(const MFnMesh& meshFn,
							   const intArrayVector &inVertex,
							   turtleArrayData &outTurtles,
							   const MFloatArray &uValue,
							   const MFloatArray &vValue,
							   const MIntArray &verValue
							   )
{
	int  inVS = inVertex.size();
	int ulen=uValue.length();
	//int verLen = verValue.length();
	turtleDataInfo currentTDI;
	MString updaStr;
	for(int ii=0;ii<inVS;++ii)
	{
		currentTDI.clear();
		getPNFromMesh( meshFn,inVertex[ii],currentTDI._pos,currentTDI._nor,updaStr );
		if( currentTDI._pos.length()>1 )
		{
			if(ii< ulen)
			{
				currentTDI._featherUV[0] = uValue[inVertex[ii][0]];
				currentTDI._featherUV[1] = vValue[inVertex[ii][0]];
			}
			outTurtles.push_back(currentTDI);
		}
	}
}
void featherTools::getTurtle_UVData( turtleArrayData &inTurtles,
								MFnMesh& meshFn,
								MString *uvSetName )
{
	int  inVS = inTurtles.size();
	float2 currentUV={0,0};
	int closesPg=0;
	MPoint tempPt;
	for(int ii=0;ii<inVS;++ii)
	{
	    tempPt = MPoint(inTurtles[ii]._pos[0]);
		meshFn.getUVAtPoint(tempPt,currentUV,MSpace::kWorld,uvSetName,&closesPg);
		inTurtles[ii]._featherUV[0] = currentUV[0];
		inTurtles[ii]._featherUV[1] = currentUV[1];
	}
}
void featherTools::getFeaComponentDrawPoints(	const MVectorArray &outPoint,
										const MVectorArray &outNormal,
										float locs,
										MPointArray &drawPts)
{
	if(outPoint.length()<=1) return;

	MVector dir = MPoint(outPoint[1]) - MPoint(outPoint[0]);
	dir *= locs;
	drawPts.append( MPoint(outPoint[0]) + dir);
	MQuaternion  quat(1.5707963f,outNormal[0]);//��ת90��
	dir = dir.rotateBy(quat);

	MPoint tempPt;
	tempPt = MPoint( outPoint[0] ) + dir*0.3f;
	drawPts.append( tempPt );
	tempPt = MPoint( outPoint[0] ) - dir*0.3f;
	drawPts.append( tempPt );
}
void featherTools::getComponentIds(const MFnMesh &growMeshFn,
									MObject &componentList,
									MIntArray &outId,
									int componentType)
{
		MStatus status=MS::kSuccess;

		//--------------�����ģ���Լ����id�ŵõ����洢��������id����ʲô��û�����ʾ���ģ�Ͷ�������
		MFnComponentListData compListFn( componentList );
		unsigned int comLen=compListFn.length();

		outId.clear();
//		int preIndex=0;
		int verNumID=0;

		if( comLen!= 0 )
		{
			MObject comp;
			MFnSingleIndexedComponent indexCompFn;
			for( unsigned int ii = 0; ii < comLen; ii++ )
			{
				comp = compListFn[ii];
				if( comp.apiType() == componentType )
				{
					indexCompFn.setObject(comp);
					unsigned int ec = indexCompFn.elementCount();
					for( unsigned int jj = 0; jj < ec; jj++ )
					{
						verNumID = indexCompFn.element(jj);
						outId.append( verNumID );
					}
				}
			}
		}
		else
		{
			int verNum=0;
			if(MFn::kMeshVertComponent ==componentType)
				verNum= growMeshFn.numVertices();
			else if(MFn::kMeshPolygonComponent ==componentType)
				verNum= growMeshFn.numPolygons();
			for( int ii = 0; ii < verNum; ii++ )
				outId.append(ii);
		}
}
void featherTools::getComponentList_verId(MObject &mesh,
							MObject &componentList,
							MIntArray &outVertexId)
{
		MStatus status=MS::kSuccess;
		MFnMesh growMeshFn(mesh);

		//--------------�����ģ���Լ����id�ŵõ����洢��������id����ʲô��û�����ʾ���ģ�Ͷ�������
		MFnComponentListData compListFn( componentList );
		unsigned int comLen=compListFn.length();
		MIntArray outFaceId;
		if( comLen!=0 )
		{
			MObject comp;
			MFnSingleIndexedComponent indexCompFn;
			for( unsigned int ii = 0; ii < comLen; ii++ )
			{
				comp = compListFn[ii];
				if( comp.apiType() == MFn::kMeshPolygonComponent )
				{
					indexCompFn.setObject(comp);
					unsigned int ec = indexCompFn.elementCount();
					for( unsigned int jj = 0; jj < ec; jj++ )
					{
					int faId = indexCompFn.element(jj);
					outFaceId.append( faId );
					}
				}
			}
		}
		else
		{
			int faceNum = growMeshFn.numPolygons();
			for( int ii = 0; ii < faceNum; ii++ )
				outFaceId.append(ii);
		}

		MItMeshPolygon polygonIt(mesh,&status);
		CHECK_MSTATUS( status );
		int preIndex=0;

		//--------------��faceת����vertex
		outVertexId.clear();
		bool ptexist=false;
		unsigned int faceNum = outFaceId.length(),verSize=0,rawVerSize=0;
		MIntArray tempIntA;
		for(unsigned int ii=0;ii<faceNum;++ii)
		{
			polygonIt.setIndex(outFaceId[ii],preIndex);

			if( polygonIt.onBoundary() ) continue;
			polygonIt.getVertices(tempIntA);
			verSize = tempIntA.length();
			for(unsigned int kk=0;kk<verSize;kk++)
			{
				ptexist = false;
				rawVerSize = outVertexId.length();
				for(unsigned int jj=0;jj<rawVerSize;++jj)
					if(tempIntA[kk] == outVertexId[jj] )
					{
							ptexist = true;
							break;
					}
				if(!ptexist) outVertexId.append(tempIntA[kk]);
			}
		}
}
bool featherTools::isObjectVisible(MObject &obj)
{
	MFnDagNode fndn(obj);;
	MFnDependencyNode fndd(obj);
	MStatus status;
	MPlug plg;
	bool visible[3];

	int pn=fndn.parentCount();
	for(int ii=-1;ii<pn;++ii)
	{
		visible[0] = true;
		visible[1] = true;
		visible[2] = true;

		if(ii==-1)
			fndd.setObject(obj);
		else
			fndd.setObject( fndn.parent(ii));

		plg = fndd.findPlug("overrideEnabled",false,&status);
		if(status != MS::kFailure)
			visible[0]=plg.asBool();
		plg = fndd.findPlug("overrideVisibility",false,&status);
		if(status!=MS::kFailure)
			visible[1]=plg.asBool();
		if(visible[0] &&!visible[1]) return false;

		plg = fndd.findPlug("visibility",false,&status);
		if(status != MS::kFailure)
			visible[2]=plg.asBool();

		if( !visible[0] && !visible[2] ) return false;
	}
	return true;
}
MStatus featherTools::nodeFromName(MString name, MObject & obj)
{
    MSelectionList tempList;
    tempList.add( name );
    if ( tempList.length() > 0 )
    {
          tempList.getDependNode( 0, obj );
          return MS::kSuccess;
    }
   return MS::kFailure;
}
MStatus featherTools::dagPathFromName(MString name, MDagPath & obj)
{
    MSelectionList tempList;
    tempList.add( name );
    if ( tempList.length() > 0 )
    {
          tempList.getDagPath( 0, obj );
          return MS::kSuccess;
    }
   return MS::kFailure;
}
MStatus featherTools::computeDataForPointDir(const MVectorArray &pos,
											MObject &mesh,
											MString &uvSetName,
											MFloatArray &ua,
											MFloatArray &va,
											MIntArray &faceIndexAry,
											MPointArray &closestPt,
											float toler)
{
	unsigned int particleSize = pos.length();
	float2 uvValue={0,0};
	int polyId=0;
	MPoint cltp,tempPt;
	MFnMesh meshFn(mesh);

	ua.clear();va.clear();faceIndexAry.clear();closestPt.clear();

	MPointArray trianglePts;
	MIntArray traingleIds;
	for(unsigned int ii=0;ii<particleSize;++ii)
	{
	    tempPt = MPoint(pos[ii]);
		if( meshFn.getUVAtPoint(tempPt,uvValue,MSpace::kWorld,&uvSetName,&polyId) ==MS::kSuccess)
		{
			ua.append(uvValue[0]);
			va.append(uvValue[1]);
			faceIndexAry.append(polyId);
			meshFn.getPointAtUV(polyId,cltp,uvValue,MSpace::kWorld,&uvSetName,toler);
			closestPt.append(cltp);
		}
	}
	return MS::kSuccess;
}
MStatus featherTools::computeDataForUVI(MDagPath &particle,MDagPath &mesh,
									MFloatArray &ua,
									MFloatArray &va,
									MIntArray &faceIndexAry,
									MIntArray &triangleVertexList)
{
	MStatus status=MS::kSuccess;

	MFnParticleSystem parFn(particle);
	MFnMesh meshFn(mesh);
	MItMeshPolygon meshPolyIt(mesh);
	int preIndex=0;

	MVectorArray pos;
	parFn.position(pos);

	unsigned int particleSize = pos.length();
	float2 uvValue={0,0};
	int polyId=0;
	MString currentUV= meshFn.currentUVSetName(&status);
	CHECK_MSTATUS( status );
	MPoint clpt;
	ua.clear();va.clear();faceIndexAry.clear();triangleVertexList.clear();

	MPointArray trianglePts;
	MIntArray traingleIds;
	int triIndex=0;
	MPoint tempPt;
	for(unsigned int ii=0;ii<particleSize;++ii)
	{
	    tempPt = MPoint(pos[ii]);
		if( meshFn.getUVAtPoint(tempPt,uvValue,MSpace::kWorld,&currentUV,&polyId) ==MS::kSuccess)
		{
			ua.append(uvValue[0]);
			va.append(uvValue[1]);
			faceIndexAry.append(polyId);

			meshFn.getPointAtUV(polyId,clpt,uvValue,MSpace::kWorld,&currentUV);

			meshPolyIt.setIndex(polyId,preIndex);
			meshPolyIt.getTriangles(trianglePts,traingleIds,MSpace::kWorld);
			if( isPointInTriangle(clpt,trianglePts,triIndex) )
			{
				triangleVertexList.append(traingleIds[triIndex]);
				triangleVertexList.append(traingleIds[triIndex+1]);
				triangleVertexList.append(traingleIds[triIndex+2]);
			}
			else
			{
				triangleVertexList.append(traingleIds[0]);
				triangleVertexList.append(traingleIds[1]);
				triangleVertexList.append(traingleIds[2]);
			}
		}
	}
	return MS::kSuccess;
}


void featherTools::getMinMax(const MFloatArray &fa,int &minID,int &maxID)
{
	float tempMin=std::numeric_limits<float>::max();
	float tempMax=std::numeric_limits<float>::min();
	unsigned int forLen=fa.length();
	for(unsigned int ii=0;ii<forLen;++ii)
	{
		if(fa[ii]<tempMin) {tempMin = fa[ii];minID=ii;}
		if(fa[ii]>tempMax) {tempMax = fa[ii];maxID=ii;}
	}
}

float featherTools::getColorHSV(const MColor &col,int hsvIndex)
{
	float hsv[3];
	col.get(MColor::kHSV,hsv[0],hsv[1],hsv[2]);
	return hsv[hsvIndex];
}
std::string featherTools::numtostr(const double value)
{
    std::ostringstream strs;
    strs << value;
    std::string str = strs.str();
    return str;
}
MString featherTools::checkNodeName(const MString &name)
{
	MStringArray strArray;
	name.split(':',strArray);

	unsigned int len=strArray.length();
	if(len==1)
		return name;
	else
	{
		MString result;
		for(unsigned int ii=0;ii<len;++ii)
		{
			result += strArray[ii];
			if( ii != (len-1) )
				result+="_";
		}
		return result;
	}
}
MString featherTools::getPadding(int f)
{
	MString ff="",result="";
	ff += abs(f);
	int bn = ff.numChars();
	for(int i=0;i<(4-bn);++i) result+="0";
	result +=ff;
	if(f<0) result = "-"+result;
	return result;
}
void featherTools::getPosNormFromTurtles(const turtles &feaTurtle,int sampleNum,int curveType,MFloatPointArray &fpa, MFloatVectorArray &fva)
{
	#pragma omp parallel sections if( jcFea_UseMultiThread ) num_threads(2)
	{
		#pragma omp  section
		{
			jcSpline3D curve;
			double percent=0;
			MPoint tempPt;
			getJCSplineTurtlesPos(feaTurtle,curve);
			for(int ii=0;ii<sampleNum;++ii)
			{
				percent=(double)ii/(double)(sampleNum-1);
				curve.getValueAt(curveType,percent,&tempPt.x,&tempPt.y,&tempPt.z);
				fpa.append(MFloatPoint(tempPt));
			}
		}
		#pragma omp  section
		{
			jcSpline3D curve;
			double percent=0;
			MVector tempVt;
			getJCSplineTurtlesNor(feaTurtle,curve);
			for(int ii=0;ii<sampleNum;++ii)
			{
				percent=(double)ii/(double)(sampleNum-1);
				curve.getValueAt(curveType,percent,&tempVt.x,&tempVt.y,&tempVt.z);
				fva.append(MFloatVector(tempVt));
			}
		}
	}
}
void featherTools::getJCSplineTurtlesSide(const MFloatPointArray &currentArray,jcSpline3D &curve)
{
	int keyTurSize=currentArray.length();
	if(keyTurSize<=1) return ;
	int addTwo=0;

	if(keyTurSize<4)
	{
		curve.setNum(keyTurSize+2);
		curve.setData(	0,-0.00001,
						currentArray[0].x,
						currentArray[0].y,
						currentArray[0].z);
		addTwo=1;
	}
	else
		curve.setNum(keyTurSize);

	float dist=0,tempf=0;
	for(int ii=0;ii<keyTurSize;++ii)
	{
		if(ii!=0)
		{
			tempf = pointDistance(currentArray[ii].x,currentArray[ii].y,currentArray[ii].z,
								  currentArray[ii-1].x,currentArray[ii-1].y,currentArray[ii-1].z);
			if(tempf<0.00001f)	tempf=0.00001f;
			dist+=tempf;
		}
		curve.setData(  ii+addTwo,(double)dist,
						currentArray[ii].x,
						currentArray[ii].y,
						currentArray[ii].z);
	}

	if(addTwo==1)
		curve.setData(  keyTurSize+1,(double)dist+0.00001,
						currentArray[keyTurSize-1].x,
						currentArray[keyTurSize-1].y,
						currentArray[keyTurSize-1].z);
}
void featherTools::getJCSplineTurtlesPos(const turtles &feaTurtle,jcSpline3D &curve)
{
	int keyTurSize=feaTurtle._turtles.size();
	if(keyTurSize<=1) return;
	int addTwo=0;

	if(keyTurSize<4)
	{
		curve.setNum(keyTurSize+2);
		curve.setData(	0,-0.00001,
						feaTurtle._turtles[0].currentPos.x,
						feaTurtle._turtles[0].currentPos.y,
						feaTurtle._turtles[0].currentPos.z);
		addTwo=1;
	}
	else
		curve.setNum(keyTurSize);

	double dist=0,tempd=0;
	for(int ii=0;ii<keyTurSize;++ii)
	{
		if(ii!=0)
		{
			tempd= feaTurtle._turtles[ii].currentPos.distanceTo(feaTurtle._turtles[ii-1].currentPos);
			if(tempd<0.00001)	tempd=0.00001;
			dist+=tempd;
		}
		curve.setData(  ii+addTwo,dist,
						feaTurtle._turtles[ii].currentPos.x,
						feaTurtle._turtles[ii].currentPos.y,
						feaTurtle._turtles[ii].currentPos.z);
	}

	if(addTwo==1)
		curve.setData(  keyTurSize+1,dist+0.00001,
						feaTurtle._turtles[keyTurSize-1].currentPos.x,
						feaTurtle._turtles[keyTurSize-1].currentPos.y,
						feaTurtle._turtles[keyTurSize-1].currentPos.z);
}
void featherTools::getJCSplineTurtlesNor(const turtles &feaTurtle,jcSpline3D &curve)
{
	int keyTurSize=feaTurtle._turtles.size();
	if(keyTurSize<=1) return;
	int addTwo=0;

	if(keyTurSize<4)
	{
		curve.setNum(keyTurSize+2);
		curve.setData(	0,-0.00001,
						feaTurtle._turtles[0].currentUp.x,
						feaTurtle._turtles[0].currentUp.y,
						feaTurtle._turtles[0].currentUp.z);
		addTwo=1;
	}
	else
		curve.setNum(keyTurSize);

	double dist=0,tempd=0;
	for(int ii=0;ii<keyTurSize;++ii)
	{
		if(ii!=0)
		{
			tempd= feaTurtle._turtles[ii].currentPos.distanceTo(feaTurtle._turtles[ii-1].currentPos);
			if(tempd<0.00001)	tempd=0.00001;
			dist+=tempd;
		}
		curve.setData(  ii+addTwo,dist,
						feaTurtle._turtles[ii].currentUp.x,
						feaTurtle._turtles[ii].currentUp.y,
						feaTurtle._turtles[ii].currentUp.z);
	}

	if(addTwo==1)
		curve.setData(  keyTurSize+1,dist+0.00001,
						feaTurtle._turtles[keyTurSize-1].currentUp.x,
						feaTurtle._turtles[keyTurSize-1].currentUp.y,
						feaTurtle._turtles[keyTurSize-1].currentUp.z);
}
void featherTools::getlsysTurtleFromSpline(int curveType,int step,int stepNum,jcSpline3D &posSp,jcSpline3D &norSp,lsysTurtle &lst)
{
	double curStep=(double)step/(double)stepNum;

	double postStep=0;
	if(step<stepNum)
		postStep =(double)(step+1)/(double)stepNum;
	else
		postStep=(double)(step-1)/(double)stepNum;

	MPoint curPt(0,0,0),postPt(0,0,0);
	posSp.getValueAt(curveType,curStep,&curPt.x,&curPt.y,&curPt.z);
	posSp.getValueAt(curveType,postStep,&postPt.x,&postPt.y,&postPt.z);
	lst.currentPos=MFloatPoint(curPt);
	if(step<stepNum)
		lst.currentDir=MFloatVector((postPt-curPt).normal());
	else
		lst.currentDir=MFloatVector((curPt-postPt).normal());

	norSp.getValueAt(curveType,curStep,&curPt.x,&curPt.y,&curPt.z);
	lst.currentUp = MFloatVector(curPt);
}


void featherTools::getPointWeightInTri(const MPoint &pt,const MPointArray &triangPts,double weights[3])
{
	double totalArea = triangleArea(triangPts[0],triangPts[1],triangPts[2]);
	weights[0] = triangleArea(pt,triangPts[1],triangPts[2])/totalArea;
	weights[1] = triangleArea(triangPts[0],pt,triangPts[2])/totalArea;
	weights[2] = triangleArea(triangPts[0],triangPts[1],pt)/totalArea;
}

double featherTools::triangleArea(const MPoint &pt1,const MPoint &pt2,const MPoint &pt3)
{

	double vx = (pt2.y-pt1.y)*(pt3.z - pt1.z) - (pt2.z-pt1.z)*(pt3.y - pt1.y);
	double vy = (pt2.z-pt1.z)*(pt3.x - pt1.x) - (pt2.x-pt1.x)*(pt3.z - pt1.z);
	double vz = (pt2.x-pt1.x)*(pt3.y - pt1.y) - (pt2.y-pt1.y)*(pt3.x - pt1.x);

	return (0.5 * MVector(vx,vy,vz).length());
}

bool featherTools::isPointInTriangle( const MPoint &pt,
						const MPoint &pt0,
						const MPoint &pt1,
						const MPoint &pt2)
{
	double splitArea=0,totalArea=0;
	splitArea =  triangleArea(pt,pt0,pt1);
	splitArea += triangleArea(pt,pt0,pt2);
	splitArea += triangleArea(pt,pt1,pt2);

	totalArea = triangleArea(pt0,pt1,pt2);
	if(jerryC::doubleEqual(splitArea,totalArea))
		return true;
	else
		return false;
}

void featherTools::getLocalPosInTriangle( const MPoint &pt,
						const MPoint &pt0,
						const MPoint &pt1,
						const MPoint &pt2,
						jcPoint2D &outLocalPos)
{

	MVector v1=pt0-pt1;
	MVector v2=pt2-pt0;

	MVector centVec=pt0-pt;
	MVector crosVec=centVec^v2;
	MVector unknowCrosVec=v1^v2;
	outLocalPos.x = crosVec.length()/unknowCrosVec.length();
	MVector realV2=(pt+outLocalPos.x*v1)-pt0;
	outLocalPos.y = realV2.length()/v2.length();
}

bool featherTools::isPointInTriangle(const MPoint &pt,const MPointArray &trianglePt,int &ptIndex)
{
	unsigned int trainglePtNum= trianglePt.length();
	double splitArea=0,totalArea=0;
	for(unsigned int ii=0;ii<trainglePtNum-2;ii+=3)
	{
		splitArea = triangleArea(pt,trianglePt[ii],trianglePt[ii+1]);
		splitArea +=triangleArea(pt,trianglePt[ii],trianglePt[ii+2]);
		splitArea +=triangleArea(pt,trianglePt[ii+1],trianglePt[ii+2]);

		totalArea =triangleArea(trianglePt[ii],trianglePt[ii+1],trianglePt[ii+2]);
		if(jerryC::doubleEqual(splitArea,totalArea))
		{
			ptIndex = ii;
			return true;
		}
	}
	return false;
}

void featherTools::getTrianleVec(int num,int seed,vector<jcPoint2D> &ptVec)
{
	CRandomMersenne randGen;
	randGen.RandomInit(seed);

	jcPoint2D temp;
	ptVec.clear();
	for( int ii=0;ii<num;++ii )
	{
		temp.x = randGen.Random();
		temp.y = randGen.Random();
		if((temp.x +temp.y)>1.0)
		{
			temp.x = 1-temp.x;
			temp.y = 1-temp.y;
		}
		ptVec.push_back(temp);
	}
}

void featherTools::ptVecInTriangle( const MPoint &pt0,const MPoint &pt1,const MPoint &pt2,
								const jcPoint2D  &ptVec,MPoint &outPt)
{
	outPt = pt0 + (pt1-pt0)*ptVec.x + (pt2-pt0)*ptVec.y;
}

void featherTools::ptVecInTriangle( const MPoint &pt1,const MPoint &pt2,const MPoint &pt3,
										const vector<jcPoint2D> &ptVec,MPointArray &outPts)
{
	MVector v1=pt2-pt1;
	MVector v2=pt3-pt1;
	outPts.clear();
	for( int ii=0;ii<ptVec.size();++ii )
		outPts.append(pt1 + v1*ptVec[ii].x + v2*ptVec[ii].y );
}

void featherTools::randPointInTriangle( const MPoint &pt1,const MPoint &pt2,const MPoint &pt3,
										int ptnum,int seed,MPointArray &outPts)
{
	MVector v1=pt2-pt1;
	MVector v2=pt3-pt1;

	CRandomMersenne randGen;
	randGen.RandomInit(seed);

	double rnd1=0,rnd2=0;
	outPts.clear();
	for( int ii=0;ii<ptnum;++ii )
	{
		rnd1 = randGen.Random();
		rnd2 = randGen.Random();
		if((rnd1+rnd2)>1.0)
		{
			rnd1=1-rnd1;
			rnd2=1-rnd2;
		}
		outPts.append(pt1+v1*rnd1+v2*rnd2);
	}
}

float featherTools::noiseRandom(int x,int y,int z,int seed)
{
	return (float)(jerryC::ValueNoise3D(x,y,z,seed)+1.0)/2.0;
}
