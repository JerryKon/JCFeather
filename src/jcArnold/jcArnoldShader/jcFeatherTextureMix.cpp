#include <cstring>
#include "utils.h"

AI_SHADER_NODE_EXPORT_METHODS(jcFeatherTexMixMethods);

					  
enum jcFeatherTexMixParam
{
	p_enable,
	
    p_sample_texture,
	p_multiply_color,
	p_multiply_alpha,

	p_tolerance,

	p_sample_enable0,
	p_sample_color0,
	p_tex_color0,
	p_tex_alpha0,
	
	p_sample_enable1,
	p_sample_color1,
	p_tex_color1,
	p_tex_alpha1,
	
	p_sample_enable2,
	p_sample_color2,
	p_tex_color2,
	p_tex_alpha2,
	
	p_sample_enable3,
	p_sample_color3,
	p_tex_color3,
	p_tex_alpha3,
	
	p_sample_enable4,
	p_sample_color4,
	p_tex_color4,
	p_tex_alpha4,
	
	p_sample_enable5,
	p_sample_color5,
	p_tex_color5,
	p_tex_alpha5,
	
    p_default_color,
	p_default_alpha
};

struct jcFeatherFeatherMixData
{
	const char*     nodename;
	AtBoolean       enable;
	AtFloat         tolerance;

	AtFloat           sample_color0;
	AtFloat           sample_color1;
	AtFloat           sample_color2;
	AtFloat           sample_color3;
	AtFloat           sample_color4;
	AtFloat           sample_color5;

	AtBoolean           sample_enable0;
	AtBoolean           sample_enable1;
	AtBoolean           sample_enable2;
	AtBoolean           sample_enable3;
	AtBoolean           sample_enable4;
	AtBoolean           sample_enable5;
};

node_parameters
{
	AiParameterBOOL( "enable", TRUE );

    AiParameterRGB( "sampleTexture", 1.0f, 0.0f, 0.0f );
    AiParameterRGB( "multiplyColor", 1.0f, 1.0f,1.0f );	
    AiParameterFLT( "multiplyAlpha", 1.0f);	
    AiParameterFLT( "hueTolerance", 10.0f );

	AiParameterBOOL( "sampleEnable0", TRUE );
    AiParameterRGB( "sampleColor0", 1.0f, 0.0f, 0.0f );
	AiParameterRGB( "texColor0", 1.0f, 1.0f,1.0f );
	AiParameterFLT( "texAlpha0", 1.0f );
	
	AiParameterBOOL( "sampleEnable1", TRUE );
    AiParameterRGB( "sampleColor1", 0.0f, 1.0f, 0.0f );
	AiParameterRGB( "texColor1", 1.0f, 1.0f,1.0f );
	AiParameterFLT( "texAlpha1", 1.0f );
	
	AiParameterBOOL( "sampleEnable2", FALSE );
    AiParameterRGB( "sampleColor2", 0.0f, 0.0f, 1.0f );
	AiParameterRGB( "texColor2", 1.0f, 1.0f,1.0f );
	AiParameterFLT( "texAlpha2", 1.0f );
	
	AiParameterBOOL( "sampleEnable3", FALSE );
    AiParameterRGB( "sampleColor3", 1.0f, 1.0f, 0.0f );
	AiParameterRGB( "texColor3", 1.0f, 1.0f,1.0f );
	AiParameterFLT( "texAlpha3", 1.0f );
	
	AiParameterBOOL( "sampleEnable4", FALSE );
    AiParameterRGB( "sampleColor4", 0.0f, 1.0f, 1.0f );
	AiParameterRGB( "texColor4", 1.0f, 1.0f,1.0f );	
	AiParameterFLT( "texAlpha4", 1.0f );
	
	AiParameterBOOL( "sampleEnable5", FALSE );
    AiParameterRGB( "sampleColor5", 1.0f, 0.0f, 1.0f );
	AiParameterRGB( "texColor5", 1.0f, 1.0f,1.0f );	
	AiParameterFLT( "texAlpha5", 1.0f );
	
    AiParameterRGB( "defaultColor", 1.0f, 1.0f, 1.0f );
	AiParameterFLT( "defaultAlpha", 1.0f );
}


node_initialize
{
	jcFeatherFeatherMixData *data = new jcFeatherFeatherMixData();
	#if (AI_VERSION_ARCH_NUM==3 && AI_VERSION_MAJOR_NUM>=4) || (AI_VERSION_ARCH_NUM>=4)
		AiNodeSetLocalData( node, data );
	#else
		node->local_data = data;
	#endif
	
	data->nodename                   = AiNodeGetStr (node, "name");
	data->enable                     = AiNodeGetBool(node, "enable");
	data->tolerance                  = AiNodeGetFlt (node, "hueTolerance");

	AtRGB col = AiNodeGetRGB (node, "sampleColor0");
	rgbToHsv(col);
	data->sample_color0		         = col.r;

	col = AiNodeGetRGB (node, "sampleColor1");
	rgbToHsv(col);
	data->sample_color1		         = col.r;

	col = AiNodeGetRGB (node, "sampleColor2");
	rgbToHsv(col);
	data->sample_color2		         = col.r;
	
	col = AiNodeGetRGB (node, "sampleColor3");
	rgbToHsv(col);
	data->sample_color3		         = col.r;
	
	col = AiNodeGetRGB (node, "sampleColor4");
	rgbToHsv(col);
	data->sample_color4		         = col.r;
	
	col = AiNodeGetRGB (node, "sampleColor5");
	rgbToHsv(col);
	data->sample_color5		         = col.r;

	data->sample_enable0             = AiNodeGetBool(node, "sampleEnable0");
	data->sample_enable1             = AiNodeGetBool(node, "sampleEnable1");
	data->sample_enable2             = AiNodeGetBool(node, "sampleEnable2");
	data->sample_enable3             = AiNodeGetBool(node, "sampleEnable3");
	data->sample_enable4             = AiNodeGetBool(node, "sampleEnable4");
	data->sample_enable5             = AiNodeGetBool(node, "sampleEnable5");

	AiMsgSetMaxWarnings(100);
}


node_update
{
}


node_finish
{
}


shader_evaluate
{
	#if (AI_VERSION_ARCH_NUM==3 && AI_VERSION_MAJOR_NUM>=4) || (AI_VERSION_ARCH_NUM>=4)
    	jcFeatherFeatherMixData* data = (jcFeatherFeatherMixData*)AiNodeGetLocalData( node );
	#else
    	jcFeatherFeatherMixData* data = (jcFeatherFeatherMixData*)node->local_data;
	#endif	

	//if not enable, output color 1 ,alpha 1
    if ( !data->enable )
    {
	    sg->out.RGBA.r = 1;
		sg->out.RGBA.g = 1;
		sg->out.RGBA.b = 1;
		sg->out.RGBA.a = 1;
		sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = 1;
        return;
    }

    AtRGB sampleTex = AiShaderEvalParamRGB( p_sample_texture );
	AtRGB multiplyCol = AiShaderEvalParamRGB( p_multiply_color );
	AtFloat multiplyAlpha = AiShaderEvalParamFlt( p_multiply_alpha );

    AtRGB outputColor; 
	outputColor.r=outputColor.g=outputColor.b=1;
    AtFloat outputAlpha=1;
    
    rgbToHsv(sampleTex);
    float dist=360;
    float tempDist=0;
  
    if(data->sample_enable0)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color0);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color0 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha0 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	if(data->sample_enable1)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color1);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color1 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha1 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	if(data->sample_enable2)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color2);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color2 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha2 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	if(data->sample_enable3)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color3);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color3 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha3 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	if(data->sample_enable4)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color4);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color4 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha4 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	if(data->sample_enable5)
	{
		tempDist = hueClosestDist(sampleTex.r,data->sample_color5);
		if(tempDist<data->tolerance)
		{
			outputColor = AiShaderEvalParamRGB( p_tex_color5 );
			outputAlpha = AiShaderEvalParamFlt( p_tex_alpha5 );
		
			outputColor *= multiplyCol;
			outputAlpha *= multiplyAlpha;

			sg->out.RGBA.r = outputColor.r;
			sg->out.RGBA.g = outputColor.g;
			sg->out.RGBA.b = outputColor.b;
			sg->out.RGBA.a = outputAlpha;
			sg->out_opacity.r = sg->out_opacity.g=sg->out_opacity.b = outputAlpha;
			return;
		}
	}

	//if we cannot find the sample texture for the sampled hue value,use the default color and alpha
	AtRGB defaultCol = AiShaderEvalParamRGB( p_default_color );
	AtFloat defaultAlpha = AiShaderEvalParamFlt( p_default_alpha );	
	
	defaultCol*=multiplyCol;
	defaultAlpha *= multiplyAlpha;

	sg->out.RGBA.r = defaultCol.r;
	sg->out.RGBA.g = defaultCol.g;
	sg->out.RGBA.b = defaultCol.b;
	sg->out.RGBA.a = defaultAlpha; 

}


node_loader
{
    if ( i > 0 )
    {
        return FALSE;
    }
    
    node->methods     = jcFeatherTexMixMethods;
    node->output_type = AI_TYPE_RGBA;
    node->name        = "jcFeatherTexMix";
    node->node_type   = AI_NODE_SHADER;
    
    strcpy( node->version, AI_VERSION );
    
    return TRUE;
}
