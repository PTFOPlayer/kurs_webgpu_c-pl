#include <fstream>
#include <iostream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>
#include "../shader.hpp"
#include "../buffer.hpp"

using namespace std;
using namespace wgpu;
const size_t buffer_u32_len = 8192;
const size_t buffer_size = 4 * buffer_u32_len;

int main(int argc, char const *argv[]) {
    Instance instance = createInstance({});
    Adapter adapter = instance.requestAdapter({});
    Device device = adapter.requestDevice({});

    Queue queue = device.getQueue();

    ShaderSourceWGSL source(Default);
    ShaderModule shader_module = create_shader_module(device, &source, "src/collatz/collatz.wgsl");

    Buffer staging = create_buffer(device, BufferUsage::MapRead | BufferUsage::CopyDst, buffer_size, "staging");
    
    Buffer output_buffer = create_buffer(device, BufferUsage::Storage | BufferUsage::CopyDst | BufferUsage::CopySrc, buffer_size, "output_buffer");

    Buffer input_buffer = create_buffer(device, BufferUsage::Storage | BufferUsage::CopyDst, buffer_size, "input_buffer");

    ComputePipelineDescriptor compute_pipeline_desc(Default);
    compute_pipeline_desc.compute = {.module = shader_module, .entryPoint = {"main", WGPU_STRLEN}};
    compute_pipeline_desc.label = {"compute_pipeline", WGPU_STRLEN};
    ComputePipeline compute_pipeline = device.createComputePipeline(compute_pipeline_desc);

    WGPUBindGroupEntry entries[] = {
        {
            .binding = 0,
            .buffer = output_buffer,
            .offset = 0,
            .size = buffer_size,
        },
        {
            .binding = 1,
            .buffer = input_buffer,
            .offset = 0,
            .size = buffer_size,
        },
    };
    BindGroupDescriptor bind_group_desc(Default);
    bind_group_desc.entries = entries;
    bind_group_desc.entryCount = 2;
    bind_group_desc.layout = compute_pipeline.getBindGroupLayout(0);
    BindGroup bind_group = device.createBindGroup(bind_group_desc);

    CommandEncoderDescriptor encoder_desc(Default);
    CommandEncoder encoder = device.createCommandEncoder(encoder_desc);

    ComputePassEncoder compute_pass_encoder = encoder.beginComputePass({});

    compute_pass_encoder.setPipeline(compute_pipeline);
    compute_pass_encoder.setBindGroup(0, bind_group, 0, nullptr);
    compute_pass_encoder.dispatchWorkgroups(buffer_u32_len, 1, 1);
    compute_pass_encoder.end();

    encoder.copyBufferToBuffer(output_buffer, 0, staging, 0, buffer_size);

    CommandBuffer command_buffer = encoder.finish();

    uint32_t in[buffer_u32_len] = {0};
    for (size_t i = 0; i < buffer_u32_len; i++) {
        in[i] = i;
    }

    queue.writeBuffer(input_buffer, 0, in, buffer_size);

    queue.submit(command_buffer);

    BufferMapCallbackInfo map_callback(Default);
    map_callback.callback = [](WGPUMapAsyncStatus, WGPUStringView, void *, void *) {};
    staging.mapAsync(MapMode::Read, 0, buffer_size, map_callback);

    while (!device.poll(true, nullptr)) {
    }

    uint32_t *returned = (uint32_t *)staging.getMappedRange(0, buffer_size);

    for (size_t i = 0; i < min(buffer_u32_len, (size_t)16); i++) {
        cout << in[i] << " : " << returned[i] << "\n";
    }
}
