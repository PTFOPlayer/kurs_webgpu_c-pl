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
.PHONY: prepare build_all build_saxpy build_collatz build_adapter run_saxpy run_collatz run_adapter

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