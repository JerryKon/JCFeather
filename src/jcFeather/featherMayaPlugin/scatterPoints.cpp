#include "scatterPoints.h"

jcScatterPointData jcScatterPointData::operator= ( const jcScatterPointData &other)
{
	this->init();
	this->_faceId = other._faceId;
	this->_triangleId = other._triangleId;
	this->_u = other._u;
	this->_v = other._v;
	this->_pointsOnFace = other._pointsOnFace;
	this->_pointFaceIndex = other._pointFaceIndex;
	this->_localPos = other._localPos;
	return *this;
}

bool jcScatterPointData::writeDataTo(const MString &file, jcScatterPointData &sctData)
{
	unsigned int pNum = (unsigned int)sctData._u.length();
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
		os.write( (char*)(&sctData._u[ii]), sizeof( float) );
		os.write( (char*)(&sctData._v[ii]), sizeof( float) );

		os.write( (char*)(&sctData._localPos[ii].x), sizeof( double) );
		os.write( (char*)(&sctData._localPos[ii].y), sizeof( double) );

		os.write( (char*)(&sctData._triangleId[ii].x), sizeof( int) );
		os.write( (char*)(&sctData._triangleId[ii].y), sizeof( int) );
		os.write( (char*)(&sctData._triangleId[ii].z), sizeof( int) );

		os.write( (char*)(&sctData._pointFaceIndex[ii]), sizeof( int) );
	}

	pNum =sctData._faceId.length();
	os.write( (char*)(&pNum), sizeof( unsigned int));
	for(unsigned int ii=0;ii<pNum;++ii)
	{
		os.write( (char*)(&sctData._faceId[ii]), sizeof( int) );
		
		unsigned int ppNum =sctData._pointsOnFace[ii].length();
		os.write( (char*)(&ppNum), sizeof( unsigned int));
		for(unsigned int jj=0;jj<ppNum;++jj)
			os.write( (char*)(&sctData._pointsOnFace[ii][jj]), sizeof( int) );
	}
	return true;
}

bool jcScatterPointData::readDataFrom(const MString &file, jcScatterPointData &sctData)
{
	std::ifstream ins(file.asChar(),std::ios::binary|std::ios::in);
	if(!ins)
	{
		MGlobal::displayError("Can't open file "+file+".");
		return false;
	}
	sctData.init();

	unsigned int vNum=0;
	ins.read(reinterpret_cast<char *>(&vNum),sizeof(unsigned int));
	sctData._u.setLength(vNum);
	sctData._v.setLength(vNum);
	sctData._localPos.resize(vNum);
	sctData._triangleId.resize(vNum);
	sctData._pointFaceIndex.setLength(vNum);
	for(unsigned int ii=0;ii<vNum;++ii)
	{
		ins.read(reinterpret_cast<char *>(&sctData._u[ii]),sizeof( float));
		ins.read(reinterpret_cast<char *>(&sctData._v[ii]),sizeof( float));

		ins.read(reinterpret_cast<char *>(&sctData._localPos[ii].x),sizeof( double));
		ins.read(reinterpret_cast<char *>(&sctData._localPos[ii].y),sizeof( double));

		ins.read(reinterpret_cast<char *>(&sctData._triangleId[ii].x),sizeof( int));
		ins.read(reinterpret_cast<char *>(&sctData._triangleId[ii].y),sizeof( int));
		ins.read(reinterpret_cast<char *>(&sctData._triangleId[ii].z),sizeof( int));

		ins.read(reinterpret_cast<char *>(&sctData._pointFaceIndex[ii]),sizeof( int));
	}

	ins.read(reinterpret_cast<char *>(&vNum),sizeof(unsigned int));
	sctData._faceId.setLength(vNum);
	sctData._pointsOnFace.resize(vNum);
	for(unsigned int ii=0;ii<vNum;++ii)
	{
		ins.read(reinterpret_cast<char *>(&sctData._faceId[ii]),sizeof( int));
		unsigned int vvNum=0;
		ins.read(reinterpret_cast<char *>(&vvNum),sizeof( unsigned int));
		sctData._pointsOnFace[ii].setLength(vvNum);
		for(unsigned int jj=0;jj<vvNum;++jj)
			ins.read(reinterpret_cast<char *>(&sctData._pointsOnFace[ii][jj]),sizeof( unsigned int));
	}
	return true;
}

void scatterPoints::init()
{
	_scatterMode = 0;
	_ptNumPerArea = 10;
	_inMesh = MObject::kNullObj;
	_inComponentList = MObject::kNullObj;
	_uv = "map1";
	_baldTex ="";
	_randSeed=21;
	_usrU.clear();
	_usrV.clear();
	_faceFromComponent=false;
	_maxDistance = 0.01;
	_usrInPosition.clear();
}

void scatterPoints::getComponentFace(MIntArray &faceIds)
{
	MFnComponentListData compListFn( _inComponentList );
	unsigned int comLen=compListFn.length();
		
	faceIds.clear();
	int faceNumID=0;

	if( comLen!= 0 )
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
					faceNumID = indexCompFn.element(jj);
					faceIds.append( faceNumID );
				}
			}
		}
	}

	if(faceIds.length()>0) 
		_faceFromComponent=true; //mark the face is from componentlist
	else 
	{
		MFnMesh meshFn(_inMesh);
		int faceCount = meshFn.numPolygons();
		for(int ii=0;ii<faceCount;++ii)
			faceIds.append(ii);
		_faceFromComponent=false;
	}

}

MStatus scatterPoints::getUVPositionsFromMesh( MFloatPointArray &outPositions)
{
	MStatus status=MS::kSuccess;

	outPositions.clear(); 
	int faceLen=_outScatterData._faceId.length();
	_outScatterData._pointsOnFace.resize(_outScatterData._faceId.length());

	MFloatArray tua,tva,ttua,ttva;

	int dummyIndex=0;
	MItMeshPolygon meshFaceIter(_inMesh);
	MFnMesh meshFn(_inMesh);

	if(_faceFromComponent==false)
	{
		if( _uv.length()==0&& meshFn.getCurrentUVSetName(_uv)!=MS::kSuccess )	return MS::kFailure;
		if( meshFn.getUVs(tua,tva,&_uv)!=MS::kSuccess) return MS::kFailure;
		_inComponentList = MObject::kNullObj;
	}
	else
	{
		for(int ii=0; ii<faceLen;++ii)
		{
			if( meshFaceIter.setIndex(_outScatterData._faceId[ii],dummyIndex)!=MS::kSuccess ||
				meshFaceIter.getUVs(ttua,ttva,&_uv)!=MS::kSuccess) 
				return MS::kFailure;
			for(unsigned int i=0;i<ttua.length();++i)
			{
				tua.append(ttua[i]);
				tva.append(ttva[i]);
			}
		}
	}
	if(tua.length()==0) return MS::kFailure;

	//---------get cell points
	int id1=0,id2=0;
	double uur[2]={0,1},vvr[2]={0,1};
	featherTools::getMinMax(tua,id1,id2);
	uur[0] = (double)tua[id1];		uur[1] = (double)tua[id2];
	featherTools::getMinMax(tva,id1,id2);
	vvr[0] = (double)tva[id1];		vvr[1] = (double)tva[id2];

	jerryC::jCell::setRange(uur,vvr);
	jerryC::jCell::GetValue(ttua,ttva);
	//---------get cell points

	MPoint fpt;
	float2 storeUV;
	unsigned int loulen=0;
	MPointArray triangPos;
	MIntArray triangVerIds;
	jcTri3Int tempTriId;
	jcPoint2D tempPt2d;
	int ulen=ttua.length();
	for(int ii=0; ii<faceLen;++ii)
	{
		if( ! meshFaceIter.setIndex(_outScatterData._faceId[ii],dummyIndex) ) continue;
		//get triangles
		if( ! meshFaceIter.getTriangles(triangPos,triangVerIds,MSpace::kWorld) ) continue;
		//get triangle num
		int trilen=triangVerIds.length()/3;//triangles num in this polygon

		for( int jj=0;jj<ulen;++jj)
		{
			storeUV[0]=ttua[jj];
			storeUV[1]=ttva[jj];

			//get current uv position in world space
			if( !meshFaceIter.getPointAtUV(fpt,storeUV,MSpace::kWorld,&_uv) ) continue;
			for(int kk=0;kk<trilen;++kk)
			{
				if(featherTools::isPointInTriangle(fpt,triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2]))
				{
					_outScatterData._pointsOnFace[ii].append(_outScatterData._u.length());
					
					_outScatterData._u.append(storeUV[0]);
					_outScatterData._v.append(storeUV[1]);
					
					featherTools::getLocalPosInTriangle(fpt,triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2],tempPt2d);
					_outScatterData._localPos.push_back(tempPt2d);

					tempTriId.x = triangVerIds[kk*3];
					tempTriId.y = triangVerIds[kk*3+1];
					tempTriId.z = triangVerIds[kk*3+2];
					_outScatterData._triangleId.push_back(tempTriId);

					_outScatterData._pointFaceIndex.append(ii);

					outPositions.append( MFloatPoint(fpt) );
					break;
				}	
			}
		}

	}
	return status;
}

MStatus scatterPoints::getAreaPositionsFromMesh( MFloatPointArray &outPositions )
{
	MStatus staus=MS::kSuccess;

	outPositions.clear();
	double uur[2]={0,1},vvr[2]={0,1};
	MFloatArray tua,tva,ttua,ttva;
	MItMeshPolygon meshFaceIter(_inMesh);
	MFnMesh meshFn(_inMesh);

	int faceLen=_outScatterData._faceId.length();
	_outScatterData._pointsOnFace.resize(faceLen);
	int dummyIndex=0;
	MPointArray triangPos;
	MIntArray triangVerIds;
	double triArea=0;
	MPointArray pts;
	vector<jcPoint2D> pt2dAry;
	float2 ptUV;
	int polyId=0;

	jcTri3Int tempTriId;
	for(int ii=0;ii<faceLen;++ii)
	{
		if(!meshFaceIter.setIndex(_outScatterData._faceId[ii],dummyIndex))continue;;
		if(!meshFaceIter.getTriangles(triangPos,triangVerIds,MSpace::kWorld)) continue;
		
		int trilen=triangVerIds.length()/3;//triangles num in this polygon
		
		for(int jj=0;jj<trilen;++jj)
		{
			triArea = featherTools::triangleArea(triangPos[jj*3],triangPos[jj*3+1],triangPos[jj*3+2]);
			triArea *=_ptNumPerArea;
			featherTools::getTrianleVec((int)clamp1_(triArea),_randSeed+ii+jj,pt2dAry);
			featherTools::ptVecInTriangle(  triangPos[jj*3],triangPos[jj*3+1],triangPos[jj*3+2],pt2dAry,pts);	
			for(int kk=0;kk<pts.length();++kk)
			{
				if( !meshFn.getUVAtPoint(pts[kk],ptUV,MSpace::kWorld,&_uv,&polyId)) continue;
				outPositions.append(MFloatPoint(pts[kk]));

				_outScatterData._pointsOnFace[ii].append(_outScatterData._u.length());

				_outScatterData._u.append(ptUV[0]);
				_outScatterData._v.append(ptUV[1]);
				_outScatterData._localPos.push_back(pt2dAry[kk]);

				tempTriId.x = triangVerIds[jj*3];
				tempTriId.y = triangVerIds[jj*3+1];
				tempTriId.z = triangVerIds[jj*3+2];
				_outScatterData._triangleId.push_back(tempTriId);

				_outScatterData._pointFaceIndex.append(ii);
			}
		}
	}
	return staus;
}

MStatus scatterPoints::getUserInPosition( MFloatPointArray &outPositions )
{
	MStatus status=MS::kSuccess;
	int posLen=_usrInPosition.length();
	if(posLen==0) return status;
	
	MFnMesh meshFn(_inMesh);
	MPoint fpt;
	float2 storeUV={0,0};

	MPointArray triangPos;
	MIntArray triangVerIds;
	int dummyIndex=0;

	MPointArray closetPt;
	MFloatArray closetU;
	MFloatArray closetV;
	MIntArray closetFace;
	for(int ii=0;ii<posLen;++ii)
	{
		meshFn.getClosestPoint(_usrInPosition[ii],fpt,MSpace::kWorld,&dummyIndex);
		if(fpt.distanceTo(_usrInPosition[ii])<=_maxDistance)
		{
			closetPt.append(fpt);
			closetFace.append(dummyIndex);
			meshFn.getUVAtPoint(fpt,storeUV,MSpace::kWorld,&_uv,&dummyIndex);
			closetU.append(storeUV[0]);
			closetV.append(storeUV[1]);
		}
	}

	int faceLen=_outScatterData._faceId.length();
	_outScatterData._pointsOnFace.resize(faceLen);

	int ulen = closetPt.length(),usrUVCount=0;
	MItMeshPolygon meshFaceIter(_inMesh);
	
	jcTri3Int tempTriId;
	jcPoint2D tempPt2d;
	for(int ii=0; ii<faceLen;++ii)
	{
		if( ! meshFaceIter.setIndex(_outScatterData._faceId[ii],dummyIndex) ) continue;
		//get triangles
		if( ! meshFaceIter.getTriangles(triangPos,triangVerIds,MSpace::kWorld) ) continue;
		//get triangle num
		int trilen=triangVerIds.length()/3;//triangles num in this polygon

		for( int jj=0;jj<ulen;++jj)
		{
			if( closetFace[jj] != _outScatterData._faceId[ii] ) continue;
			for(int kk=0;kk<trilen;++kk)
			{
				if(featherTools::isPointInTriangle(closetPt[jj],triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2]))
				{
					_outScatterData._pointsOnFace[ii].append(_outScatterData._u.length());
					
					_outScatterData._u.append(closetU[jj]);
					_outScatterData._v.append(closetV[jj]);
					featherTools::getLocalPosInTriangle(closetPt[jj],triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2],tempPt2d);
					_outScatterData._localPos.push_back(tempPt2d);

					tempTriId.x = triangVerIds[kk*3];
					tempTriId.y = triangVerIds[kk*3+1];
					tempTriId.z = triangVerIds[kk*3+2];
					_outScatterData._triangleId.push_back(tempTriId);
					_outScatterData._pointFaceIndex.append(ii);

					outPositions.append( MFloatPoint(closetPt[jj]) );
					break;
				}	
			}
		}
	}
	return status;
}

MStatus scatterPoints::addUsrUVData(MFloatPointArray &outPositions)
{
	MStatus status=MS::kSuccess;
	if(_usrU.length()==0) return status;

	MPoint fpt;
	float2 storeUV;
	
	MPointArray triangPos;
	MIntArray triangVerIds;
	int dummyIndex=0;

	int faceLen=_outScatterData._faceId.length();
	_outScatterData._pointsOnFace.resize(_outScatterData._faceId.length());

	int ulen=_usrU.length(),usrUVCount=0;
	MItMeshPolygon meshFaceIter(_inMesh);
	jcTri3Int tempTriId;
	jcPoint2D tempPt2d;
	for(int ii=0; ii<faceLen;++ii)
	{
		if(usrUVCount>ulen)break;
		if( ! meshFaceIter.setIndex(_outScatterData._faceId[ii],dummyIndex) ) continue;
		//get triangles
		if( ! meshFaceIter.getTriangles(triangPos,triangVerIds,MSpace::kWorld) ) continue;
		//get triangle num
		int trilen=triangVerIds.length()/3;//triangles num in this polygon

		for( int jj=0;jj<ulen;++jj)
		{
			storeUV[0]=_usrU[jj];
			storeUV[1]=_usrV[jj];

			//get current uv position in world space
			if( !meshFaceIter.getPointAtUV(fpt,storeUV,MSpace::kWorld,&_uv) ) continue;
			for(int kk=0;kk<trilen;++kk)
			{
				if(featherTools::isPointInTriangle(fpt,triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2]))
				{
					_outScatterData._pointsOnFace[ii].append(_outScatterData._u.length());
					
					_outScatterData._u.append(storeUV[0]);
					_outScatterData._v.append(storeUV[1]);
					featherTools::getLocalPosInTriangle(fpt,triangPos[kk*3],triangPos[kk*3+1],triangPos[kk*3+2],tempPt2d);
					_outScatterData._localPos.push_back(tempPt2d);

					tempTriId.x = triangVerIds[kk*3];
					tempTriId.y = triangVerIds[kk*3+1];
					tempTriId.z = triangVerIds[kk*3+2];
					_outScatterData._triangleId.push_back(tempTriId);
					_outScatterData._pointFaceIndex.append(ii);

					outPositions.append( MFloatPoint(fpt) );
					usrUVCount++;
					break;
				}	
			}
		}
	}
	return status;
}

void scatterPoints::computeBaldTexture(MFloatPointArray &outPos)
{
	MStatus status = MS::kSuccess;
	if(_baldTex.length()!=0 && outPos.length()>0 )
	{	
		MFloatVectorArray col,alph;
		status = featherTools::sampleTexture(_baldTex,&_outScatterData._u,&_outScatterData._v,col,alph);
		if(!status) return;

		MFloatPointArray resultPositions;
		MFloatArray resultua,resultva;
		
		jcScatterPointData tempData;
		tempData._faceId=_outScatterData._faceId;

		//tempData._pointsOnFace.resize(_outScatterData._faceId.length());
		int faceLens=_outScatterData._faceId.length();
		int ptsOF=_outScatterData._pointsOnFace.size();
		tempData._pointsOnFace.resize(_outScatterData._pointsOnFace.size());
		int ulen=alph.length();
		for( int ii=0;ii<ulen;++ii)
			if( featherTools::noiseRandom(0,50,ii+21,_randSeed) < 
				featherTools::getColorHSV(MColor(col[ii].x,col[ii].y,col[ii].z),2))
			{
				resultPositions.append(outPos[ii]);

				tempData._pointsOnFace[_outScatterData._pointFaceIndex[ii]].append(tempData._u.length());
				
				tempData._localPos.push_back(_outScatterData._localPos[ii]);
				tempData._u.append(_outScatterData._u[ii]);
				tempData._v.append(_outScatterData._v[ii]);
				
				tempData._triangleId.push_back(_outScatterData._triangleId[ii]);
				tempData._pointFaceIndex.append(_outScatterData._pointFaceIndex[ii]);
			}
		
		_outScatterData.init();
		outPos.clear();

		outPos = resultPositions;
		_outScatterData = tempData;		
	}
}

MStatus scatterPoints::getFinalPosition( MFloatPointArray &outPos)
{
	MStatus status = MS::kSuccess;
	MFloatArray uary,vary;
	
	_outScatterData.init();
	outPos.clear();

	getComponentFace(_outScatterData._faceId );
	if(_scatterMode==1&& jerryC::jCell::m_xSegment>0 &&jerryC::jCell::m_zSegment>0)
		status = getUVPositionsFromMesh(outPos);
	else if(_scatterMode==0 && _ptNumPerArea>0)
		status = getAreaPositionsFromMesh(outPos);
	else if(_scatterMode==2 )
		status = getUserInPosition(outPos);
	if(!status) return MS::kFailure;

	computeBaldTexture(outPos);
	
	if(_scatterMode==1)	
		addUsrUVData(outPos);

	if(_outScatterData._localPos.size()==0)
		_outScatterData.init();

	return MS::kSuccess;
}

