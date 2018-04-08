#ifndef _mathFunchtions_
#define _mathFunchtions_

#include <vector>
#include <cmath>
#include <cstddef>
#include <stdlib.h>
using namespace std;
namespace jerryC{
	const float radians_per_degree=0.017453292f;
	const float pi=3.1415926f;
	////-------得到[0,max)区间的随机整数
	inline float random( const float & maxRange )
	{
		return (((float) rand() / (float) RAND_MAX) * maxRange);
	}
	inline float random( const float & minRange,const float & maxRange )
	{
		return minRange+random(1.0)*(maxRange-minRange);
	}
	inline double clamp1_(const double &a)
	{
		if(a<1) return 1;
		else return a;
	}
	inline float clamp0_1(const float & a)
	{
		if(a>1.0f) return 1.0f;
		if(a<0.0f) return 0.0f;
		return a;
	}
	inline float distSquare(const float &x1,const float &y1,const float &z1,
							const float &x2,const float &y2,const float &z2)
	  {
		  return (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2);
	  }

	//fast sqrt
	inline float sqrtf( const float& x )
	{
		union
		{
			float f;
			int i;
		} rsq; // inverse square root

		rsq.f = x;
		float xhalf = x * 0.5f;

		rsq.i = 0x5f3759df - ( rsq.i >> 1 );
		rsq.f = rsq.f * ( 1.5f - xhalf * rsq.f * rsq.f);   // 1st iteration

		return (x* rsq.f); // square root of x = x * (1/sqrt(x))
	}

	inline float pointDistance(const float &x1,const float &y1,const float &z1,
							   const float &x2,const float &y2,const float &z2)
	  {
		  return sqrt( (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2) );
	  }

	template <typename T>
	T clamp(T minn,T maxn,T valuen)
	{
			if(valuen<minn) return minn;
			if(valuen>maxn) return maxn;
			return valuen;
	}

	inline bool doubleEqual(const double &a,const double & b)
	{
		if( abs(a-b)<0.0000000001 )
			return true;
		else
			return false;
	}
	inline float CubicInterpF (float n0, float n1, float n2, float n3,
		float a)
	  {
		  float p = (n3 - n2) - (n0 - n1);
		  float q = (n0 - n1) - p;
		  float r = n2 - n0;
		  float s = n1;
		  return p * a * a * a + q * a * a + r * a + s;
	  }

	inline float CatmullInterpF (float n0, float n1, float n2, float n3,
	float a)
	{
		float p = -0.5f*n0 + 1.5f*n1 - 1.5f*n2 + 0.5f*n3;
		float q = n0 - 2.5f*n1 + 2*n2 - 0.5f*n3;
		float r = -0.5f*n0 + 0.5f*n2;
		float s = n1;
		return p * a * a * a + q * a * a + r * a + s;
	}
	inline float SmoothInterpF(float a,float b,float x)
	{
		float p = -2*x*x*x + 3*x*x;
		return ( a*(1-p) + b*p );
	}
	inline float MayaSmoothInterpF(const float &a,const float &b,const float &x)
	{
		float p = 0.5f * (cos( (x+1)*pi) + 1);
		return ( a*(1-p) + b*p );
	}

	class idDist
	{
		public:
			idDist();
			idDist(long id,float dist){_id=id; _distance=dist;}
			~idDist();
		public:
			long _id;
			float _distance;
	};

	void idDistSort(std::vector<idDist> &s, int l, int r);

	class jcFeaNearID
	{
		public:
			jcFeaNearID();
			jcFeaNearID(int newSize);
			~jcFeaNearID();
			void clear();
			long* getValue(int index);
			void setValue(unsigned int index,long newID);
			int getRealCount();
			int getSize();
		private:
			long* _id;
			int _size;
			int _realNum;
	};

	class jcString
	{
		public:
			jcString();
			~jcString();

			bool getSpace(unsigned long size);
			void clear();
		public:
			char *_data;
			unsigned long _length;
	};

	
	typedef struct 
	{
		double x;
		double y;
		double z;
	}jcPoint3D;
	typedef struct 
	{
		double x;
		double y;
	}jcPoint2D;
	typedef struct 
	{
		float x;
		float y;
		float z;
	}jcPoint3DF;

	typedef struct 
	{
		float x;
		float y;
	}jcPoint2DF;

	typedef struct 
	{
		int x;
		int y;
		int z;
	}jcTri3Int;

};
#endif
