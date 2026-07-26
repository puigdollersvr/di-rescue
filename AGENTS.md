# DI Rescue — Reglas para agentes de IA

Este archivo coordina el trabajo alternado entre Codex y Devin Desktop. Las
reglas se aplican a todo el repositorio.

## Orden de lectura

Antes de modificar código:

1. leer este archivo;
2. leer `docs/CONTEXT_PROTOTYPE.md` completo;
3. leer `docs/tasks_prototype.md`;
4. leer `HANDOFF.md`;
5. leer `DECISIONS.md`;
6. inspeccionar el repositorio y sus cambios reales.

`docs/CONTEXT_PROTOTYPE.md` y `docs/tasks_prototype.md` son las fuentes de
verdad para el alcance técnico y los criterios de aceptación. No crear copias
en la raíz.

## Alcance actual

- Prototipo macOS Apple Silicon en C++17, JUCE y CMake.
- Audio Unit obligatorio; VST3 ya está configurado y se conserva mientras no
  complique el prototipo.
- Cadena de uso: `DI → DI Rescue → NAM → cab/IR`.
- Controles: `Instrument`, `Restore` y `Bypass`.
- La UI aprobada, los medidores IN/OUT y los LEDs de clipping forman parte del
  prototipo.
- Presupuesto total de referencia: 6–8,5 horas.
- No pedir grabaciones nuevas; validar con DIs existentes y sin incorporar
  audio de terceros al repositorio.

No añadir emulación de previo, válvulas o transformador; matching, Learn,
perfiles old/fresh; Clean, Hum, Gate, EQ, compresor, limitador, Input/Output
Gain, presets, instalador, firma o notarización.

## Protocolo de trabajo

- Ejecutar una sola tarea de `docs/tasks_prototype.md` por iteración, salvo que
  el usuario pida expresamente otra cosa.
- Si `HANDOFF.md` describe una tarea parcial, continuar esa tarea antes de
  empezar otra.
- No marcar una tarea como terminada sin comprobar todos sus criterios.
- Registrar comandos, pruebas, resultados y limitaciones en `HANDOFF.md`.
- Actualizar `PROTOTYPE_BUILD.md` al verificar el entorno o los comandos.
- Actualizar `PROTOTYPE_RESULTS.md` únicamente con pruebas realmente hechas.
- No dar por hecho que un archivo existente, un commit o una compilación de
  otro entorno prueban que una tarea está terminada.
- No ejecutar dos agentes a la vez sobre el mismo checkout.

## Reglas DSP de tiempo real

Dentro de `processBlock`:

- no asignar memoria;
- no bloquear;
- no acceder a disco o red;
- no escribir logs;
- no reconstruir objetos DSP;
- evitar NaN, infinitos y denormals problemáticos.

Preparar memoria, filtros y estados en `prepareToPlay`, y reiniciarlos
correctamente. Suavizar los cambios que puedan producir clicks. La medición
para la UI debe cruzar desde el hilo de audio mediante atómicos o un mecanismo
lock-free.

## Reglas de cambio

- Preservar cambios existentes y evitar reescrituras ajenas a la tarea.
- Preferir cambios pequeños, legibles y reversibles.
- No añadir dependencias salvo necesidad demostrada.
- No incluir builds, plugins compilados, audios, datasets, credenciales ni
  rutas locales.
- No hacer push, abrir PR ni publicar releases salvo petición explícita.
- Revisar con `REVIEW.md` antes de aceptar trabajo DSP o de tiempo real.

## Cierre de cada sesión

Actualizar `HANDOFF.md` con:

- tarea y estado exactos;
- archivos modificados;
- comandos y resultados;
- pruebas pendientes o bloqueos;
- cambios sin commit;
- próxima acción única y concreta.
