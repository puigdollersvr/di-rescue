# DI Rescue — Literatura y decisiones DSP

Última revisión: 2026-07-27.

## Alcance de la evidencia

No se ha encontrado un estudio que valide una cadena exacta
`high-shelf + transient shaper + tanh` como transformación universal de una DI
doméstica en una DI profesional. La literatura sí respalda por separado las
tres dimensiones tratadas por el prototipo: distribución espectral,
microdinámica del ataque y estructura armónica.

Por tanto, DI Rescue se define como un **acondicionador conservador previo a
NAM**, no como una emulación de previo, válvula, transformador, interfaz o
cuerdas nuevas. La corrección solo puede recuperar perceptualmente información
compatible con la señal existente; no puede reconstruir información perdida de
forma inequívoca.

## Evidencia utilizada

### Desgaste de cuerdas

Grzybowski, Wrzeciono y Bayat estudiaron el desgaste de cuerdas de guitarra
eléctrica mediante análisis tiempo-frecuencia y pitch. El resultado no permite
reducir el desgaste a una única pérdida fija de agudos: también aparecen
cambios temporales y en la relación entre parciales.

- Artículo:
  <https://doi.org/10.3390/s25133989>
- Texto abierto:
  <https://pmc.ncbi.nlm.nih.gov/articles/PMC12252302/>

**Decisión:** mantener espectro, ataque y armónicos como módulos separados. No
presentar el shelf como reconstrucción física de una cuerda nueva.

### Brillo y timbre

La literatura de timbre identifica el centroide o distribución espectral, el
tiempo de ataque y la estructura de armónicos como dimensiones perceptualmente
relevantes. El centroide aislado también varía con el ruido, el tono fundamental
y los transitorios.

- Peeters et al., *The Timbre Toolbox*:
  <https://www.mcgill.ca/mpcl/files/mpcl/peeters_2011_jasa.pdf>
- Kazazis et al., escalado de descriptores espectrales:
  <https://doi.org/10.3389/fpsyg.2022.835401>
- Caclin et al., correlatos de dimensiones tímbricas:
  <https://doi.org/10.1121/1.1929229>

**Decisiones:**

- medir potencia suavizada, no amplitud absoluta media;
- comparar las bandas en escala logarítmica;
- normalizar aproximadamente por ancho de banda;
- suavizar el detector y no convertir el ataque en una corrección espectral
  instantánea;
- conservar referencias distintas para Guitar y Bass.

### Variabilidad propia de la guitarra

Pastilla, bobinado, posición de pulsación, cuerda y traste modifican de forma
importante la respuesta espectral de una DI.

- Batchelor et al., análisis controlado de bobinado y calibre:
  <https://arxiv.org/abs/2409.19782>
- Mohamad, Harte y Dixon, posición de pastilla y pulsación:
  <https://webspace.eecs.qmul.ac.uk/s.e.dixon/pub/2017/MohamadHarteDixon-ICASSP2017.pdf>
- Mohamad, Dixon y Harte, transformación de posición de pastilla:
  <https://www.ntnu.edu/documents/1001201110/1266017954/DAFx-15_submission_45.pdf>

**Decisión:** no existe un umbral universal de “DI profesional”. Las
referencias iniciales del prototipo son tolerantes y deberán calibrarse con
medianas e intervalos intercuartílicos de DIs limpias variadas.

### Microdinámica

La relación entre una envolvente corta y otra larga permite representar
microdinámica de manera más estable frente a cambios generales de nivel.

- Nercessian, McClellan y Lukin:
  <https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_21.pdf>

**Decisión:** calcular el cociente de energías rápida/lenta en dB y aplicar una
ganancia limitada solo cuando el cociente supera una pequeña zona neutra.

### No linealidad y aliasing

Una no linealidad digital sin protección produce armónicos por encima de
Nyquist que reaparecen como aliasing. Oversampling y métodos
antiderivative-antialiasing (ADAA) son soluciones documentadas.

- Parker et al.:
  <https://www.dafx.de/paper-archive/2016/dafxpapers/20-DAFx-16_paper_41-PN.pdf>

**Decisión:** la rama armónica usa una `tanh` normalizada con ADAA de primer
orden y resta la respuesta lineal equivalente. Esto evita que el supuesto
“enriquecedor” sea en realidad una segunda copia lineal de la banda, reduce
aliasing y conserva el contrato de latencia cero.

### Nivel antes de NAM

Un modelo NAM calibrado solo recibe el nivel físico esperado si los efectos
anteriores mantienen una relación de nivel conocida.

- Documentación oficial:
  <https://neural-amp-modeler.readthedocs.io/en/latest/tutorials/calibration.html>

**Decisión:** no usar AGC, normalización por nota, limitador ni una atenuación
global grande. La compensación es pequeña y determinista; su ajuste definitivo
depende de mediciones sobre el paquete de validación.

## Algoritmo implementado

### Detector espectral

Para cada banda se sigue potencia suavizada:

```text
E_band[n] = smooth(x_band[n]²)
```

Se aproxima una densidad de energía dividiendo por el ancho en octavas. El
descriptor de brillo es:

```text
brightnessDb =
    10 log10((0.65 E_presence + 0.35 E_top + ε) / (E_body + ε))
```

Los pesos y referencias siguientes son puntos iniciales, no constantes
universales:

| Modo | Referencia inicial | Recorrido de déficit |
| --- | ---: | ---: |
| Guitar | −9 dB | 12 dB |
| Bass | −12 dB | 14 dB |

El déficit pasa por `smoothstep` y se suaviza con ataque de 150 ms y retorno de
500 ms. Por debajo de −70 dBFS de potencia el objetivo vuelve a cero.

### Curva Restore

- `0–65`: zona conservadora gobernada por el detector;
- `65–100`: zona de audición progresiva;
- `100`: shelf máximo claramente audible aunque la DI sea brillante.

La zona natural usa un shelf máximo de 2,5 dB. La zona de audición puede llegar
a 4,5 dB. El extremo sirve para escuchar qué está haciendo el algoritmo; no se
presupone que sea el mejor ajuste.

### Ataque

```text
relativeTransientDb =
    10 log10((E_fast + ε) / (E_slow + ε))
```

Existe una zona neutra inicial de 1 dB. La ganancia máxima es 1,2 dB en la zona
natural y añade hasta 0,8 dB en la zona de audición.

### Armónicos

La rama se filtra antes y después de la no linealidad. Para
`f(x) = tanh(d x) / d`, ADAA de primer orden calcula la pendiente secante de su
antiderivada:

```text
F(x) = log(cosh(d x)) / d²
```

A continuación se resta la pendiente secante de la identidad, equivalente al
promedio de las dos muestras. El resultado es un residuo no lineal sin fuga
lineal dominante.

### Cambio Guitar/Bass

El cambio no sustituye coeficientes mientras el procesamiento está a plena
mezcla. Primero se cruza durante aproximadamente 5 ms hacia la DI limpia, se
cambia y reinicia el banco específico del instrumento y se vuelve a introducir
el procesamiento. Se evita asignar memoria o bloquear el hilo de audio.

## Parámetros pendientes de calibración

No deben darse por definitivos hasta completar P041–P042:

- referencias de brillo e intervalo de tolerancia;
- pesos de Presence y Top;
- compensación de nivel;
- cantidad del residuo armónico;
- puntos exactos de las bandas;
- ganancia de la zona de audición.

## Corpus permitido para calibración offline

- IDMT-SMT-Audio-Effects: guitarra y bajo, diferentes instrumentos, pastillas
  y formas de pulsación; licencia de evaluación CC BY-NC-ND 4.0:
  <https://www.idmt.fraunhofer.de/en/publications/datasets/audio_effects.html>
- EGFxSet: cinco posiciones de pastilla y tonos limpios anotados; CC BY 4.0:
  <https://egfxset.github.io/>
- IDMT-SMT-Bass: tres bajos, tres ajustes de pastilla y diez técnicas:
  <https://www.idmt.fraunhofer.de/en/publications/datasets/bass.html>

Los datasets no se incorporarán al repositorio. Sirven para obtener
estadísticas offline y comprobar falsos positivos, no para afirmar que
contienen una referencia universal de estudio.

## Validación requerida

1. Corregir y verificar primero que APVTS entrega `Restore` en sus unidades
   declaradas 0–100.
2. Medir Restore 0, 25, 50, 75 y 100 sobre DIs apagadas, neutras y brillantes.
3. Comprobar RMS de sustain y peak sin normalización interna.
4. Medir aliasing con senoides a 44,1 y 48 kHz.
5. Cambiar Guitar/Bass y Bypass durante reproducción y buscar clicks.
6. Repetir las comparaciones después del mismo NAM y cab/IR.
7. Igualar volumen únicamente para la prueba auditiva, no dentro del plugin.

Hasta completar esta validación el estado correcto es **DSP implementado,
ajuste sonoro pendiente**, no “DI profesional demostrada”.
