#include <fstream>
#include <iostream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include "../include/webgpu.hpp"

using namespace std;
using namespace wgpu;

int main(int argc, char const *argv[]) {
    Instance instance = createInstance({});
    Adapter adapter = instance.requestAdapter({});
    Device device = adapter.requestDevice({});

    AdapterInfo adapter_info;
    adapter.getInfo(&adapter_info);

    switch (adapter_info.adapterType) {
        case WGPUAdapterType_DiscreteGPU:
            cout << "Adapter Type: dGPU" << "\n";
            break;
        case WGPUAdapterType_IntegratedGPU:
            cout << "Adapter Type: iGPU" << "\n";
            break;
        case WGPUAdapterType_CPU:
            cout << "Adapter Type: CPU" << "\n";
            break;
        case WGPUAdapterType_Unknown:
            cout << "Adapter Type: Unknown" << "\n";
            break;
        case WGPUAdapterType_Force32:
            cout << "Adapter Type: Force32" << "\n";
            break;
        default:
            break;
    }

    switch (adapter_info.backendType) {
        case WGPUBackendType_Undefined:
            cout << "Backend Type: Undefined" << "\n";
            break;
        case WGPUBackendType_Null:
            cout << "Backend Type: Null" << "\n";
            break;
        case WGPUBackendType_WebGPU:
            cout << "Backend Type: WebGPU" << "\n";
            break;
        case WGPUBackendType_D3D11:
            cout << "Backend Type: D3D11" << "\n";
            break;
        case WGPUBackendType_D3D12:
            cout << "Backend Type: D3D12" << "\n";
            break;
        case WGPUBackendType_Metal:
            cout << "Backend Type: Metal" << "\n";
            break;
        case WGPUBackendType_Vulkan:
            cout << "Backend Type: Vulkan" << "\n";
            break;
        case WGPUBackendType_OpenGL:
            cout << "Backend Type: OpenGL" << "\n";
            break;
        case WGPUBackendType_OpenGLES:
            cout << "Backend Type: OpenGLES" << "\n";
            break;
        case WGPUBackendType_Force32:
            cout << "Backend Type: Force32" << "\n";
            break;
        default:
            break;
    }
    cout << "GPU:\n";
    cout << "\t" << adapter_info.vendor.data << "\n";
    cout << "\t" << adapter_info.description.data << "\n";
    cout << "\t" <<  adapter_info.device.data << "\n";
    cout << "\tDevID: " <<  adapter_info.deviceID << "\n";

    Limits limits;
    adapter.getLimits(&limits);

    cout << "Adapter limits:\n";
    cout << "\tmax texture Dimension1D: " << limits.maxTextureDimension1D << "\n";
    cout << "\tmax texture Dimension2D: " << limits.maxTextureDimension2D << "\n";
    cout << "\tmax texture Dimension3D: " << limits.maxTextureDimension3D << "\n";
    cout << "\tmax texture ArrayLayers: " << limits.maxTextureArrayLayers << "\n";

    
    return 0;
}
