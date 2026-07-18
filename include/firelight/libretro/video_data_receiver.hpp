#pragma once

#include "libretro/libretro.h"
#include <cstddef>

typedef void (*proc_address_t)();

namespace firelight::libretro {
    class IVideoDataReceiver {
    public:
        virtual ~IVideoDataReceiver() = default;

        virtual void receive(const void *data, unsigned width, unsigned height,
                             size_t pitch) = 0;

        virtual retro_hw_context_type getPreferredHwRender() = 0;

        virtual void setHwRenderInterface(retro_hw_render_callback *iface) = 0;

        virtual void setSystemAVInfo(retro_system_av_info *info) = 0;

        virtual void setPixelFormat(retro_pixel_format *format) = 0;

        virtual void setScreenRotation(unsigned rotation) = 0;

        virtual void setHwRenderContextNegotiationInterface(
            retro_hw_render_context_negotiation_interface *iface) = 0;

        virtual void getHwRenderInterface(retro_hw_render_interface **iface) = 0;

        // TODO
        // Called by Core::~Core() after context_destroy but before coreLib->unload()
        // Renderer must destroy all resources that require DLL function pointers here
        // (destroy_device, etc.). Default no-op for non-Vulkan renderers
        virtual void destroyHwContext() {
        }
    };
} // namespace firelight::libretro
