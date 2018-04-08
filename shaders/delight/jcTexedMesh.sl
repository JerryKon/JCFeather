#include <jcRmUtility.h>

surface jcTexedMesh(
	float Kd = .5;
	float Ka = 1;
	float Ks = .0;
	float roughness = .1;
	color specularcolor = 1; 
	
	float  hueTolerance=10;
	string sampleTexture="";
	
	string multiplyTexture="";
	color  multiplyColor=1;
	
	color  col0=color(1,0,0);
	string tex0="";
	
	color  col1=color(0,1,0);
	string tex1="";
	
	color  col2=color(0,0,1);
	string tex2="";
	
	color  col3=color(1,1,0);
	string tex3="";
	
	color  col4=color(0,1,1);
	string tex4="";
	
	color  col5=color(1,0,1);
	string tex5="";
	
	uniform float jcshells=0;
	uniform float jcshellt=0;
	uniform float jcshellid=0;)
{
	normal Nf =  faceforward( normalize(N), I );
	vector V  = - normalize( I );
		
	color sampleTexColor=color(1,0,0);
	float sampleAlpha=1;
	if(sampleTexture!="")
		getUniformTextureCol(sampleTexture,jcshells,1-jcshellt,sampleTexColor,sampleAlpha);
		
	color multiplyTexColor=1;
	float multiplyAlpha=1;
	if(multiplyTexture!="")
		getUniformTextureCol(multiplyTexture,jcshells,1-jcshellt,multiplyTexColor,multiplyAlpha);
	
	multiplyTexColor *= multiplyColor;
	
	color texedOutColor=1;
	float texedOutAlpha=1;
	jcTexedMeshCall(
		sampleTexColor,
		multiplyTexColor,
		hueTolerance,
		
		col0,
		tex0,
		
		col1,
		tex1,
		
		col2,
		tex2,
		
		col3,
		tex3,
		
		col4,
		tex4,		
		
		col5,
		tex5,
		
		jcshells,
		1-jcshellt,
		
		texedOutColor,
		texedOutAlpha	
	);
	
	Ci = ( Cs * (Ka * ambient() + Kd * diffuse(Nf)) + 
		specularcolor * Ks * specular(Nf, V, roughness) );	
		
	
	Oi = Os * texedOutAlpha;
	Ci *= texedOutColor;
	Ci *=Oi;
}