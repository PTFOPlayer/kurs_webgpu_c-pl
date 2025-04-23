# Bufory

W poprzednim dziale poznaliśmy jak implementować bufory po stronie shadera. 

Dla przypomnienia:
```typescript
@group(0)
@binding(0)
var<storage, read_write> x: array<f32>; 
@group(0)
@binding(1)
var<storage, read> y: array<f32>; 
@group(0)
@binding(2)
var<storage, read> a: f32;
```

Teraz musimy je stworzyc po stronie hosta

## Tworzenie buforów
Zazwyczaj jest tworzony więcej niż jeden bufor shader dlatego wygodniej będzie stworzyć funkcję tworzącą bufory, funkcja ta jest dosyć prosta: 

> plik src/buffer.hpp
```cpp
#pragma once
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

Buffer create_buffer(Device & device, BufferUsage usage, uint32_t size, string label) {
    // deskryptor bufora, domyślny
    BufferDescriptor desc(Default);

    // nazwa bufora (używana przy debugowaniu)
    desc.label = {label.data(), WGPU_STRLEN};
    // zastosowanie bufora
    desc.usage = usage;
    // rozmiar bufora
    desc.size = size;

    return device.createBuffer(desc);
}
```

Użycie tej funkcji wygląda następująco:

```cpp
const size_t buffer_f32_len = 8192;
const size_t buffer_size = 4 * buffer_f32_len;

int main(int argc, char const *argv[]) {
    ...

    Buffer staging = create_buffer(device, BufferUsage::MapRead | BufferUsage::CopyDst, buffer_size, "staging");
    
    Buffer x_buffer = create_buffer(device, BufferUsage::Storage | BufferUsage::CopyDst | BufferUsage::CopySrc, buffer_size, "x_buffer");

    Buffer y_buffer = create_buffer(device, BufferUsage::Storage | BufferUsage::CopyDst, buffer_size, "y_buffer");

    Buffer a_buffer = create_buffer(device, BufferUsage::Storage | BufferUsage::CopyDst, sizeof(float), "a_buffer");
}
```

Tworzymy tutaj bufory odpowiadające tym które są zdefiniowane po stronie shadera, ale dodatkowo powstaje nam jeszcze jeden tkzw. "Staging Buffer". Jest to bufor pośredniczący którego używa sie do transferowania danych używanych przez shader. Pozwala on zyskać na prędkości operacji ponieważ dane nie są bezpośrednio mapowane w pamięci hosta.

We wszystkich buforach drugim argumentem jest jego zastosowanie, przekazujemy tutaj typ oraz jakie operacje po stronie hosta mogą być wykonywane na danym buforze:
* `CopyDst` - dane mogą być skopiowane do bufora
* `CopySrc` - dane mogą być kopiowane z bufora
* `MapRead` - dane mogą być odczytane i przekazane do hosta za pomocą mapowania

## Pipeline obliczeniowy (polska nazwa to "potok")

Kolejnym krokiem będzie stworzenie pipelineu obliczeniowego. Służy on do utworzenia grupy bindingów i powiązania z nimi buforów.


Deskryptor pipelineu ma dwa póla które trzeba ustawić:
* compute - określa moduł którego używamy i jego punkt wejścia (funkcja main)
* label (opcjonalny) - nazwa używana w celach debugowania

```cpp

int main(int argc, char const *argv[]) {
    ...
    // deskryptor pipelineu
    ComputePipelineDescriptor compute_pipeline_desc(Default);
    // wskazanie używanego modułu shaderów i punktu wejścia
    compute_pipeline_desc.compute = {.module = shader_module, .entryPoint = {"main", WGPU_STRLEN}};
    // nazwa popelineu (używana przy debugowaniu)
    compute_pipeline_desc.label = {"compute_pipeline", WGPU_STRLEN};
    // tworzenie pipelineu
    ComputePipeline compute_pipeline = device.createComputePipeline(compute_pipeline_desc);
}
```


Teraz przy użyciu tego pipelineu oraz wcześniej utworzonych buforów można utworzyć grupę bindingów. Najpierw utworzymy tablicę wpisów w grupie. 

Struktura wpisu:
* binding - id bindigu w grupie
* buffer - bufor który przypisujemy do tego bindingu
* offset - ilość bajtów którą pomijamy w buforze
* size - rozmiar bufora

Tworzenie tablicy wpisów:
```cpp

int main(int argc, char const *argv[]) {
    ...
    
    WGPUBindGroupEntry entries[] = {{
                                        .binding = 0,
                                        .buffer = x_buffer,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 1,
                                        .buffer = y_buffer,
                                        .offset = 0,
                                        .size = buffer_size,
                                    },
                                    {
                                        .binding = 2,
                                        .buffer = a_buffer,
                                        .offset = 0,
                                        .size = sizeof(float),
                                    }};
}
```

Teraz możemy utworzyć grupę bindingów. W deskryptorze grupy musimy ustawić ilość wpisów oraz tablicę którą podajemy. Potrzebny nam będzie układ grupy tj. `BindGroupLayout` który pobieramy z pipelineu, jest on defniniowany przez pipeline na bazie podanego modułu. 

```cpp

int main(int argc, char const *argv[]) {
    ...
    BindGroupDescriptor bind_group_desc(Default);
    // tablica wpisów
    bind_group_desc.entries = entries;
    // ilośc wpisów
    bind_group_desc.entryCount = 3;
    // układ grupy
    bind_group_desc.layout = compute_pipeline.getBindGroupLayout(0);
    // tworzenie grupy
    BindGroup bind_group = device.createBindGroup(bind_group_desc);

}
```