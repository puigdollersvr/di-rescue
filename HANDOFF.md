# DI Rescue — Handoff Codex/Devin

Última actualización documental: 2026-07-26.

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

## Evidencia que falta

- No se ha conciliado todavía cada checkbox del tasklist con el código actual.
- No consta aquí una compilación Debug o Release verificada en macOS.
- No consta una validación `auval`, una carga en Logic ni una prueba sonora.
- La existencia del scaffold no demuestra que P001, P010, P011 o P030 cumplan
  todos sus criterios.

## Próxima acción

Auditar P001, P010, P011 y P030, en ese orden, contra el código y el historial.
Marcar únicamente lo respaldado por evidencia. En un Mac con el entorno
disponible, ejecutar primero los comandos reales de configuración/build y
registrarlos en `PROTOTYPE_BUILD.md`.

## Cambios sin commit

Desconocidos: cada agente debe inspeccionar su checkout antes de editar.

## Regla para el siguiente agente

No empezar el DSP sonoro porque el scaffold compile o parezca completo.
Continuar la primera tarea realmente pendiente siguiendo `AGENTS.md` y dejar
este archivo actualizado al terminar.
