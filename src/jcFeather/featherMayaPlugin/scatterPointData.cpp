#include "scatterPointData.h"

const MTypeId  scatterPointData::id(0x81042);
const MString  scatterPointData::typeName("scatterPointData");

void* scatterPointData::creator()
{
    return new scatterPointData;
}

scatterPointData::scatterPointData():MPxData()
{
}

scatterPointData::~scatterPointData()
{}

void scatterPointData::setValue( const jcScatterPointData &newValue )
{
	m_PointData.init();
	m_PointData = newValue;
}

void scatterPointData::copy ( const MPxData& other )
{
    setValue( ( (const scatterPointData&)other).m_PointData );
}

MTypeId scatterPointData::typeId() const
{
     return scatterPointData::id;
}

MString scatterPointData::name() const
{ 
     return scatterPointData::typeName;
}
