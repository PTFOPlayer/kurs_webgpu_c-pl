#pragma once
#include <iostream>
#include <string>

#include "../include/webgpu.h"
#include "../include/wgpu.h"

class ComputePass {
   private:
    WGPUComputePassEncoder compute_pass_encoder;

   public:
    ComputePass(WGPUComputePassEncoder compute_pass_encoder);

    void set_pipeline(WGPUComputePipeline pipeline);
    void set_bindgroup(size_t bindgroup_id, WGPUBindGroup bind_group);
    void dispatch(size_t x, size_t y, size_t z);
    void finish();
    ~ComputePass();
};