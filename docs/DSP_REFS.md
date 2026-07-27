# DI Rescue — Referencias DSP iniciales

Punto de partida extraído de `docs/CONTEXT_PROTOTYPE.md`. **Todos los valores son iniciales y están sujetos a ajuste tras escucha**; no pretenden ser un estándar universal de DI profesional.

## Orden de la cadena

```text
Input Safety
→ AdaptiveSpectralRestorer
→ TransientRestorer
→ HarmonicRestorer
→ LevelCompensator
→ Output Safety/Meter
```

## Ganchos máximos de ganancia

| Restaurador | Ganancia máxima a `Restore = 100` | Notas |
| --- | --- | --- |
| `AdaptiveSpectralRestorer` | +2,5 dB natural; +4,5 dB en audición | Adaptativo hasta 65; 65–100 hace el efecto progresivamente evidente. |
| `TransientRestorer` | +1,2 dB natural; +2,0 dB máximo | Solo cuando la relación rápida/lenta indique ataque. |
| `HarmonicRestorer` | residuo no lineal sutil | `tanh` normalizada con ADAA; sin fuga lineal dominante. |

## Bandas para `AdaptiveSpectralRestorer`

| Modo | Cuerpo/referencia | Presencia/definición | Extremo superior/ataque |
| --- | --- | --- | --- |
| Guitar | 120–800 Hz | 1.5–4.5 kHz | 4.5–8 kHz |
| Bass | 60–400 Hz | 700 Hz–2 kHz | 2–5 kHz |

## Bandas para `HarmonicRestorer`

| Modo | Filtro previo de la rama no lineal | Notas |
| --- | --- | --- |
| Guitar | 1.2–2 kHz (punto de partida) | Eliminar graves y limitar extremo superior si hay `fizz`. |
| Bass | 600 Hz–1.2 kHz (punto de partida) | Conservar el fundamental del bajo. |

## Tiempos de `TransientRestorer`

| Modo | Envelope rápido | Envelope lento | Comentario |
| --- | --- | --- | --- |
| Guitar | pendiente de ajuste | pendiente de ajuste | Ataque más definido sin aumentar sustain. |
| Bass | pendiente de ajuste | pendiente de ajuste | Posiblemente tiempos algo más lentos que en guitarra. |

## Input/Output Safety

- Filtro DC blocker + subsónico suave ~15–25 Hz.
- Gestión de denormals, `NaN` e `infinitos`.
- Sin normalizador, limitador ni soft clipper.
- `Restore = 0` solo puede diferenciarse del bypass por la limpieza de DC/subgraves.

## Compensación de nivel

- Determinista, asociada a `Restore`.
- Objetivo: RMS a corto plazo dentro de ±0.5 dB respecto a la entrada en material de validación.
- No normalizar por nota ni perseguir constantemente el RMS.


## Revisión respaldada por literatura — 2026-07-27

- El detector sigue potencia al cuadrado y compara densidades aproximadas por
  ancho en octavas.
- El brillo se expresa como relación logarítmica entre Presence/Top y Body.
- Las referencias iniciales son −9 dB con 12 dB de recorrido para Guitar y
  −12 dB con 14 dB de recorrido para Bass.
- El transient restorer usa la relación rápida/lenta en dB, con una zona neutra
  de 1 dB.
- La rama armónica resta la respuesta lineal equivalente de la `tanh` y usa
  ADAA de primer orden para reducir aliasing conservando latencia cero.
- La compensación global de −3 dB se sustituye por una compensación pequeña
  asociada a la corrección realmente aplicada.
- `Restore` leído mediante APVTS ya está en 0–100; no se multiplica por 100.
- Los fundamentos, fórmulas, fuentes y límites están en
  [`LITERATURE_AND_DSP.md`](LITERATURE_AND_DSP.md).

Todos estos valores siguen pendientes de calibración sonora en P041–P042.
