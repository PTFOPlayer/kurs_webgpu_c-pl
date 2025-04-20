# Przygotowanie projektu

## API WebGPU

Są dwa główne api WebGPU:
1. wgpu-native
2. Dawn

Wgpu-native zostało stworzone przez Mozille, jest najbardziej aktualną i najwydajniejszą implementacją natywnego WebGPU.

Dawn został stworzony przez google, jest trochę prostrzy w użyciu ale jest mniej wydajny.

My będziemy korzystać z wgpu-native.

## Wymagane narzędzia

W będziemy używali paru narzędzi w celu budowania plików wykonywalnych i generacji środowiska

Lista narzędzi:
1. g++ (kompilowanie projektu)
2. make (ułatwienie kompilacji i budowania środowiska)
3. cargo (jest konieczny do zbudowania wgpu-native)
4. cmake (budowa SDL)

## Generowanie środowiska

W celu ułatwienia i przyśpieszenia tworzenia aplikacji z wykorzystaniem webgpu przygotowałem prosty `Makefile` który zawiera parę komend które generują odpowiednie środowisko

Można go znaleźć w repozytorium.

Jeśli wykonamy `make prepare` w katalogu w którym znajduje się nasz `Makefile` zostanie utworzona następująca struktura projektu:

```
|---Makefile
|---src
|   |---include
|       |---libwgpu_native.a
|       |---libwgpu_native.so
|       |---webgpu.h
|       |---webgpu.hpp
|       |---wgpu.h
|---WebGPU-Cpp (headery dla CPP, teoretycznie można usunąć to repozytorium po etapie kopiowania)
|---wgpu-native (tutaj budowane są pliki libwgpu_native.a/.so, również można usunąć po etapie kopiowania)
|---SDL (biblioteka SDL to generowania okna)
```

Następnie wykonanie `make cleanup` sprzątnie nam zbędne rzeczy pozostałe po etapie `prepare`, tj. usunięcie folderu `WebGPU-Cpp` oraz `wgpu-native`

## Pierwszy program

Aby móc używać `WebGPU-Cpp` należy przed inportem biblioteki zdefiniować `WEBGPU_CPP_IMPLEMENTATION`, bez tego nie możemy używać implementacji dla Cpp i mamy tylko dostęp do implementacji dla C która jest inna i bardziej "surowa".

```cpp

#define WEBGPU_CPP_IMPLEMENTATION

#include <webgpu.hpp>

using namespace std;
using namespace wgpu;

int main() {
	...
}
```