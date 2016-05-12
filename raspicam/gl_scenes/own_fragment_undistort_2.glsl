#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
uniform sampler2D alt_tex;
uniform sampler2D undist;
varying vec2 texcoord;

vec2 lookup(vec2 incoord){
    vec4 temp = texture2D(undist, incoord).rgba;
    return vec2(((temp.r) + (temp.g)/256.0), ((temp.b) + (temp.a)/256.0)+0.133);
}

void main()
{  
    #ifdef A
    vec2 lookup_coord = lookup(texcoord);
    gl_FragColor = texture2D(alt_tex, lookup_coord);
    gl_FragColor = texture2D(tex, lookup_coord);
    #else
    vec2 lookup_coord = lookup(texcoord);
    gl_FragColor = texture2D(tex, lookup_coord);
    gl_FragColor = texture2D(alt_tex, lookup_coord);
    #endif
}