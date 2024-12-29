#pragma once

#include "libretro/libretro.h"
#include <cstddef>

typedef void (*proc_address_t)();

typedef void (*context_reset_func)();

typedef void (*context_destroy_func)();

namespace firelight::libretro {
    class IVideoDataReceiver {
    public:
        virtual ~IVideoDataReceiver() = default;

        virtual void receive(const void *data, unsigned width, unsigned height,
                             size_t pitch) = 0;

        virtual retro_hw_context_type getPreferredHwRender() = 0;

        virtual void getHwRenderContext(retro_hw_context_type &contextType, unsigned int &major, unsigned int &minor) =
        0;

        virtual proc_address_t getProcAddress(const char *sym) = 0;

        virtual void setResetContextFunc(context_reset_func) = 0;

        virtual void setDestroyContextFunc(context_destroy_func) = 0;

        virtual uintptr_t getCurrentFramebufferId() = 0;

        virtual void setSystemAVInfo(retro_system_av_info *info) = 0;

        virtual void setPixelFormat(retro_pixel_format *format) = 0;

        virtual void setHwRenderContextNegotiationInterface(retro_hw_render_context_negotiation_interface *iface) =
        0;

        virtual void setHwRenderInterface(retro_hw_render_interface **iface) = 0;
    };
} // namespace firelight::libretro
