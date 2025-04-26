#pragma once
#include <fstream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <SDL3/SDL.h>

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

Surface create_surface(SDL_Window* window, Instance& instance) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    void* surface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);

    if (!display || !surface) {
        std::cerr << "Failed to get Wayland handles from SDL3 window" << std::endl;
        exit(1);
    }

    WGPUSurfaceSourceWaylandSurface waylandDesc = {};
    waylandDesc.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    waylandDesc.display = display;
    waylandDesc.surface = surface;

    SurfaceDescriptor desc = {};
    desc.nextInChain = (WGPUChainedStruct*)&waylandDesc;

    return instance.createSurface(desc);
}

void configure_surface(Surface& surface, Device device) {
    SurfaceConfiguration config = {};
    config.usage = TextureUsage::RenderAttachment;
    config.format = WGPUTextureFormat_BGRA8Unorm;
    config.width = 800;
    config.height = 600;
    config.presentMode = PresentMode::Fifo;
    config.device = device;
    surface.configure(config);
}
