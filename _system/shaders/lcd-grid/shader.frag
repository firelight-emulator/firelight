#version 450
// Handheld-LCD look: a faint dark grid between source pixels, as if each pixel
// is a cell in an LCD matrix. Param[0] = grid strength.
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
    vec3 col = texture(Source, vTexCoord).rgb;
    float strength = clamp(Param[0].x, 0.0, 1.0);

    // Tent that peaks at the centre of each source pixel and dips to 0 at the
    // pixel edges, so the boundaries read as thin grid lines.
    vec2 f = fract(vTexCoord * SourceSize.xy);
    vec2 tent = smoothstep(0.0, 0.5, f) * smoothstep(1.0, 0.5, f);
    float cell = tent.x * tent.y;

    col *= mix(1.0 - strength, 1.0, cell);
    FragColor = vec4(col, 1.0);
}
