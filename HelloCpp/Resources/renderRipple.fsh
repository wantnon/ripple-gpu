

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform sampler2D texSource;
uniform float step_s;
uniform float step_t;

void main() {

    //float time = CC_Time[1];
	//
	//     a
	//   c * d
	//     b
	//
	vec2 up=v_texCoord+vec2(0,-step_t);
	vec2 dn=v_texCoord+vec2(0,step_t);
	vec2 left=v_texCoord+vec2(-step_s,0);
	vec2 right= v_texCoord+vec2(step_s,0);
	float a=texture2D(texSource, up).r;
	float b=texture2D(texSource, dn).r;
	float c=texture2D(texSource, left).r;
	float d=texture2D(texSource, right).r;
    // clamp
  //  float kOneDiv1048= 0.00095419847328244274809160305343511;
    float s_offset = (c - d)/32;//  kOneDiv1048;
	float t_offset = (b - a)/32;//  kOneDiv1048;
	gl_FragColor=texture2D(CC_Texture0,v_texCoord+vec2(s_offset,t_offset));

//	gl_FragColor=texture2D(texSource,v_texCoord);
}

