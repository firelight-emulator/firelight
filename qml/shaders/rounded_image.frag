#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float widthPx;
    float heightPx;
    float topLeftPx;
    float topRightPx;
    float bottomLeftPx;
    float bottomRightPx;
};

// Signed distance to a rounded box, evaluated per output pixel so the corner
// antialiases against the real device pixels regardless of scale factor. Each
// quadrant folds onto the same corner, so picking its radius first gives four
// independent corners out of one distance function
void main() {
    vec2 sizePx = vec2(widthPx, heightPx);
    vec2 halfSize = sizePx * 0.5;

    vec2 centered = qt_TexCoord0 * sizePx - halfSize;

    // Texture coordinates grow downward, so a positive y is the lower half
    float corner = centered.y < 0.0
        ? (centered.x < 0.0 ? topLeftPx : topRightPx)
        : (centered.x < 0.0 ? bottomLeftPx : bottomRightPx);

    float r = clamp(corner, 0.0, min(halfSize.x, halfSize.y));

    vec2 q = abs(centered) - (halfSize - r);
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - r;

    float aa = fwidth(dist);
    float coverage = 1.0 - smoothstep(-aa, aa, dist);

    fragColor = texture(source, qt_TexCoord0) * coverage * qt_Opacity;
}
