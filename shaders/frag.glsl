#version 330 core

in vec2 TexCoord;
layout (location = 0) out vec4 color;

uniform sampler2D texImage;

uniform int compIdx;
uniform float alpha;
uniform float blendOrig;
uniform float blendRgbComp;

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 interp(float a, vec3 b, float d) {
    return vec3(a * (1.0 - d) + b.x * d,
    a * (1.0 - d) + b.y * d,
    a * (1.0 - d) + b.z * d);
}

void main() {
    vec4 texColor = texture(texImage, TexCoord);
    vec3 hsv = rgb2hsv(texColor.rgb);
    vec3 interpolated = interp(hsv[compIdx], texColor.rgb, blendOrig);
    vec3 interpolated2 = interp(texColor[compIdx], interpolated, 1.0 - blendRgbComp);
    color = vec4(interpolated2, alpha);
}