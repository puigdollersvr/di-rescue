# DI Rescue — Resultados del prototipo

Completar durante P041–P042. No decidir por impresión general sin registrar
niveles y comparaciones.

## Configuración

- Fecha:
- Versión/commit:
- Interfaz:
- Sample rate:
- Block size:
- Modelo NAM:
- Ajuste de Input de NAM:
- Cab/IR:
- Nivel de escucha:

## Material utilizado

| ID | Instrumento/fuente | Pastilla/técnica | Estado percibido | Procedencia y derechos |
| --- | --- | --- | --- | --- |
| DI-01 |  |  | apagada/brillante/neutra |  |
| DI-02 |  |  | apagada/brillante/neutra |  |
| DI-03 |  |  | apagada/brillante/neutra |  |

No copiar DIs ni rutas locales al repositorio.

## Medidas

| DI | Restore | Peak entrada | Peak salida | RMS entrada | RMS salida | Diferencia RMS |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
|  | 0 |  |  |  |  |  |
|  | 25 |  |  |  |  |  |
|  | 50 |  |  |  |  |  |
|  | 75 |  |  |  |  |  |
|  | 100 |  |  |  |  |  |

## Comparaciones ciegas o semiciegas

| Prueba | DI | Después de NAM | Variante elegida | Naturalidad 1–5 | Definición 1–5 | Fizz 1–5 | Observaciones |
| ---: | --- | --- | --- | ---: | ---: | ---: | --- |
| 1 |  | sí/no | bypass/procesada |  |  |  |  |
| 2 |  | sí/no | bypass/procesada |  |  |  |  |
| 3 |  | sí/no | bypass/procesada |  |  |  |  |
| 4 |  | sí/no | bypass/procesada |  |  |  |  |
| 5 |  | sí/no | bypass/procesada |  |  |  |  |
| 6 |  | sí/no | bypass/procesada |  |  |  |  |
| 7 |  | sí/no | bypass/procesada |  |  |  |  |
| 8 |  | sí/no | bypass/procesada |  |  |  |  |
| 9 |  | sí/no | bypass/procesada |  |  |  |  |
| 10 |  | sí/no | bypass/procesada |  |  |  |  |

## Comprobación por módulo

### AdaptiveSpectralRestorer

- Mejora DI apagada:
- Respeta DI brillante:
- Modulación o bombeo:

### TransientRestorer

- Ataque:
- Sustain:
- Picos:

### HarmonicRestorer

- Armónicos útiles:
- Fizz:
- Aliasing:
- Conservación del fundamental del bajo:

### LevelCompensator y Bypass

- Diferencia máxima de RMS:
- Bypass sin clicks:
- NAM recibe más saturación por nivel:

### UI y medidores

- Respuesta IN/OUT:
- Retención de clipping:
- Apertura/cierre sin afectar al audio:

## Problemas y riesgos

-

## Tiempo

- Tiempo total:
- Desviación respecto a 6–8,5 horas:

## Decisión

Seleccionar exactamente una:

- [ ] `CONTINUAR`: cumple los criterios y merece la versión completa.
- [ ] `REVISAR DSP`: hay potencial y se autoriza una corrección acotada.
- [ ] `DETENER`: la mejora es pequeña, inconsistente o artificial.

### Justificación

Pendiente.

### Próxima acción

Pendiente.
