#version 450
// Sharp bilinear: crisp integer-ish upscaling that keeps texels square but
// antialiases their edges, avoiding both the blur of plain bilinear and the
// shimmer of nearest-neighbour at non-integer scales.
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(binding = 1) uniform sampler2D Source;
layout(std140, binding = 0) uniform UBO {
    vec4 SourceSize;   // xy = px, zw = 1/px
    vec4 OutputSize;
    vec4 Frame;
    vec4 Param[8];
};

void main() {
    vec2 texel = vTexCoord * SourceSize.xy;
    vec2 scale = max(floor(OutputSize.xy / SourceSize.xy), vec2(1.0));

    vec2 texelCenter = floor(texel) + 0.5;
    vec2 frac = texel - texelCenter;
    // Squeeze the sampling into the sharp centre of each texel, blending only
    // across a 1-output-pixel-wide seam.
    vec2 region = clamp(frac * scale, -0.5, 0.5);
    vec2 uv = (texelCenter + region) * SourceSize.zw;

    FragColor = vec4(texture(Source, uv).rgb, 1.0);
}
