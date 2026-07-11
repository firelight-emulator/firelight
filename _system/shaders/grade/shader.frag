#version 450
// A plain color-grading filter (no scanlines): warm/cool temperature,
// saturation, contrast and brightness. Handy for taming washed-out or overly
// cold cores. Params: temperature, saturation, contrast, brightness.
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

    // Temperature: push red vs blue.
    float t = Param[0].x;
    col.r *= 1.0 + 0.12 * t;
    col.b *= 1.0 - 0.12 * t;

    // Saturation around luma.
    float luma = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(vec3(luma), col, Param[1].x);

    // Contrast around mid-grey, then brightness.
    col = (col - 0.5) * Param[2].x + 0.5;
    col *= Param[3].x;

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
