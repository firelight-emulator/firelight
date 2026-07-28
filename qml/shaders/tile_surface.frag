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
    float topLight;
    float bottomShade;
    float bevelFrac;
};

// Rounded-corner mask (same SDF as rounded_image.frag) plus a bevel: a rim of
// light along the top-facing edges and shade along the bottom-facing edges,
// fading to a flat face, so the tile reads as a raised 3D object lit from above
void main() {
    vec2 sizePx = vec2(widthPx, heightPx);
    vec2 halfSize = sizePx * 0.5;
    float r = min(radiusPx, min(halfSize.x, halfSize.y));

    vec2 centered = qt_TexCoord0 * sizePx - halfSize;
    vec2 q = abs(centered) - (halfSize - r);
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - r;

    float aa = fwidth(dist);
    float coverage = 1.0 - smoothstep(-aa, aa, dist);

    // Outward edge normal from the distance-field gradient
    vec2 n = normalize(vec2(dFdx(dist), dFdy(dist)) + vec2(1e-5));

    // Light from the top, biased slightly to the left (screen y points down)
    vec2 lightDir = normalize(vec2(-0.35, -1.0));
    float ndl = dot(n, lightDir);

    // Rim band: strongest at the edge, gone by bevel width into the face
    float bevelPx = clamp(min(sizePx.x, sizePx.y) * bevelFrac, 1.5, 9.0);
    float rim = (1.0 - smoothstep(0.0, bevelPx, -dist)) * step(dist, 0.0);

    float light = 1.0 + rim * (ndl >= 0.0 ? topLight * ndl : bottomShade * ndl);

    vec4 tex = texture(source, qt_TexCoord0);
    tex.rgb *= light;

    fragColor = tex * coverage * qt_Opacity;
}
