#pragma once
#include <fstream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

ShaderModule create_shader_module(Device & device, ShaderSourceWGSL *source, string path) {
    ifstream shader_file = ifstream(path);
    string shader((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());
    
    source->code = {shader.data(), WGPU_STRLEN};

    ShaderModuleDescriptor shader_module_desc;
    shader_module_desc.nextInChain = (WGPUChainedStruct*)source;

    return device.createShaderModule(shader_module_desc);

}