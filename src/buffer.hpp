#pragma once
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

Buffer create_buffer(Device & device, BufferUsage usage, uint32_t size, string label) {
    BufferDescriptor desc(Default);
    desc.label = {label.data(), WGPU_STRLEN};
    desc.usage = usage;
    desc.size = size;

    return device.createBuffer(desc);
}