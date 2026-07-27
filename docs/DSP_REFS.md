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
| `AdaptiveSpectralRestorer` | +2 a +3 dB | Solo cuando el `dullness score` indique DI apagada. |
| `TransientRestorer` | +0.5 a +1.5 dB | Solo ataque; el sustain debe cambiar muy poco. |
| `HarmonicRestorer` | bajo/nivel sutil | Mezcla a nivel bajo, evitar `fizz`. |

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
