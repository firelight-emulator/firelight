#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radiusPx;
    float ringWidthPx;
    float widthPx;
    float heightPx;
    float phase;
    float padPx;
    vec4 colorA;
    vec4 colorB;
};

const float TWO_PI = 6.28318530718;

// A rounded-rect ring whose two colors rotate around it. The band is inset by
// padPx from the item bounds so it and its antialiasing never clip the edges
void main() {
    vec2 sizePx = vec2(widthPx, heightPx);
    vec2 halfSize = sizePx * 0.5;
    vec2 innerHalf = halfSize - vec2(padPx);
    float r = min(radiusPx, min(innerHalf.x, innerHalf.y));

    vec2 centered = qt_TexCoord0 * sizePx - halfSize;
    vec2 q = abs(centered) - (innerHalf - r);
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - r;

    float halfW = ringWidthPx * 0.5;
    float aa = fwidth(dist) * 0.5;
    float alpha = 1.0 - smoothstep(halfW - aa, halfW + aa, abs(dist));

    float ang = atan(centered.y, centered.x) / TWO_PI + 0.5;
    float t = fract(ang + phase);
    float mixT = 0.5 - 0.5 * cos(t * TWO_PI);
    mixT = smoothstep(0.12, 0.88, mixT);
    vec4 col = mix(colorA, colorB, mixT);

    fragColor = col * alpha * qt_Opacity;
}
