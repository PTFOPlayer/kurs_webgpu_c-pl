#include <fstream>
#include <iostream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include "../include/webgpu.hpp"

using namespace std;
using namespace wgpu;
const size_t buffer_f32_len = 8192;
const size_t buffer_size = 4 * buffer_f32_len;

int main(int argc, char const *argv[]) {
    Instance instance = createInstance({});
    Adapter adapter = instance.requestAdapter({});
    Device device = adapter.requestDevice({});

    Queue queue = device.getQueue();

    ifstream shader_file = ifstream("src/saxpy/saxpy.wgsl");
    string shader((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());

    ShaderSourceWGSL source(Default);
    source.code = {shader.data(), WGPU_STRLEN};

    ShaderModuleDescriptor shader_module_desc;
    shader_module_desc.nextInChain = (WGPUChainedStruct *)&source;

    ShaderModule shader_module = device.createShaderModule(shader_module_desc);

    BufferDescriptor staging_desc(Default);
    staging_desc.usage = BufferUsage::MapRead | BufferUsage::CopyDst;
    staging_desc.size = buffer_size;

    Buffer staging = device.createBuffer(staging_desc);

    BufferDescriptor x_buffer_desc(Default);
    x_buffer_desc.label = {"x_buffer", WGPU_STRLEN};
    x_buffer_desc.usage = BufferUsage::Storage | BufferUsage::CopyDst | BufferUsage::CopySrc;
    x_buffer_desc.size = buffer_size;

    Buffer x_buffer = device.createBuffer(x_buffer_desc);

    BufferDescriptor y_buffer_desc(Default);
    y_buffer_desc.label = {"y_buffer", WGPU_STRLEN};
    y_buffer_desc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    y_buffer_desc.size = buffer_size;

    Buffer y_buffer = device.createBuffer(y_buffer_desc);

    BufferDescriptor a_buffer_desc(Default);
    a_buffer_desc.label = {"a_buffer", WGPU_STRLEN};
    a_buffer_desc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    a_buffer_desc.size = sizeof(float);

    Buffer a_buffer = device.createBuffer(a_buffer_desc);

    ComputePipelineDescriptor compute_pipeline_desc(Default);
    compute_pipeline_desc.compute = {.module = shader_module, .entryPoint = {"main", WGPU_STRLEN}};
    compute_pipeline_desc.label = {"compute_pipeline", WGPU_STRLEN};
    ComputePipeline compute_pipeline = device.createComputePipeline(compute_pipeline_desc);

    WGPUBindGroupEntry entries[] = {{
                                        .binding = 0,
                                        .buffer = x_buffer,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 1,
                                        .buffer = y_buffer,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 2,
                                        .buffer = a_buffer,
                                        .offset = 0,
                                        .size = sizeof(float),
                                    }};
    BindGroupDescriptor bind_group_desc(Default);
    bind_group_desc.entries = entries;
    bind_group_desc.entryCount = 3;
    bind_group_desc.layout = compute_pipeline.getBindGroupLayout(0);
    BindGroup bind_group = device.createBindGroup(bind_group_desc);

    CommandEncoderDescriptor encoder_desc(Default);
    CommandEncoder encoder = device.createCommandEncoder(encoder_desc);

    ComputePassEncoder compute_pass_encoder = encoder.beginComputePass({});

    compute_pass_encoder.setPipeline(compute_pipeline);
    compute_pass_encoder.setBindGroup(0, bind_group, 0, nullptr);
    compute_pass_encoder.dispatchWorkgroups(buffer_f32_len, 1, 1);
    compute_pass_encoder.end();

    encoder.copyBufferToBuffer(x_buffer, 0, staging, 0, buffer_size);

    CommandBuffer command_buffer = encoder.finish();

    float x_num[buffer_f32_len] = {0};
    float y_num[buffer_f32_len] = {0};
    float a = 3;
    for (size_t i = 0; i < buffer_f32_len; i++) {
        x_num[i] = i;
        y_num[i] = 3 * i;
    }

    queue.writeBuffer(x_buffer, 0, x_num, buffer_size);
    queue.writeBuffer(y_buffer, 0, y_num, buffer_size);
    queue.writeBuffer(a_buffer, 0, &a, sizeof(float));

    queue.submit(command_buffer);

    BufferMapCallbackInfo map_callback(Default);
    map_callback.callback = [](WGPUMapAsyncStatus, WGPUStringView, void *, void *) {};
    staging.mapAsync(MapMode::Read, 0, buffer_size, map_callback);

    while (!device.poll(true, nullptr)) {
    }

    float *returned = (float *)staging.getMappedRange(0, buffer_size);

    for (size_t i = 0; i < min(buffer_f32_len, (size_t)16); i++) {
        cout << returned[i] << " ";
    }
}
