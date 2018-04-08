#ifndef _splineLib_
#define _splineLib_

#include "spline.h"
#include "mathFunctions.h"
#include "noise.h"
#include <vector>
using namespace jerryC;
using namespace std;
class jcSpline3D;

class jcSpline
{
	friend class jcSpline3D;
	public:
		jcSpline();
		jcSpline(int num);
		~jcSpline();
		void setNum(int num);
		int  getNum();
		void setData(int index,double x,double y);
		void setYData(int index,double y);
		void setXData(int index,double x);
		bool getValueAt(int ct,double t, double* yv);//t-->[0,1]
		bool getArrayValueAt(int ct,const vector<double> &ts,vector<double> &yvs);
	public:
		enum
		{
			kLinear=0,
			kBSpline=1,
			kBezier=2,
			kCatmullRom=3
		};
	private:

		double *_xValue;
		double *_yValue;
		int _num;
};

class jcSpline3D:public jcSpline
{
	public:
		jcSpline3D();
		jcSpline3D(int num);
		~jcSpline3D();
		void setNum(int num);
		void setData(int index,double x,double y0,double y1,double y2);
		void setYData(int index,double y0,double y1,double y2);
		bool getValueAt(int ct,double t, double* yv0,double * yv1,double * yv2);
		bool getArrayValueAt(int ct,const vector<double> &ts,vector<jcPoint3D> &yvs);

	private:
		double *_yValue1;
		double *_yValue2;
};

struct jcInterpolation
{
	vector<float> _positions;
	vector<float> _values;
	vector<int> _interps;

	void init()
	{
		_positions.clear();
		_values.clear();
		_interps.clear();
	}
	void assign(const jcInterpolation &jci)
	{
		init();
		_positions=jci._positions;
		_values=jci._values;
		_interps=jci._interps;
	}
	void append(jcInterpolation &interData)
	{
		for(vector<float>::iterator iter=interData._positions.begin();
			iter!=interData._positions.end();
			++iter)
				_positions.push_back(*iter);
		for(vector<float>::iterator iter=interData._values.begin();
			iter!=interData._values.end();
			++iter)
				_values.push_back(*iter);
		for(vector<int>::iterator iter=interData._interps.begin();
			iter!=interData._interps.end();
			++iter)
				_interps.push_back(*iter);

	}
};

class jcRamp;
class jcRampBase
{
	friend class jcRamp;
	private:
		void getParameter(float pos, float &parm,vector<int> &indices);
		void sortRamp();
	public:
		jcRampBase();
		~jcRampBase();
		void addEntries(jcInterpolation &interData);
		void setEntries(jcInterpolation &interData);
		void getEntries(jcInterpolation &interData);
		bool getValueAtPosition(float position, float &value);
		int numEntries();
		void setSort(bool needSort);
		void clear();

		jcRampBase operator= ( const jcRampBase &other);
	public:
		enum interPolation
		{
			//2012
			kNone=0,
			kLinear=1,
			kSmooth=2,
			kSpline=3,	
			//------2011
			//kLinear=0,
			//kNone=1,
			//kSpline=2,
			//kSmooth=3,
		};
	private:
		jcInterpolation _data;
		bool _needSort;
};

#endif
