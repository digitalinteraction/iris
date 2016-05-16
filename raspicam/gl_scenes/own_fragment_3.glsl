#extension GL_OES_EGL_image_external : require
uniform sampler2D renderTexture;
varying vec2 texcoord;
uniform vec2 tex_unit;

void main()
{  
    //vec4 pixelColor = texture2D(renderTexture, texcoord);
    //float luminance = pixelColor.r * 0.299 + pixelColor.g * 0.587 + pixelColor.b * 0.114;
    //gl_FragColor = vec4(luminance, luminance, luminance, 1.0);
    float x = tex_unit.x + texcoord.x;
    float y = tex_unit.y + texcoord.y;
    x = texcoord.x;
    y = texcoord.y;
    float x1 = x - RES_X;
    float y1 = y - RES_Y;
    float x2 = x + RES_X;
    float y2 = y + RES_Y;
    vec4 p0 = texture2D(renderTexture, vec2(x1, y1));
    vec4 p1 = texture2D(renderTexture, vec2(x, y1));
    vec4 p2 = texture2D(renderTexture, vec2(x2, y1));
    vec4 p3 = texture2D(renderTexture, vec2(x1, y));
    vec4 p5 = texture2D(renderTexture, vec2(x2, y));
    vec4 p6 = texture2D(renderTexture, vec2(x1, y2));
    vec4 p7 = texture2D(renderTexture, vec2(x, y2));
    vec4 p8 = texture2D(renderTexture, vec2(x2, y2));
    vec4 v =  p0 + (2.0 * p1) + p3 -p6 + (-2.0 * p7) + -p8;
    vec4 h =  p0 + (2.0 * p3) + p7 -p2 + (-2.0 * p5) + -p8;
    vec4 end = sqrt(h*h + v*v);
    if((end.r + end.g + end.b) < 10.0){
        gl_FragColor = texture2D(renderTexture, texcoord);
    }else{
        gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    }
    //gl_FragColor = end;
    gl_FragColor.a = 1.0;

    //gl_FragColor = texture2D(renderTexture, texcoord);
}
