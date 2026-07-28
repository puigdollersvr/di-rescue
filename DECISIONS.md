# DI Rescue — Decisiones confirmadas

Registro breve de decisiones de alcance que no deben reabrirse sin nueva
instrucción del usuario.

| Decisión | Estado |
| --- | --- |
| Finalidad | Acondicionar una DI intacta pero apagada antes de Neural Amp Modeler. |
| Cadena | `DI → DI Rescue → NAM → cab/IR`. |
| Alcance | Prototipo de validación, no producto comercial. |
| Presupuesto | 6–8,5 horas, incluida la UI aprobada y sus medidores. |
| Plataforma | macOS Apple Silicon; Audio Unit obligatorio. |
| Formato adicional | VST3 se conserva porque ya está configurado. |
| Tecnología | C++17, JUCE 8.0.13 fijado en CMake. |
| Controles | Instrument, Restore y Bypass. |
| UI | Mockup aprobado implementado con componentes/dibujo JUCE, no como fondo raster. |
| Medición | IN/OUT y clipping mediante comunicación lock-free con la UI. |
| Material | DIs existentes; no se requieren grabaciones nuevas para V1. |
| Nivel | A/B aproximadamente dentro de ±0,5 dB de RMS a corto plazo. |
| Referencia tonal | No se promete igualar una DI concreta de estudio ni unas cuerdas nuevas. |
| Detector | Potencia por bandas, densidad aproximada por octava y relación logarítmica; referencias separadas Guitar/Bass. |
| Restore | 0–65 es zona conservadora adaptativa; 65–100 es zona de audición y 100 debe ser claramente audible. |
| Armónicos | `tanh` normalizada con residuo no lineal y ADAA de primer orden; no se denomina emulación de válvulas. |
| Parámetros APVTS | `getRawParameterValue("restore")` entrega el valor declarado 0–100, no un 0–1 normalizado. |
| Color de previo | No se emulan válvulas, transformadores ni una interfaz concreta. |
| Ruido | Sin Clean, Hum, Gate ni reducción de ruido en el prototipo. |
| Aprendizaje | Sin Learn, matching, perfiles ni pares old/fresh. |
| Repositorio | No incluir audios, datasets, builds ni plugins compilados. |

Las especificaciones detalladas viven en `docs/CONTEXT_PROTOTYPE.md`; si este
resumen y ese documento difieren, prevalece el documento de contexto.
