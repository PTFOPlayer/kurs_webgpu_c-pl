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

## Generowanie środowiska

W celu ułatwienia i przyśpieszenia tworzenia aplikacji z wykorzystaniem webgpu przygotowałem prosty `Makefile` który zawiera parę komend które generują odpowiednie środowisko

```Makefile
wgpu_path = ./wgpu-native
wgpu_cpp_path = ./WebGPU-Cpp
include_path = ./src/include
out = a.out

cxx_args = -L$(include_path) \
	-Wl,-Bstatic -lwgpu_native -Wl,-Bdynamic \
	-o $(out)

saxpy = ./src/main_saxpy.cpp
collatz = ./src/main_collatz.cpp
adapter = ./src/adapter_info.cpp

.all: build_all
.PHONY: prepare build_all build_saxpy build_collatz build_adapter run_saxpy run_collatz run_adapter cleanup

build_wgpu:
	-git clone --recursive https://github.com/gfx-rs/wgpu-native
	cd wgpu-native; cargo build --release

build_wgpu_headers:
	-git clone https://github.com/eliemichel/WebGPU-Cpp/


prepare: build_wgpu build_wgpu_headers
	mkdir -p src
	mkdir -p $(include_path)
	cp $(wgpu_path)/ffi/wgpu.h $(include_path)/wgpu.h
	cp $(wgpu_path)/ffi/webgpu-headers/webgpu.h $(include_path)/webgpu.h
	cp $(wgpu_path)/target/release/libwgpu_native.a $(include_path)/libwgpu_native.a
	cp $(wgpu_path)/target/release/libwgpu_native.so $(include_path)/libwgpu_native.so
	cp $(wgpu_cpp_path)/wgpu-native/webgpu.hpp $(include_path)/webgpu.hpp
	sed -i 's|#include <webgpu/\(.*\)>|#include "\1"|' $(include_path)/webgpu.hpp

cleanup: 
	rm -r WebGPU-Cpp
	rm -r wgpu-native

build_saxpy: 
	g++ $(saxpy) $(cxx_args) -o saxpy.out

build_collatz: 
	g++ $(collatz) $(cxx_args) -o collatz.out

build_adapter: 
	g++ $(adapter) $(cxx_args) -o adapter.out
 
build_all: build_saxpy build_collatz build_adapter
	

run_saxpy: build_saxpy
	./saxpy.out

run_collatz: build_collatz
	./collatz.out

run_adapter: build_adapter
	./adapter.out
```

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
|---WebGPU-Cpp (teoretycznie można usunąć to repozytorium po etapie kopiowania)
|---wgpu-native (tutaj budowane są pliki libwgpu_native.a/.so, również można usunąć po etapie kopiowania)
```

Następnie wykonanie `make cleanup` sprzątnie nam zbędne rzeczy pozostałe po etapie `prepare`, tj. usunięcie folderu `WebGPU-cpp` oraz `wgpu-native`