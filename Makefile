wgpu_path = ./wgpu-native
wgpu_cpp_path = ./WebGPU-Cpp
include_path = ./src/include
out = a.out

cxx_args = -L$(include_path) \
	-Wl,-Bstatic -lwgpu_native -Wl,-Bdynamic \
	-o $(out)

saxpy = ./src/main_saxpy.cpp
collatz = ./src/main_collatz.cpp

.all: build
.PHONY: prepare build

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

build:
	g++ $(saxpy) $(cxx_args) -o saxpy.out
	g++ $(collatz) $(cxx_args) -o collatz.out
