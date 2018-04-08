#ifndef _scatterPointData_
#define _scatterPointData_

#include <maya/MPxData.h> 
#include <maya/MArgList.h>
#include <vector>
#include "scatterPoints.h"

class scatterPointData : public MPxData
{
	public:
							 scatterPointData();
		virtual              ~scatterPointData();

		virtual void            copy( const MPxData& );
		MTypeId                 typeId() const; 
		MString                 name() const;

		void                    setValue( const jcScatterPointData &newValue );

		static  const MString    typeName;
		static  const MTypeId    id;
		static void*       creator();

	public:
		jcScatterPointData m_PointData;
};

#endif