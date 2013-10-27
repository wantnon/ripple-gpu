//********************************************************
//           by yang chao (wantnon)
//           2013-10-20
//			 blog: http://350479720.qzone.qq.com
//********************************************************

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;

//uniform sampler2D CC_Texture0;
uniform sampler2D texSource;
uniform sampler2D texDest;
uniform float step_s;
uniform float step_t;
uniform vec2 touchPos_winSpace;
uniform bool touchValid;
uniform vec2 winSize;
uniform vec2 bufferTexSize;


void main() {

    //float time = CC_Time[1];
    float waterHorizon=0.5;
	//
	//     a
	//   c * d
	//     b
	//
    //in cocos2d-x, the read in image data is not fliped,
    //so normal texture's origin is at left top corner,
    //but FBO's origin is at left bottom corner,
    //so render to texture cause upside down.
    //see:http://www.pouet.net/topic.php?which=8966
    //http://stackoverflow.com/questions/11861336/opengl-es-render-to-textture-appears-upside-down
    //if we want get upside up render result, two method:
    //method 1, let the source upside down, then the render result will be upside up.
    //method 2, in shader, will change texCoord.y to 1-texCoord.y, then the render result will be upside up.
    //here we use the second method.
    vec2 texCoord=vec2(v_texCoord.x,1.0-v_texCoord.y);
    
	vec2 up=texCoord+vec2(0.0,-step_t);
	vec2 dn=texCoord+vec2(0.0,step_t);
	vec2 left=texCoord+vec2(-step_s,0.0);
	vec2 right= texCoord+vec2(step_s,0.0);
	float a=texture2D(texSource, up).r-waterHorizon;
	float b=texture2D(texSource, dn).r-waterHorizon;
	float c=texture2D(texSource, left).r-waterHorizon;
	float d=texture2D(texSource, right).r-waterHorizon;
	float destCenter=texture2D(texDest,texCoord).r-waterHorizon;
    float rippleStrength=32.0;
    float result=(a+b+c+d-2.0*destCenter)*(0.5-0.5/rippleStrength);
    vec2 pos=texCoord*winSize;//use gl_FragCoord.xy*(winSize.x/bufferTexSize.x) also right. gl_FragCoord is the viewport space coord(origin at left up corner)
	float R=20.0;
	float H=1.0;
	float dis=distance(pos,touchPos_winSpace);
	if(touchValid&&dis<R){
		result=-H/2.0*(cos(3.1415926*dis/R)+1.0);

	}
	gl_FragColor =vec4(result+waterHorizon,c-d,b-a,1.0);
    
}

