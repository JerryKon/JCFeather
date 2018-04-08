#ifndef _glDrawFeather_
#define _glDrawFeather_

#include <maya/MColor.h>
#include <vector>
#include <GL/gl.h>
#include "singleFeather.h"

namespace jerryC{

inline void  drawFeatherWithColor( const std::vector< singleFeather > &feathers ,const MColor &col)
{
glColor3f(col.r,col.g,col.b);
	for(unsigned int jj=0; jj<feathers.size();++jj)
	{
		glBegin(GL_LINE_STRIP);
		for(unsigned int ii=0;ii<feathers[jj].rachis.length();++ii)
				glVertex3f(feathers[jj].rachis[ii].x,feathers[jj].rachis[ii].y,feathers[jj].rachis[ii].z);
		glEnd();

		unsigned int barbuleNum=0;
		//draw left
		for(unsigned int ii=0;ii<feathers[jj].leftBarbules.size();++ii)
		{
			barbuleNum = feathers[jj].leftBarbules[ii].length();
			glBegin(GL_LINE_STRIP);

			for(unsigned int kk=0;kk<barbuleNum;++kk)
				glVertex3f(feathers[jj].leftBarbules[ii][kk].x,feathers[jj].leftBarbules[ii][kk].y,feathers[jj].leftBarbules[ii][kk].z);
			glEnd();
		}

		//draw right
		for(unsigned int ii=0;ii<feathers[jj].rightBarbules.size();++ii)
		{
			barbuleNum = feathers[jj].rightBarbules[ii].length();
			glBegin(GL_LINE_STRIP);

			for(unsigned int kk=0;kk<feathers[jj].rightBarbules[ii].length();++kk)
				glVertex3f(feathers[jj].rightBarbules[ii][kk].x,feathers[jj].rightBarbules[ii][kk].y,feathers[jj].rightBarbules[ii][kk].z);
			glEnd();
		}
	}
}
inline void  drawFeather(const std::vector< singleFeather > &feathers ,
						 const float &ch,
						 const MColorArray &fourColor)
{
	float r,g,b;
	unsigned int tempIndexSize=0;
	for(unsigned int jj=0; jj<feathers.size();++jj)
	{
		glBegin(GL_LINE_STRIP);
		tempIndexSize = feathers[jj].rachis.length();
		for(unsigned int ii=0;ii<tempIndexSize;++ii)
			{
				r = LinearInterpF(fourColor[0].r,fourColor[1].r,(float)ii/(float)tempIndexSize);
				g = LinearInterpF(fourColor[0].g,fourColor[1].g,(float)ii/(float)tempIndexSize);
				b = LinearInterpF(fourColor[0].b,fourColor[1].b,(float)ii/(float)tempIndexSize);
				glColor3f(r,g,b);
				glVertex3f(feathers[jj].rachis[ii].x,feathers[jj].rachis[ii].y,feathers[jj].rachis[ii].z);
			}
		glEnd();


		//draw left
		for(unsigned int ii=0;ii<feathers[jj].leftBarbules.size();++ii)
		{
			tempIndexSize = feathers[jj].leftBarbules[ii].length();
			glBegin(GL_LINE_STRIP);
			for(unsigned int kk=0;kk<tempIndexSize;++kk)
			{
				r = LinearInterpF(fourColor[2].r,fourColor[3].r,(float)kk/(float)tempIndexSize);
				g = LinearInterpF(fourColor[2].g,fourColor[3].g,(float)kk/(float)tempIndexSize);
				b = LinearInterpF(fourColor[2].b,fourColor[3].b,(float)kk/(float)tempIndexSize);

				glColor3f( r*ch*feathers[jj].mainColor.r,g*ch*feathers[jj].mainColor.g,b*ch*feathers[jj].mainColor.b);
				glVertex3f(feathers[jj].leftBarbules[ii][kk].x,feathers[jj].leftBarbules[ii][kk].y,feathers[jj].leftBarbules[ii][kk].z);

			}
			glEnd();
		}

		//draw right
		for(unsigned int ii=0;ii<feathers[jj].rightBarbules.size();++ii)
		{
			tempIndexSize = feathers[jj].rightBarbules[ii].length();
			glBegin(GL_LINE_STRIP);

			for(unsigned int kk=0;kk<tempIndexSize;++kk)
			{
				r = LinearInterpF(fourColor[2].r,fourColor[3].r,(float)kk/(float)tempIndexSize);
				g = LinearInterpF(fourColor[2].g,fourColor[3].g,(float)kk/(float)tempIndexSize);
				b = LinearInterpF(fourColor[2].b,fourColor[3].b,(float)kk/(float)tempIndexSize);

				glColor3f( r*ch*feathers[jj].mainColor.r,g*ch*feathers[jj].mainColor.g,b*ch*feathers[jj].mainColor.b);
				glVertex3f(feathers[jj].rightBarbules[ii][kk].x,feathers[jj].rightBarbules[ii][kk].y,feathers[jj].rightBarbules[ii][kk].z);
			}
			glEnd();
		}
	}
}
inline void  drawFeatherProxyBB(const std::vector< singleFeather > &feathers ,const MColor &col)
{
	glColor3f(col.r,col.g,col.b);
	MFloatPoint minPt,maxPt;
	for(unsigned int jj=0; jj<feathers.size();++jj)
	{
		minPt = (MFloatPoint)feathers[jj].proxyBBox.min();
		maxPt = (MFloatPoint)feathers[jj].proxyBBox.max();

		glBegin(GL_LINE_STRIP);//画出bb的底平面的线条
		glVertex3f(minPt.x,minPt.y,maxPt.z);
		glVertex3f(minPt.x,minPt.y,minPt.z);
		glVertex3f(maxPt.x,minPt.y,minPt.z);
		glVertex3f(maxPt.x,minPt.y,maxPt.z);
		glVertex3f(minPt.x,minPt.y,maxPt.z);
		glEnd();

		glBegin(GL_LINE_STRIP);//画出bb的高平面的线条
		glVertex3f(minPt.x,maxPt.y,maxPt.z);
		glVertex3f(minPt.x,maxPt.y,minPt.z);
		glVertex3f(maxPt.x,maxPt.y,minPt.z);
		glVertex3f(maxPt.x,maxPt.y,maxPt.z);
		glVertex3f(minPt.x,maxPt.y,maxPt.z);
		glEnd();

		glBegin(GL_LINES);//画出链接低平面和搞平面的四条线
		glVertex3f(minPt.x,minPt.y,maxPt.z);
		glVertex3f(minPt.x,maxPt.y,maxPt.z);

		glVertex3f(minPt.x,minPt.y,minPt.z);
		glVertex3f(minPt.x,maxPt.y,minPt.z);

		glVertex3f(maxPt.x,minPt.y,minPt.z);
		glVertex3f(maxPt.x,maxPt.y,minPt.z);

		glVertex3f(maxPt.x,minPt.y,maxPt.z);
		glVertex3f(maxPt.x,maxPt.y,maxPt.z);
		glEnd();
	}
}
};

#endif
