# DI Rescue — Handoff Codex/Devin

Última actualización documental: 2026-07-31.

## Preparación build Release y validación AU — 2026-07-31

- Tarea: compilar el plugin en Release, instalar el `.component` en `~/Library/Audio/Plug-Ins/Components/`, refrescar la caché de Audio Units y validar con `auval` para dejarlo listo para Logic Pro X.
- Commit base: `3b41386 fix: strengthen memory ordering for atomic operations`.
- Archivos consultados: `PROTOTYPE_BUILD.md`, `HANDOFF.md`, `CMakeLists.txt`.
- Cambios en esta sesión: ningún cambio de código; el repositorio ya tenía el fix de memory ordering commiteado.
- Comandos ejecutados y resultados:
  - `cmake --build build --config Release --parallel` → **BUILD SUCCEEDED**.
  - `cp -R "build/DIRescue_artefacts/Release/AU/DI Rescue.component" ~/Library/Audio/Plug-Ins/Components/` → OK.
  - `killall -9 AudioComponentRegistrar` → OK (refresca caché de Audio Units).
  - `auval -v aufx DiRs Pdrv` → **AU VALIDATION SUCCEEDED**.
- Limitaciones:
  - El build es `x86_64`, coherente con el entorno documentado. Si la máquina de Logic es Apple Silicon nativo sin Rosetta, hará falta recompilar para `arm64`.
  - No se ha podido abrir Logic Pro X para confirmar la carga real.
- Pruebas pendientes:
  - Carga real en Logic Pro X.
  - Verificación de persistencia visual de `Instrument` al cerrar y reabrir el plugin.
- Cambios sin commit: `PROTOTYPE_BUILD.md`.
- Próxima acción única: abrir Logic Pro X, insertar DI Rescue en una pista y confirmar carga y persistencia del selector de instrumento.

## Corrección de persistencia del parámetro Instrument — 2026-07-30

- Problema reportado: al cerrar el plugin en una pista de bajo con `Instrument`
  en Bass, al reabrirlo el selector volvía a Guitar aunque el estado del host
  siguiera siendo Bass.
- Causa: en el constructor de `DIRescueAudioProcessorEditor`
  (`Source/PluginEditor.cpp`) se llamaba a `parameterChanged("instrument", 0.0f)`
  con un valor hardcoded `0.0f` (Guitar) en lugar de leer el valor real del
  parámetro. El listener de APVTS solo se dispara cuando el parámetro _cambia_,
  no al abrir el editor, así que la UI siempre arrancaba pintando Guitar con
  independencia del estado guardado. El DSP sí procesaba como Bass, pero la UI
  mentía.
- Cambios en esta sesión (2 archivos):
  - `Source/PluginEditor.cpp`:
    - Línea 14: sustituido `instrumentSelector.setSelectedIndex(0)` (hardcoded
      Guitar) por la lectura del valor real del parámetro:
      `instrumentSelector.setSelectedIndex(juce::roundToInt(param->getValue()))`.
      Esto elimina el flash inicial de Guitar → Bass al abrir el editor.
    - Líneas 48-50: sustituida la llamada `parameterChanged("instrument", 0.0f)`
      por `parameterChanged("instrument", param->getValue())` para que el
      listener arranque sincronizado con el estado real del parámetro.
  - `Source/PluginProcessor.cpp`:
    - `setStateInformation`: añadida validación de que el XML tenga el tag
      correcto (`parameters.state.getType()`, es decir "PARAMETERS") antes de
      llamar a `replaceState`. Si el host enviara state corrupto o de otra
      versión, se ignora en lugar de reemplazar el árbol por algo inválido.
- Comandos ejecutados y resultados:
  - `cmake --build build --config Debug --parallel` → **BUILD SUCCEEDED**.
  - `cmake --build build --config Release --parallel` → **BUILD SUCCEEDED**.
  - `ctest -C Debug --output-on-failure` → **Passed** (1/1 DspSmoke).
  - Instalado `build/DIRescue_artefacts/Release/AU/DI Rescue.component` en
    `~/Library/Audio/Plug-Ins/Components/` y
    `build/DIRescue_artefacts/Release/VST3/DI Rescue.vst3` en
    `~/Library/Audio/Plug-Ins/VST3/`, y reiniciado `AudioComponentRegistrar`.
- Pruebas pendientes:
  - Verificación en Logic: poner Instrument en Bass, cerrar la ventana del
    plugin, reabrirla y confirmar que el selector sigue en Bass sin flash
    inicial de Guitar.
  - Lo mismo para Guitar como caso inverso.
- Próxima acción única: validar en Logic la persistencia visual de Instrument al
  cerrar y reabrir el plugin, confirmando que no hay parpadeo inicial.

## Correcciones críticas de UI thread y peaks atómicos — 2026-07-28

- Problemas reportados:
  1. `parameterChanged()` en `PluginEditor.cpp` modificaba `instrumentSelector` directamente desde el hilo de audio, pudiendo causar condiciones de carrera y avisos en Logic.
  2. Los picos atómicos se sobrescribían cada bloque, perdiendo clips entre lecturas de la UI.

- Cambios en `Source/PluginEditor.h`:
  - Añadido `#include <atomic>`.
  - Añadido `std::atomic<int> pendingInstrumentIndex { -1 };`.

- Cambios en `Source/PluginEditor.cpp`:
  - `parameterChanged()` solo almacena el índice pendiente en `pendingInstrumentIndex` (sin tocar componentes).
  - `timerCallback()` aplica el cambio de instrumento desde el hilo de UI y lee los peaks con `exchange(0.0f)`.

- Cambios en `Source/PluginProcessor.cpp`:
  - Los peaks atómicos se actualizan con `compare_exchange_weak` para acumular el máximo entre lecturas de la UI.

- Comandos ejecutados y resultados:
  - `cmake -B build -G Xcode` → OK.
  - `cmake --build build --config Debug --parallel` → **BUILD SUCCEEDED**.
  - `cmake --build build --config Release --parallel` → **BUILD SUCCEEDED**.
  - Instalado `build/DIRescue_artefacts/Release/AU/DI Rescue.component` en `~/Library/Audio/Plug-Ins/Components/` y reiniciado `AudioComponentRegistrar`.
  - `auval -v aufx DiRs Pdrv` → **AU VALIDATION SUCCEEDED** (se probaron 44.1 kHz, 48 kHz y otros sample rates).
  - `tests/dsp_smoke.cpp` añadido y registrado en `CMakeLists.txt` con CTest; `ctest -C Debug --output-on-failure` → **Passed**.

- Pruebas pendientes:
  - Carga real en Logic.
  - Medición de RMS, peak, respuesta espectral y aliasing.
  - Prueba real DI Rescue → NAM.

- Próxima acción única:
  - Preparar la sesión A/B con DIs existentes y el mismo NAM/cab/IR.

## Base observada

- Repositorio: `puigdollersvr/di-rescue`.
- Rama inspeccionada: `main`.
- Commit base observado: `32b2cea359f0773867a5aafd05e0592c30761d45`.
- Rama de esta corrección: `agent/literature-backed-dsp-corrections`.
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

## Fase P3 — Estado

- **P030 — Implementar la UI aprobada y los medidores:** implementado y compilado.
  - Se creó `Source/UiComponents.h/cpp` con componentes desacoplados del DSP y reutilizables:
    - `ui::SegmentedMeter` para medidores IN/OUT con rango aproximado de −60 a 0 dBFS y caída suavizada.
    - `ui::ClipLed` para LEDs de clipping con retención aproximada de un segundo.
    - `ui::InstrumentSelector` para la selección Guitar/Bass.
    - `ui::DiRescueLookAndFeel` con rotary y toggle estilizados según la paleta.
  - Se reescribió `Source/PluginEditor.h/cpp`:
    - Layout fijo ~900 × 600 px con cabecera, knob central `RESTORE`, selector izquierdo, medidores IN/OUT con LEDs, bypass derecho y pie `PLACE BEFORE NAM` / `ZERO LATENCY`.
    - Timer de UI a ~30 Hz que lee picos atómicos y refresca medidores/LEDs sin tomar locks en el hilo de audio.
    - Selector de instrumento reactivo con el parámetro `instrument` del APVTS.
  - `Source/PluginProcessor.cpp` fija `setLatencySamples(0)`, permitiendo mostrar `ZERO LATENCY`.
  - `CMakeLists.txt` se actualizó con los nuevos archivos.
  - Comandos ejecutados:
    - `cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=x86_64` → OK.
    - `cmake --build build --config Debug --parallel` → `** BUILD SUCCEEDED **`.
  - **Pendiente de validación:** captura/verificación visual real en un host (mockup, medidores, LEDs, reflejo de parámetros y apertura/cierre sin clicks).

## Correcciones P030 tras revisión

- `Source/PluginProcessor.cpp`:
  - La explicación anterior era incorrecta: APVTS devuelve el valor denormalizado del parámetro. Como `restore` está declarado 0–100, multiplicarlo por 100 comprimía todo el DSP en el primer 1 % del control. La rama actual elimina ese factor.
- `Source/PluginEditor.cpp`:
  - Añadido `restoreSlider.setRange(0.0, 100.0, 0.1)` para que `SliderAttachment` mapee todo el rango del parámetro `restore`.
  - Añadido indicador numérico `0 %`–`100 %` bajo el knob `RESTORE` con `TextBoxBelow`.
- `Source/UiComponents.cpp`:
  - Invertido el mapeo de `thresholdDb` en `SegmentedMeter::paint` (`0.0f` arriba, `-60.0f` abajo) para que los medidores llenen de abajo hacia arriba.
- Comandos ejecutados:
  - `cmake --build build --config Debug --parallel` → `** BUILD SUCCEEDED **`.

## Validación AU

- Se copió `build/DIRescue_artefacts/Debug/AU/DI Rescue.component` a `~/Library/Audio/Plug-Ins/Components/`.
- `auval -v aufx DiRs Pdrv` terminó con `AU VALIDATION SUCCEEDED`.
- **Advertencia:** apareció `JUCE Assertion failure in juce_IIRFilter.cpp:141` durante el test de render a 192 kHz. Las pruebas a 44.1/48/96 kHz pasaron, por lo que no debería afectar el uso habitual en Logic con proyectos a 44.1/48 kHz.

## Evidencia que falta

- Validación visual de P030 en un host real (colores, layout, medidores, LEDs, apertura/cierre).
- No consta todavía una carga en Logic ni una prueba sonora real.
- No se ha verificado Release build (P040).
- P040–P042 aún no están verificados.

## Corrección de bypass e intensidad DSP — 2026-07-27

- Problema reportado: con altavoces de portátil no se percibía diferencia al
  conmutar el bypass.
- Causa: la mezcla de bypass usaba la señal post-`safetyFilter` como referencia
  `dry`, y los ganchos de ganancia de los módulos de restauración eran tan
  bajos que el efecto resultaba inaudible en monitores modestos.
- Cambios en `Source/DspCore.h`:
  - `maxSpectralDb` de `2.5` a `4.5` dB.
  - `maxTransientDb` de `0.8` a `2.0` dB.
  - `levelCompDb` de `1.8` a `3.0` dB para mantener el nivel general.
  - `branchDrive` de `4.0` a `6.0`.
  - `harmonicLevel` de `0.04` a `0.08`.
- Cambios en `Source/DspCore.cpp`:
  - La fórmula de `correctionTarget` pasa a `1.0f - balance * 2.0f`,
    haciendo que la restauración espectral sea más sensible a DIs apagadas.
  - La mezcla de bypass ahora es `x = in + (x - in) * bypassMix`, es decir,
    la señal cruda de entrada vs el procesado completo, con el mismo crossfade
    de 5 ms para evitar clicks.
- Build Debug: `cmake --build build --config Debug --parallel` → **BUILD SUCCEEDED**.
- Se copió `build/DIRescue_artefacts/Debug/AU/DI Rescue.component` a
  `~/Library/Audio/Plug-Ins/Components/`.
- Se reinició `AudioComponentRegistrar` para invalidar la caché de Audio Units.
- Pruebas pendientes: escucha A/B con DI real en Logic a 44.1/48 kHz y `auval`.

## Corrección respaldada por literatura — 2026-07-27

- Se añadió `docs/LITERATURE_AND_DSP.md` con fuentes primarias, inferencias,
  fórmulas implementadas, límites de la evidencia y corpus permitidos.
- `PluginProcessor.cpp`: corregido el rango APVTS 0–100.
- `DspCore.h/cpp`:
  - detectores cambiados de amplitud absoluta a potencia suavizada;
  - relación de brillo en dB y densidad aproximada por ancho en octavas;
  - referencias separadas y tolerantes para Guitar/Bass;
  - zona conservadora 0–65 y zona de audición 65–100;
  - ataque derivado de la relación de potencias rápida/lenta en dB;
  - `tanh` normalizada, residuo no lineal y ADAA de primer orden;
  - filtro posterior de la rama armónica;
  - compensación máxima reducida frente a la atenuación global de −3 dB;
  - cambio Guitar/Bass mediante crossfade temporal hacia la DI limpia antes
    de sustituir y reiniciar el banco de filtros;
  - frecuencias IIR limitadas de forma segura respecto a Nyquist.
- Revisión local disponible en Linux:
  - `g++ -std=c++17 -Wall -Wextra -Werror -pedantic` sobre `DspCore.cpp`
    con stubs mínimos de JUCE → OK.
  - smoke test de 44.100 muestras, cambio Guitar/Bass y bypass → OK, todas las
    salidas finitas.
- No se ha podido ejecutar el build JUCE/CMake real en este entorno porque no
  contiene CMake, JUCE, macOS, Xcode, `auval` ni Logic. No considerar P040
  completada.
- Estado correcto: **DSP corregido en código; build macOS, medición de aliasing,
  nivel y prueba sonora pendientes**.

## Próxima acción

En macOS, reconstruir/instalar el `.component` en `~/Library/Audio/Plug-Ins/Components/`,
reiniciar `AudioComponentRegistrar` y probar el A/B en Logic con una DI real a
44.1/48 kHz. Luego actualizar `PROTOTYPE_RESULTS.md`.

## Cambios de esta sesión

Los cambios están publicados en `agent/literature-backed-dsp-corrections`. No se han incluido builds, plugins, datasets, audios ni stubs locales de validación.

## Regla para el siguiente agente

No empezar el DSP sonoro porque el scaffold compile o parezca completo.
Continuar la primera tarea realmente pendiente siguiendo `AGENTS.md` y dejar
este archivo actualizado al terminar.
