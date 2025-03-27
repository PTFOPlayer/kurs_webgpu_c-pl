#include <iostream>
#include <string>

#include "include/webgpu.h"
#include "include/wgpu.h"
#include "wrapper/compute_pass.hpp"
#include "wrapper/wrapper.hpp"

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

    WGPUBuffer storage_buffer_y =
        wrapper.create_buffer("storage_buffer_y", WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst, buffer_size);

    WGPUBuffer storage_buffer_a =
        wrapper.create_buffer("storage_buffer_a", WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst, sizeof(float));

    WGPUComputePipeline compute_pipeline = wrapper.create_compute_pipeline("compute_pipeline", shdr_mod, "main");

    WGPUBindGroupEntry entries[] = {{
                                        .binding = 0,
                                        .buffer = storage_buffer_x,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 1,
                                        .buffer = storage_buffer_y,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 2,
                                        .buffer = storage_buffer_a,
                                        .offset = 0,
                                        .size = sizeof(float),
                                    }};

    WGPUBindGroup bind_group = wrapper.create_compute_bind_group("bind_group", compute_pipeline, 0, 3, entries);

    WGPUCommandEncoder command_encoder = wrapper.create_command_encoder("command_encoder");

    ComputePass compute_pass = wrapper.create_compute_pass_encoder("compute_pass", command_encoder);

    compute_pass.set_pipeline(compute_pipeline);
    compute_pass.set_bindgroup(0, bind_group);

    compute_pass.dispatch(buffer_size, 1, 1);
    compute_pass.finish();

    wgpuCommandEncoderCopyBufferToBuffer(command_encoder, storage_buffer_x, 0, staging_buffer, 0, buffer_size);

    WGPUCommandBufferDescriptor command_buff_desc = {
        .label = {"command_buffer", WGPU_STRLEN},
    };
    WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(command_encoder, &command_buff_desc);

    float x_num[buffer_f32_len] = {0};
    float y_num[buffer_f32_len] = {0};
    float a = 10;
    for (size_t i = 0; i < buffer_f32_len; i++) {
        x_num[i] = i;
        y_num[i] = i * 2;
    }

    wgpuQueueWriteBuffer(queue, storage_buffer_x, 0, x_num, buffer_size);
    wgpuQueueWriteBuffer(queue, storage_buffer_y, 0, y_num, buffer_size);
    wgpuQueueWriteBuffer(queue, storage_buffer_a, 0, &a, sizeof(float));
    wgpuQueueSubmit(queue, 1, &command_buffer);

    wgpuBufferMapAsync(
        staging_buffer, WGPUMapMode_Read, 0, buffer_size,
        (const WGPUBufferMapCallbackInfo){
            .callback = [](WGPUMapAsyncStatus status, WGPUStringView message, void *userdata1, void *userdata2) {}});

    while (!wgpuDevicePoll(wrapper.device, true, NULL)) {
    }

    float *buf = (float *)wgpuBufferGetMappedRange(staging_buffer, 0, buffer_size);
    assert(buf);

    for (size_t i = 0; i < min(buffer_f32_len, (size_t)16); i++) {
        cout << buf[i] << "\n";
    }
}
