wgpu_path = ./wgpu-native
wgpu_cpp_path = ./WebGPU-Cpp
sdl_path = ./SDL
include_path = ./src/include
out = a.out

cxx_args = \
	-I$(include_path) \
	-L$(include_path) \
	-Wl,-Bstatic \
	-lwgpu_native \
	-lSDL3 \
	-Wl,-Bdynamic \
	-o $(out)

saxpy = ./src/saxpy/saxpy.cpp
collatz = ./src/collatz/collatz.cpp
adapter = ./src/adapter_info/adapter_info.cpp
triangle = ./src/triangle/triangle.cpp

default: build_all

.PHONY: build_wgpu build_wgpu_headers build_sdl prepare build_all build_saxpy build_collatz build_adapter run_saxpy run_collatz run_adapter cleanup build_triangle run_triangle

build_wgpu:
	-git clone --recursive https://github.com/gfx-rs/wgpu-native
	cd wgpu-native; cargo build --release

build_wgpu_headers:
	-git clone https://github.com/eliemichel/WebGPU-Cpp/

build_sdl:
	-git clone https://github.com/libsdl-org/SDL/
	cd SDL; mkdir -p build
	cd SDL/build; cmake .. -DSDL_STATIC=ON -DSDL_SHARED=OFF -DCMAKE_BUILD_TYPE=Release
	cd SDL/build; make

prepare: build_wgpu build_wgpu_headers build_sdl
	mkdir -p src
	mkdir -p $(include_path)
	cp $(wgpu_path)/ffi/wgpu.h $(include_path)/wgpu.h
	cp $(wgpu_path)/ffi/webgpu-headers/webgpu.h $(include_path)/webgpu.h
	cp $(wgpu_path)/target/release/libwgpu_native.a $(include_path)/libwgpu_native.a
	cp $(wgpu_path)/target/release/libwgpu_native.so $(include_path)/libwgpu_native.so
	cp $(wgpu_cpp_path)/wgpu-native/webgpu.hpp $(include_path)/webgpu.hpp
	cp $(sdl_path)/build/libSDL3.a $(include_path)/libSDL3.a
	cp $(sdl_path)/include/SDL3 $(include_path) -r
	sed -i 's|#include <webgpu/\(.*\)>|#include "\1"|' $(include_path)/webgpu.hpp

cleanup: 
	-rm -rf WebGPU-Cpp
	-rm -rf wgpu-native
	-rm -rf SDL

build_saxpy: 
	g++ $(saxpy) $(cxx_args) -o saxpy.out

build_collatz: 
	g++ $(collatz) $(cxx_args) -o collatz.out

build_adapter: 
	g++ $(adapter) $(cxx_args) -o adapter.out
 
build_triangle:
	g++ $(triangle) $(cxx_args) -o triangle.out

build_all: build_saxpy build_collatz build_adapter build_triangle
	

run_saxpy: build_saxpy
	./saxpy.out

run_collatz: build_collatz
	./collatz.out

run_adapter: build_adapter
	./adapter.out

run_triangle: build_triangle
	SDL_VIDEODRIVER=wayland ./triangle.out