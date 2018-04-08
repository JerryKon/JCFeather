#include "featherTurtleData.h"

const MTypeId  featherTurtleData::id(0x81036);
const MString  featherTurtleData::typeName("featherTurtleData");

void* featherTurtleData::creator()
{
    return new featherTurtleData;
}

featherTurtleData::featherTurtleData() :MPxData()
{
}

featherTurtleData::~featherTurtleData()
{}

turtleDataInfo featherTurtleData::value() const
{
    return m_turtleValue;
}

void featherTurtleData::setValue( const turtleDataInfo &newValue  )
{
	m_turtleValue.clear();
    m_turtleValue._pos = newValue._pos;
    m_turtleValue._nor = newValue._nor;
    m_turtleValue._leftPt = newValue._leftPt;
    m_turtleValue._rightPt = newValue._rightPt;
	m_turtleValue._exMeshFea= newValue._exMeshFea;
	m_turtleValue._featherUV[0] = newValue._featherUV[0];
	m_turtleValue._featherUV[1] = newValue._featherUV[1];

}

void featherTurtleData::copy ( const MPxData& other )
{
    setValue( ( (const featherTurtleData&)other).m_turtleValue );
}

MTypeId featherTurtleData::typeId() const
{
     return featherTurtleData::id;
}

MString featherTurtleData::name() const
{ 
     return featherTurtleData::typeName;
}

