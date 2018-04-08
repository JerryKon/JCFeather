#include "preFeatherMesh.h"

preFeatherAttr::preFeatherAttr(){ init();}

preFeatherAttr::~preFeatherAttr(){}

void preFeatherAttr::init()
{
	_globalScale =1.0f;
	_randScale = 0;
	_rotateOffset[0]=_rotateOffset[1]=_rotateOffset[2]=0;
	_useRotNoise = false;
	_rotNoiseFre=1;
	_rotNoiseAmp[0]=_rotNoiseAmp[1]=_rotNoiseAmp[2]=0;
	_rotNoisePha[0]=_rotNoisePha[1]=_rotNoisePha[2]=0;
	_randSeed =1;
	_rotOrder = MTransformationMatrix::kZYX;

	_instanceMesh = MObject::kNullObj;
	_inGrownMesh  = MObject::kNullObj;
	_instanceTurtleData.clear();
	_uvSetName ="";
	_scaleTexture = "";

	_rotateArray.clear();
	_scaleArray.clear();
	_controlBend.clear();
	_controlColorMaps.clear();
	_mapIlluminaceValues.clear();
	_mapIlluminaceIndex.clear();

	_scatterData = NULL;
	_verDirTable.clear();
	_pointGrowInfo.clear();

	_vertexScale.clear();
	_vertexRotate.clear();
	
	_bendActive = false;
	_bendPosition = 0.8f;
	_bendCurvature = 0;
	_bendRandomCurvature = 0;
	_vertexBendCurvature.clear();
	_vertexBendPosition.clear();
}

void preFeatherAttr::initVerDirTable(const verDirVector  &verDirPoint)
{
	_verDirTable.clear();
	int vdsize=verDirPoint.size();
	for(int ii=0;ii<vdsize;++ii)
		_verDirTable.insert( pair<int,int>( verDirPoint[ii]._vertexId,verDirPoint[ii]._dirVertexId ) );
}

void preFeatherAttr::computeVerDirDatas( map<int,MVector> &idDir,MVectorArray &verNormals)
{
	idDir.clear();
	verNormals.clear();

	MFnMesh mfn(_inGrownMesh);
	MPoint oriPt,tarPt;
	MVector dir;
	map<int,int>::iterator mapIter;
	for(mapIter=_verDirTable.begin();mapIter!=_verDirTable.end();++mapIter)
	{
		if( !mfn.getPoint(mapIter->first,oriPt,MSpace::kWorld) )continue ;
		if( !mfn.getPoint(mapIter->second,tarPt,MSpace::kWorld) )continue ;
		dir = tarPt-oriPt;
		dir.normalize();
		idDir.insert(pair<int,MVector>(mapIter->first,dir));
	}
	int verLen=mfn.numVertices();
	verNormals.setLength(verLen);
	for(unsigned int ii=0;ii<verLen;++ii)
		mfn.getVertexNormal(ii,true,verNormals[ii],MSpace::kWorld);
}

bool preFeatherAttr::getVerDirData(int id, map<int,MVector> &idDir,MVector &dir)
{
	map<int,MVector>::iterator findIter=idDir.find(id);
	if(findIter==idDir.end())return false;
	else
	{
		dir = findIter->second;
		return true;
	}
}

bool preFeatherAttr::getTriangleIntpDir(const jcTri3Int &currentTriId,map<int,MVector> &mapVector,double weight[],MVector &outDir)
{
	bool triangHasData[3];
	MVectorArray triangDir(3);
	MVector dirIfNotEnough;
	
	triangHasData[0] = getVerDirData(currentTriId.x,mapVector,triangDir[0]);
	if(triangHasData[0]) dirIfNotEnough=triangDir[0];

	triangHasData[1] = getVerDirData(currentTriId.y,mapVector,triangDir[1]);
	if(triangHasData[1]) dirIfNotEnough=triangDir[1];

	triangHasData[2] = getVerDirData(currentTriId.z,mapVector,triangDir[2]);
	if(triangHasData[2]) dirIfNotEnough=triangDir[2];
	int hasData=(int)triangHasData[0]+(int)triangHasData[1]+(int)triangHasData[2];
	if(hasData==3) 
		outDir = weight[0]*triangDir[0] + weight[1]*triangDir[1] + weight[2]*triangDir[2];
	else if(hasData<3&&hasData>0)
		outDir=dirIfNotEnough;
	else 
		return false;
	return true;
}

void preFeatherAttr::initScatterData(jcScatterPointData *scatData)
{
	_scatterData = scatData;
}

void preFeatherAttr::prepareData()
{
	getVertexWeightMapValues();//get vertex scale and rotate
	getGrowPosData();//get grow positions
}

bool preFeatherAttr::GenMesh(  MObject &meshData)
{
	//---------得到polygon vertex的相关信息
	polyObj outPoly;
	GenMeshBase(outPoly);//gen mesh
	if( outPoly.convertToMObject(meshData,&_uvSetName) )
		return true;
	else
		return false;
}

void preFeatherAttr::GenFeatherTurtleArray(turtleArrayData &outTurtleArray)
{
	if( !GenTurtleArrayBase(outTurtleArray))
		outTurtleArray.clear();
}

void preFeatherAttr::getGrowPosData()//----将多边形vertex的位置，法线，方向(已经归一化)，uv，以及id号记录下来
{
	MStatus status=MS::kSuccess;
	
	MItMeshPolygon meshFaceIter(_inGrownMesh,&status);	
	if(!status) return;

	MFnMesh meshFn(_inGrownMesh);

	MVector currentNormal,finalDir,rot,scal;
	MPoint currentPt;
	float2 uvValue;
	int dummyIndex=0;
	MPointArray triPts(3);
	double triWeight[3];
	jcTri3Int currentTriId;
	double tempCurvature=1;

	map<int,MVector> verDir;
	MVectorArray allNormals;
	computeVerDirDatas( verDir ,allNormals);//compute all the vertex direction
	
	_pointGrowInfo.clear();

	int faceLen = _scatterData->_faceId.length();
	for(int ii=0;ii<faceLen;++ii)
	{
		if(!meshFaceIter.setIndex(_scatterData->_faceId[ii],dummyIndex)) continue;
		
		int uvLen=_scatterData->_pointsOnFace[ii].length();//get uv data length

		for(int jj=0;jj<uvLen;++jj)
		{
			//----------get three points position of triangle
			currentTriId = _scatterData->_triangleId[ _scatterData->_pointsOnFace[ii][jj] ];
			if(!meshFn.getPoint(currentTriId.x,triPts[0],MSpace::kWorld))continue;		
			if(!meshFn.getPoint(currentTriId.y,triPts[1],MSpace::kWorld))continue;	
			if(!meshFn.getPoint(currentTriId.z,triPts[2],MSpace::kWorld))continue;

			//----------get grow position and normal
			uvValue[0]=_scatterData->_u[ _scatterData->_pointsOnFace[ii][jj] ];
			uvValue[1]=_scatterData->_v[ _scatterData->_pointsOnFace[ii][jj] ];

			featherTools::ptVecInTriangle(triPts[0],triPts[1],triPts[2],
										  _scatterData->_localPos[_scatterData->_pointsOnFace[ii][jj]],
										  currentPt);
			//if( !meshFaceIter.getNormal(currentNormal,MSpace::kWorld) ) continue;//get normal
									
			//--------interpolate the direction of current point
			featherTools::getPointWeightInTri(currentPt,triPts,triWeight);
			currentNormal = triWeight[0]*allNormals[currentTriId.x]+
							triWeight[1]*allNormals[currentTriId.y]+
							triWeight[2]*allNormals[currentTriId.z];

			if ( !getTriangleIntpDir(currentTriId,verDir,triWeight,finalDir)) continue;
			getTriangleIntpRotScal(currentTriId,triWeight,rot,scal,tempCurvature);//get the vertex paint's and controller's rot scal
			
			if(scal.x<=0.00001f&&scal.y<=0.00001f&&scal.z<=0.00001f) continue;
			//----------------------------------------------------------------

			//------store the data
			_pointGrowInfo._dir.append(finalDir.normal());
			_pointGrowInfo._nor.append(currentNormal.normal());
			_pointGrowInfo._pts.append(currentPt);
			_pointGrowInfo._u.append(uvValue[0]);
			_pointGrowInfo._v.append(uvValue[1]);
			_pointGrowInfo._rotate.append(rot);
			_pointGrowInfo._scale.append(scal);
			_pointGrowInfo._bendCur.append(tempCurvature);
		}
	}
	if(!_bendActive) _pointGrowInfo._bendCur.clear();
}

void preFeatherAttr::bendPoint(const MFloatPointArray &inPts,float xPos,float curvature,
							   MFloatPointArray &outPts)
{
	if( curvature==0 ) 
	{
		outPts=inPts;
		return;
	}
	outPts.clear();
	unsigned int ptLen=inPts.length();
	float dist=0;
	float radius=1/curvature;

	MQuaternion rotQua;
	MFloatPoint originPt(xPos,0,0);
	MVector originVec(0,-radius,0);
	originVec.normalize();
	for(unsigned int ii=0;ii<ptLen;++ii)
	{
		dist = inPts[ii].x - xPos;
		if(dist<=0)
		{
			outPts.append(inPts[ii]);
			continue;
		}
		rotQua.setToZAxis(dist/radius);

		originPt.z=inPts[ii].z;
		originPt.y=radius+inPts[ii].y;
		outPts.append( originPt + abs(radius) * MFloatVector(originVec.rotateBy(rotQua)));
	}
}

bool preFeatherAttr::GenMeshBase( polyObj &finalPoly)
{
	MStatus status = MS::kSuccess;
	int vertexSize = _pointGrowInfo._u.length();
	if(vertexSize==0)return false;

	//-------------get instance mesh data structure
	polyObj instanceObj;
	if( !instanceObj.initWithMObject(_instanceMesh,&_uvSetName) )
		return false;
	int instanceVerSize = instanceObj.pa.length();
	
	float distanceMinMax[2]={std::numeric_limits<float>::max(),std::numeric_limits<float>::min()};
	if(_bendActive)
	{
		for(int ii=0;ii<instanceVerSize;++ii)
		{
			if(distanceMinMax[0]>instanceObj.pa[ii].x)
				distanceMinMax[0]=instanceObj.pa[ii].x;
		    if(distanceMinMax[1]<instanceObj.pa[ii].x)
				distanceMinMax[1]=instanceObj.pa[ii].x;
		}
		distanceMinMax[1]=distanceMinMax[1]-distanceMinMax[0];
	}

	MFnMesh growMeshFn(_inGrownMesh,&status);

	//-------scale texture
	if(_texScaleData.length()==0) getScaleTexture();

	int scaleLen=_texScaleData.length();

	//---------------------------------------init variables
	MVector currentRight,newRight;
	MPoint ptAtFace,currentPt,noisePt;
	MTransformationMatrix transMatrix,rotMatrix,scaleMatrix;
	MMatrix transMat;
	MQuaternion quat1,quat2;

	double currentRN=0;
	bool useInterpolate=false;
	double currentScale[3]={1,1,1},initRotate[3]={_rotateOffset[0],_rotateOffset[1],_rotateOffset[2]},noiseRot[3]={0,0,0},currentRot[3]={0,0,0};
	
	float fmatrixV[4][4];
	const double initMatrixRot[3]={0,0,0};
	
	int verRS=_vertexRotate.length();
	int verSS=_vertexScale.length();
	int mapSize=_controlColorMaps.length();
	MFloatPointArray *meshPtArray = new MFloatPointArray[vertexSize];
	MFloatPointArray currentPtAry;
	//clock_t t1,t2;
	//t1=clock();

	//-----------after some tests, i found out only when the vertexSize>500,the multi thread will work well.
	#pragma omp parallel for if( jcFea_UseMultiThread && vertexSize>500 ) num_threads(jcFea_MaxThreads)\
					private(currentPtAry,currentRight,currentPt,quat1,newRight,quat2)\
					private(transMatrix,rotMatrix,scaleMatrix,currentRot,noisePt,noiseRot)\
					private(currentRN,currentScale,fmatrixV,transMat)
	for(int ii=0;ii<vertexSize;++ii)
	{
		if(!_bendActive)
			currentPtAry = instanceObj.pa;
		else
			bendPoint(  instanceObj.pa,
						distanceMinMax[0]+_bendPosition*distanceMinMax[1],
						_bendCurvature + _pointGrowInfo._bendCur[ii] + _bendRandomCurvature * jerryC::ValueNoise3D((int)(_pointGrowInfo._u[ii]*5000), 
																					 (int)(_pointGrowInfo._v[ii]*6000),
																					 10,_randSeed ),
						currentPtAry);

		currentRight = _pointGrowInfo._dir[ii]^_pointGrowInfo._nor[ii];
		currentPt = _pointGrowInfo._pts[ii];


		//---------------------------------------------------------------------scale transform
		currentRN = 1- featherTools::noiseRandom( (int)(_pointGrowInfo._u[ii]*10000), (int)(_pointGrowInfo._v[ii]*10000),120,_randSeed ) *_randScale;
		currentScale[0] = currentScale[1]=currentScale[2]=1;
		currentScale[0]*=(currentRN*_globalScale);
		currentScale[1]*=(currentRN*_globalScale);
		currentScale[2]*=(currentRN*_globalScale);

		if(ii<scaleLen) //------scale with texture
		{
			currentScale[0] *= _texScaleData[ii].x;
			currentScale[1] *= _texScaleData[ii].y;
			currentScale[2] *= _texScaleData[ii].z;
		}
		if(verSS>0||mapSize>0)
		{
			currentScale[0] *= _pointGrowInfo._scale[ii].x;		
			currentScale[1] *= _pointGrowInfo._scale[ii].y;		
			currentScale[2] *= _pointGrowInfo._scale[ii].z;
		}

		scaleMatrix.setScale( currentScale,MSpace::kWorld );
		scaleMatrix.setScalePivot(MPoint(0,0,0),MSpace::kWorld,0);

		//--------------------------------------------------------------------rotate transform
		//以当前朝前和朝右做x，z轴，计算从原坐标到当前坐标的转换矩阵  旋转四元组
		quat1 = MVector(0,1,0).rotateTo(_pointGrowInfo._nor[ii]);
		newRight = MVector(0,0,1).rotateBy(quat1);
		quat2 = newRight.rotateTo(currentRight);

		rotMatrix.setRotation(initMatrixRot,_rotOrder);
		rotMatrix.setRotatePivot(MPoint(0,0,0),MSpace::kWorld,0);
		rotMatrix.addRotation(initRotate,_rotOrder,MSpace::kTransform);
		if(verSS>0||mapSize>0)
		{
			currentRot[0]=_pointGrowInfo._rotate[ii].x;		
			currentRot[1]=_pointGrowInfo._rotate[ii].y;		
			currentRot[2]=_pointGrowInfo._rotate[ii].z;
			rotMatrix.addRotation(currentRot,_rotOrder,MSpace::kTransform);		
		}
		
		if( _useRotNoise )//--------------add noise rotation
		{
			noisePt = MPoint(_pointGrowInfo._u[ii],0,_pointGrowInfo._v[ii]);
			noiseRot[0] = getRotNoise(noisePt,_randSeed) * _rotNoiseAmp[0] * jerryC::radians_per_degree;
			noiseRot[1] = getRotNoise(noisePt,_randSeed+100) * _rotNoiseAmp[1]* jerryC::radians_per_degree;
			noiseRot[2] = getRotNoise(noisePt,_randSeed+200) * _rotNoiseAmp[2]* jerryC::radians_per_degree;
			rotMatrix.addRotation(noiseRot,_rotOrder,MSpace::kTransform);
		}
		rotMatrix.addRotationQuaternion(quat1.x,quat1.y,quat1.z,quat1.w,MSpace::kTransform);
		rotMatrix.addRotationQuaternion(quat2.x,quat2.y,quat2.z,quat2.w,MSpace::kTransform);
		
		//---------------------------------------------------------------------translate transform
		transMatrix.setTranslation( MVector(currentPt),MSpace::kWorld );

		//---------------------------------------------------------------------do the transform
		transMat = scaleMatrix.asMatrix()*rotMatrix.asMatrix()*transMatrix.asMatrix();
		transMat.get(fmatrixV);
		MFloatMatrix fmatrix(fmatrixV);

		for(int kk=0;kk<instanceVerSize;++kk)
			currentPtAry[kk] *= fmatrix;
		meshPtArray[ii]=currentPtAry;
	}

	//--------add all the mesh together
	polyObj currentPoly;
	finalPoly.init();
	currentPoly.initWith(instanceObj);
	for(int ii=0;ii<vertexSize;++ii)
	{
		currentPoly.pa = meshPtArray[ii];
		finalPoly.appendMesh(currentPoly);
	}
	//t2=clock();
	//MString info;
	//info +=jcFea_UseMultiThread;
	//info +=" ";
	//info +=jcFea_MaxThreads;
	//info +=" ";
	//info +=(double)(t2-t1);
	//MGlobal::displayInfo(info);

	if(meshPtArray) delete [] meshPtArray;
	return true;
}

bool preFeatherAttr::GenTurtleArrayBase(turtleArrayData &outTurtleArray)
{
	MStatus status = MS::kSuccess;
	int vertexSize = _pointGrowInfo._u.length();
	if(vertexSize==0)return false;


	MFnMesh growMeshFn(_inGrownMesh,&status);

	//-------scale texture
	if(_texScaleData.length()==0) getScaleTexture();
	int scaleLen=_texScaleData.length();

	//---------------------------------------init variables
	MVector currentRight,newRight;
	MPoint ptAtFace,currentPt,noisePt;
	MTransformationMatrix transMatrix,rotMatrix,scaleMatrix;
	MMatrix transMat;
	MQuaternion quat1,quat2;

	double currentRN=0;
	bool useInterpolate=false;
	double currentScale[3]={1,1,1},initRotate[3]={_rotateOffset[0],_rotateOffset[1],_rotateOffset[2]},noiseRot[3]={0,0,0},currentRot[3]={0,0,0};
	
	const double initMatrixRot[3]={0,0,0};
	
	int verRS=_vertexRotate.length();
	int verSS=_vertexScale.length();
	int mapSize=_controlColorMaps.length();
	turtleDataInfo currentTurtleData;
	//clock_t t1,t2;
	//t1=clock();

	//-----------after some tests, i found out only when the vertexSize>500,the multi thread will work well.
	#pragma omp parallel for if( false&&jcFea_UseMultiThread && vertexSize>500 ) num_threads(jcFea_MaxThreads)\
					private(currentTurtleData,currentRight,currentPt,quat1,newRight,quat2)\
					private(transMatrix,rotMatrix,scaleMatrix,currentRot,noisePt,noiseRot)\
					private(currentRN,currentScale,transMat)
	for(int ii=0;ii<vertexSize;++ii)
	{
		currentRight = _pointGrowInfo._dir[ii]^_pointGrowInfo._nor[ii];
		currentPt = _pointGrowInfo._pts[ii];

		//---------------------------------------------------------------------scale transform
		currentRN = 1- featherTools::noiseRandom( (int)(_pointGrowInfo._u[ii]*10000), (int)(_pointGrowInfo._v[ii]*10000),120,_randSeed ) *_randScale;
		currentScale[0] = currentScale[1]=currentScale[2]=1;
		currentScale[0]*=(currentRN*_globalScale);
		currentScale[1]*=(currentRN*_globalScale);
		currentScale[2]*=(currentRN*_globalScale);

		if(ii<scaleLen) //------scale with texture
		{
			currentScale[0] *= _texScaleData[ii].x;
			currentScale[1] *= _texScaleData[ii].y;
			currentScale[2] *= _texScaleData[ii].z;
		}
		if(verSS>0||mapSize>0)
		{
			currentScale[0] *= _pointGrowInfo._scale[ii].x;		
			currentScale[1] *= _pointGrowInfo._scale[ii].y;		
			currentScale[2] *= _pointGrowInfo._scale[ii].z;
		}

		scaleMatrix.setScale( currentScale,MSpace::kWorld );
		scaleMatrix.setScalePivot(MPoint(0,0,0),MSpace::kWorld,0);

		//--------------------------------------------------------------------rotate transform
		//以当前朝前和朝右做x，z轴，计算从原坐标到当前坐标的转换矩阵  旋转四元组
		quat1 = MVector(0,1,0).rotateTo(_pointGrowInfo._nor[ii]);
		newRight = MVector(0,0,1).rotateBy(quat1);
		quat2 = newRight.rotateTo(currentRight);

		rotMatrix.setRotation(initMatrixRot,_rotOrder);
		rotMatrix.setRotatePivot(MPoint(0,0,0),MSpace::kWorld,0);
		rotMatrix.addRotation(initRotate,_rotOrder,MSpace::kTransform);
		if(verSS>0||mapSize>0)
		{
			currentRot[0]=_pointGrowInfo._rotate[ii].x;		
			currentRot[1]=_pointGrowInfo._rotate[ii].y;		
			currentRot[2]=_pointGrowInfo._rotate[ii].z;
			rotMatrix.addRotation(currentRot,_rotOrder,MSpace::kTransform);		
		}
		
		if( _useRotNoise )//--------------add noise rotation
		{
			noisePt = MPoint(_pointGrowInfo._u[ii],0,_pointGrowInfo._v[ii]);
			noiseRot[0] = getRotNoise(noisePt,_randSeed) * _rotNoiseAmp[0] * jerryC::radians_per_degree;
			noiseRot[1] = getRotNoise(noisePt,_randSeed+100) * _rotNoiseAmp[1]* jerryC::radians_per_degree;
			noiseRot[2] = getRotNoise(noisePt,_randSeed+200) * _rotNoiseAmp[2]* jerryC::radians_per_degree;
			rotMatrix.addRotation(noiseRot,_rotOrder,MSpace::kTransform);
		}
		rotMatrix.addRotationQuaternion(quat1.x,quat1.y,quat1.z,quat1.w,MSpace::kTransform);
		rotMatrix.addRotationQuaternion(quat2.x,quat2.y,quat2.z,quat2.w,MSpace::kTransform);
		
		//---------------------------------------------------------------------translate transform
		transMatrix.setTranslation( MVector(currentPt),MSpace::kWorld );

		//---------------------------------------------------------------------do the transform
		transMat = scaleMatrix.asMatrix()*rotMatrix.asMatrix()*transMatrix.asMatrix();

		_instanceTurtleData.transform(transMat,currentTurtleData);
		currentTurtleData._featherUV[0] = _pointGrowInfo._u[ii];
		currentTurtleData._featherUV[1] = _pointGrowInfo._v[ii];

		//-----------------transform turtle data
		outTurtleArray.push_back(currentTurtleData);
	}
	//t2=clock();
	//MString info;
	//info +=jcFea_UseMultiThread;
	//info +=" ";
	//info +=jcFea_MaxThreads;
	//info +=" ";
	//info +=(double)(t2-t1);
	//MGlobal::displayInfo(info);
	return true;
}

void preFeatherAttr::getVertexWeightMapValues()
{
	unsigned int mapSize=_controlColorMaps.length();
	if(mapSize==0)return;

	MItMeshVertex meshIt(_inGrownMesh);
	MFnMesh meshFn(_inGrownMesh);

	// get uv value for each vertex in this mesh
	MFloatArray texu(meshFn.numVertices(),0),texv(meshFn.numVertices(),0);
	int ii=0;
	float2 uvValue;
	for(meshIt.reset();!meshIt.isDone();meshIt.next())
	{
		if(!meshIt.getUV(uvValue,&_uvSetName)) continue;
		else
		{
			texu[ii]=uvValue[0];
			texv[ii]=uvValue[1];
		}
		ii++;
	}
	
	MFloatVectorArray col,trans;
	unsigned int uvLength=texu.length();
	MFloatArray tempValue;
	_mapIlluminaceValues.clear();
	_mapIlluminaceIndex.clear();
	MFloatArray valueSum(uvLength,0);
	float value=0;
	for(unsigned int ii=0;ii<mapSize;++ii)
	{
		if(	featherTools::sampleTexture(_controlColorMaps[ii],&texu,&texv,col,trans)==MS::kSuccess)
		{
			tempValue.clear();
			for(unsigned int jj=0;jj<uvLength;++jj)
			{
				value = featherTools::getColorHSV(MColor(col[jj].x,col[jj].y,col[jj].z),2);
				if(value<0.00001f)value=0.00001f;
				valueSum[jj] += value;
				tempValue.append( value );
			}
			_mapIlluminaceValues.push_back(tempValue);
			_mapIlluminaceIndex.append(ii);
		}
	}

	unsigned int colMapLength=_mapIlluminaceValues.size();
	for(unsigned int jj=0;jj<colMapLength;++jj)
		for(unsigned int ii=0;ii<uvLength;++ii)
				_mapIlluminaceValues[jj][ii]/=valueSum[ii];
}

void preFeatherAttr::getTriangleIntpRotScal(const jcTri3Int &triIds,double triWeight[],MVector &rot,MVector &scal,double &curvature)
{
	int vertexPaintSize0 = _vertexRotate.length();
	int vertexPaintSize1 = _vertexScale.length();
	int vertexCurSize = _vertexBendCurvature.length();

	MFnMesh meshFn(_inGrownMesh);
	int numVert = meshFn.numVertices();
	
	//------------------------vertex paint rot and scale
	if( vertexPaintSize0==numVert )
		rot =	_vertexRotate[triIds.x] * triWeight[0]+
				_vertexRotate[triIds.y] * triWeight[1]+ 
				_vertexRotate[triIds.z] * triWeight[2];
	else
		rot.x=rot.y=rot.z=0;

	if(vertexPaintSize1==numVert)
		scal = _vertexScale[triIds.x] * triWeight[0]+
			   _vertexScale[triIds.y] * triWeight[1]+ 
			   _vertexScale[triIds.z] * triWeight[2];	
	else
		scal.x=scal.y=scal.z=1;

	if(_bendActive && vertexCurSize==numVert)
		curvature = _vertexBendCurvature[triIds.x] * triWeight[0]+
				   _vertexBendCurvature[triIds.y] * triWeight[1]+ 
				   _vertexBendCurvature[triIds.z] * triWeight[2];	
	else
		curvature=0;
	//----------------------controller rot and scale
	if(_mapIlluminaceValues.size()>0)
	{
		MVectorArray tempRot(3,MVector(0,0,0)),tempScal(3,MVector(0,0,0));
		MDoubleArray tempBend(3,0);

		getInterpolateWeightData(triIds.x,tempRot[0],tempScal[0],tempBend[0]);
		getInterpolateWeightData(triIds.y,tempRot[1],tempScal[1],tempBend[1]);
		getInterpolateWeightData(triIds.z,tempRot[2],tempScal[2],tempBend[2]);

		rot +=	tempRot[0] * triWeight[0]+
				tempRot[1] * triWeight[1]+ 
				tempRot[2] * triWeight[2];
		scal.x *= (tempScal[0].x * triWeight[0]+
					tempScal[1].x * triWeight[1]+ 
					tempScal[2].x * triWeight[2]);
		scal.y *= (tempScal[0].y * triWeight[0]+
					tempScal[1].y * triWeight[1]+ 
					tempScal[2].z * triWeight[2]);
		scal.z *= (tempScal[0].x * triWeight[0]+
					tempScal[1].y * triWeight[1]+ 
					tempScal[2].z * triWeight[2]);

		curvature +=tempBend[0] * triWeight[0]+
					tempBend[1] * triWeight[1]+ 
					tempBend[2] * triWeight[2];
	}
}

void preFeatherAttr::getInterpolateWeightData(unsigned int index,MVector &outRot,MVector &outScale,double &curvature)
{
	unsigned int colMapLength=_mapIlluminaceValues.size();
	if(colMapLength==0) 
	{
		outRot.x = outRot.y= outRot.z=0;
		outScale.x = outScale.y= outScale.z=1;
		curvature=0;
		return;
	}
	else
	{
		outRot.x = outRot.y= outRot.z=0;
		outScale.x = outScale.y= outScale.z=0;
		curvature=0;
	}

	for(unsigned int jj=0;jj<colMapLength;++jj)
	{
		if(_mapIlluminaceValues[jj][index]<0.001)continue;
		outRot.x += _mapIlluminaceValues[jj][index] * _rotateArray[_mapIlluminaceIndex[jj]].x;
		outRot.y += _mapIlluminaceValues[jj][index] * _rotateArray[_mapIlluminaceIndex[jj]].y;
		outRot.z += _mapIlluminaceValues[jj][index] * _rotateArray[_mapIlluminaceIndex[jj]].z;

		outScale.x += _mapIlluminaceValues[jj][index] * _scaleArray[_mapIlluminaceIndex[jj]].x;
		outScale.y += _mapIlluminaceValues[jj][index] * _scaleArray[_mapIlluminaceIndex[jj]].y;
		outScale.z += _mapIlluminaceValues[jj][index] * _scaleArray[_mapIlluminaceIndex[jj]].z;

		curvature  += _mapIlluminaceValues[jj][index] * _controlBend[_mapIlluminaceIndex[jj]];
	}
}

void preFeatherAttr::getScaleTexture()
{
	MFloatVectorArray trans;
	if( _scaleTexture.length()!=0) //------a texture is connected
		featherTools::sampleTexture(_scaleTexture,&_pointGrowInfo._u,&_pointGrowInfo._v,_texScaleData,trans);//---sample the texture
}
