#version 450
// Crisp sharp-bilinear upscale plus gentle scanlines — a clean, low-cost
// "sharp CRT" default. Param[0] = scanline depth.
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(binding = 1) uniform sampler2D Source;
layout(std140, binding = 0) uniform UBO {
    vec4 SourceSize;
    vec4 OutputSize;
    vec4 Frame;
    vec4 Param[8];
};

void main() {
    // Sharp-bilinear sampling (crisp texels, antialiased edges).
    vec2 texel = vTexCoord * SourceSize.xy;
    vec2 scale = max(floor(OutputSize.xy / SourceSize.xy), vec2(1.0));
    vec2 center = floor(texel) + 0.5;
    vec2 region = clamp((texel - center) * scale, -0.5, 0.5);
    vec3 col = texture(Source, (center + region) * SourceSize.zw).rgb;

    // Scanline keyed to source rows.
    float line = vTexCoord.y * SourceSize.y;
    float scan = 0.5 + 0.5 * cos(6.28318530718 * line);
    col *= mix(1.0, scan, clamp(Param[0].x, 0.0, 1.0));

    FragColor = vec4(col, 1.0);
}
