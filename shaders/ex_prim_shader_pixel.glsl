#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D al_tex;
uniform vec3 tint;
uniform vec3 lum;
uniform int test;

varying vec4 varying_color;
varying vec2 varying_texcoord;
varying vec2 uv;

const vec2 light = vec2(0.25, 0.8);
const int steps = 130;
const float f_steps = float(steps);


void main()
{
   /*
   vec4 tmp = varying_color * texture2D(al_tex, varying_texcoord);
   vec4 Color = texture2D(al_tex, varying_texcoord);
   vec4 color = Color;
   if (test > 0)
   {
        color = vec4( vec3( Color.r * lum.r + Color.g * lum.g+Color.b * lum.b), Color.a);
   }
   gl_FragColor = color;
   */
    
    vec2 uv = varying_texcoord;
   	float dx = (light.x - uv.x) / f_steps;
    float dy = (light.y - uv.y) / f_steps;

    vec4 Color = varying_color * texture2D(al_tex, varying_texcoord);
    vec4 tmp = varying_color * texture2D(al_tex, varying_texcoord);
    gl_FragColor = vec4(Color.r, Color.g, Color.b, Color.a);
    
   if (test > 0) {
    for(int i = 0; i < steps; i++) {
        float sample_x = uv.x + (dx * float(i));
        float sample_y = uv.y + (dy * float(i));
        vec4 color = texture2D(al_tex, vec2(sample_x, sample_y));
        if(color.r < 0.165) {
    	    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    	    break;
    	}
    }
    
    if(distance(uv, light) < 0.005) {
    	gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
  }
}
