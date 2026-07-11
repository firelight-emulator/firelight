#version 450
// A fuller CRT look: barrel curvature, scanlines, an RGB aperture mask at output
// resolution, brightness boost and edge vignette. Params: scanline, mask,
// curvature, brightboost.
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(binding = 1) uniform sampler2D Source;
layout(std140, binding = 0) uniform UBO {
    vec4 SourceSize;
    vec4 OutputSize;
    vec4 Frame;
    vec4 Param[8];
};

// Barrel distortion around the screen centre.
vec2 curve(vec2 uv, float amount) {
    uv = uv * 2.0 - 1.0;
    uv *= 1.0 + dot(uv, uv) * amount;
    return uv * 0.5 + 0.5;
}

void main() {
    float scanline = clamp(Param[0].x, 0.0, 1.0);
    float mask = clamp(Param[1].x, 0.0, 1.0);
    float curvature = Param[2].x;
    float boost = Param[3].x;

    vec2 uv = curve(vTexCoord, curvature);
    // Off-screen after warping -> black (the tube edge).
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 col = texture(Source, uv).rgb * boost;

    // Scanlines keyed to source rows.
    float line = uv.y * SourceSize.y;
    float scan = 0.5 + 0.5 * cos(6.28318530718 * line);
    col *= mix(1.0, scan, scanline);

    // RGB aperture mask across output columns.
    float m = mod(gl_FragCoord.x, 3.0);
    vec3 cell = vec3(m < 1.0 ? 1.0 : 0.55,
                     (m >= 1.0 && m < 2.0) ? 1.0 : 0.55,
                     m >= 2.0 ? 1.0 : 0.55);
    col *= mix(vec3(1.0), cell, mask);

    // Subtle vignette.
    vec2 v = uv * (1.0 - uv.yx);
    float vig = pow(v.x * v.y * 15.0, 0.25);
    col *= clamp(vig, 0.0, 1.0);

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
