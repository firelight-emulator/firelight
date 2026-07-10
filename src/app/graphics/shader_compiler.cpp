#include "shader_compiler.hpp"

// QShaderBaker lives in Qt's ShaderTools module (private API). Linked via
// Qt6::ShaderToolsPrivate in CMakeLists.txt. This include path is the
// documented location for the runtime baker.
#include <QtShaderTools/private/qshaderbaker_p.h>

#include <QTemporaryFile>

namespace firelight::graphics::shader_compiler {

std::optional<QShader> compile(const QShader::Stage stage, const QString &glsl,
                               QString *error) {
  QShaderBaker baker;

  // The baker reads GLSL from a file path; write the in-memory source to a
  // temp file so user-provided shaders (loaded as strings) can be baked.
  QTemporaryFile tmp;
  tmp.setAutoRemove(true);
  if (!tmp.open()) {
    if (error) {
      *error = QStringLiteral("could not create a temp file for compilation");
    }
    return std::nullopt;
  }
  tmp.write(glsl.toUtf8());
  tmp.flush();

  baker.setSourceFileName(tmp.fileName(), stage);

  // Target every backend Qt's RHI might use, plus SPIR-V. Versions chosen to
  // match Qt's own defaults (GLSL 120/150 desktop + 100es, HLSL 5.0, MSL 1.2).
  QList<QShaderBaker::GeneratedShader> targets = {
      {QShader::SpirvShader, QShaderVersion(100)},
      {QShader::GlslShader, QShaderVersion(120)},
      {QShader::GlslShader, QShaderVersion(150)},
      {QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)},
      {QShader::HlslShader, QShaderVersion(50)},
      {QShader::MslShader, QShaderVersion(12)},
  };
  baker.setGeneratedShaders(targets);
  baker.setGeneratedShaderVariants({QShader::StandardShader});

  const QShader shader = baker.bake();
  if (!shader.isValid()) {
    if (error) {
      *error = baker.errorMessage();
    }
    return std::nullopt;
  }
  return shader;
}

} // namespace firelight::graphics::shader_compiler
