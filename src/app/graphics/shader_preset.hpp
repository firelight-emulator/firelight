#pragma once

#include <QString>
#include <QVector>
#include <optional>

namespace firelight::graphics {

// One tunable parameter exposed by a shader preset. Rendered as a slider in the
// picker and passed to the shader as a uniform.
struct ShaderParameter {
  QString key;
  QString label;
  float minValue = 0.0f;
  float maxValue = 1.0f;
  float step = 0.01f;
  float defaultValue = 0.0f;
};

// One render pass. `fragmentGlsl` is the on-disk path to the fragment shader
// (Vulkan-GLSL dialect); the chain supplies a shared fullscreen-triangle vertex
// shader. `scale` multiplies the source size to get this pass's output size.
struct ShaderPass {
  QString fragmentGlsl;   // absolute path
  float scale = 1.0f;     // output size = scale * source size
  QString filter = "linear"; // "linear" | "nearest"
  QString wrap = "clamp";    // "clamp" | "repeat"
};

// A Firelight shader preset (.flsp): an ordered list of passes plus declared
// parameters. Kept deliberately close to the RetroArch slang model so a
// librashader-backed loader can later produce the same structure.
struct ShaderPreset {
  QString id;      // stable id (folder name / file stem)
  QString name;    // display name
  QString path;    // absolute path to the .flsp file (or folder)
  bool builtIn = false;
  // When true the chain keeps the previous frame's final output and binds it as
  // an extra sampler (binding 2, "Feedback"), enabling afterglow / persistence /
  // NTSC-style effects.
  bool feedback = false;
  QVector<ShaderPass> passes;
  QVector<ShaderParameter> parameters;
};

namespace shader_preset {
// Loads and validates a preset from a .flsp JSON file. Returns nullopt and, when
// `error` is non-null, a human-readable reason on failure. Pass paths are
// resolved relative to the preset file's directory.
std::optional<ShaderPreset> loadFromFile(const QString &path,
                                         QString *error = nullptr);

// Parses a preset from an in-memory JSON string. `baseDir` resolves relative
// pass paths; `id`/`path` stamp the returned preset. Exposed for testing.
std::optional<ShaderPreset> parse(const QString &json, const QString &baseDir,
                                  const QString &id, const QString &path,
                                  QString *error = nullptr);
} // namespace shader_preset

} // namespace firelight::graphics
