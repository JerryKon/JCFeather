#ifndef _vertexDirectionData_
#define _vertexDirectionData_

#include <maya/MPxData.h> 
#include <maya/MArgList.h>
#include <vector>
#include "featherInfo.h"

class vertexDirectionData : public MPxData
{
	public:
							 vertexDirectionData();
		virtual              ~vertexDirectionData();

		virtual void            copy( const MPxData& );
		MTypeId                 typeId() const; 
		MString                 name() const;

		void                    setValue( const verDirVector &newValue );

		static  const MString    typeName;
		static  const MTypeId    id;
		static void*       creator();

	public:
		verDirVector m_verDir;
};

#endif