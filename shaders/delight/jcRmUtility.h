
#ifndef __jcRmUtility_h
#define __jcRmUtility_h

float hueClosestDist(float h0;float h1;)
{
	float d0=abs(h0-h1);
	float d1=360-d0;
	if(d0<=d1) return d0;
	else return d1;
	}
	
	
void mayaColorH(color i_inRgb;output  float o_outH;)
{
	color o_outHsv = ctransform("rgb", "hsv", i_inRgb);
	o_outH = comp( o_outHsv, 0 ) * 360.0;
	}
	
void getTextureCol(
	string tex; 
	output color o_outColor; 
	output float o_outAlpha;
)
{
	extern float s;
	extern float t;
	o_outColor = color texture(tex,s,t,"lerp", 1);
	float nChannels=0;
	textureinfo(tex, "channels", nChannels);
	if( nChannels > 3 )
		o_outAlpha = float texture(tex[3],s,t,"lerp", 1);
	else
		o_outAlpha=1;
}	


void getUniformTextureCol(
	string tex; 
	uniform float jcshells;
	uniform float jcshellt;
	output color o_outColor; 
	output float o_outAlpha;
)

{
	o_outColor = color texture(tex,jcshells,jcshellt,"lerp", 1);
	float nChannels=0;
	textureinfo(tex, "channels", nChannels);
	if( nChannels > 3 )
		o_outAlpha = float texture(tex[3],jcshells,jcshellt,"lerp", 1);
	else
		o_outAlpha=1;
}

//currently, we just make the maximize texture num 5
void jcTexedMeshCall(

	/*
	should be connected to a texture, the color hue will be used to mesuare the distance
	between itself and sample0,1,2,3,4,5. The closest color will be used. 
	*/
	color sampleTexture;
	
	//mutiply this color to the closest texture color  texColor0,1,2,3,4,5.
	color baseColor;
	
	float hueToler;
	
	uniform color sample0;
	string tex0;

	uniform color sample1;
	string tex1;
	
	uniform color sample2;
	string tex2;
	
	uniform color sample3;
	string tex3;
	
	uniform color sample4;
	string tex4;
	
	uniform color sample5;
	string tex5;
	
	
	uniform float jcshells;
	uniform float jcshellt;
			
	output color o_outColor;
	output float o_outAlpha;

)
{
	float sampleTextureHue=0;
	mayaColorH(sampleTexture,sampleTextureHue);
	
	color sampleC=1;
	float sampleA=1;
	float dist=360;
	
	float tempDist=360;	
	float tempH=0;
	
	//---------------0
	if(tex0!="")
	{
		mayaColorH(sample0,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex0,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	//---------------1
	if(tex1!="")
	{
		mayaColorH(sample1,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex1,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	//---------------2
	if(tex2!="")
	{
		mayaColorH(sample2,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex2,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	//---------------3
	if(tex3!="")
	{
		mayaColorH(sample3,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex3,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	//---------------4
	if(tex4!="")
	{
		mayaColorH(sample4,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex4,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	//---------------5
	if(tex5!="")
	{
		mayaColorH(sample5,tempH);
		tempDist = hueClosestDist(sampleTextureHue,tempH);
		if(tempDist<hueToler)	
		{
			getTextureCol(tex5,sampleC,sampleA);
			o_outColor=baseColor * sampleC;
			o_outAlpha=sampleA;
			return;
		}
	}
	
	//if we did not find the sample texture,use the first one as default.
	if(tex0!="")
	{
		getTextureCol(tex0,sampleC,sampleA);
		o_outColor=baseColor * sampleC;
		o_outAlpha=sampleA;
	}
}
#endif /* __jcRmUtility_h */