#include "compute_pass.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "../include/webgpu.h"
#include "../include/wgpu.h"
#include "wrapper.hpp"

ComputePass::ComputePass(WGPUComputePassEncoder compute_pass_encoder) : compute_pass_encoder(compute_pass_encoder) {}

void ComputePass::set_pipeline(WGPUComputePipeline pipeline) {
    wgpuComputePassEncoderSetPipeline(this->compute_pass_encoder, pipeline);
}
void ComputePass::set_bindgroup(size_t bindgroup_id, WGPUBindGroup bind_group) {
    wgpuComputePassEncoderSetBindGroup(this->compute_pass_encoder, 0, bind_group, 0, NULL);
}

void ComputePass::dispatch(size_t x, size_t y, size_t z) {
    wgpuComputePassEncoderDispatchWorkgroups(this->compute_pass_encoder, x, y, z);
}

void ComputePass::finish() {
    wgpuComputePassEncoderEnd(this->compute_pass_encoder);
    wgpuComputePassEncoderRelease(this->compute_pass_encoder);
    this->~ComputePass();
}

ComputePass::~ComputePass() {}
