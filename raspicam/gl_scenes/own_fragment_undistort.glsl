#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
varying vec2 texcoord;
const float k1 = 0.25555923577415596;
const float k2 = -0.030785168080071819;
const float k3 = -0.0038011414947686610;
const float k4 = -0.047099769209054239;

vec4 color(samplerExternalOES tex, vec2 pos) {
    if(pos.x < 0.0 || pos.y < 0.0 || pos.x > 1.0 || pos.y > 1.0) {
        return vec4(1.0, 1.0, 1.0, 1.0); // white
    } else {
        return texture2D(tex, pos);
    }
}

vec2 clamp(vec2 pos) {
    float x = pos.x;
    float y = pos.y;

    if(x < 0.0 || y < 0.0) {
        x = 0.0;
        y = 0.0;
    }

    if(x > 1.0 || y > 1.0) {
        x = 1.0;
        y = 1.0;
    }

    return vec2(x, y);
}

vec2 texcoordtopos(vec2 tex) {
    return 2.0 * vec2(tex.x - 0.5, tex.y - 0.5);
}

vec2 postotexcoord(vec2 pos) {
    return vec2(pos.x / 2.0 + 0.5, pos.y / 2.0 + 0.5);
}

vec2 toPolar(vec2 point) {
    float r = length(point);
    float theta = atan(point.y, point.x);
    return vec2(r, theta);
}

vec2 toPoint(vec2 pol) {
    float r = pol.x;
    float theta = pol.y;
    float x = r * cos(theta);
    float y = r * sin(theta);
    return vec2(x, y);
}

vec2 fisheye(vec2 pos) {
    vec2 p = toPolar(pos);
    float r = p.x;
    float theta = p.y;
    float s = 0.76;
    float lambda = 3.8342;
    float rr = s * log(1.0 + lambda * r);
    vec2 pp = vec2(rr, theta);
    return toPoint(pp);
}

vec2 transform(vec2 tex) {
    return postotexcoord(fisheye(texcoordtopos(tex)));
}

void main() {
    gl_FragColor = color(tex, transform(texcoord));
}
