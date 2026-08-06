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
    float fillPx;
    vec4 colorA;
    vec4 colorB;
    vec4 colorC;
    vec4 colorD;
    vec4 fillColor;
};

const float HALF_PI = 1.57079632679;

// Distance travelled clockwise through one quadrant: along the straight run it
// entered by, around the corner, then out along the next run
float quadrantAt(float first, float second, float firstRun, float secondRun, float r, float arc) {
    float a = first - firstRun;
    float b = second - secondRun;

    if (a <= 0.0) {
        return first;
    }

    if (b > 0.0) {
        return firstRun + r * (HALF_PI - atan(b, a));
    }

    return firstRun + arc + (secondRun - second);
}

// How far around the outline a point lies, 0..1, measured as distance travelled
// rather than as an angle from the centre. Straight runs count their length and
// corners their arc, so the sweep holds one speed the whole way round whatever
// the shape: an angle races through the ends of a long thin rect, and a
// rectangle's corners break the speed on a circle, which is all corner
float perimeterAt(vec2 p, vec2 ext, float r) {
    float runX = max(ext.x - r, 0.0);
    float runY = max(ext.y - r, 0.0);
    float arc = HALF_PI * r;
    float quarter = runX + arc + runY;
    float u = abs(p.x);
    float v = abs(p.y);

    float d;
    if (p.x >= 0.0 && p.y <= 0.0) {
        d = quadrantAt(u, v, runX, runY, r, arc);
    } else if (p.x >= 0.0) {
        d = quarter + quadrantAt(v, u, runY, runX, r, arc);
    } else if (p.y >= 0.0) {
        d = 2.0 * quarter + quadrantAt(u, v, runX, runY, r, arc);
    } else {
        d = 3.0 * quarter + quadrantAt(v, u, runY, runX, r, arc);
    }

    return d / (4.0 * quarter);
}

// A rounded-rect ring whose four colors travel around it. The band is inset by
// padPx from the item bounds so it and its antialiasing never clip the edges.
// fillColor paints the gap between the surrounded item and the band, which is
// fillPx wide measured in from the band's centreline
void main() {
    vec2 sizePx = vec2(widthPx, heightPx);
    vec2 halfSize = sizePx * 0.5;
    vec2 innerHalf = halfSize - vec2(padPx);
    float r = min(radiusPx, min(innerHalf.x, innerHalf.y));

    vec2 centered = qt_TexCoord0 * sizePx - halfSize;
    vec2 q = abs(centered) - (innerHalf - r);
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - r;

    float halfW = ringWidthPx * 0.5;

    // A full derivative rather than half of one: the band is only a few pixels
    // wide, and on a tight curve — an icon button is a circle — half a pixel of
    // fade is not enough to hide the steps
    float aa = fwidth(dist);
    float alpha = 1.0 - smoothstep(halfW - aa, halfW + aa, abs(dist));

    // Four stops, a quarter of the way round each, every one blending into the
    // next — so colorA sits opposite colorC and the two quarters between them
    // differ, which a palindrome cannot do. Subtracting the phase runs it clockwise
    float t = fract(perimeterAt(centered, innerHalf, r) - phase);
    float seg = t * 4.0;
    int stop = int(seg);
    float f = seg - float(stop);

    vec4 col = stop == 0 ? mix(colorA, colorB, f)
             : stop == 1 ? mix(colorB, colorC, f)
             : stop == 2 ? mix(colorC, colorD, f)
                         : mix(colorD, colorA, f);

    // The surrounded item's own edge. Running the fill under the band as well
    // keeps a hairline of background from showing between the two
    float inner = -fillPx;
    float fillA = smoothstep(inner - aa, inner + aa, dist) * (1.0 - smoothstep(halfW - aa, halfW + aa, dist));

    fragColor = (fillColor * fillA * (1.0 - alpha) + col * alpha) * qt_Opacity;
}
