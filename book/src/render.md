# Render

> kod tworzenia okna jest stworzony pod WAYLAND

Renderowanie znacznie różni się od obliczeń. Wymaga od nas użycia zewnętrznej biblioteki do wyświetlania obrazu, w naszym wypadku użyjemy SDL3.

## Tworzenie powierzchni

Pierwszym krokiem jest stworzenie okna. W konstruktorze okna SDL podajemy nazwę okna, rozmiary (800x600) oraz backend na jakim ma działać okno.
```cpp
    SDL_Window* window = SDL_CreateWindow("SDL3 + WebGPU", 800, 600, SDL_WINDOW_VULKAN);
    if (!window) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return 1;
    }
```

Poniższy kod przedstawia tworzenie okna i powierzchni. 

Z utworzonego okna wyciągamy informację o kanale komunikacji oraz powierzchni do której będziemy renderowali, następnie tworzymy na ich podstawie powierzchnię kompatybilną z webgpu.
```cpp
Surface create_surface(SDL_Window* window, Instance& instance) {
    
    // pobranie informacji na temat okna
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    // kanału kominikacji z wayland
    void* display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    // powierzchnia na której będziemy renderować
    void* surface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        
    // upewnienie 
    if (!display || !surface) {
        std::cerr << "Failed to get Wayland handles from SDL3 window!" << std::endl;
        exit(1);
    }

    // stworzenie deskryptora powierzchni
    WGPUSurfaceSourceWaylandSurface waylandDesc = {};
    waylandDesc.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    waylandDesc.display = display;
    waylandDesc.surface = surface;

    SurfaceDescriptor desc = {};
    desc.nextInChain = (WGPUChainedStruct*)&waylandDesc;


    // zwracamy powierzchnię webgpu
    return instance.createSurface(desc);
}
```

Użycie powyższej funkcji oraz nadanie adapterowi informacji o tym że ma być kompatybilny z daną powierzchnią.

```cpp
    Instance instance = createInstance({});
    
    Surface surface = create_surface(window, instance);
    
    // ustawienie w adapterze kompatybilności z powierzchnią
    RequestAdapterOptions adapter_options(Default);
    adapter_options.compatibleSurface = surface;

    Adapter adapter = instance.requestAdapter(adapter_options);

    Device device = adapter.requestDevice({});
```

Teraz należy skonfigurować powierzchnie w odpowiedni sposób. 

W funkcji która do tego służy musimy ustawić takie pola w configuracji:
* `.usage` - mówi o tym jak będzie używana powierzchnia, `TextureUsage::RenderAttachment` oznacza że będzie wyświetlała obraz
* `.format` - określa format kolorów, my będziemy używać BGRA8
* `.width` i `.height` - określają rozdzielczośc
* `.presentMode` - określa jak kolejne klatki mają przechodzić do ekranu, `PresentMode::Fifo` oznacza że synchronizujemy render z częstotliwością renderowania
* `.device` - określa urządzenie które rysuje na tej powierzchni

```cpp
void configure_surface(Surface& surface, Device device) {
    SurfaceConfiguration config = {};
    config.usage = TextureUsage::RenderAttachment;
    config.format = WGPUTextureFormat_BGRA8Unorm;
    config.width = 800;
    config.height = 600;
    config.presentMode = PresentMode::Fifo;
    config.device = device;
    surface.configure(config);
}
```
Wywołanie tej funkcji wygląda w następujący sposób

```cpp
    configure_surface(surface, device);
```

## Pipeline renderowania

Pierwszym krokiem jest załadowanie shadera, wykonujemy to tak samo jak w przykładach obliczeniowych.

```cpp
    ShaderSourceWGSL source(Default);
    ShaderModule shader_module = create_shader_module(device, &source, "src/triangle/triangle.wgsl");
```

Teraz musimy skonfigurować dwa punkty wejścia, `vertex`, odpowiadający za geometrię, oraz `fragment` odpowiadający za tekstury.

Najpierw tworzymy deskryptor pipelineu

```cpp
    RenderPipelineDescriptor render_desc(Default);
```

Konfiguracja shadera `vertex` jest dosyć prosta. W naszym deskryptorze w polu `.vertex` definiujemy następujące pola:
* `.module` - moduł z którego korzystamy
* `.entryPointe` - punkt wejścia (tak samo jak w przykładach obliczeniowych)
* `.constantCount` - liczba stałych
* `.constants` - stałe, brak to nullptr


```cpp
    render_desc.vertex.module = shader_module;
    render_desc.vertex.entryPoint = {"vertex", WGPU_STRLEN};
    render_desc.vertex.constantCount = 0;
    render_desc.vertex.constants = nullptr;
```

Teraz w polu `.primitive` trzeba zdefiniować w jakiej topologii będą tworzone trójkąty z których będzie składała się geometria: 
* `.topology` - Jaki format mają dane podawane do `vertex` - `PrimitiveTopology::TriangleList` oznacza że będzie to lista trójkątów
* `.stripIndexFormat` - indeksowanie pasków, nie ma znaczcenia przy użyciu listy trójkątów
* `.frontFace` - jak definiowany jest przód trójkąta - `FrontFace::CWW` oznacza że przód jest jeśli punkty na liście idą z kierunkiem odwrotnych do kierunku wskazówek zegara.
* `.cullMode` - czy usuwamy niewidoczne trójkąty
```cpp
    render_desc.primitive.topology = PrimitiveTopology::TriangleList;
    render_desc.primitive.stripIndexFormat = IndexFormat::Undefined;
    render_desc.primitive.frontFace = FrontFace::CCW;
    render_desc.primitive.cullMode = CullMode::None;
```

    BlendState blend_state;
    blend_state.color.srcFactor = BlendFactor::SrcAlpha;
    blend_state.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blend_state.color.operation = BlendOperation::Add;

    blend_state.alpha.srcFactor = BlendFactor::Zero;
    blend_state.alpha.dstFactor = BlendFactor::One;
    blend_state.alpha.operation = BlendOperation::Add;

    ColorTargetState color_target;
    color_target.format = WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;
    color_target.blend = &blend_state;
    color_target.writeMask = ColorWriteMask::All; 

    FragmentState frag_state;

    frag_state.module = shader_module;
    frag_state.entryPoint = {"fragment", WGPU_STRLEN};
    frag_state.constantCount = 0;
    frag_state.constants = nullptr;
    frag_state.targetCount = 1;
    frag_state.targets = &color_target;

    render_desc.fragment = &frag_state;

    render_desc.depthStencil = nullptr;
    render_desc.layout = nullptr;

    RenderPipeline pipeline = device.createRenderPipeline(render_desc);
