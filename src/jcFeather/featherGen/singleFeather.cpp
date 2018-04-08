#include "singleFeather.h"
//--------------------
polyObj::polyObj(){init();};
polyObj::~polyObj(){init();};
void polyObj::init()
{
	faceCounts.clear();
	faceConnects.clear();
	pa.clear();
	ua.clear();
	va.clear();
	uvids.clear();
}
void polyObj::initWith(const polyObj &other)
{
	init();
	faceCounts  = other.faceCounts;
	faceConnects = other.faceConnects;
	pa  = other.pa;
	ua  = other.ua;
	va  = other.va;
	uvids  = other.uvids;
}
bool polyObj::initWithMObject(const MObject &meshData, const MString *uvset)
{
	if(!meshData.hasFn(MFn::kMesh)) return false;
	MStatus status=MS::kSuccess;
	MFnMesh meshFn;
	status = meshFn.setObject(meshData);
	status = meshFn.getPoints(pa);
	MString uvSetNew;
	if(!uvset)
		uvSetNew = meshFn.currentUVSetName();
	else
		uvSetNew=*uvset;

	status = meshFn.getUVs(ua,va,&uvSetNew);
	MIntArray uvC;
	status = meshFn.getAssignedUVs(uvC,uvids,&uvSetNew);
	status = meshFn.getVertices(faceCounts,faceConnects);
	if(status==MS::kSuccess)
		return true;
	else
		return false;
}
bool polyObj::convertToDagMObject(MObject &meshData,const MString *uvset)
{
	MFnMesh meshFn;
	MStatus status=MS::kSuccess;

	meshData = meshFn.create(pa.length(),
				faceCounts.length(),
				pa,
				faceCounts,
				faceConnects,
				MObject::kNullObj,
				&status);
	MString uvSetNew;
	if(!uvset)
		uvSetNew = meshFn.currentUVSetName();
	else
		uvSetNew=*uvset;
	if(status==MS::kSuccess)
	{
		meshFn.clearUVs(&uvSetNew);
		meshFn.setUVs( ua, va );
		meshFn.assignUVs( faceCounts, uvids,&uvSetNew);
		return true;
	}
	else 
		return false;
}
bool polyObj::convertToMObject(MObject &meshData,const MString *uvset)
{
	MFnMesh meshFn;
	MStatus status=MS::kSuccess;
	MFnMeshData meshDataFn;
	meshData = meshDataFn.create();
	meshFn.create(pa.length(),
				faceCounts.length(),
				pa,
				faceCounts,
				faceConnects,
				meshData,
				&status);
	CHECK_MSTATUS(status);
	MString uvSetNew;
	if(!uvset)
		uvSetNew = meshFn.currentUVSetName();
	else
		uvSetNew=*uvset;
	if(status==MS::kSuccess)
	{
		meshFn.clearUVs(&uvSetNew);
		meshFn.setUVs( ua, va );
		meshFn.assignUVs( faceCounts, uvids,&uvSetNew);
		meshFn.updateSurface();
		return true;
	}
	else 
		return false;
}
void polyObj::appendMesh(const polyObj &mesh)
{
	int isInParallel = omp_in_parallel();
	#pragma omp parallel if( !isInParallel && jcFea_UseMultiThread && jcFea_MaxThreads>=2) num_threads(2)
    {
		#pragma omp sections
		{
			#pragma omp section
            {
				for(unsigned int i=0;i<mesh.faceCounts.length();++i)
					faceCounts.append(mesh.faceCounts[i]);
				for(unsigned int i=0;i<mesh.faceConnects.length();++i)
					faceConnects.append(pa.length() + mesh.faceConnects[i]);
				for(unsigned int i=0;i<mesh.pa.length();++i)
					pa.append(mesh.pa[i]);
			}

			#pragma omp section
            {
				for(unsigned int i=0;i<mesh.uvids.length();++i)
					uvids.append( ua.length()+ mesh.uvids[i]);
				for(unsigned int i=0;i<mesh.ua.length();++i)
					ua.append(mesh.ua[i]);
				for(unsigned int i=0;i<mesh.va.length();++i)
					va.append(mesh.va[i]);
			}
		}
	}
}
void polyObj::transform(const MFloatMatrix &matrix)
{
	for(int ii=0;ii<pa.length();++ii)
		pa[ii] = pa[ii]*matrix;
}
singleFeather::singleFeather(){clear();}
singleFeather::~singleFeather(){clear();}
void singleFeather::clear()
{
	mainColor = MColor(1,1,1);
	colorTexOverride="";
	randSeed = 0;
	rachisLength=0;
	rachis.clear();
	rachisWidth.clear();
	rachisNormal.clear();
	surfaceUV[0]=surfaceUV[1]=0;

	rachisCylinder.init();	
	barbuleFace.init();

	leftBarbules.clear();
	rightBarbules.clear();
	leftBarbuleRachisPos.clear();
	rightBarbuleRachisPos.clear();
	barbuleWidth.clear();
	leftBarbuleWidth.clear();
	rightBarbuleWidth.clear();
	leftBarbuleLenPer.clear();
	rightBarbuleLenPer.clear();
	leftBarbuleNormal.clear();
	rightBarbuleNormal.clear();
	proxyBBox.clear();
	rachisMeshBBox.clear();
}