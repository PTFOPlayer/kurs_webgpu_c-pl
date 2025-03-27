#include "wrapper.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "../include/webgpu.h"
#include "../include/wgpu.h"
#include "compute_pass.hpp"

WGPUWrapper::~WGPUWrapper() {}

WGPUWrapper::WGPUWrapper() {
    this->instance = wgpuCreateInstance(nullptr);

    wgpuInstanceRequestAdapter(
        instance, nullptr,
        WGPURequestAdapterCallbackInfo{
            .callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message,
                           void *adapter_out, void *) { *(WGPUAdapter *)adapter_out = adapter; },
            .userdata1 = &this->adapter});

    wgpuAdapterRequestDevice(
        adapter, nullptr,
        (const WGPURequestDeviceCallbackInfo){
            .callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *device_out,
                           void *) { *(WGPUDevice *)device_out = device; },
            .userdata1 = &this->device});

    assert(this->instance && this->adapter && this->device);
}

WGPUQueue WGPUWrapper::get_queue() {
    WGPUQueue q = wgpuDeviceGetQueue(this->device);
    assert(q);
    return q;
}

WGPUShaderModule WGPUWrapper::get_shader_module(string path) {
    ifstream shader_file = ifstream(path);
    string shader((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());

    WGPUShaderSourceWGSL source = {
        .chain = {.sType = WGPUSType_ShaderSourceWGSL},
        .code = {shader.data(), WGPU_STRLEN},
    };

    WGPUShaderModuleDescriptor shdr_desc = {
        .nextInChain = (WGPUChainedStruct *)&source,
        .label = {path.data(), WGPU_STRLEN},
    };

    WGPUShaderModule shdr_mod = wgpuDeviceCreateShaderModule(this->device, &shdr_desc);

    return shdr_mod;
}

WGPUBuffer WGPUWrapper::create_buffer(string lablel, WGPUBufferUsage flags, size_t size, bool is_mapped_at_creation) {
    WGPUBufferDescriptor buffer_desc = {
        .label = {lablel.data(), WGPU_STRLEN},
        .usage = flags,
        .size = size,
        .mappedAtCreation = is_mapped_at_creation,
    };
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(this->device, &buffer_desc);

    return buffer;
}

WGPUBindGroup WGPUWrapper::create_compute_bind_group(string label, WGPUComputePipeline compute_pipeline,
                                                     size_t bindgroup_id, size_t entry_count,
                                                     WGPUBindGroupEntry *entries) {
    WGPUBindGroupLayout bind_group_layout = wgpuComputePipelineGetBindGroupLayout(compute_pipeline, bindgroup_id);

    WGPUBindGroupDescriptor bind_group_desc = {
        .label = {label.data(), WGPU_STRLEN}, .layout = bind_group_layout, .entryCount = entry_count, .entries = entries};
    return wgpuDeviceCreateBindGroup(this->device, &bind_group_desc);
}

WGPUComputePipeline WGPUWrapper::create_compute_pipeline(string label, WGPUShaderModule shdr_mod, string entry ) {
    WGPUComputePipelineDescriptor compute_desc = {
        .label = {label.data(), WGPU_STRLEN},
        .compute =
            {
                .module = shdr_mod,
                .entryPoint = {entry.data(), WGPU_STRLEN},
            },
    };
    return wgpuDeviceCreateComputePipeline(this->device, &compute_desc);

}

WGPUCommandEncoder WGPUWrapper::create_command_encoder(string label) {
    WGPUCommandEncoderDescriptor encoder_desc = {
        .label = {label.data(), WGPU_STRLEN},
    };
    return wgpuDeviceCreateCommandEncoder(this->device, &encoder_desc);
}

ComputePass WGPUWrapper::create_compute_pass_encoder(string label, WGPUCommandEncoder command_encoder) {
    WGPUComputePassDescriptor compute_pass_desc = {
        .label = {label.data(), WGPU_STRLEN},
    };
    return ComputePass(wgpuCommandEncoderBeginComputePass(command_encoder, &compute_pass_desc));
}