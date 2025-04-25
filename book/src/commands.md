# Enkoder komend i kolejka

Aby przekazać GPU informacje na temat działań jakie ma wykonać muszą one być odpowiednio zakodowane. Do tego służy enkoder komend

## Enkoder

Tworzenie enkodera jest bardzo proste, deskryptory enkodera ogólnego i enkodera obliczeniowego posiadają tylko pole `label` które jest używane do ustawienia nazwy do debugowania, u nas występuje tylko po jednym z każdych enkoderów więc w obu przypadkach pole możemy pominąć

```cpp    
    CommandEncoder encoder = device.createCommandEncoder({});
    ComputePassEncoder compute_pass_encoder = encoder.beginComputePass({});
```

W enkoderze obliczeniowym trzeba ustawić 3 konkretne rzeczy zanim będzie można go przekazać do kolejki do wykonania

1. Ustawienie pipelineu
```cpp
    // pipeline który wcześniej stworzyliśmy
    compute_pass_encoder.setPipeline(compute_pipeline);
```
2. Ustawienie grupy bindignów
   * arg1: id grupy - 0
   * arg2: zmienna zawierająca informacje o grupie
   * arg3: przesunięcia dynamiczne - pomijamy więcj 0 (używane przy bardzo dużych buforach gdzie chcemy przekazywać tylko część bufora a nie całość)
   * arg4: wskaźnik na tablicę przesunięć - pomijamy, nullptr
```cpp
    // ustawienie grupy którą utworzyliśmy
    compute_pass_encoder.setBindGroup(0, bind_group, 0, nullptr);
```
3. Ustawienie na jakiej liczbie rdzeni uruchamiamy nasz shader

    Jeżeli byśmy mieli w shaderze ustawiony `@workgroup_size` inny niż 1 to należy wszystkie wymiary podawane w tym miejscu podzielić przez odpowiadające im wartości w `@workgroup_size`
```cpp
    compute_pass_encoder.dispatchWorkgroups(buffer_f32_len, 1, 1);
```

Teraz trzeba zakończyć enkoder obliczeniowy (ważne żeby to zrobić przed kolejnym użyciem enkodera ogólnego).
```cpp
    compute_pass_encoder.end();
```
---

Ostatnim krokiem będzie zakodowanie informacji o konieczności skopiowania danych z bufora `x` do `staging`.
```cpp 
    encoder.copyBufferToBuffer(x_buffer, 0, staging, 0, buffer_size);
```

Tearz możemy zakończyć kodowanie i wyciągnąć z enkodera zakodowany bufor komend.
```cpp
    CommandBuffer command_buffer = encoder.finish();
```

## Kolejka

Rolą kolejki jest przekazanie danych oraz komend z hosta do urządzenia. 

```cpp
    // tworzenie buforów po stronie hosta i zapełnianie ich danymi
    float x_num[buffer_f32_len] = {0};
    float y_num[buffer_f32_len] = {0};
    float a = 3;
    for (size_t i = 0; i < buffer_f32_len; i++) {
        x_num[i] = i;
        y_num[i] = 3 * i;
    }

    // tworzenie kolejki
    Queue queue = device.getQueue();

    // skopiowanie danych z hosta do urządzenia
    queue.writeBuffer(x_buffer, 0, x_num, buffer_size);
    queue.writeBuffer(y_buffer, 0, y_num, buffer_size);
    queue.writeBuffer(a_buffer, 0, &a, sizeof(float));

    queue.submit(command_buffer);

```

Kopiowanie danych odbywa się za pomocą metody `.writeBuffer`. Przyjmuje ona jako argumenty:
* arg1: bufor po stronie urządzeni
* arg2: offset po jakim zaczynamy wpisywanie
* arg3: bufor po tsronie hosta
* arg4: ile bajtów chcemy przekazać

Przekazywanie buforu komend oraz zakończenie (możemy wykonać submit ponownie z nowym buforem komend) kolejki wykonywane za pomocą metody `.submit` która przyjmuje bufor komend. Ten krok powoduje wykonanie się kodu shadera na urządzeniu.

## Odczyta danych

Teraz jak shader został wykonany możemy odczytać dane po obliczeniach. Zrobimy to z użyciem bufora pośredniczacego `staging`. Aby móc poprawnie go odczytać trzeba go zmapować. Mapowanie jest wykonywane asynchronicznie ponieważ urządzenie potrzebuje czasu na przekazanie danych. Jako argument przyjmuje strukture zawierającą lambdę "C style" (tzn nie może mieć przekazanych argumentów z środowiska poprzez []). W następnym kroku odczekujemy na zakończenie mapowania więc możemy zostawić lambdę pustą (ale musi ona istnieć).  


```cpp
    BufferMapCallbackInfo map_callback(Default);
     
    // mapowanie buforów 
    map_callback.callback = [](WGPUMapAsyncStatus, WGPUStringView, void *, void *) {};
    staging.mapAsync(MapMode::Read, 0, buffer_size, map_callback);

    // oczekiwanie na zakończenie się procesów po stronie GPU
    while (!device.poll(true, nullptr)) {}
```

Teraz możemy odczytać dane i je wyświetlić w terminalu. Odczyt wykonujemy za pomocą metody `.getMappedRange` na buforze pośredniczącym
```cpp
    // zbieranie danych z bufora
    float *returned = (float *)staging.getMappedRange(0, buffer_size);

    // wyświetlanie danych, max 16 elementów żeby nie zasyfić sobie terminala
    for (size_t i = 0; i < min(buffer_f32_len, (size_t)16); i++) {
        cout << returned[i] << " ";
    }
```
