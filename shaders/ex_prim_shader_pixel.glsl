#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D al_tex;
uniform sampler2D cob_normal;

uniform vec3 tint;
uniform vec3 lum;
uniform int test;

varying vec4 varying_color;
varying vec2 varying_texcoord;
varying vec2 uv;

const vec2 light = vec2(0.25, 0.8);
const int steps = 130;
const float f_steps = float(steps);

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
uniform vec3 LightPos;        //light position, normalized
uniform vec4 LightColor;      //light RGBA -- alpha is intensity
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform vec3 Falloff;         //attenuation coefficients


void main()
{

    //LightPos = vec3(0.3, 0.3, 0.1);

    //RGBA of our diffuse color
    vec4 DiffuseColor = texture2D(al_tex, varying_texcoord);

    //RGB of our normal map
    vec3 NormalMap = texture2D(cob_normal, varying_texcoord).rgb;

    //The delta position of light
    vec3 LightDir = vec3(LightPos.xy - (gl_FragCoord.xy / Resolution.xy), LightPos.z);

    //Correct for aspect ratio
    LightDir.x *= Resolution.x / Resolution.y;

    //Determine distance (used for attenuation) BEFORE we normalize our LightDir
    float D = length(LightDir);

    //normalize our vectors
    vec3 N = normalize(NormalMap * 2.0 - 1.0);
    vec3 L = normalize(LightDir);

    //Pre-multiply light color with intensity
    //Then perform "N dot L" to determine our diffuse term
    vec3 Diffuse = (LightColor.rgb * LightColor.a) * max(dot(N, L), 0.0);

    //pre-multiply ambient color with intensity
    vec3 Ambient = AmbientColor.rgb * AmbientColor.a;

    //calculate attenuation
    float Attenuation = 1.0 / ( Falloff.x + (Falloff.y*D) + (Falloff.z*D*D) );

    //the calculation which brings it all together
    vec3 Intensity = Ambient + Diffuse * Attenuation;
    vec3 FinalColor = DiffuseColor.rgb * Intensity;
    gl_FragColor = varying_color * vec4(FinalColor, DiffuseColor.a);
    //gl_FragColor = vec4(FinalColor.r, FinalColor.g, FinalColor.b, .3);

   /*
   // Shadow part, need to think better on this
   // XXX Needs correct ordering on what casts shadows

   vec4 tmp = varying_color * texture2D(al_tex, varying_texcoord);
   vec4 Color = texture2D(al_tex, varying_texcoord);
   vec4 color = Color;
   if (test > 0)
   {
        color = vec4( vec3( Color.r * lum.r + Color.g * lum.g+Color.b * lum.b), Color.a);
   }
   gl_FragColor = color;
    
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
  */

}
