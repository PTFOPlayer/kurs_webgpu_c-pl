#include <fstream>
#include <iostream>
#include <string>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu.hpp>

#include <SDL3/SDL.h>

#include "../surface.hpp"
#include "../shader.hpp"
using namespace std;
using namespace wgpu;


int main(int argc, char const *argv[]) {
	SDL_Window* window = SDL_CreateWindow("SDL3 + WebGPU", 800, 600, SDL_WINDOW_VULKAN);
    if (!window) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return 1;
    }
    
    Instance instance = createInstance({});
    
    Surface surface = create_surface(window, instance);
    
    RequestAdapterOptions adapter_options(Default);
    adapter_options.compatibleSurface = surface;

    Adapter adapter = instance.requestAdapter(adapter_options);
    Device device = adapter.requestDevice({});

    configure_surface(surface, device);

    ShaderSourceWGSL source(Default);
    ShaderModule shader_module = create_shader_module(device, &source, "src/triangle/triangle.wgsl");

    RenderPipelineDescriptor render_desc(Default);
    render_desc.vertex.module = shader_module;
    render_desc.vertex.entryPoint = {"vertex", WGPU_STRLEN};
    render_desc.vertex.constantCount = 0;
    render_desc.vertex.constants = nullptr;

    render_desc.primitive.topology = PrimitiveTopology::TriangleList;
    render_desc.primitive.stripIndexFormat = IndexFormat::Undefined;
    render_desc.primitive.frontFace = FrontFace::CCW;
    render_desc.primitive.cullMode = CullMode::None;

    BlendState blend_state;
    blend_state.color.srcFactor = BlendFactor::SrcAlpha;
    blend_state.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blend_state.color.operation = BlendOperation::Add;

    blend_state.alpha.srcFactor = BlendFactor::Zero;
    blend_state.alpha.dstFactor = BlendFactor::One;
    blend_state.alpha.operation = BlendOperation::Add;

    ColorTargetState color_target;
    color_target.format = WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;
    color_target.blend = &blend_state;
    color_target.writeMask = ColorWriteMask::All; 

    FragmentState frag_state;

    frag_state.module = shader_module;
    frag_state.entryPoint = {"fragment", WGPU_STRLEN};
    frag_state.constantCount = 0;
    frag_state.constants = nullptr;
    frag_state.targetCount = 1;
    frag_state.targets = &color_target;

    render_desc.fragment = &frag_state;

    render_desc.depthStencil = nullptr;
    render_desc.layout = nullptr;

    RenderPipeline pipeline = device.createRenderPipeline(render_desc);

    Queue queue = device.getQueue();

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    
        SurfaceTexture surface_texture;
        surface.getCurrentTexture(&surface_texture);
        Texture texture = surface_texture.texture;
        TextureViewDescriptor view_desc = {};
        view_desc.baseMipLevel = 0;
        view_desc.mipLevelCount = 1;
        view_desc.baseArrayLayer = 0;
        view_desc.arrayLayerCount = 1;
        view_desc.dimension = TextureViewDimension::_2D;
        view_desc.format = WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;
        view_desc.aspect = TextureAspect::All;
        TextureView texture_view = texture.createView(view_desc);
    
        CommandEncoder command_encoder = device.createCommandEncoder({});
        RenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        RenderPassColorAttachment att(Default);
        att.view = texture_view;
        att.loadOp = LoadOp::Clear;
        att.storeOp = StoreOp::Store;
        att.clearValue = Color{0.0f, 0.0f, 0.0f, 1.0f};
        render_pass_desc.colorAttachments = &att;
    
        RenderPassEncoder pass = command_encoder.beginRenderPass(render_pass_desc);
        pass.setPipeline(pipeline);
        pass.draw(3, 1, 0, 0);
        pass.end();
    
        CommandBuffer command_buffer = command_encoder.finish();
        queue.submit(command_buffer);
        surface.present();
    }

    return 0;
}

