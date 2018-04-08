#include "splineLib.h"
#include <maya/MGlobal.h>
#include <maya/MString.h>

jcSpline::jcSpline()
{
	_xValue=NULL;
	_yValue=NULL;
	_num=0;
}
jcSpline::jcSpline(int num)
{
	_xValue = new double[num];
	_yValue = new double[num];
	_num = num;
}
jcSpline::~jcSpline()
{
	if(_xValue) 
	{
		delete [] _xValue;
		_xValue=NULL;
	}
	if(_yValue) 
	{
		delete [] _yValue;
		_yValue=NULL;
	}
	_num=0;
}
void jcSpline::setNum(int num)
{
	if(_num == num && _xValue && _yValue) return;

	if(_xValue) delete [] _xValue;
	if(_yValue) delete [] _yValue;

	_xValue = new double[num];
	_yValue = new double[num];
	_num = num;
}
int jcSpline::getNum()
{
	return _num;
}
void jcSpline::setYData(int index,double y)
{
	_yValue[index]=y;
}
void jcSpline::setXData(int index,double x)
{
	_xValue[index]=x;
}
void jcSpline::setData(int index,double x,double y)
{
	_xValue[index]=x;
	_yValue[index]=y;
}
bool jcSpline::getValueAt(int ct,double t,  double* yv)//t-->[0,1]
{
	if(_num<=0) return false;

	double temp=0;
	if( ct == kBezier )
		bc_val( _num-1, t, _xValue, _yValue, &temp, yv );
	else if( ct == kBSpline)
		*yv = spline_b_val(_num,_xValue,_yValue,t);
	else if( ct == kCatmullRom && _num>=4)
		spline_overhauser_val(1,_num,_xValue,_yValue,t * _xValue[_num-1],yv);
	else if( ct == kLinear)
		spline_linear_val(_num,_xValue,_yValue,t,yv,&temp);
	else
		return false;

	return true;
}
bool jcSpline::getArrayValueAt(int ct,const vector<double> &ts,vector<double> &yvs)
{
	if(_num<=0) return false;

	double temp=0;
	int ptsize=ts.size();
	yvs.resize(ptsize);

	if( ct == kBezier )
	{
		for(int ii=0;ii<ptsize;++ii)
			bc_val( _num-1, ts[ii], _xValue, _yValue, &temp, &( yvs[ii]) );
	}
	else if( ct == kBSpline)
	{
		for(int ii=0;ii<ptsize;++ii)
			 yvs[ii] = spline_b_val(_num,_xValue,_yValue,ts[ii]);
	}
	else if( ct == kCatmullRom && _num>=4)
	{
		for(int ii=0;ii<ptsize;++ii)
			spline_overhauser_val(1,_num,_xValue,_yValue,ts[ii] * _xValue[_num-1],&( yvs[ii]));
	}
	else if( ct == kLinear)
	{
		for(int ii=0;ii<ptsize;++ii)
			spline_linear_val(_num,_xValue,_yValue,ts[ii],&( yvs[ii]),&temp);
	}
	else
		return false;

	return true;
}

jcSpline3D::jcSpline3D()
{
	_yValue1=NULL;
	_yValue2=NULL;
}

jcSpline3D::jcSpline3D(int num):jcSpline(num)
{
	_yValue1 = new double[num];
	_yValue2 = new double[num];
}

jcSpline3D::~jcSpline3D(){}

void jcSpline3D::setNum(int num)
{
	if(_num == num && _xValue && _yValue && _yValue1 && _yValue2) return;

	if(_xValue) delete [] _xValue;
	if(_yValue) delete [] _yValue;
	if(_yValue1) delete [] _yValue1;
	if(_yValue2) delete [] _yValue2;

	_xValue = new double[num];
	_yValue = new double[num];
	_yValue1 = new double[num];
	_yValue2 = new double[num];

	_num = num;
}

void jcSpline3D::setData(int index,double x,double y0,double y1,double y2)
{
	_xValue[index] = x;
	_yValue[index] = y0;
	_yValue1[index] = y1;
	_yValue2[index] = y2;
}

void jcSpline3D::setYData(int index,double y0,double y1,double y2)
{
	_yValue[index] = y0;
	_yValue1[index] = y1;
	_yValue2[index] = y2;
}

bool jcSpline3D::getValueAt(int ct,double t, double* yv0,double * yv1,double * yv2)
{

	if(_num<=0) return false;

	double temp=0;
	if( ct == kBezier )
	{
		bc_val( _num-1, t, _xValue, _yValue, &temp, yv0 );
		bc_val( _num-1, t, _xValue, _yValue1, &temp, yv1 );
		bc_val( _num-1, t, _xValue, _yValue2, &temp, yv2 );
	}
	else if( ct == kBSpline)
	{
		*yv0 = spline_b_val(_num,_xValue,_yValue,t*_xValue[_num-1]);
		*yv1 = spline_b_val(_num,_xValue,_yValue1,t*_xValue[_num-1]);
		*yv2 = spline_b_val(_num,_xValue,_yValue2,t*_xValue[_num-1]);
	}
	else if( ct == kCatmullRom && _num>=4)
	{
		spline_overhauser_val(1,_num,_xValue,_yValue,t * _xValue[_num-1],yv0);
		spline_overhauser_val(1,_num,_xValue,_yValue1,t * _xValue[_num-1],yv1);
		spline_overhauser_val(1,_num,_xValue,_yValue2,t * _xValue[_num-1],yv2);
	}
	else if( ct == kLinear)
	{
		spline_linear_val(_num,_xValue,_yValue,t*_xValue[_num-1],yv0,&temp);
		spline_linear_val(_num,_xValue,_yValue1,t*_xValue[_num-1],yv1,&temp);
		spline_linear_val(_num,_xValue,_yValue2,t*_xValue[_num-1],yv2,&temp);
	}
	else
		return false;

	return true;
}

bool jcSpline3D::getArrayValueAt(int ct,const vector<double> &ts,vector<jcPoint3D> &yvs)
{
	if(_num<=0) return false;

	double temp=0;
	int ptsize=ts.size();
	yvs.resize(ptsize);

	if( ct == kBezier )
	{
		for(int ii=0;ii<ptsize;++ii)
		{
			bc_val( _num-1, ts[ii], _xValue, _yValue, &temp,  &( yvs[ii].x) );
			bc_val( _num-1, ts[ii], _xValue, _yValue1, &temp, &( yvs[ii].y) );
			bc_val( _num-1, ts[ii], _xValue, _yValue2, &temp, &( yvs[ii].z) );
		}
	}
	else if( ct == kBSpline)
	{
		for(int ii=0;ii<ptsize;++ii)
		{
			yvs[ii].x = spline_b_val(_num,_xValue,_yValue,ts[ii]*_xValue[_num-1]);
			yvs[ii].y = spline_b_val(_num,_xValue,_yValue1,ts[ii]*_xValue[_num-1]);
			yvs[ii].z = spline_b_val(_num,_xValue,_yValue2,ts[ii]*_xValue[_num-1]);
		}
	}
	else if( ct == kCatmullRom && _num>=4)
	{
		for(int ii=0;ii<ptsize;++ii)
		{
			spline_overhauser_val(1,_num,_xValue,_yValue,ts[ii] * _xValue[_num-1], &( yvs[ii].x));
			spline_overhauser_val(1,_num,_xValue,_yValue1,ts[ii] * _xValue[_num-1],&( yvs[ii].y));
			spline_overhauser_val(1,_num,_xValue,_yValue2,ts[ii] * _xValue[_num-1],&( yvs[ii].z));
		}
	}
	else if( ct == kLinear)
	{
		for(int ii=0;ii<ptsize;++ii)
		{
			spline_linear_val(_num,_xValue,_yValue,ts[ii]*_xValue[_num-1], &( yvs[ii].x),&temp);
			spline_linear_val(_num,_xValue,_yValue1,ts[ii]*_xValue[_num-1],&( yvs[ii].y),&temp);
			spline_linear_val(_num,_xValue,_yValue2,ts[ii]*_xValue[_num-1],&( yvs[ii].z),&temp);
		}
	}
	else
		return false;

	return true;

}


jcRampBase::jcRampBase()
{
	_data.init();
	_needSort = true;
}

jcRampBase::~jcRampBase()
{}

int jcRampBase::numEntries()
{
	return _data._positions.size();
}

void jcRampBase::addEntries(jcInterpolation &interData)
{
	_data.append(interData);
	sortRamp();
}

void jcRampBase::setEntries(jcInterpolation &interData)
{	
	_data.init();
	_data.assign(interData);
	sortRamp();
}

void jcRampBase::getEntries(jcInterpolation &interData)
{	
	interData.init();
	interData.assign(_data);
}

void jcRampBase::setSort(bool needSort)
{
	_needSort=needSort;
}

void jcRampBase::sortRamp()
{
	int entryS= numEntries();
	if( entryS<=1 || !_needSort )return;

	jerryC::idDist tempIDD;
	vector<jerryC::idDist> sortDist;
	for(int ii=0;ii<entryS;++ii)
		sortDist.push_back(jerryC::idDist(ii,_data._positions[ii]));

	jerryC::idDistSort(sortDist,0,entryS-1);
	
	jcInterpolation tempJCI;
	for(vector<jerryC::idDist>::iterator iter=sortDist.begin();
		iter !=sortDist.end();
		++iter)
	{
		tempJCI._interps.push_back(_data._interps[iter->_id]);
		tempJCI._positions.push_back(_data._positions[iter->_id]);
		tempJCI._values.push_back(_data._values[iter->_id]);
	}
	_data.assign(tempJCI);
	_needSort = false;
}

void jcRampBase::getParameter(float pos, float &parm,vector<int> &indices)
{
	int numEn=numEntries();
	float start=0,end=1;

	indices.clear();
	if( pos <= _data._positions[0] )
	{
		parm=0;
		indices.push_back(0);
	}
	else if( pos >= _data._positions[numEn-1] )
	{
		parm=1;
		indices.push_back(numEn-1);	
	}
	else
	{
		for(int ii=0;ii<(numEn-1);++ii)
		{
			if(pos>=_data._positions[ii] && pos<_data._positions[ii+1])
			{
				indices.clear();
				if(_data._interps[ii] == jcRampBase::kSpline)
				{
					indices.push_back( jerryC::clamp(0,numEn-1,ii-1));
					indices.push_back( ii );
					indices.push_back( ii+1);
					indices.push_back( jerryC::clamp(0,numEn-1,ii+2));
					start = _data._positions[ indices[1] ];
					end   = _data._positions[ indices[2] ];
				}
				else
				{
					indices.push_back( ii);
					indices.push_back( jerryC::clamp(0,numEn-1,ii+1));
					start = _data._positions[ indices[0] ];
					end   = _data._positions[ indices[1] ];
				}
				parm = ( pos - start) / (  end - start  );
				break;
			}
		}
	}
}

bool jcRampBase::getValueAtPosition(float position, float &value)
{
	int numEn=numEntries();
	if( numEn<=0 ) return false;

	float parmeter=0;
	vector<int> start_end_index;
	getParameter(position, parmeter,start_end_index);


	int indiceSize=start_end_index.size();
	if( indiceSize == 1 )  //the position is before the frist point or after the last point
		value=_data._values[start_end_index[0]];
	else if( indiceSize == 2 ) //if the position is in kNone kLinear kSmooth mode
	{
		if(_data._interps[start_end_index[0]] == jcRampBase::kNone) 
			value = _data._values[ start_end_index[0] ];
		else if(_data._interps[start_end_index[0]] == jcRampBase::kLinear ) 
			value = jerryC::LinearInterpF( _data._values[ start_end_index[0] ] , _data._values[ start_end_index[1] ] , parmeter );
		else if(_data._interps[start_end_index[0]] == jcRampBase::kSmooth)
			value = jerryC::MayaSmoothInterpF( _data._values[ start_end_index[0] ] , _data._values[ start_end_index[1] ] , parmeter );
		return false;
	}
	else if( indiceSize == 4 )//the position is in kSpline mode,so it needs 4 values to interpolate
	{
		value = jerryC::CatmullInterpF( _data._values[start_end_index[0]],
										_data._values[start_end_index[1]],
										_data._values[start_end_index[2]],
										_data._values[start_end_index[3]],
										parmeter);
	}
	else
		return false;

	return true;
}

void jcRampBase::clear()
{
	_data.init();
}

jcRampBase jcRampBase::operator= ( const jcRampBase &other)
{
	this->_data.assign(other._data);
	this->_needSort = other._needSort;
	this->sortRamp();
	return *this;
}
