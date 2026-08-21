#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radiusPx;
    float widthPx;
    float heightPx;
    float featherPx;
    float thicknessPx;
};

// The same rounded-box SDF the crisp mask uses, with the edge ramped inward over
// featherPx so the content fades out rather than ending. The ramp runs inward
// only: a layer texture holds nothing past the item's bounds, so half a ramp
// centred on the boundary would land on pixels that were never drawn
void main() {
    vec2 sizePx = vec2(widthPx, heightPx);
    vec2 halfSize = sizePx * 0.5;
    float r = min(radiusPx, min(halfSize.x, halfSize.y));

    vec2 centered = qt_TexCoord0 * sizePx - halfSize;
    vec2 q = abs(centered) - (halfSize - r);
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - r;

    // At zero feather this collapses to the one-pixel antialias, so the effect can
    // stay attached across an animation that starts and ends crisp
    float edge = max(featherPx, fwidth(dist));
    float coverage = smoothstep(0.0, edge, -dist);

    // thicknessPx > 0 ramps the fill inward from the edge, reaching clear that far in
    float inner = thicknessPx > 0.0 ? smoothstep(0.0, thicknessPx, thicknessPx + dist) : 1.0;

    fragColor = texture(source, qt_TexCoord0) * min(coverage, inner) * qt_Opacity;
}
