# DI Rescue — Reglas de revisión

Aplicar estas reglas a cualquier revisión de código o PR del prototipo.

## Bloqueantes

Marcar como bloqueante cualquier cambio que:

- asigne memoria, bloquee, acceda a disco/red o escriba logs desde
  `processBlock`;
- permita NaN, infinitos o denormals problemáticos;
- cambie coeficientes o parámetros sin smoothing cuando pueda causar clicks;
- comparta incorrectamente estados DSP entre canales;
- omita `prepareToPlay` o `reset` para estados persistentes;
- introduzca limitador, normalización, soft clipping o compresión general;
- aumente el nivel hacia NAM sin compensación documentada;
- no declare la latencia real;
- aplique una curva espectral fija idéntica a todas las DIs;
- rompa la neutralidad de `Restore = 0`;
- haga que la UI bloquee o escriba directamente en el hilo de audio;
- muestre `ZERO LATENCY` cuando no sea cierto;
- añada controles o funciones fuera del alcance;
- incorpore audios, datasets, binarios, credenciales o rutas personales.

## Comprobar específicamente

- estabilidad en 44,1 y 48 kHz;
- comportamiento con bloques pequeños;
- silencio y transición desde/hacia silencio;
- automatización rápida de Restore y Bypass;
- clicks al cambiar Guitar/Bass;
- aliasing del HarmonicRestorer;
- conservación del fundamental del bajo;
- peak y RMS antes/después;
- interacción con NAM a nivel igualado;
- atómicos o mecanismo lock-free entre medición y UI;
- caída de medidores, retención de clipping y cierre del editor;
- fidelidad funcional al mockup sin utilizarlo como fondo.

## Evidencia requerida

Una tarea no se aprueba solo porque compile. El handoff debe incluir:

- comando de build;
- prueba ejecutada;
- resultado observado o medido;
- limitación conocida;
- actualización coherente de `docs/tasks_prototype.md`, `HANDOFF.md` y, cuando
  corresponda, `PROTOTYPE_BUILD.md` o `PROTOTYPE_RESULTS.md`.

## No exigir todavía

- Windows o Linux;
- AAX;
- instalador, firma o notarización;
- entrenamiento neuronal, Learn o matching;
- emulación de previo, válvulas o transformador;
- pruebas automatizadas exhaustivas;
- publicación de una release.
