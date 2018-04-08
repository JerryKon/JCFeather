import maya.OpenMaya as mom

def myRotateVector(degree,f0,f1,f2,r0,r1,r2):
	quat = mom.MQuaternion(degree, mom.MVector(f0,f1,f2))
	newVector = mom.MVector(r0,r1,r2).rotateBy(quat)
	result=''
	result+=str(newVector.x)
	result+=' '
	result+=str(newVector.y)
	result+=' '
	result+=str(newVector.z)
	return result
	
def nameToNode(name):
    selList = mom.MSelectionList()
    selList.add(name);
    obj = mom.MObject()
    selList.getDependNode(0,obj)
    return obj
    
def setNewDirection(nodeName,f0,f1,f2,r0,r1,r2):
    nodeTransform = mom.MFnTransform(nameToNode(nodeName))
    transMatrix=nodeTransform.transformation()
    
    fowardDir=mom.MVector(f0,f1,f2)
    rightDir=mom.MVector(r0,r1,r2)
    
    quat1=mom.MVector(1,0,0).rotateTo(fowardDir)
    transMatrix.addRotationQuaternion(quat1.x,quat1.y,quat1.z,quat1.w,mom.MSpace.kTransform)
    newright=mom.MVector(0,0,1).rotateBy(quat1); 
    quat2 = newright.rotateTo(rightDir)
    transMatrix.addRotationQuaternion(quat2.x,quat2.y,quat2.z,quat2.w,mom.MSpace.kTransform)   
    nodeTransform.set(transMatrix) 
  
def getNearestPointOnCurve(curveName,p1,p2,p3):
    curveFn = mom.MFnNurbsCurve(nameToNode(curveName))
    msu = mom.MScriptUtil()
    uparam = msu.asDoublePtr()
    msu.setDouble ( uparam, 1.0 )
    oldpt = mom.MPoint(p1,p2,p3)
    pt = curveFn.closestPoint( oldpt,uparam)
    up = msu.getDouble(uparam)
    tangent = curveFn.tangent(up)
    
    result=''
    result+=str(pt.x)
    result+=' '
    result+=str(pt.y) 
    result+=' '
    result+=str(pt.z)  
    result+=' '      
    result+=str(tangent.x)
    result+=' '
    result+=str(tangent.y) 
    result+=' '
    result+=str(tangent.z) 
    result+=' '
    result+=str(up)           
    return result

def getClosestUVOnMeshFromPos(pos,meshNode,uValAry,vValAry):
	parPosLen=pos.length()
	if parPosLen==0:
		return 0

	meshFn=mom.MFnMesh(meshNode)
	uvSet=meshFn.currentUVSetName()

	pArray = [0,0]
	x1 = mom.MScriptUtil()
	x1.createFromList( pArray, 2 )
	uv = x1.asFloat2Ptr()
	uValAry.clear()
	vValAry.clear()
	for i in range(0,parPosLen):
		currentPt=mom.MPoint(pos[i])
		meshFn.getUVAtPoint(currentPt,uv,mom.MSpace.kWorld,uvSet)
		uValAry.append( mom.MScriptUtil.getFloat2ArrayItem( uv, 0, 0 ))
		vValAry.append( mom.MScriptUtil.getFloat2ArrayItem( uv, 0, 1 ))
	return 1

def getClosestUVOnSurfaceFromPos(pos,surfaceNode,uValAry,vValAry):
	parPosLen=pos.length()
	if parPosLen==0:
		return 0

	surFn=mom.MFnNurbsSurface(surfaceNode)
	dbs0 = mom.MScriptUtil(0.0)
	db0_pt=dbs0.asDoublePtr()
	dbs1 = mom.MScriptUtil(0.0)
	db1_pt=dbs1.asDoublePtr()
	
	for i in range(0,parPosLen):
		currentPt=mom.MPoint(pos[i])
		surFn.closestPoint(currentPt,db0_pt,db1_pt,0,0.000001,mom.MSpace.kWorld)
		uValAry.append( dbs0.getDouble(db0_pt))
		vValAry.append( dbs1.getDouble(db1_pt))
	return 1
	    
def getClosestData(uAry,vAry,geo,pos):
    if geo.node().hasFn(mom.MFn.kMesh) :
        getClosestUVOnMeshFromPos(pos,geo,uAry,vAry)
    elif geo.hasFn(mom.MFn.kNurbsSurface) :
    	getClosestUVOnSurfaceFromPos(pos,geo,uAry,vAry)
    else:
    	mom.MGlobal.displayError('JCFeather : Only nurbs surface or mesh geometry are supported.')	
    
##select a geometry(nurbsSurface or mesh) and a jcFeatherComponentArray node to update the uv array attribute         
def updateJCFComAryUV():
    selList=mom.MSelectionList()
    mom.MGlobal.getActiveSelectionList(selList)
    if selList.length()!=2:
        mom.MGlobal.displayError('JCFeather : Select a geometry and a jcFeatherComponentArray node.')
    geoObj=mom.MDagPath()
    comAry=mom.MDagPath()
    selList.getDagPath(0,geoObj)
    selList.getDagPath(1,comAry)
    
    geoObj.extendToShape()
    comAry.extendToShape()
    
    nodeFn = mom.MFnDependencyNode()
    nodeFn.setObject(comAry.node())
    
    posPlug = nodeFn.findPlug('outRootPositionArray')
    vectFnData = mom.MFnVectorArrayData( posPlug.asMObject())
    vecAry = vectFnData.array()
    uArrayData=mom.MDoubleArray()
    vArrayData=mom.MDoubleArray()
    
    getClosestData(uArrayData,vArrayData,geoObj,vecAry)
    
    uPlug = nodeFn.findPlug('uArrayData')
    floatFnData0 = mom.MFnDoubleArrayData( uPlug.asMObject())
    floatFnData0.set(uArrayData)
    uPlug.setMObject(floatFnData0.object())
    
    vPlug = nodeFn.findPlug('vArrayData')
    floatFnData1 = mom.MFnDoubleArrayData( vPlug.asMObject())  
    floatFnData1.set(vArrayData)
    vPlug.setMObject(floatFnData1.object())
    
    mom.MGlobal.displayInfo('JCFeather : Update jcFeatherComponentArray node uvArray data '+geoObj.fullPathName()+' --> ' +comAry.fullPathName())