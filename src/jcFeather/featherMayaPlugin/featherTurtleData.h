#ifndef _featherTurtleData_
#define _featherTurtleData_

#include <maya/MPxData.h> 
#include <maya/MArgList.h> 
#include "featherInfo.h"

class featherTurtleData : public MPxData
{
	public:
							 featherTurtleData();
		virtual              ~featherTurtleData();

		virtual void            copy( const MPxData& );
		MTypeId                 typeId() const; 
		MString                 name() const;

		turtleDataInfo          value() const;
		void                    setValue( const turtleDataInfo &newValue );

		static  const MString    typeName;
		static  const MTypeId    id;
		static void*       creator();

	public:
		turtleDataInfo   m_turtleValue; 
};

#endif