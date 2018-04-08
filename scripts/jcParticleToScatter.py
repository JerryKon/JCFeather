import maya.OpenMaya as mom
import maya.OpenMayaFX as momf
import math
import jcNodeUtility as jcnu

##distance in uv coordinate
def isDistanceSmaller(oldu,oldv,newu,newv,maxDist):
    distu=abs(oldu-newu)
    distv=abs(oldv-newv)
    dist=math.sqrt(distu*distu+distv*distv)
    if dist<maxDist:
        return 1
    else:
        return 0
        
def addNewUvToUV(oldUAry,oldVAry,newUAry,newVAry,dist):
	for j in range(0,newUAry.length()):
		exist = 0
		for i in range(0,oldUAry.length()):
			if isDistanceSmaller(oldUAry[i],oldVAry[i],newUAry[j],newVAry[j],dist) ==1:
				 exist = 1 
				 break
		if exist ==0:
			oldUAry.append(newUAry[j])
			oldVAry.append(newVAry[j])
			
###if the new vector points is far away enough, add them to the old vec        
def addNewParticleToVector(newVec,oldVec,dist):
	for i in range(0,newVec.length()):
		exist=0
		for j in range(0,oldVec.length()):
			if mom.MPoint(newVec[i].x,newVec[i].y,newVec[i].z).distanceTo(mom.MPoint(oldVec[j].x,oldVec[j].y,oldVec[j].z)) < dist :
				exist = 1
				break
		if exist ==0:
			oldVec.append(newVec[i])
			
def removeUVArrayData(oldU,oldV,newU,newV):
    newULen=newU.length()
    oldULen=oldU.length()
    resultU=mom.MDoubleArray()
    resultV=mom.MDoubleArray()
    for i in range(0,newUlen):
        for j in range(j,oldULen):
            if (abs(oldU[j]-newU[i])>0.00001) and (abs(oldV[j]-newV[i])>0.00001) :
                resultU.append(oldU[j])
                resultV.append(oldV[j])
                
##get selected particle or particle component position                
def getPartticlePos(parDag,parCom):
    parPos=mom.MVectorArray()
    parDag.extendToShape()
    parSysFn = momf.MFnParticleSystem(parDag)
    parSysFn.position(parPos)
    
    isParticle= parDag.hasFn(mom.MFn.kParticle)
    isNParticle = parDag.hasFn(mom.MFn.kNParticle)
    if (isParticle==0) and (isNParticle==0) :
        return parPos
    
    if ( parCom.isNull()==0 ) and ( parCom.apiType()==mom.MFn.kDynParticleSetComponent):
        compFn = mom.MFnSingleIndexedComponent(parCom)
        idArray=mom.MIntArray()
        compFn.getElements(idArray)
        newPos=mom.MVectorArray()
        if idArray.length()!=0:
            for i in range(0,idArray.length()):
                newPos.append(parPos[idArray[i]])
            return newPos
        else:
            return parPos
    return parPos
    
##get the closest positions on mesh of the particles
def getParticleClosestPointOnMesh(parDag,parCom,meshNode,ptArray):
    parPos = getPartticlePos(parDag,parCom)
    parPosLen=parPos.length()
    if parPosLen==0:
        return 0
    isMesh = meshNode.hasFn(mom.MFn.kMesh)
    if (isMesh==0):
        return 0
    
    meshFn=mom.MFnMesh(meshNode)
    tempPt=mom.MPoint()
    faceId=0
    
    for i in range(0,parPosLen):
        currentPt=mom.MPoint(parPos[i].x,parPos[i].y,parPos[i].z)
        meshFn.getClosestPoint(currentPt,tempPt,mom.MSpace.kWorld)
        ptArray.append(mom.MVector(tempPt.x,tempPt.y,tempPt.z))
    return 1
    
##get the closest uv values of the particles
def getParticleClosestUVOnMesh(parDag,parCom,meshNode,uValAry,vValAry):
    parPos = getPartticlePos(parDag,parCom)
    dagPath=mom.MDagPath()
    mom.MDagPath.getAPathTo(meshNode,dagPath)
    jcnu.getClosestUVOnMeshFromPos(parPos,dagPath,uValAry,vValAry)
    
##clear inPosition attribute of jcScatter
def clearJCScatterInPosition(scatterDag):
    scatterDag.extendToShape()
    dgFn=mom.MFnDependencyNode(scatterDag.node())
    posAryPlug = dgFn.findPlug('inPosition')
    
    vecDDFn = mom.MFnVectorArrayData(posAryPlug.asMObject())
    oldUArray=vecDDFn.array()
    oldUArray.clear()
    posAryPlug.setMObject(vecDDFn.object())

    ##clear user uv array attribute data
def clearJCScatterUserData(scatterDag):
    scatterDag.extendToShape()
    dgFn=mom.MFnDependencyNode(scatterDag.node())
    uAryPlug = dgFn.findPlug('userUArray')
    vAryPlug = dgFn.findPlug('userVArray')
    
    uDDFn = mom.MFnDoubleArrayData(uAryPlug.asMObject())
    vDDFn = mom.MFnDoubleArrayData(vAryPlug.asMObject()) 
    oldUArray=uDDFn.array()
    oldVArray=vDDFn.array()
    oldUArray.clear()
    oldVArray.clear()
    uAryPlug.setMObject(uDDFn.object())
    vAryPlug.setMObject(vDDFn.object())

    ##use particle position to operate jcScatter's point data
    ##operation 0 add, 1 replace, 2 remove, 3 clear,
def addParticlePosToScatter(operation,toleranceForRemove):        
    sel=mom.MSelectionList()    
    mom.MGlobal.getActiveSelectionList(sel)
    scatterDag=mom.MDagPath()

    ##clear scatter data
    if (operation==3) and (sel.length()==1):
        sel.getDagPath(0,scatterDag)
        scatterDag.extendToShape()
        dgFn=mom.MFnDependencyNode(scatterDag.node())
        sctModePlug = dgFn.findPlug('scatterMode')
        sctMd=sctModePlug.asShort()
        mom.MGlobal.displayInfo('JCFeather : Scatter operate on node '+dgFn.name())
        mom.MGlobal.displayInfo('JCFeather : Clear scatter user point array.')
        if sctMd==1 :
            clearJCScatterUserData(scatterDag)
        elif sctMd==2 :
            clearJCScatterInPosition(scatterDag)
        return
    
    if sel.length()!=2:
        mom.MGlobal.displayError('JCFeather : Select a particle and a jcScatter node.')

    particleDag=mom.MDagPath()
    particleComponent=mom.MObject()
    sel.getDagPath(0,particleDag,particleComponent)
    
    sel.getDagPath(1,scatterDag)
    scatterDag.extendToShape()
    dgFn=mom.MFnDependencyNode(scatterDag.node())
    growMeshPlug=dgFn.findPlug('inGrowMesh')
    sctModePlug = dgFn.findPlug('scatterMode')
    sctMd=sctModePlug.asShort()
    
    plugArray = mom.MPlugArray()
    growMeshPlug.connectedTo(plugArray,1,0)
    if plugArray.length()==0:
        return
    
    mom.MGlobal.displayInfo('JCFeather : Scatter operate on node '+dgFn.name())
    
    if sctMd==1 :
    	##############################################     if the jcScatter is in UV scatter mode

        mom.MGlobal.displayInfo('JCFeather : Scatter UV mode.')
        
        uAryPlug = dgFn.findPlug('userUArray')
        vAryPlug = dgFn.findPlug('userVArray')       
        uArray = mom.MDoubleArray()
        vArray = mom.MDoubleArray()
        if (getParticleClosestUVOnMesh(particleDag,particleComponent,plugArray[0].node() ,uArray,vArray)==0):
            return
        mom.MGlobal.displayInfo('JCFeather : Select particle data size '+str(uArray.length()))
        if operation==1:
            ######################################          replace
            uDDFn = mom.MFnDoubleArrayData()
            vDDFn = mom.MFnDoubleArrayData()        
            uAryPlug.setMObject(uDDFn.create(uArray))
            vAryPlug.setMObject(vDDFn.create(vArray))
            mom.MGlobal.displayInfo( 'JCFeather : Replace scatter userUVArray data.')
        elif operation==0:
            ######################################          add
            defaultVecArray=mom.MDoubleArray()
            try:
                uDDFn = mom.MFnDoubleArrayData(uAryPlug.asMObject())
            except:
                uDDFn = mom.MFnDoubleArrayData()
                uAryPlug.setMObject(uDDFn.create(defaultVecArray))
            try:
                vDDFn = mom.MFnDoubleArrayData(vAryPlug.asMObject())
            except:
                vDDFn = mom.MFnDoubleArrayData()
                vAryPlug.setMObject(vDDFn.create(defaultVecArray))
                
            oldUArray=uDDFn.array()
            oldVArray=vDDFn.array()
            oldLen =oldUArray.length()
            addNewUvToUV(oldUArray,oldVArray,uArray,vArray,toleranceForRemove)
         
            uAryPlug.setMObject(uDDFn.object())
            vAryPlug.setMObject(vDDFn.object())
            mom.MGlobal.displayInfo('JCFeather : Add scatter userUVArray data. '+str(oldLen) +' --> '+str(oldUArray.length()))
        elif operation==2:
            ######################################      remove
            try:
                uDDFn = mom.MFnDoubleArrayData(uAryPlug.asMObject())
            except:
                mom.MGlobal.displayError("JCFeather : No userUArray data to remove.")
            try:
                vDDFn = mom.MFnDoubleArrayData(vAryPlug.asMObject())
            except:
                mom.MGlobal.displayError("JCFeather : No userVArray data to remove.")
                
            oldUArray=uDDFn.array()
            oldVArray=vDDFn.array()

            newULen=uArray.length()
            oldULen=oldUArray.length()
            
            resultU=mom.MDoubleArray()
            resultV=mom.MDoubleArray()
            
            for j in range(0,oldUArray.length()):
                for i in range(0,newULen):
                    if isDistanceSmaller(oldUArray[j],oldVArray[j],uArray[i],vArray[i],toleranceForRemove)==1:
                        oldUArray.remove(j)
                        oldVArray.remove(j)
            uAryPlug.setMObject(uDDFn.object())
            vAryPlug.setMObject(vDDFn.object())
            mom.MGlobal.displayInfo('JCFeather : Remove scatter userUVArray data. '+str(oldULen)+' --> '+str(oldUArray.length()))
    elif sctMd==2:
    	##### if the jcScatter is in InPosition scatter mode

        mom.MGlobal.displayInfo('JCFeather : Scatter inPosition mode.')
        inPositionPlug = dgFn.findPlug('inPosition')
        clstPtAry = getPartticlePos(particleDag,particleComponent)

        print ('JCFeather : Select particle data size '+str(clstPtAry.length()))
        if operation==1:
            ###########################                            replace
            uDDFn = mom.MFnVectorArrayData()   
            inPositionPlug.setMObject(uDDFn.create(clstPtAry))
            mom.MGlobal.displayInfo( 'JCFeather : Replace scatter inPosition data.')
        elif operation==0:
            ############################                             add
            try:
                uDDFn = mom.MFnVectorArrayData(inPositionPlug.asMObject())
            except:
                
                uDDFn = mom.MFnVectorArrayData()
                defaultVecArray=mom.MVectorArray()
                inPositionPlug.setMObject(uDDFn.create(defaultVecArray))
                
            myOldVecAry=uDDFn.array()
            oldLen = myOldVecAry.length()
            addNewParticleToVector(clstPtAry,myOldVecAry,toleranceForRemove)    
            inPositionPlug.setMObject(uDDFn.object())
            mom.MGlobal.displayInfo('JCFeather : Add scatter inPosition data. '+str(oldLen)+' --> '+str(myOldVecAry.length()))
        elif operation==2:
            #############################                            remove
            try:
                uDDFn = mom.MFnVectorArrayData(inPositionPlug.asMObject())
            except:
                mom.MGlobal.displayError("JCFeather : No inPosition data to remove.")
            oldVArray=uDDFn.array()
            
            newULen=clstPtAry.length()
            oldULen=oldVArray.length()
            
            result=mom.MVectorArray()
            idAry=mom.MIntArray()
            for j in range(0,oldVArray.length()):
                for i in range(0,newULen):
                    dist = mom.MPoint(oldVArray[j].x,oldVArray[j].y,oldVArray[j].z).distanceTo(mom.MPoint(clstPtAry[i].x,clstPtAry[i].y,clstPtAry[i].z)) 
                    if dist<toleranceForRemove :
                        oldVArray.remove(j)
            inPositionPlug.setMObject(uDDFn.object())
            mom.MGlobal.displayInfo('JCFeather : Remove scatter inPosition data. '+str(oldULen)+' --> '+str(oldVArray.length()))