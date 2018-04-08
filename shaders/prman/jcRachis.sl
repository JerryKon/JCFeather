
/*ø…“‘ ‰≥ˆFeatherID*/
surface jcRachis (float jcFeaAmb = .5 ,
						 jcFeadiff = 0.6, 
						 jcFeaspec = .8, 
						 jcFeagloss = .07,
						 jcFeaselfshad=1,
						 jcFeaTipO=1.0;
				   color jcFeaspecColor=1,
				   		 jcFeaRootC=0.7,
				   		 jcFeaTipC=0.4;
				   string jcFeaColorTex="";
				   output varying color DiffuseColor,DiffuseDirect,SpecularDirect,SpecularColor,FeatherID,DiffuseDirectShadow,SpecularDirectShadow;)
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
 
 mixed = mix( jcFeaRootC ,jcFeaTipC ,v );

 Oi = mix( 1, jcFeaTipO ,v );
 
 DiffuseColor = Cs * mixed;
 DiffuseDirect = (Oi *Cdiff*DiffuseColor);
 DiffuseDirectShadow *= (Oi*DiffuseColor) ;
 SpecularColor = jcFeaspecColor;
 SpecularDirect = (Cspec * Oi*jcFeaspecColor);
 SpecularDirectShadow *= (Oi*jcFeaspecColor);

 FeatherID = Oi;
 Ci = DiffuseDirect -DiffuseDirectShadow+ SpecularDirect -SpecularDirectShadow; /*sum terms and premult color x opac */
 }