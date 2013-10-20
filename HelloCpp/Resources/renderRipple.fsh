//********************************************************
//           by yang chao (wantnon)
//           2013-10-20
//			 blog: http://350479720.qzone.qq.com
//********************************************************

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
    vec2 gb=texture2D(texSource,v_texCoord).gb;//c-d,b-a在updateRipple.fsh中已保存到g和b分量中，所以此处直接取即可
    float s_offset = gb.x;
	float t_offset = gb.y;
	gl_FragColor=texture2D(CC_Texture0,v_texCoord+vec2(s_offset,t_offset));
}

