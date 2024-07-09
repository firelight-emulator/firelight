#pragma once

#include "emulation_manager.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"

#include <QOpenGLFunctions>
#include <QQuickFramebufferObject>

class EmulatorRenderer final : public QQuickFramebufferObject::Renderer,
                               public QOpenGLFunctions,
                               public firelight::ManagerAccessor {
public:
  explicit EmulatorRenderer(const EmulationManager *manager, std::shared_ptr<libretro::Core> core);

protected:
  ~EmulatorRenderer() override;

  void synchronize(QQuickFramebufferObject *fbo) override;

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

  void render() override;

private:
  std::shared_ptr<libretro::Core> m_core;
  EmulationManager *m_manager = nullptr;
  QOpenGLFramebufferObject *m_fbo = nullptr;

  bool m_runAFrame = false;

  int m_nativeWidth = 0;
  int m_nativeHeight = 0;

  void receiveVideoData(const void *data, unsigned width, unsigned height,
                        size_t pitch) const;

  std::function<void()> m_resetContextFunction = nullptr;
  std::function<void()> m_destroyContextFunction = nullptr;
};
