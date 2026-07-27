# DI Rescue — Handoff Codex/Devin

Última actualización documental: 2026-07-27.

## Base observada

- Repositorio: `puigdollersvr/di-rescue`.
- Rama inspeccionada: `main`.
- Commit observado: `551f7b3ad701d2cfd6925e622fd2a3b6817156ad`.
- El repositorio ya contiene un proyecto JUCE/CMake, código de procesador y
  editor, documentación de alcance y el mockup aprobado.

## Estado conocido

- `CMakeLists.txt` fija C++17 y JUCE 8.0.13.
- Están configurados AU y VST3 con bundle
  `com.puigdollersvr.direscue`.
- El procesador contiene un scaffold pass-through, APVTS para Instrument,
  Restore y Bypass, y valores atómicos de peak para la UI.
- Existe un scaffold del editor y una referencia visual bajo `docs/design/`.
- `docs/CONTEXT_PROTOTYPE.md` y `docs/tasks_prototype.md` reflejan el alcance
  actual de 6–8,5 horas e incluyen la UI y los medidores.

## Fase P0 — Estado

- **P001 — Fijar el entorno:** realizado.
  - Se detectaron macOS 26.5.2, Xcode 26.5 (17F42), Apple Clang 21.0.0, arquitectura `x86_64` Intel.
  - JUCE 8.0.13 queda fijado por `FetchContent` con tag concreto (no rama flotante).
  - El Audio Unit mantiene el bundle identifier `com.puigdollersvr.direscue`.
  - `PROTOTYPE_BUILD.md` queda documentado con el entorno y los comandos a ejecutar.
  - **Bloqueo:** `cmake` no está instalado en `PATH`; es necesario instalarlo antes de poder configurar/build.
- **P002 — Trasladar las referencias DSP mínimas:** realizado.
  - Se creó `docs/DSP_REFS.md` con bandas, ganchos de ganancia, tiempos transitorios y reglas de seguridad, marcados como punto de partida.

## Fase P1 — Estado

- **P010 — Crear el proyecto JUCE/CMake:** realizado.
  - `cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=x86_64` terminó correctamente.
  - `cmake --build build --config Debug --parallel` terminó correctamente.
  - Se generaron `build/DIRescue_artefacts/Debug/AU/DI Rescue.component` y `.../VST3/DI Rescue.vst3`.
  - `auval -v aufx DiRs Pdrv` devolvió `AU VALIDATION SUCCEEDED` tras copiar el `.component` a `~/Library/Audio/Plug-Ins/Components/`.
- **P011 — Crear parámetros y estado mínimo:** realizado.
  - `AudioProcessorValueTreeState` declara `instrument`, `restore` y `bypass`.
  - Los attachments del editor (`ComboBox`, `Slider`, `ToggleButton`) conectan con APVTS.
  - `getStateInformation`/`setStateInformation` guardan y recuperan el estado XML.
  - `auval` pasó los tests de parámetros y renderizado sin clicks ni crashes.

## Fase P2 — Estado

- **P020 — Implementar Input Safety:** implementado.
  - `ChannelDsp` aplica un filtro `makeHighPass` a 20 Hz para DC + subsónico.
  - Gestión de NaN/inf y denormales mediante `safeValue` y `ScopedNoDenormals`.
  - Medición de peak de entrada/salida por bloque.
- **P021 — AdaptiveSpectralRestorer:** implementado.
  - Detectores IIR de banda para Guitar/Bass.
  - `dullness score` suavizado y compuerta de señal útil.
  - Corrección con high-shelf fija mezclada proporcionalmente a `Restore`.
- **P022 — TransientRestorer:** implementado.
  - Envelopes rápido/lento y reforzamiento del ataque limitado.
  - Escalado por `Restore`.
- **P023 — HarmonicRestorer:** implementado.
  - Rama paralela filtrada, no linealidad suave `tanh`, mezcla baja.
  - Sin oversampling (se añadirá solo si se detecta aliasing en validación).
- **P024 — Integrar cadena, compensación y bypass:** implementado.
  - Orden: Input Safety → AdaptiveSpectral → Transient → Harmonic → LevelComp → Bypass.
  - Compensación determinista por `Restore`.
  - Bypass con crossfade suavizado.
  - `cmake --build build --config Debug --parallel` terminó correctamente (`** BUILD SUCCEEDED **`).

## Correcciones post-revisión

- `Source/DspCore.h/cpp`:
  - Precomputa los coeficientes IIR para Guitar y Bass en `prepareToPlay` (`buildCoefficients`), evitando asignaciones de memoria en `processBlock` cuando cambia `Instrument`.
  - `setInstrument` asigna coeficientes ya calculados sin reiniciar los estados IIR, dejando que `reset()` reinicie los filtros al arrancar o cuando el host lo solicita. Esto reduce los clicks al cambiar de Guitar/Bass en tiempo real.
  - `processSample` utiliza `juce::Decibels::decibelsToGain` en lugar de `std::pow` para la compensación de nivel.
- `Source/PluginProcessor.cpp`:
  - Guardas contra punteros nulos de `getRawParameterValue`.
  - `std::atomic<float>` se lee con `.load()` para evitar conversiones ambiguas.
  - Índices a `channelDsp` convertidos a `size_t` para evitar avisos de signo.
- `CMakeLists.txt`: eliminado `Source/DspCore.h` de `target_sources`.
- `cmake --build build --config Debug --parallel` terminó correctamente (`** BUILD SUCCEEDED **`) tras las correcciones.

## Evidencia que falta

- No consta todavía una carga en Logic ni una prueba sonora real.
- No se ha verificado Release build (P040).
- P030 y P040–P042 aún no están verificados.

## Próxima acción

Continuar con **P040 — Validar build y estabilidad mínima**: compilar Release,
ejecutar `auval`, probar 44.1/48 kHz y verificar que el DSP no produce clicks,
NaN ni clipping en una DI real.

## Cambios sin commit

- `Source/DspCore.h` y `Source/DspCore.cpp` creados.
- `Source/PluginProcessor.h` y `Source/PluginProcessor.cpp` modificados.
- `CMakeLists.txt` actualizado con los nuevos archivos fuente.
- `PROTOTYPE_BUILD.md` actualizado.
- `docs/DSP_REFS.md` creado.
- `docs/tasks_prototype.md` actualizado con tareas marcadas.
- `HANDOFF.md` actualizado.
- Directorio `build/` generado (no se debe incluir en el repositorio; está en `.gitignore`).

## Regla para el siguiente agente

No empezar el DSP sonoro porque el scaffold compile o parezca completo.
Continuar la primera tarea realmente pendiente siguiendo `AGENTS.md` y dejar
este archivo actualizado al terminar.
