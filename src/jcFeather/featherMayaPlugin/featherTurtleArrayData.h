#ifndef _featherTurtleArrayData_
#define _featherTurtleArrayData_

#include <maya/MPxData.h> 
#include <maya/MArgList.h> 
#include "featherInfo.h"

class featherTurtleArrayData : public MPxData
{
	public:
							 featherTurtleArrayData();
		virtual              ~featherTurtleArrayData();

		virtual void            copy( const MPxData& );
		MTypeId                 typeId() const; 
		MString                 name() const;

		turtleArrayData         value() const;
		void                    setValue( const turtleArrayData &newValue );

		static  const MString    typeName;
		static  const MTypeId    id;
		static void*       creator();

	public:
		turtleArrayData   m_turtleArray; 
};

#endif