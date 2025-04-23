# Ładowanie shaderów

Shader w webgpu to program uruchamiany na GPU, który określa sposób przetwarzania danych. Dla przykładu SAXPY ( Single precision A X plus Y) wykonuje następujące obliczenia:

$$
    z_i = a*x_i + y_i,  \forall i \in 0, N
$$

Gdzie `x` i `y` są wetkrami danych, `a` jest zmienną wspólną a `z` jest wektorem wyjściowym (często jest wykorzystywany ponownie w tym celu wektor `x`)

## Język shaderów

Niestety ale w przeciwieństwie do Cudy czy ROCm, webgpu wykorzystuje własny język do pisania shaderów. Jest to WGSL, został on opracowany we współpracy deweloperów JS, TS i Rust, a więcj dziedziczy też z tych języków składnię i niektóre elementy działania.

Przykład kodu dla SAXPY:
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

fn saxpy(x: f32, y: f32) -> f32 {
    return (a*x) + y;
}

@compute
@workgroup_size(1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    x[global_id.x] = saxpy(x[global_id.x], y[global_id.x]);
}
```

Oczywiście bez wyjaśnienia całość jest conajmniej niezrozumiała.

## Typy

WGSL jest silnie typowanym językiem, to znaczy że wszelkie konwersje typów muszą być definiowane ręcznie oraz typy każdej zmiennej i każdej funkcji muszą być znane lub wnioskowalne na etapie kompilacji.

### Typy proste:
* `i32` - w CPP odpowiada typowi int32_t
* `u32` - w CPP odpowiada typowi uint32_t 
* `f32` - w CPP odpowiada typowi float (float jest zawsze 32 bit)
* `bool` - to samo co bool w cpp

### Typy wektorowe o stałej długości:
* `vec2<t>` - 2 elementowy wektor (xy), generyczny
* `vec3<t>` - 3 elementowy wektor (xyz), generyczny
* `vec4<t>` - 4 elementowy wektor (xyzw), generyczny
* `vec2i`, `vec2u`, `vec2f` - 2 elementowy wektor (xy) zawierający typ i32, u32 lub f32
* `vec3i`, `vec3u`, `vec3f` - 3 elementowy wektor (xyz) zawierający typ i32, u32 lub f32
* `vec4i`, `vec4u`, `vec4f` - 4 elementowy wektor (xyzw) zawierający typ i32, u32 lub f32

Konstruowanie wektora:
```typescript
    let a = vec2f(1.0, 2.0) // tworzy wektor z elementami 1.0 i 2.0
    let b = vec4f(1.0) // wszystkie 4 elementy wektora to 1.0
    let c = vec3f(a, 3.0) // tworzy wektor 3 elementowy kopjując pierwsze dwa elementy z zmiennej `a`
```

### Typy macierzowe 
Istnieją w rozmiarach od 2x2 do 4x4, należy podmienić C i R na konkretny rozmiar
* `matCxR<t>` - macierz generyczna

### Tablice
* `array<t, N>` - tablica ze stałym rozmiarem
* `array<t>` - tablica z rozmiarem definiowanym dynamicznie (nie zaleca się używania poza bindingami)

### Struktury 
Są definiowane w taki sam sposób jak w klasy w TS i struktury w Rust
```typescript
struct SomeStruct {
    field1: u32,
    field2: f32,
    field3: array<i32, 12>
}
```

### Wskaźniki
Typ `ptr`, zazwyczaj ich typ nie jest tworzony ręcznie, działa dokładnie jak pointer w C
```typescript
fn f() {
  var x: f32 = 1.5;
  let px = &x;  // wskaxnik do x
  *px = 3.0;    
  // x to teraz 3.0
}
```



## Binding (polska nazwa "powiązanie" brzmi dziwnie)

Binding jest podstawowym sposobem na przekazywanie danych między różnymi shaderami lub z hostem. 

Konkretne bindingi są w grupach, maksymalnie 4 grupy na urządzenie. Konkretne limity bindingów w danej grupie są różne w zalezności od backendu ale my skupimy się na [Vulkanie](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#limits-minmax), ze względu na kompatybilność wsteczną ten backend ma najbardziej ograniczone ilości buforów.

Z załączonej [specyfikacji](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#limits-minmax) możemy wyczytać że limity wynoszą:
* 12 uniform buffers
* 4 storage buffers
* 16 sampled images (textures)
* 4 storage images
* 16 samplers

Aby zadeklarować binding musimy zdefiniować jego grupę za pomocą `@group(id)` oraz który to binding w tej grupie za pomocą `@binging(id)`. Do tego musimy jeszcze zdefiniować do czego służy dnay binding i czy jest tylko do odczytu (`read`) czy do odczytu i zaispu (`read_write`), robimy to podczas deklaracji zmiennej, np. `var<storage, read_write> <nazwa>: <typ>;` stworzy nam bufor do którego możemy zapisywać dane i z którego możemy odczytywać.

> id musi być zawsze znane na etapie kompilacji

W powyższym przykładzie mamy parę różnych bindingów

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

## Funkcje


### Klasyczne funkcje
Funkcje są podobne do tych w TS i Rust, deklaruje je się przy użyciu `fn`, typ zwaracany funkcji definiuje się za pomocą `->` po argumentach np. funkcja z powyższego przykładu:

```typescript
fn saxpy(x: f32, y: f32) -> f32 {
    return (a*x) + y;
}
```

### "Punkt wejścia"
Zazwyczaj nazywa się `main` ale w WGSL nie ma obowiązku żeby tak się nazywała. Oznaczamy ją odpowiednim znacznikiem przed deklaracją, za pomocą `@compute` dla shaderów obliczeniowych i za pomocą `@vertex` lub `@fragment` dla shaderów renderujących. Do tego w przypadku shaderów obliczeniowych deklaruje się minimalny rozmiar grupy obliczeniowej za pomocą `@workgroup_size(rozmiar grupy...)`, służy to do wykonania optymalizacji i ograniczenia wywołań po stronie procesora. W naszym przypadku użyjemy po prostu `@workgroup_size(1)`, można też zdefiniować to jako `@workgroup_size(1, 1, 1)`, oznacza to to samo, czyli w każdym wymiarze minimalna liczba wywołań to 1 (wymiary wywodzą się z struktury trzystopniowej, patrz [Jak działa karta graficzna](./gpu.md)).


```
@compute
@workgroup_size(1)
fn main(...) {
    ...
}
```

### Argumenty wbudowane

Funkcje oprócz argumentów podawanych z innych funkcji mogą przyjmować argumenty wbudowane, deklaruje sie je za pomocą `@buildin(nazwa)`. Argumenty wbudowane pochodzą z środowiska uruchomieniowego i pozwalają na dostarczenie do "punktu wejścia" (funkcji od której zaczyna pracę dany shader) inforamcji potrzebnych do wykonania operacji na odpowiednich danych. 

Dobrym przykładem argumetów wbudowanych jest `@builtin(global_invocation_id)`. Zwraca on informację o instancji odpalonej funkcji

W powyższym przykładzie wykorzystujemy tą informację do indeksowania buforów.
```typescript
@compute
@workgroup_size(1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    x[global_id.x] = saxpy(x[global_id.x], y[global_id.x]);
}
```

Dokładna lista argumentów wbudowanych jest dostępna w sepcyfikacji webgpu na [w3.org](https://www.w3.org/TR/WGSL/#builtin-inputs-outputs)

## Ładowanie shaderów

Robi się to poprzez tworzenie modułów shadera (`ShaderModule`). Po stronie hosta odczytujemy cały kod konkrentego shadera i na jego bazie tworzymy moduł, w tym czasie shader jest też kompilowany. 

Czemu nie jest kompilowany na etapie kompilacji kodu hosta? Ponieważ nie wiemy jakie jest urządzenie docelowe i jaki jest backend docelowy. Dzięki kompilowaniu kodu shadera dopiero na urządzeniu docelowym zyskujemy znaczny przyrost wydajności kosztem małego opóźnienia przy odpalaniu programu.

Ponieważ będziemy odczytywać moduły w każdym przykładzie zrobimy sobie do tego funkcję. 

> plik: src/shader.hpp
```cpp
#pragma once
#include <fstream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

ShaderModule create_shader_module(Device & device, ShaderSourceWGSL *source, string path) {
    // odczytanie kodu z pliku o podanej scieżce 
    ifstream shader_file = ifstream(path);
    // czarna magia z iteratorami która kopjuje nam kod z `ifstream` do `string`
    string shader((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());
    
    // zapisanie kodu w `ShaderSourceWGSL` (arg 2)
    source->code = {shader.data(), WGPU_STRLEN};

    // stworzenie deskryptora modułu
    ShaderModuleDescriptor shader_module_desc;
    shader_module_desc.nextInChain = (WGPUChainedStruct*)source;

    // tworzymy moduł i go zwracamy(tutaj też odbywa się kompilacja)
    return device.createShaderModule(shader_module_desc);

}
```

Jak użyć powyższego kodu?



```cpp
int main(int argc, char const *argv[]) {
    ...
    // tworzymy zmienną na przechowanie kodu shadera
    ShaderSourceWGSL source(Default);
    // tworzymy moduł używając wcześniej utworzonej funkcji
    ShaderModule shader_module = create_shader_module(device, &source, "src/saxpy/saxpy.wgsl");
}
```