wgpu_path = ./wgpu-native
include_path = ./include
.all: build
.PHONY: prepare build

prepare: 
	mkdir -p include
	cp $(wgpu_path)/ffi/wgpu.h $(include_path)/wgpu.h
	cp $(wgpu_path)/ffi/webgpu-headers/webgpu.h $(include_path)/webgpu.h
	cp $(wgpu_path)/target/release/libwgpu_native.a $(include_path)/libwgpu_native.a
	cp $(wgpu_path)/target/release/libwgpu_native.so $(include_path)/libwgpu_native.so

build:
	g++ main.cpp -L$(include_path) -Wl,-Bstatic -lwgpu_native -Wl,-Bdynamic -o a.out

run: build
	./a.out
