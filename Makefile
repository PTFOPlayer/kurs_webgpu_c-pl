wgpu_path = ./wgpu-native
include_path = ./src/include
out = a.out

cxx_args = -L$(include_path) \
	-Wl,-Bstatic -lwgpu_native -Wl,-Bdynamic \
	-o $(out)

cpp_files = $(shell find ./src -name '*.cpp')

.all: build
.PHONY: prepare build

build_wgpu:
	-git clone --recursive https://github.com/gfx-rs/wgpu-native
	cd wgpu-native; cargo build --release

prepare: build_wgpu 
	mkdir -p src
	mkdir -p $(include_path)
	cp $(wgpu_path)/ffi/wgpu.h $(include_path)/wgpu.h
	cp $(wgpu_path)/ffi/webgpu-headers/webgpu.h $(include_path)/webgpu.h
	cp $(wgpu_path)/target/release/libwgpu_native.a $(include_path)/libwgpu_native.a
	cp $(wgpu_path)/target/release/libwgpu_native.so $(include_path)/libwgpu_native.so

build:
	g++ $(cpp_files) $(cxx_args)

run: build
	./a.out