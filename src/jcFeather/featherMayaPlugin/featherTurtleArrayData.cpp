#include "featherTurtleArrayData.h"

const MTypeId  featherTurtleArrayData::id(0x81038);
const MString  featherTurtleArrayData::typeName("featherTurtleArrayData");

void* featherTurtleArrayData::creator()
{
    return new featherTurtleArrayData;
}

featherTurtleArrayData::featherTurtleArrayData() :MPxData()
{
}

featherTurtleArrayData::~featherTurtleArrayData()
{}

turtleArrayData featherTurtleArrayData::value() const
{
    return m_turtleArray;
}

void featherTurtleArrayData::setValue( const turtleArrayData &newValue  )
{
	m_turtleArray.clear();
	int ts = newValue.size();
	m_turtleArray.resize(ts);
	for(int i=0;i<ts;i++)
		m_turtleArray[i] = newValue[i];
}

void featherTurtleArrayData::copy ( const MPxData& other )
{
    setValue( ( (const featherTurtleArrayData&)other).m_turtleArray );
}

MTypeId featherTurtleArrayData::typeId() const
{
     return featherTurtleArrayData::id;
}

MString featherTurtleArrayData::name() const
{ 
     return featherTurtleArrayData::typeName;
}
