# DI Rescue — Contexto del prototipo

## 1. Propósito

Construir en aproximadamente **6–8,5 horas** un prototipo funcional de DI Rescue para responder una única pregunta:

> ¿La combinación de restauración espectral adaptativa, recuperación de ataque y enriquecimiento armónico mejora de forma clara y natural las DIs existentes del usuario al pasar después por Neural Amp Modeler?

Este prototipo no es todavía una versión publicable. Debe contener el núcleo sonoro suficiente para tomar una decisión informada de continuar o detener el proyecto y la interfaz visual aprobada, implementada de forma reutilizable para no rehacerla si se continúa.

Cadena de prueba:

```text
DI existente → DI Rescue Prototype → Neural Amp Modeler → cab/IR
```

Equipo principal del usuario:

- Gibson SG con humbuckers;
- PRS SE con humbuckers;
- bajo pasivo tipo P-Bass;
- Hotone Ampero utilizado como interfaz USB;
- Logic Pro en macOS;
- Neural Amp Modeler después de DI Rescue.

No se solicitarán grabaciones nuevas, pares old/fresh ni sesiones de calibración. Se utilizarán DIs que el usuario ya tenga y, si faltara alguna categoría, material de evaluación cuya licencia permita ese uso.

## 2. Hipótesis que debe validar

Una DI intacta pero algo apagada puede mejorar perceptualmente antes de NAM mediante tres intervenciones pequeñas:

1. recuperar de forma adaptativa algo de presencia cuando la relación entre cuerpo y agudos indique apagamiento;
2. reforzar únicamente el inicio de las notas;
3. generar una cantidad baja de armónicos superiores a partir del contenido existente.

La hipótesis queda refutada si la mejora solo procede de aumentar volumen, si aparece fizz, si todas las guitarras convergen hacia el mismo timbre o si el resultado después de NAM no se elige de forma consistente frente al bypass.

## 3. Definición de éxito

El prototipo se considera prometedor únicamente si cumple todos estos puntos:

- en DIs apagadas, la versión procesada se prefiere en al menos 7 de 10 comparaciones ciegas o semiciegas;
- la diferencia sigue siendo apreciable después del mismo NAM y cab/IR;
- una DI suficientemente brillante no se vuelve áspera ni claramente peor;
- `Restore = 50` produce una mejora sutil pero reconocible;
- `Restore = 100` no suena a excitador evidente, fuzz o treble booster;
- el RMS a corto plazo se mantiene aproximadamente dentro de ±0.5 dB respecto a la entrada en las muestras de validación;
- no aparecen clicks, bombeo, clipping nuevo ni cambios extraños durante silencios;
- la salida continúa comportándose como una DI cruda apropiada para alimentar NAM.
- la interfaz reproduce de forma reconocible el mockup aprobado y sus medidores funcionan sin bloquear el hilo de audio.

El prototipo no necesita demostrar que transforma cualquier DI en una DI de estudio ni que iguala exactamente unas cuerdas nuevas.

## 4. Plataforma y tecnología

Alcance obligatorio:

- macOS;
- Apple Silicon;
- Audio Unit cargable en Logic Pro;
- efecto mono;
- C++ con JUCE y CMake;
- versión estable y concreta de JUCE;
- 44.1 y 48 kHz;
- bloques habituales entre 32 y 1024 samples;
- procesamiento causal y apto para tiempo real.

Opcional únicamente si no añade trabajo relevante:

- VST3;
- entrada estéreo duplicando estados independientes por canal;
- 88.2 y 96 kHz.

No dedicar tiempo en el prototipo a Windows, Linux, AAX, instaladores, firma, notarización ni distribución binaria.

## 5. Interfaz y controles

El archivo `di-rescue-ui-mockup.png` es la referencia visual oficial y debe implementarse ya con componentes y dibujo vectorial de JUCE. No utilizar el PNG como fondo de la interfaz.

1. `Instrument`: Guitar / Bass.
2. `Restore`: 0–100 %, valor inicial 0.
3. `Bypass`: off/on.
4. Medidores segmentados `IN` y `OUT`.
5. LEDs rojos independientes de clipping de entrada y salida.

No añadir Output, Input Gain, Clean, Hum, Gate, EQ, Amp, Cab, Drive ni presets.

Los parámetros deben poder automatizarse y deben suavizarse donde sea necesario. El guardado de estado es deseable, pero no debe retrasar la prueba sonora si surge un problema incidental.

### 5.1 Distribución

- Ventana fija de aproximadamente 900 × 600 px y proporción 3:2.
- Cabecera con `DI RESCUE`, `DI CONDITIONER FOR NAM` y `v0.1 OPEN SOURCE`.
- Gran knob `RESTORE` en el centro.
- Selector vertical `INSTRUMENT` a la izquierda.
- Medidores `IN` y `OUT` debajo del selector.
- Interruptor `BYPASS` a la derecha.
- Texto inferior `PLACE BEFORE NAM`.
- Mostrar `ZERO LATENCY` únicamente si el plugin declara realmente cero samples.

### 5.2 Estética

- fondo exterior `#050606`;
- panel principal `#171819`;
- zonas profundas `#0D0E0F`;
- texto principal `#D8D1C4`;
- acento ámbar `#D7903F`;
- medidor verde `#39D42F`;
- clipping rojo `#D62B2B`;
- apariencia sobria de herramienta de estudio, sin estética gaming;
- tipografía del sistema o una fuente open source correctamente licenciada;
- dibujo nítido en pantallas Retina.

### 5.3 Medidores

- El procesador publicará peaks de entrada y salida mediante valores atómicos o un mecanismo lock-free.
- El editor leerá los valores con un timer aproximado de 20–30 Hz.
- Rango visual orientativo de −60 a 0 dBFS.
- Ataque visual rápido y caída suavizada.
- Los LEDs se encenderán al detectar una muestra en clipping digital y mantendrán el aviso aproximadamente un segundo.
- Cerrar la UI no debe afectar al audio ni a la medición interna.

## 6. Núcleo DSP

Orden inicial:

```text
Input Safety
→ AdaptiveSpectralRestorer
→ TransientRestorer
→ HarmonicRestorer
→ LevelCompensator
→ Output Safety/Meter
```

### 6.1 Input Safety

- Eliminar DC mediante un filtro adecuado.
- Aplicar un filtro subsónico fijo y suave alrededor de 15–25 Hz.
- Gestionar denormals, NaN e infinitos.
- No utilizar normalizador, limitador ni soft clipper.
- `Restore = 0` solo puede diferenciarse del bypass por esta limpieza fuera del rango útil.

### 6.2 AdaptiveSpectralRestorer

Objetivo: detectar una DI relativamente apagada y aplicar una corrección amplia y limitada, no una EQ creativa.

Implementación mínima:

- filtros IIR y seguidores de energía preasignados;
- comparación de energía entre cuerpo, presencia y extremo superior;
- `dullness score` suavizado entre 0 y 1;
- umbral de señal útil para no reaccionar al silencio;
- retorno lento a estado neutro;
- ganancia máxima aproximada de 2–3 dB con `Restore = 100`;
- zona de tolerancia suficientemente amplia para no tratar automáticamente una humbucker oscura como defectuosa;
- ausencia de FFT, lookahead, archivos externos y asignaciones en el hilo de audio.

Bandas iniciales:

| Modo | Cuerpo/referencia | Presencia/definición | Extremo superior/ataque |
| --- | --- | --- | --- |
| Guitar | 120–800 Hz | 1.5–4.5 kHz | 4.5–8 kHz |
| Bass | 60–400 Hz | 700 Hz–2 kHz | 2–5 kHz |

Estas bandas son puntos de partida. Se permite ajustarlas durante la escucha, pero cada cambio debe documentarse brevemente.

### 6.3 TransientRestorer

- Calcular un envelope rápido y otro lento.
- Derivar una medida de transitorio estable.
- Reforzar solo los ataques.
- Ganancia máxima aproximada de 0.5–1.5 dB a `Restore = 100`.
- Utilizar tiempos distintos para Guitar y Bass si mejora claramente la respuesta.
- No aumentar apreciablemente el sustain.

### 6.4 HarmonicRestorer

- Crear una rama paralela prefiltrada.
- Filtrar antes de la no linealidad:
  - Guitar: comenzar las pruebas entre 1.2 y 2 kHz;
  - Bass: comenzar entre 600 Hz y 1.2 kHz.
- Utilizar una no linealidad simétrica y suave.
- Eliminar graves de la rama y limitar su extremo superior si genera fizz.
- Mezclarla a nivel bajo en función de `Restore`.
- Evaluar aliasing a 44.1 kHz.
- Añadir oversampling 2× solo si el aliasing resulta audible o evidente en una prueba sencilla; si se añade latencia, declararla correctamente.

### 6.5 LevelCompensator

- Aplicar una compensación determinista asociada a la cantidad máxima introducida por Restore.
- No normalizar cada nota ni perseguir constantemente el RMS.
- Evitar que NAM distorsione más por una simple subida de nivel.
- Objetivo inicial: RMS a corto plazo dentro de ±0.5 dB en el paquete de validación, aceptando pequeñas diferencias en los ataques.

### 6.6 Bypass

- Debe permitir A/B fiable.
- Evitar clicks mediante rampa o crossfade corto.
- No reiniciar estados de forma que el retorno al procesado produzca un pico.

## 7. Reglas de implementación

En `processBlock`:

- no asignar memoria;
- no bloquear mutexes;
- no acceder a disco;
- no escribir logs;
- no reconstruir objetos DSP completos;
- no ejecutar análisis offline;
- mantener estados separados por canal si se acepta estéreo.

Preparar buffers, filtros y estados en `prepareToPlay`. Reiniciarlos correctamente en `reset`.

Prioridad:

1. sonido y A/B fiable;
2. estabilidad en Logic;
3. interfaz aprobada reutilizable;
4. código comprensible.

No invertir tiempo en arquitectura extensible, abstracciones generales o optimizaciones que no afecten a la prueba.

## 8. Validación

Utilizar al menos:

- una DI existente de guitarra con humbuckers que parezca apagada;
- una DI existente de guitarra razonablemente brillante;
- una DI existente de P-Bass;
- power chords o palm mutes;
- notas sostenidas;
- bajo con púa si está disponible.

Para cada muestra:

1. fijar el mismo NAM, cab/IR y configuración;
2. probar Bypass, Restore 25, 50, 75 y 100;
3. comprobar peak y RMS;
4. ajustar el volumen de comparación si la compensación aún no es exacta;
5. alternar versiones sin mirar el estado del plugin siempre que sea posible;
6. registrar preferencia, naturalidad, definición, fizz y comportamiento del ataque.

Crear `PROTOTYPE_RESULTS.md` con:

- DIs utilizadas y procedencia;
- modelo NAM e IR;
- valores probados;
- medidas de nivel;
- resultados de las comparaciones;
- problemas observados;
- decisión final: `CONTINUAR`, `REVISAR DSP` o `DETENER`.

## 9. Fuera de alcance

- análisis completo de datasets públicos;
- dossier científico exhaustivo;
- entrenamiento o inferencia neuronal;
- perfiles, Learn o matching old/fresh;
- simulación de pastillas activas;
- emulación de previo, transformador o válvulas;
- calibración de NAM;
- reducción de ruido, hum o gate;
- compresión general;
- EQ de mezcla;
- reparación de clipping;
- pruebas automatizadas exhaustivas;
- soporte completo de sample rates y hosts;
- README público, licencia final y guía de contribución;
- instalador, firma, notarización y release.

## 10. Entregables

Al terminar deben existir:

- proyecto JUCE/CMake compilable;
- Audio Unit de prueba;
- código fuente del núcleo DSP;
- interfaz JUCE fiel al mockup aprobado, reutilizable en la versión completa;
- medidores IN/OUT y LEDs de clipping funcionales;
- instrucciones cortas de build y ubicación del `.component`;
- `PROTOTYPE_RESULTS.md`;
- recomendación explícita sobre si merece la pena desarrollar la versión completa.
