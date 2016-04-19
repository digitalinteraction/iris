#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
uniform texture_2d undist;
//uniform float offset;
//const float waves = 2.0;
varying vec2 texcoord;
void main(void) {
    //float x = texcoord.x + 0.05 * sin(offset + (texcoord.y * waves * 2.0 * 3.141592));
    //float y = texcoord.y + 0.05 * sin(offset + (texcoord.x * waves * 2.0 * 3.141592));
    //float x = texcoord.x;
    //float y = texcoord.y;
    //if (y < 1.0 && y > 0.0 && x < 1.0 && x > 0.0) {
    //   vec2 pos = vec2(x, y);
    //   gl_FragColor = texture2D(tex, pos);
    //}
    //else {
    //   gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    //}
    
    gl_FragColor = texture2D(tex, texcoord);

}

