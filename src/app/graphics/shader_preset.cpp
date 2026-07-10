#include "shader_preset.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace firelight::graphics::shader_preset {

namespace {
ShaderParameter parseParameter(const QJsonObject &obj) {
  ShaderParameter p;
  p.key = obj.value("key").toString();
  p.label = obj.value("label").toString(p.key);
  p.minValue = static_cast<float>(obj.value("min").toDouble(0.0));
  p.maxValue = static_cast<float>(obj.value("max").toDouble(1.0));
  p.step = static_cast<float>(obj.value("step").toDouble(0.01));
  p.defaultValue = static_cast<float>(obj.value("default").toDouble(0.0));
  return p;
}
} // namespace

std::optional<ShaderPreset> parse(const QString &json, const QString &baseDir,
                                  const QString &id, const QString &path,
                                  QString *error) {
  const auto setError = [&](const QString &msg) {
    if (error) {
      *error = msg;
    }
    return std::nullopt;
  };

  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    return setError(QStringLiteral("invalid JSON: %1")
                        .arg(parseError.errorString()));
  }
  const auto obj = doc.object();

  ShaderPreset preset;
  preset.id = id;
  preset.path = path;
  preset.name = obj.value("name").toString(id);

  if (!obj.contains("passes") || !obj.value("passes").isArray()) {
    return setError(QStringLiteral("preset has no 'passes' array"));
  }
  const auto passes = obj.value("passes").toArray();
  if (passes.isEmpty()) {
    return setError(QStringLiteral("preset has an empty 'passes' array"));
  }

  const QDir dir(baseDir);
  for (const auto &value : passes) {
    if (!value.isObject()) {
      return setError(QStringLiteral("a pass entry is not an object"));
    }
    const auto passObj = value.toObject();
    const auto frag = passObj.value("fragment").toString(
        passObj.value("glsl").toString());
    if (frag.isEmpty()) {
      return setError(QStringLiteral("a pass is missing its 'fragment' path"));
    }
    ShaderPass pass;
    pass.fragmentGlsl = QFileInfo(frag).isAbsolute() ? frag : dir.filePath(frag);
    pass.scale = static_cast<float>(passObj.value("scale").toDouble(1.0));
    pass.filter = passObj.value("filter").toString("linear");
    pass.wrap = passObj.value("wrap").toString("clamp");
    preset.passes.push_back(std::move(pass));
  }

  if (obj.contains("parameters") && obj.value("parameters").isArray()) {
    for (const auto &value : obj.value("parameters").toArray()) {
      if (value.isObject()) {
        auto param = parseParameter(value.toObject());
        if (!param.key.isEmpty()) {
          preset.parameters.push_back(std::move(param));
        }
      }
    }
  }

  return preset;
}

std::optional<ShaderPreset> loadFromFile(const QString &path, QString *error) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = QStringLiteral("could not open %1").arg(path);
    }
    return std::nullopt;
  }
  const auto json = QString::fromUtf8(f.readAll());
  const QFileInfo info(path);
  return parse(json, info.absolutePath(), info.completeBaseName(),
               info.absoluteFilePath(), error);
}

} // namespace firelight::graphics::shader_preset
