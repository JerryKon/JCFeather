#ifndef _jCell_
#define _jCell_

//----------------------------------------------------cell noise
#include <vector>
#include <maya/MFloatArray.h>
#include "noise.h"

using namespace jerryC;

namespace jerryC{

	class jCell{

		public:
			jCell(){}

			~jCell(){}

			static void setRange(double ur[],double vr[]);
			static void GetValue(MFloatArray &ua,MFloatArray &va);

		public:
			static double m_uRange[2];

			static double m_vRange[2];

			static double m_xOffset;

			static double m_zOffset;

			static int m_xSegment;
			
			static int m_zSegment;

			static double m_frequency;
			
			static double m_jitter;

			static int m_seed;


	};

};

#endif