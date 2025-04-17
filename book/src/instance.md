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

Adapter przyjmuje w swoim deskryptorze (``) kilka argumentów:
| Typ i pole                            | Opis                                                              | Wartość domyślna                                  |
| ------------------------------------- | ----------------------------------------------------------------- | ------------------------------------------------- |
| `WGPUFeatureLevel featureLevel`       | Określa jakich możliwości wymaga adapter                          | domyślnie zaadaptuje się do dostępnego urządzenia |
| `WGPUPowerPreference powerPreference` | Okreśja w jakim profilu energetycznym ma sie znajdować urządzenie | domyślnie bierze aktualny profil systemu          |
| `WGPUBool forceFallbackAdapter`       | Okreśja czy dany adapter ma posiadać adapter awaryjny             | domyślnie false                                   |
| `WGPUBackendType backendType`         | Okreśja jakiego backendu chcemy od adaptera                       | domyślnie null                                    |

Jeśli naprzykład chcemy stworzyć adapter z domyślnymi ustawieniami możemy podać pusty deskryptor `{}`;

Tworzenie adaptera:
```cpp

int main() {
    ...
    Adapter adapter = instance.requestAdapter({});
}
```
