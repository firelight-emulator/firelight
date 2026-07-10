#pragma once

#include <QString>
#include <optional>
#include <rhi/qshader.h>

namespace firelight::graphics::shader_compiler {

// Compiles a GLSL source string to a QShader (SPIR-V + all backend targets),
// so the resulting shader runs on whatever RHI backend the app is using
// (D3D11 / OpenGL / Vulkan / Metal). `stage` is Vertex or Fragment. Returns
// nullopt (and, when `error` is non-null, a reason) on failure.
//
// Isolated here because it is the one place that depends on Qt's shader baker
// (Qt6::ShaderToolsPrivate). If that dependency is unavailable in a given build,
// only this file needs to change (or be stubbed); the rest of the shader chain
// consumes plain QShader objects.
std::optional<QShader> compile(QShader::Stage stage, const QString &glsl,
                               QString *error = nullptr);

} // namespace firelight::graphics::shader_compiler
