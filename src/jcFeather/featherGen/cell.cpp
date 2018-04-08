#include "cell.h"

namespace jerryC
{
	double jCell::m_uRange[2]={0,1};
	double jCell::m_vRange[2]={0,1};
	double jCell::m_zOffset=0;
	double jCell::m_xOffset=0;

	double jCell::m_frequency=10;
	int jCell::m_xSegment=10;
	int jCell::m_zSegment=10;
	double jCell::m_jitter=0.1;
	int  jCell::m_seed=0;

	void jCell::setRange(double ur[],double vr[])
	{
			m_uRange[0]=ur[0];
			m_uRange[1]=ur[1];

			m_vRange[0]=vr[0];
			m_vRange[1]=vr[1];
	}
	void jCell::GetValue(MFloatArray &ua,MFloatArray &va)
	{
		  ua.clear();
		  va.clear();
		  double udistR = (m_uRange[1] -m_uRange[0])/(double)m_xSegment;
		  double vdistR = (m_vRange[1] -m_vRange[0])/(double)m_zSegment;
		  if(udistR<0.000001||vdistR<0.000001) return;

		  double uu=0,vv=0;
		  for(int ii=0;ii<m_xSegment;++ii)
		  	for(int jj=0;jj<m_zSegment;++jj)
		  		{
			  		uu = (double)ii*udistR+udistR/2.0 + m_uRange[0];
			  		vv = (double)jj*vdistR+vdistR/2.0 + m_vRange[0];

					uu +=	udistR*0.5 * m_jitter * GradientCoherentNoise3D(uu*m_frequency, 0, vv*m_frequency, m_seed,1 ) ;
					vv +=	vdistR*0.5 * m_jitter * GradientCoherentNoise3D(uu*m_frequency, 0, vv*m_frequency, m_seed+3052 ,1 ) ;

					if((jj+1)%2==0)
						uu += m_xOffset;
					else
						uu -= m_xOffset;

					if((ii+1)%2==0)
						vv += m_zOffset;
					else
						vv -= m_zOffset;

					ua.append((float)uu);va.append((float)vv);
				}
	}
}
