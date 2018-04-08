void maya_hsvToRgb( color i_inHsv;output  color o_outRgb;)
{
	float H = comp( i_inHsv, 0 ) / 360.0;
	color tmp = color( H, comp(i_inHsv, 1), comp(i_inHsv, 2) );	
	o_outRgb = ctransform("hsv", "rgb", tmp);
}


void maya_rgbToHsv( color i_inRgb;output  color o_outHsv;)
{
	o_outHsv = ctransform("rgb", "hsv", i_inRgb);
	
	float H = comp( o_outHsv, 0 ) * 360.0;
	setcomp( o_outHsv, 0, H );
}

/*可以输出FeatherID*/
surface jcFeather_pass (float jcFeaAmb = .5 ,
						 jcFeadiff = 0.6, 
						 jcFeaspec = .8, 
						 jcFeagloss = .07,
						 jcFeaselfshad=1,
						 jcFeaUVProjectScale=1,
						 jcFeaBaseOpacity=1.0,
						 jcFeaFadeOpacity=1.0,
						 jcFeaHueVar = 0,
						 jcFeaSatVar = 0,
						 jcFeaValVar = 0,
						 jcFeaVarFreq = 100,
						 jcFeaRandSeed = 0;
						 
				   color jcFeaspecColor=1,
				   		 jcFeaRootC=0.7,
				   		 jcFeaTipC=0.4,
				   		 jcFeaRachisRootC=0.7,
				   		 jcFeaRachisTipC=0.4;
				   uniform float jcFeaBarU=0,jcFeaSide=0,jcFeaBarLenPer=1;
				   string jcFeaColorTex="";
				   output varying color DiffuseColor=0,DiffuseDirect=0,SpecularDirect=0,SpecularColor=0,FeatherID=0,DiffuseDirectShadow=0,SpecularDirectShadow=0;)
 { 
 vector T = normalize (dPdv); /* tangent along length of hair */
 vector V = -normalize(I); /* V is the view vector */
 float df2 = 0,diffterm=0;
 color Cspec = 0, Cdiff = 0; /* collect specular & diffuse light */
 color mixed=1;
 float rawspec=0;
 color Cl2=0,Cl3=0;
 float  vt=0;
 float	tl=0;
 float nondiff=0, nonspec=0;
 SpecularColor=SpecularDirectShadow=DiffuseDirectShadow=DiffuseColor=0;
 color Cl_noshd=0;
 /* Looping over lights, per apadoca paper, the math's a bit different */

 illuminance (P) 
   {
        df2=(T.normalize(L));  
        df2*=df2;
        df2=1.0-df2;
        if (df2<0) df2=0;
	    if (df2>0)df2=sqrt(df2);    
   
	    if( 0 == lightsource("_cl_noshadow", Cl_noshd) )
		  	Cl_noshd = Cl;
	    Cl2 = Cl_noshd*jcFeaselfshad+(1-jcFeaselfshad); /* limits the gamut of shadowing */
	
	    Cl3 = (Cl_noshd -Cl)*jcFeaselfshad+(1-jcFeaselfshad);
	   	  		
		lightsource("__nondiffuse", nondiff);
	    if (nondiff < 1)
	  	{
	       diffterm=df2; /* diffuse */
		   if (diffterm<0) diffterm=0;
	
	       diffterm=(1.0-jcFeadiff)+diffterm*jcFeadiff; /* limits gamut of diffuse term */
	       
	       Cdiff += Cl2*diffterm;  /*diffuse  */      
	       DiffuseDirectShadow  += Cl3*diffterm;   
	   }

	   if( 0 == lightsource("__nonspecular", nonspec) )
		    nonspec = 0;
	   if( nonspec < 1 )
		{
		   float sq2;
	   	   vt =  V.T ;
		   sq2=1.0-vt*vt;
		   if (sq2<0) sq2=0;
		   if (sq2>0)
		   sq2=sqrt(sq2);
	       rawspec = df2* sqrt( 1.0- vt * vt ) -  (normalize(L). T ) * vt; /* raw specular */
	   	    if (rawspec<0) rawspec=0;	
	   	       
	       Cspec += Cl2*pow( rawspec, 1.0 / ( 3.0 * ( .101 - jcFeagloss ) ) )*.5;    /*specular exponent  */
	       SpecularDirectShadow += Cl3*pow( rawspec, 1.0 / ( 3.0 * ( .101 - jcFeagloss ) ) )*.5; 	
		}
  }
  
  
 color texColor= color(1,1,1); 
 float aplha=1;
 float feaBarU=jcFeaBarU,feaBarV=0;
 uniform float nChannels=3; 
 if(jcFeaSide==0) /*left barbule*/
 	feaBarV = 0.5 - (v*jcFeaBarLenPer*jcFeaUVProjectScale)/2.0;
 else if(jcFeaSide==1)/*right barbule*/
 	feaBarV = 0.5 + (v*jcFeaBarLenPer*jcFeaUVProjectScale)/2.0;
 	
 /*get the color of barbule texture*/ 
 if(jcFeaColorTex!="")
 {  
	 if(jcFeaSide!=-1)	
	 {
			 //如果曲线是barbule则获取贴图的对应像素,分左右barbule
		 	 texColor = color texture(jcFeaColorTex,feaBarV,1-feaBarU,"lerp", 1);
			 textureinfo(jcFeaColorTex, "channels", nChannels);
			 if( nChannels > 3 )
			    aplha = float texture(jcFeaColorTex[3],feaBarV,1-feaBarU,"lerp", 1);
		 
	 }
 }
 /*get the color of barbule texture*/  	
 Oi=Os*aplha*mix(jcFeaBaseOpacity,jcFeaFadeOpacity,v);
 mixed =Cs;
  
 if(jcFeaSide!=-1) 	 
 {
 	 mixed *= mix( jcFeaRootC ,jcFeaTipC ,v );
 	 mixed *= texColor;
 	  	 
 	 uniform float tempValue=0;
 	 uniform color randColor=0;
 	 
 	 //add rand hue
 	 if(jcFeaHueVar>0)
 	 {
	 	 tempValue = cellnoise(jcFeaBarU*jcFeaVarFreq+jcFeaRandSeed,jcFeaSide*jcFeaVarFreq+jcFeaRandSeed)*2.0-1.0;//noise映射到[-1,1],最终映射到[-jcFeaHueVar,jcFeaHueVar]的一个范围	 	 	 
	 	 tempValue *= jcFeaHueVar;
	 	 setcomp(randColor,0,tempValue);
	 }
 	 
 	 //add rand sat
 	 if(jcFeaSatVar>0)
 	 {
	 	 //加整数是为了增加随机性，使其与hue的随机值不一样，[-jcFeaSatVar,jcFeaSatVar]
	 	 tempValue =  cellnoise(jcFeaBarU*jcFeaVarFreq+jcFeaRandSeed+15461,jcFeaSide*jcFeaVarFreq+jcFeaRandSeed+87545)*2.0-1.0;
	 	 tempValue *= jcFeaSatVar;
	 	 setcomp(randColor,1,tempValue);
	 }
 	 
 	 //add rand value  	 
 	 if(jcFeaValVar>0)
 	 {
	 	 //加整数是为了增加随机性，使其与hue和sat的随机值不一样，[-jcFeaValVar,jcFeaValVar]
	 	 tempValue =  cellnoise(jcFeaBarU*jcFeaVarFreq+jcFeaRandSeed+65421,jcFeaSide*jcFeaVarFreq+jcFeaRandSeed+98754)*2.0-1.0;
	 	 tempValue *= jcFeaValVar;
	 	 setcomp(randColor,2,tempValue);
	 }
 	  	
	 //将rgb转换成hsv 并使用随机值
 	 color hsvColor=0; 	 	 
 	 maya_rgbToHsv(mixed,hsvColor);
 	 setcomp( hsvColor,0,mod( comp(randColor, 0) + comp(hsvColor, 0),360.0 ) );
 	 setcomp( hsvColor,1,clamp( comp(randColor, 1) + comp(hsvColor, 1),0,1 ) );
 	 setcomp( hsvColor,2,clamp( comp(randColor, 2) + comp(hsvColor, 2),0,1 ) ); 
 	 //将hsv转换成rgb 	 	 
 	 maya_hsvToRgb( hsvColor,mixed );
 	 	
 }
 else
 	mixed *= mix( jcFeaRachisRootC,jcFeaRachisTipC ,v );
 
 DiffuseColor        = mixed ;
 DiffuseDirect       = (Oi *Cdiff*DiffuseColor);
 DiffuseDirectShadow *= (Oi*DiffuseColor) ;
 SpecularColor       = jcFeaspecColor;
 SpecularDirect      = Cspec * Oi*jcFeaspecColor;
 SpecularDirectShadow*= (Oi*jcFeaspecColor);

 FeatherID = Oi;
 Ci = DiffuseDirect -DiffuseDirectShadow+ SpecularDirect -SpecularDirectShadow; /*sum terms and premult color x opac */
 }