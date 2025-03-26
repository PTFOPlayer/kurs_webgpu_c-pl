#include <iostream>
#include <string>

#include "include/webgpu.h"
#include "include/wgpu.h"
#include "wrapper.hpp"

using namespace std;
const size_t buffer_f32_len = 8192;
const size_t buffer_size = 4 * buffer_f32_len;

int main(int argc, char const *argv[]) {
    WGPUWrapper wrapper = WGPUWrapper();
    WGPUQueue queue = wrapper.get_queue();

    WGPUShaderModule shdr_mod = wrapper.get_shader_module("shader.wgsl");

    WGPUBuffer staging_buffer =
        wrapper.create_buffer("staging_buffer", WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst, buffer_size);

    WGPUBuffer storage_buffer_x = wrapper.create_buffer(
        "storage_buffer_x", WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc, buffer_size);

    WGPUComputePipelineDescriptor compute_desc = {
        .label = {"compute_pipeline", WGPU_STRLEN},
        .compute =
            {
                .module = shdr_mod,
                .entryPoint = {"main", WGPU_STRLEN},
            },
    };

    WGPUComputePipeline compute_pipeline = wgpuDeviceCreateComputePipeline(wrapper.device, &compute_desc);

    WGPUBindGroupLayout bind_group_layout = wgpuComputePipelineGetBindGroupLayout(compute_pipeline, 0);

    WGPUBindGroupEntry entries[] = {(const WGPUBindGroupEntry){
        .binding = 0,
        .buffer = storage_buffer_x,
        .offset = 0,
        .size = buffer_size,
    }};

    WGPUBindGroup bind_group = wrapper.create_compute_bind_group("bind_group", compute_pipeline, 1, entries);

    WGPUCommandEncoder command_encoder = wrapper.create_command_encoder("command_encoder");

    WGPUComputePassEncoder compute_pass_encoder = wrapper.create_compute_pass_encoder("compute_pass", command_encoder);
    
    wgpuComputePassEncoderSetPipeline(compute_pass_encoder, compute_pipeline);
    wgpuComputePassEncoderSetBindGroup(compute_pass_encoder, 0, bind_group, 0, NULL);

    wgpuComputePassEncoderDispatchWorkgroups(compute_pass_encoder, buffer_size, 1, 1);
    wgpuComputePassEncoderEnd(compute_pass_encoder);
    wgpuComputePassEncoderRelease(compute_pass_encoder);

    wgpuCommandEncoderCopyBufferToBuffer(command_encoder, storage_buffer_x, 0, staging_buffer, 0, buffer_size);

    WGPUCommandBufferDescriptor command_buff_desc = {
        .label = {"command_buffer", WGPU_STRLEN},
    };
    WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(command_encoder, &command_buff_desc);

    float x_num[buffer_f32_len] = {0};
    for (size_t i = 0; i < buffer_f32_len; i++) {
        x_num[i] = i;
    }

    wgpuQueueWriteBuffer(queue, storage_buffer_x, 0, x_num, buffer_size);
    wgpuQueueSubmit(queue, 1, &command_buffer);

    wgpuBufferMapAsync(staging_buffer, WGPUMapMode_Read, 0, buffer_size,
                       (const WGPUBufferMapCallbackInfo){
                           .callback = [](WGPUMapAsyncStatus status, WGPUStringView message, void *userdata1,
                                          void *userdata2) { printf(" buffer_map status=%#.8x\n", status); }});
    wgpuDevicePoll(wrapper.device, true, NULL);

    float *buf = (float *)wgpuBufferGetMappedRange(staging_buffer, 0, buffer_size);
    assert(buf);

    for (size_t i = 0; i < min(buffer_f32_len, (size_t)16); i++) {
        cout << buf[i] << "\n";
    }
}
