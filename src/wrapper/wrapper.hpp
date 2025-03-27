#pragma once
#include <iostream>
#include <string>

#include "../include/webgpu.h"
#include "../include/wgpu.h"
#include "compute_pass.hpp"
#define assert(x)                                                                                              \
    if (!(x)) {                                                                                                \
        std::cout << "assert failed: " << #x << "\non line: " << __LINE__ << "\nfunc: " << __func__ << "\n\n"; \
        exit(-1);                                                                                              \
    }

using namespace std;

class WGPUWrapper {
   public:
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;

    WGPUWrapper();
    WGPUQueue get_queue();
    WGPUShaderModule get_shader_module(string path);
    WGPUBuffer create_buffer(string lable, WGPUBufferUsage flags, size_t size, bool is_mapped_at_creation = false);

    WGPUBindGroup create_compute_bind_group(string label, WGPUComputePipeline compute_pipeline, size_t bindgroup_id,
                                            size_t entry_count, WGPUBindGroupEntry entries[]);
    WGPUComputePipeline create_compute_pipeline(string label, WGPUShaderModule shdr_mod, string entry);

    WGPUCommandEncoder create_command_encoder(string label);

    ComputePass create_compute_pass_encoder(string label, WGPUCommandEncoder command_encoder);
    ~WGPUWrapper();
};
