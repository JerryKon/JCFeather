#include "vertexDirectionData.h"

const MTypeId  vertexDirectionData::id(0x81037);
const MString  vertexDirectionData::typeName("vertexDirectionData");

void* vertexDirectionData::creator()
{
    return new vertexDirectionData;
}

vertexDirectionData::vertexDirectionData():MPxData()
{
}

vertexDirectionData::~vertexDirectionData()
{}

void vertexDirectionData::setValue( const verDirVector &newValue )
{
	m_verDir.clear();

	size_t newS = newValue.size();
	m_verDir.resize(newS);
	for(size_t ii=0;ii<newS;++ii)
		m_verDir[ii]= newValue[ii];
}

void vertexDirectionData::copy ( const MPxData& other )
{
    setValue( ( (const vertexDirectionData&)other).m_verDir );
}

MTypeId vertexDirectionData::typeId() const
{
     return vertexDirectionData::id;
}

MString vertexDirectionData::name() const
{ 
     return vertexDirectionData::typeName;
}
