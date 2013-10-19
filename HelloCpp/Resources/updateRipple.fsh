

#ifdef GL_ES
precision mediump float;
#endif

varying highp vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform sampler2D texSource;
uniform sampler2D texDest;
uniform float step_s;
uniform float step_t;
uniform vec2 touchPos;
uniform bool touchValid;


void main() {

    //float time = CC_Time[1];
	//
	//     a
	//   c * d
	//     b
	//
	vec2 up=v_texCoord+vec2(0,step_t);
	vec2 dn=v_texCoord+vec2(0,-step_t);
	vec2 left=v_texCoord+vec2(-step_s,0);
	vec2 right= v_texCoord+vec2(step_s,0);
	float a=texture2D(texSource, up).r;
	float b=texture2D(texSource, dn).r;
	float c=texture2D(texSource, left).r;
	float d=texture2D(texSource, right).r;
	float x=texture2D(texDest,v_texCoord).r;
    float result=(a+b+c+d-2*x)*0.5;
	vec2 pos=gl_FragCoord.xy;//fragment's window space coord
	if(touchValid&&abs(pos.x-touchPos.x)<3&&abs(pos.y-touchPos.y)<3){
		result+=0.5;
	}
	gl_FragColor =vec4(result,result,result,1);
	

	//gl_FragColor =texture2D(texSource,v_texCoord);
}
