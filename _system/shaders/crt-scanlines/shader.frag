#version 450
// Simple, cheap CRT scanlines: darkens the gaps between source scanlines and
// boosts brightness to compensate, approximating the look of a CRT beam. Two
// parameters: Param[0] = strength, Param[1] = brightness boost.
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
    float strength = Param[0].x;
    float boost = Param[1].x;

    vec3 col = texture(Source, vTexCoord).rgb * boost;

    // One dark band per source scanline. cos() is 1 at the centre of a line and
    // dips between lines; strength controls how deep the gap gets.
    float line = vTexCoord.y * SourceSize.y;
    float beam = 0.5 + 0.5 * cos(6.28318530718 * (line - 0.5));
    col *= mix(1.0, beam, clamp(strength, 0.0, 1.0));

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
