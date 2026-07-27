# DI Rescue — Tasklist del prototipo

## Reglas de ejecución

- Leer `CONTEXT_PROTOTYPE.md` completo antes de modificar código.
- El objetivo es decidir si el DSP mejora el sonido, no producir una versión publicable.
- Trabajar en el orden indicado y realizar una tarea por iteración.
- Mantener un build ejecutable desde P010.
- No implementar elementos incluidos en “Fuera de alcance”.
- Reproducir `di-rescue-ui-mockup.png` con componentes JUCE reutilizables; no replantear el diseño.
- No pedir al usuario grabaciones nuevas.
- No marcar una tarea como terminada sin comprobar sus criterios.
- Presupuesto total: **6–8,5 horas**. La UI y los medidores añaden aproximadamente 1–1,5 horas. Si una incidencia amenaza con superar 8,5 horas, detenerse y documentar el bloqueo antes de ampliar el alcance.

---

## Fase P0 — Preparación

### [x] P001 — Fijar el entorno

**Estimación:** 10–15 min.

**Objetivo:** utilizar una configuración mínima y reproducible.

**Acciones:**

- Detectar macOS, arquitectura, Xcode, CMake y compilador.
- Elegir y fijar una versión estable concreta de JUCE.
- Confirmar el identificador del Audio Unit.
- Crear `PROTOTYPE_BUILD.md` con los comandos reales.

**Criterios de aceptación:**

- Las versiones y comandos están documentados.
- No se utiliza una rama flotante de JUCE.
- No se instalan herramientas innecesarias.

### [x] P002 — Trasladar las referencias DSP mínimas

**Estimación:** 10–15 min.

**Objetivo:** empezar con parámetros razonables sin ejecutar todavía el estudio completo de datasets.

**Acciones:**

- Extraer de `CONTEXT_PROTOTYPE.md` las bandas, límites de ganancia y restricciones.
- Crear una tabla breve de constantes iniciales para Guitar y Bass.
- Marcar cada constante como punto de partida sujeto a escucha.

**Criterios de aceptación:**

- Guitar y Bass tienen valores iniciales separados.
- No se presenta ninguna curva como estándar universal de DI profesional.
- El prototipo no necesita descargar ningún dataset.

---

## Fase P1 — Plugin cargable

### [x] P010 — Crear el proyecto JUCE/CMake

**Estimación:** 30–40 min.

**Objetivo:** disponer de un Audio Unit mínimo que pase audio en Logic.

**Acciones:**

- Crear el proyecto JUCE con CMake.
- Configurarlo como efecto mono para Apple Silicon.
- Generar Audio Unit; generar VST3 solo si resulta automático y no añade incidencias.
- Crear `PluginProcessor` y `PluginEditor`.
- Implementar pass-through inicial.
- Compilar Debug.

**Criterios de aceptación:**

- CMake configura correctamente.
- El `.component` se genera.
- Logic o un host de prueba carga el plugin.
- El audio pasa sin cambios ni crashes.

### [x] P011 — Crear parámetros y estado mínimo

**Estimación:** 15–20 min.

**Objetivo:** poder controlar y automatizar la prueba.

**Parámetros:**

- `instrument`: Guitar / Bass.
- `restore`: 0–100 %, default 0.
- `bypass`: off/on.

**Acciones:**

- Utilizar `AudioProcessorValueTreeState`.
- Añadir smoothing donde corresponda.
- Implementar restauración de estado si no introduce una incidencia significativa.

**Criterios de aceptación:**

- Los tres parámetros aparecen en el host.
- Sus valores llegan correctamente al procesador.
- Cambiarlos durante reproducción no provoca clicks ni crashes.

---

## Fase P2 — Núcleo sonoro

### [x] P020 — Implementar Input Safety

**Estado:** implementado en código; validación sonora pendiente.

**Estimación:** 15–20 min.

**Objetivo:** asegurar una entrada válida sin modificar el rango musical.

**Acciones:**

- Añadir DC blocker.
- Añadir filtro subsónico suave.
- Gestionar denormals y valores no finitos.
- Medir peak de entrada y salida.

**Criterios de aceptación:**

- Silencio produce silencio.
- No aparecen NaN ni infinitos.
- Con Restore a 0, la diferencia se limita a DC y subgraves fuera del rango útil.
- No existe limitador, normalizador ni soft clipper.

### [x] P021 — Implementar AdaptiveSpectralRestorer

**Estado:** implementado en código; validación sonora pendiente.

**Estimación:** 40–55 min.

**Objetivo:** corregir más una DI apagada que una DI ya brillante.

**Acciones:**

- Crear detectores IIR para las tres bandas de Guitar y Bass.
- Implementar envelopes y umbral de señal útil.
- Calcular relaciones relativas y un `dullness score` entre 0 y 1.
- Aplicar filtros anchos o shelf suave con máximo inicial de 2–3 dB.
- Escalar con Restore.
- Suavizar la corrección y devolverla lentamente a cero durante silencio.

**Criterios de aceptación:**

- Restore a 0 desactiva el módulo.
- Una señal deliberadamente apagada recibe más corrección que una brillante.
- El silencio no mueve el detector erráticamente.
- No se aplica la misma curva fija a todo.
- No hay FFT, lookahead, lectura de archivos ni asignaciones en `processBlock`.

### [x] P022 — Implementar TransientRestorer

**Estado:** implementado en código; validación sonora pendiente.

**Estimación:** 20–30 min.

**Objetivo:** recuperar algo de definición en el inicio de las notas.

**Acciones:**

- Calcular envelopes rápido y lento.
- Derivar la medida de transitorio.
- Aplicar como máximo aproximadamente 0.5–1.5 dB.
- Añadir constantes separadas para Guitar y Bass si son necesarias.
- Escalar y suavizar con Restore.

**Criterios de aceptación:**

- El ataque aumenta sutilmente sin clicks.
- El sustain cambia muy poco.
- Restore a 0 es neutro.
- Palm mutes no producen picos descontrolados.

### [x] P023 — Implementar HarmonicRestorer

**Estado:** implementado en código; validación sonora pendiente.

**Estimación:** 25–40 min.

**Objetivo:** recuperar sensación de armónicos sin convertir la DI en un efecto.

**Acciones:**

- Crear una rama paralela prefiltrada.
- Utilizar bandas distintas para Guitar y Bass.
- Aplicar una no linealidad suave y simétrica.
- Filtrar graves y fizz de la rama.
- Mezclar a un nivel bajo controlado por Restore.
- Evaluar aliasing a 44.1 kHz con senoides y notas agudas.
- Añadir oversampling 2× únicamente si es claramente necesario.

**Criterios de aceptación:**

- Restore a 0 excluye la rama.
- Una senoide genera armónicos controlados.
- No suena a fuzz ni a treble booster.
- El P-Bass conserva el fundamental.
- No aparece aliasing molesto en la prueba básica.

### [x] P024 — Integrar cadena, compensación y bypass

**Estado:** implementado en código; validación sonora pendiente.

**Estimación:** 25–35 min.

**Objetivo:** producir una comparación fiable antes de NAM.

**Acciones:**

- Integrar los módulos en el orden de `CONTEXT_PROTOTYPE.md`.
- Implementar compensación determinista de nivel.
- Implementar bypass con crossfade o rampa corta.
- Declarar latencia al host si existe.
- Verificar que `processBlock` no asigna ni bloquea.

**Criterios de aceptación:**

- No hay clicks al cambiar Restore, Instrument o Bypass.
- Restore a 0 es neutro salvo Input Safety.
- El RMS a corto plazo queda aproximadamente dentro de ±0.5 dB en las muestras iniciales.
- No se genera clipping nuevo.
- NAM no recibe más nivel de forma deliberada.

---

## Fase P3 — Interfaz reutilizable

### [ ] P030 — Implementar la UI aprobada y los medidores

**Estimación:** 60–90 min.

**Objetivo:** implementar ahora la interfaz definitiva para evitar rehacerla si el DSP supera la validación.

**Acciones:**

- Utilizar `di-rescue-ui-mockup.png` como referencia, no como imagen de fondo.
- Crear un `LookAndFeel` propio con la paleta de `CONTEXT_PROTOTYPE.md`.
- Implementar cabecera, gran knob Restore, selector Guitar/Bass y Bypass respetando la distribución aprobada.
- Implementar medidores segmentados IN/OUT con rango aproximado de −60 a 0 dBFS.
- Transferir los peaks desde el procesador mediante valores atómicos o un mecanismo lock-free.
- Actualizar medidores desde un timer de UI de aproximadamente 20–30 Hz.
- Aplicar caída visual suavizada y LEDs de clipping con retención aproximada de un segundo.
- Mostrar `ZERO LATENCY` solo si es verdadero.
- Mantener ventana fija cercana a 900 × 600 px y dibujo nítido en Retina.
- Mantener los componentes desacoplados del DSP para poder reutilizarlos sin reescritura.

**Criterios de aceptación:**

- Una captura se reconoce claramente como la implementación del mockup aprobado.
- Jerarquía, posiciones, textos y paleta se mantienen.
- Todos los controles funcionan.
- La interfaz refleja los parámetros del host.
- Los medidores reaccionan correctamente y los LEDs distinguen clipping de entrada y salida.
- La UI no toma locks ni obliga al hilo de audio a esperar.
- Abrir y cerrar la ventana no interrumpe el audio.
- No aparecen controles adicionales.
- No será necesario rehacer la estructura visual para la versión completa.

---

## Fase P4 — Validación técnica y sonora

### [ ] P040 — Validar build y estabilidad mínima

**Estimación:** 15–20 min.

**Objetivo:** asegurar que los fallos técnicos no invaliden la escucha.

**Acciones:**

- Compilar Release.
- Ejecutar `auval`.
- Probar 44.1 y 48 kHz.
- Probar varios tamaños de bloque.
- Automatizar Restore y alternar Bypass durante reproducción.
- Verificar que los medidores siguen el audio, caen suavemente y no afectan a la estabilidad.

**Criterios de aceptación:**

- Release compila.
- `auval` termina correctamente.
- No hay crashes, clicks persistentes ni valores no finitos.
- El plugin funciona en Logic con una DI existente.

### [ ] P041 — Preparar la sesión A/B

**Estimación:** 20–30 min.

**Objetivo:** comparar de forma repetible y a volumen igualado.

**Acciones:**

- Seleccionar DIs existentes de humbucker, fuente brillante y P-Bass.
- Añadir el mismo NAM y cab/IR después de cada instancia.
- Preparar Bypass y Restore 25/50/75/100.
- Medir peak y RMS antes de escuchar.
- Configurar comparaciones ciegas o semiciegas cuando sea posible.
- No realizar nuevas grabaciones ni incluir audios de terceros en el repositorio.

**Criterios de aceptación:**

- Las rutas y ajustes de NAM son idénticos.
- Las diferencias de nivel están controladas.
- La sesión permite repetir cada comparación rápidamente.

### [ ] P042 — Ejecutar prueba y decidir

**Estimación:** 30–40 min.

**Objetivo:** decidir si merece la pena construir la versión completa.

**Acciones:**

- Realizar al menos 10 comparaciones con material apagado.
- Comprobar además que el material brillante no empeora.
- Registrar naturalidad, definición, ataque, fizz y preferencia.
- Crear `PROTOTYPE_RESULTS.md`.
- Asignar una decisión:
  - `CONTINUAR`: cumple los criterios de éxito;
  - `REVISAR DSP`: hay potencial, pero falla un criterio corregible;
  - `DETENER`: la mejora es pequeña, inconsistente o artificial.

**Criterios de aceptación:**

- El resultado contiene medidas y preferencias, no solo impresiones generales.
- Se escucha también después de NAM y cab/IR.
- La decisión está justificada.
- No se continúa automáticamente con el empaquetado y publicación final.

---

## Definición de terminado

- Existe un AU Release cargable en Logic.
- Guitar, Bass, Restore y Bypass funcionan.
- Los tres restauradores están activos y limitados.
- El nivel está suficientemente compensado para un A/B honesto.
- No hay crashes, clicks persistentes, NaN ni clipping generado.
- La UI aprobada, los medidores y los LEDs de clipping están implementados y son reutilizables.
- Se ha probado con DIs existentes, sin solicitar nuevas grabaciones.
- `PROTOTYPE_RESULTS.md` concluye `CONTINUAR`, `REVISAR DSP` o `DETENER`.
- El tiempo empleado y cualquier desviación respecto a las 6–8,5 horas están documentados.
