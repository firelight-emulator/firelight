#version 450
// Phosphor persistence: bright pixels leave a decaying trail, like a real CRT.
// Uses the Feedback sampler (previous frame's final output). Params:
// persistence (trail length), scanline depth.
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(binding = 1) uniform sampler2D Source;
layout(binding = 2) uniform sampler2D Feedback;
layout(std140, binding = 0) uniform UBO {
    vec4 SourceSize;
    vec4 OutputSize;
    vec4 Frame;
    vec4 Param[8];
};

void main() {
    vec3 cur = texture(Source, vTexCoord).rgb;

    // Scanlines on the current frame.
    float line = vTexCoord.y * SourceSize.y;
    float scan = 0.5 + 0.5 * cos(6.28318530718 * line);
    cur *= mix(1.0, scan, clamp(Param[1].x, 0.0, 1.0));

    // The trail is the previous output decayed; keep whichever is brighter so
    // trails fade out rather than smearing dark.
    vec3 prev = texture(Feedback, vTexCoord).rgb * clamp(Param[0].x, 0.0, 0.95);
    vec3 col = max(cur, prev);

    FragColor = vec4(col, 1.0);
}
