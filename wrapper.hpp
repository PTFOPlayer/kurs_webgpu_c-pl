#include <iostream>
#include <string>

#include "include/webgpu.h"
#include "include/wgpu.h"
#define assert(x)                                                         \
    if (!(x)) {                                                           \
        std::cout << "assert failed: " << #x << "\non line: " << __LINE__ \
                  << "\nfunc: " << __func__ << "\n\n";                    \
        exit(-1);                                                         \
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
    ~WGPUWrapper();
};

WGPUWrapper::~WGPUWrapper() {}

WGPUWrapper::WGPUWrapper() {
    this->instance = wgpuCreateInstance(nullptr);

    wgpuInstanceRequestAdapter(
        instance, nullptr,
        WGPURequestAdapterCallbackInfo{
            .callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter,
                           WGPUStringView message, void *adapter_out,
                           void *) { *(WGPUAdapter *)adapter_out = adapter; },
            .userdata1 = &this->adapter});

    wgpuAdapterRequestDevice(
        adapter, nullptr,
        (const WGPURequestDeviceCallbackInfo){
            .callback = [](WGPURequestDeviceStatus status, WGPUDevice device,
                           WGPUStringView message, void *device_out,
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
    string shader((std::istreambuf_iterator<char>(shader_file)),
                  std::istreambuf_iterator<char>());

    WGPUShaderSourceWGSL source = {
        .chain = {.sType = WGPUSType_ShaderSourceWGSL},
        .code = {&shader[0], WGPU_STRLEN},
    };

    WGPUShaderModuleDescriptor shdr_desc = {
        .nextInChain = (WGPUChainedStruct *)&source,
        .label = {.data = &path[0], .length = WGPU_STRLEN},
    };

    WGPUShaderModule shdr_mod =
        wgpuDeviceCreateShaderModule(this->device, &shdr_desc);

    return shdr_mod;
}