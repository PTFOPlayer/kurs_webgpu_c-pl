# Instancja, adapter i urządzenie

## Instancja

Instancja jest strukturą definiującą czy dany program aktualnie używa WGPU. Powinna być tworzona tylko jedna na program i na końcu jego działania zwalniana. 

Tworzenie instancji:
```cpp
int main() {
    Instance instance = createInstance({});
}
```

## Adapter

Adapter przechowuje informacje o tym którego urządzenia używamy, za pomocą jakiego backendu się komunikujemy oraz jakie możliwości i limity ma dano urządzenie. Adapter jest tworzony na bazie instancji, podczas tworzenia możemy określić z jakiego typu urządzenia preferujemy korzystać lub jakich możliwości oczekujemy od tego urządzenia.

Adapter przyjmuje w swoim deskryptorze (`RequestAdapterOptions`) kilka argumentów:
| Typ i pole                            | Opis                                                               | Wartość domyślna                                  |
| ------------------------------------- | ------------------------------------------------------------------ | ------------------------------------------------- |
| `WGPUFeatureLevel featureLevel`       | Określa jakich możliwości wymaga adapter                           | domyślnie zaadaptuje się do dostępnego urządzenia |
| `WGPUPowerPreference powerPreference` | Okreśja w jakim profilu energetycznym ma sie znajdować urządzenie  | domyślnie bierze aktualny profil systemu          |
| `WGPUBool forceFallbackAdapter`       | Okreśja czy dany adapter ma posiadać adapter awaryjny              | domyślnie false                                   |
| `WGPUBackendType backendType`         | Okreśja jakiego backendu chcemy od adaptera                        | domyślnie null                                    |
| `WGPUSurface compatibleSurface`       | Określa z jaką powierzchnią do renderowania ma mieć kompatybilność | domyślnie null                                    |

Jeśli naprzykład chcemy stworzyć adapter z domyślnymi ustawieniami możemy podać pusty deskryptor `{}`;

Tworzenie adaptera z domyślnym deskryptorem:
```cpp

int main() {
    ...
    Adapter adapter = instance.requestAdapter({});
}
```

Tworzenie adaptera z kompatybilnego z powierzchnią:
```cpp

int main() {
    ...
    tworzenie instancji, okna oraz powierzchni do jakiej będziemy renderować
    ...

    RequestAdapterOptions adapter_options(Default);
    adapter_options.compatibleSurface = surface;

    Adapter adapter = instance.requestAdapter(adapter_options);

}
```


### Informacje o adapterze
> kompletny kod w przykładzie: src/adapter_info/adapter_info.cpp
Z adaptera można wydostać znaczącą ilość danych, między innymi ograniczenia urządzenia, maksymalne rozmiary tekstur, informacje o producencie i modelu urządzenia oraz bitmapę możliwości urządzenia. 

Przykład wydostawania danych z urządzenia:
```cpp
#include <iostream>
#include <string>
#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

int main(int argc, char const *argv[]) {
    Instance instance = createInstance({});
    Adapter adapter = instance.requestAdapter({});
    
    AdapterInfo adapter_info;
    adapter.getInfo(&adapter_info);
    
    ...
}
```

W tym momencie w strukturze `AdapterInfo` znajdują się wszystkie zebrane informacje. 

Takie dane jak typ urządzenia czy typ backendu są przechowywane jako enum, w takim wypadku trzeba użyć *switch case* do wydostania danych (indeksowanie się enumem do tablicy nie pomoże, wartości nie są pokolei).

### Typy urządzeń

Typ urządzenia nie ma dla nas większego znaczenia dopóki nie wpływa on na wydajność. Zazwyczaj dostępne jest urządzenie które jest albo *dGPU* albo *iGPU*, czasem jednak z różnych powodów mogą być one niedostepne (np. błąd drivera) lub może ich fizczyni nie być (np. serwery z samym procesorem). Jeżeli zależy nam na wydajności powinniśmy unikać odpalania programu jeśli nie znajdzie urządzenia które jest GPU.

Dostępne typy urządzeń:
| Adapter | opis                                                                                                                                 |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| dGPU    | Dedykowana karta graficzna (oddzielna)                                                                                               |
| iGPU    | Karta zintegrowana w procesor                                                                                                        |
| CPU     | Procesor, używany w przypadku kiedy nie ma możliwości użycia żadnego innego urządzenia w celu renderowania                           |
| Unknown | Urządzenie które nie zostało rozpoznane jako żaden z typów, urządzenie nadal nadaje się do renderowania tylko nieznany jest jego typ |

Aktualnie używany typ urządzenia:
```cpp
int main() {
    ...
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

    ...
}
```

### Typy backendów

Urządzenia obsługują szereg backendów, niestety nie wszystkie wspierają te same. Zdarza się również że niektóre urządzenia lepiej działają z jednym backendem. Z tych powodów Webgpu jest wstanie wybierać z którego backendu będzie korzystało aby najlepiej się dostosować.

Typy backendów:
| Backend   | Opis                                                                                                                        |
| --------- | --------------------------------------------------------------------------------------------------------------------------- |
| Undefined | Niezdefiniowany, ten typ backendu oznacza że backend jeszcze nie został wybrany                                             |
| Null      | Jedynie do testowania, nie wykorzystuje się go w praktyce                                                                   |
| WebGPU    | Dla środowisk webowych gdzie webgpu komunikuje się z przeglądarką która już korzysta z jakiegoś backendu                    |
| D3D11     | DirectX 11, tylko dla Windows                                                                                               |
| D3D12     | DirectX 12, tylko dla Windows, preferowany nad D3D11                                                                        |
| Metal     | Dla MacOS                                                                                                                   |
| Vulkan    | Międzyplatformowy backend, jest priorytetowy jeśli inny backend nie jest wybrany ze względów wydajności lub kompatybilności |
| OpenGL    | Kompatybilność na wyższym poziomie niż Vulkan, często jest wybierany na kartach czysto obliczeniowych (np. Nvidia GRID)     |
| OpenGLES  | Wersja OpenGL dla systemów embedded                                                                                         |

Aktualnie używany backend:
```cpp
int main() {
    ...
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
        default:
            break;
    }
    ...
}
```

### Limity

Limity są bardzo prostą strukturą, znajdują się tu tylko 4 pola:
1. maxTextureDimension1D - maksymalny rozmiar textur 1D
2. maxTextureDimension2D - maksymalny rozmiar textur 2D (tj dla wartości 1024 textura może być 1024x1024)
3. maxTextureDimension3D - maksymalny rozmiar textur 3D (tj dla wartości 1024 textura może być 1024x1024x1024)
4. maxTextureArrayLayers - maksymalna długość tablicy warstw textur

```cpp

int main() {
    ...

    Limits limits;
    adapter.getLimits(&limits);

    cout << "Adapter limits:\n";
    cout << "\tmax texture Dimension1D: " << limits.maxTextureDimension1D << "\n";
    cout << "\tmax texture Dimension2D: " << limits.maxTextureDimension2D << "\n";
    cout << "\tmax texture Dimension3D: " << limits.maxTextureDimension3D << "\n";
    cout << "\tmax texture ArrayLayers: " << limits.maxTextureArrayLayers << "\n";

    ...
}
```

## Urządzenie

`Device` jest dla nas wskaźnikiem na konkretne urządzenie w danym adapterze. Będziemy go używać do tworzenie buforów, tworzenia kolejki zadań oraz wykonywania programów. Konkretne funkcje będą opisane w następnych częściach.

```cpp
int main() {
    ...
    Device device = adapter.requestDevice({});
    ...
}
```