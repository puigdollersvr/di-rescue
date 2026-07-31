# DI Rescue — Entorno y comandos del prototipo

Completar únicamente con datos obtenidos del entorno real. No presentar un
comando como verificado sin ejecutarlo.

## Configuración fijada en el repositorio

- Proyecto: `DIRescue` 0.1.0.
- Lenguaje: C++17.
- Sistema de build: CMake 3.22 o posterior.
- JUCE: `8.0.13` referenciado de forma concreta mediante `FetchContent` (`GIT_TAG 8.0.13`, no rama flotante).
- Formatos: AU y VST3.
- Producto: `DI Rescue`.
- Bundle identifier: `com.puigdollersvr.direscue`.
- Manufacturer code: `Pdrv`.
- Plugin code: `DiRs`.

## Entorno macOS verificado

- Fecha: 2026-07-28.
- macOS: 26.5.2 (build 25F84).
- Arquitectura: `x86_64` (`uname -m`); `uname -p` reporta `i386`; CPU Intel Core i9-9980HK.
- Xcode: `/Applications/Xcode.app/Contents/Developer`, `Xcode 26.5 Build 17F42`.
- Apple Clang: `Apple clang version 21.0.0 (clang-2100.1.1.101)`; target `x86_64-apple-darwin25.5.0`.
- CMake: `/usr/local/bin/cmake` version `4.4.0`.
- Generador CMake: `Xcode`.
- Host de prueba: `auval -v aufx DiRs Pdrv`.

## Configuración Debug

```bash
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=x86_64
```

## Build Debug

```bash
cmake --build build --config Debug --parallel
```

## Configuración Release

```bash
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=x86_64
```

## Build Release

```bash
cmake --build build --config Release --parallel
```

## Artefactos

- Audio Unit: `build/DIRescue_artefacts/${CONFIG}/DI Rescue.component`.
- VST3: `build/DIRescue_artefacts/${CONFIG}/DI Rescue.vst3`.
- Ubicación de prueba: `~/Library/Audio/Plug-Ins/Components/` y `~/Library/Audio/Plug-Ins/VST3/`.

## Validación AU

```bash
auval -v aufx DiRs Pdrv
```

## Tests DSP mínimos

```bash
ctest -C Debug --output-on-failure
```

- `tests/dsp_smoke.cpp` cubre silencio, identidad en bypass, finitud, Restore=0 conservativo, cambios sin discontinuidades y respuesta a Restore.

## Resultado

- CMake configure: **OK** (`cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=x86_64`).
- Debug build: **OK** (`cmake --build build --config Debug --parallel`).
- Release build: **OK** (`cmake --build build --config Release --parallel`).
- `auval`: **PASS** (`auval -v aufx DiRs Pdrv` tras copiar el `DI Rescue.component` Release a `~/Library/Audio/Plug-Ins/Components/` y reiniciar `AudioComponentRegistrar`).
- `ctest` (Debug): **PASS** (`1/1 Test #1: DspSmoke ... Passed`).
- Re-validado 2026-07-31: build Release OK, `auval` PASS, componente instalado en `~/Library/Audio/Plug-Ins/Components/`, caché AU refrescada.
- Carga en Logic: pendiente.
- Limitaciones: el entorno detectado es `x86_64` Intel, mientras que el alcance del prototipo prioriza Apple Silicon. El build actual se hizo nativo sobre esta máquina; si se necesita un binario Apple Silicon se deberá compilar en el hardware correspondiente.

No añadir instalador al prototipo ni utilizar una referencia flotante de JUCE.
