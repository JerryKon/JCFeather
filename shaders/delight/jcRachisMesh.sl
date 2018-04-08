
surface jcRachisMesh(float Ks=.9, Kd=.8, Ka=0, roughness=.1; 
					color  specularcolor=1 ,
					       jcFeaRachisRootC=0.7,
				   		   jcFeaRachisTipC=0.4;)
{
    normal Nf;
    vector V;

    Nf = faceforward( normalize(N), I );
    V = -normalize(I);
    
	color mixed=1;
	mixed = mix( jcFeaRachisRootC, jcFeaRachisTipC ,t );
    Oi = Os;
    Ci = Os * ( Cs *mixed* (Ka*ambient() + Kd*diffuse(Nf)) +
	 	specularcolor * Ks * specular(Nf,V,roughness) );
}