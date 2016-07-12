uniform sampler2D picture;
varying vec2 texcoord;

void main()
{  
    gl_FragColor = texture2D(picture, texcoord);
}