# DI Rescue — Prompts de trabajo

## Empezar con Codex o Devin

```text
Lee AGENTS.md y todos los archivos que indica. Inspecciona el estado real del
repositorio y ejecuta únicamente la primera tarea realmente pendiente de
docs/tasks_prototype.md. Comprueba sus criterios, actualiza HANDOFF.md y no
amplíes el alcance.
```

## Continuar una sesión

```text
Lee AGENTS.md, docs/CONTEXT_PROTOTYPE.md, docs/tasks_prototype.md, HANDOFF.md y
DECISIONS.md. Si HANDOFF.md indica una tarea parcial, continúa solo esa tarea;
si no, ejecuta únicamente la primera tarea pendiente. Verifica sus criterios y
deja un handoff completo.
```

## Pedir una tarea concreta

```text
Ejecuta únicamente la tarea P0XX de docs/tasks_prototype.md siguiendo
AGENTS.md. No empieces la siguiente. Compila o valida lo indicado, marca la
tarea solo si cumple todos sus criterios y actualiza HANDOFF.md con comandos,
resultados, archivos y próxima acción.
```

## Revisar trabajo del otro agente

```text
Revisa los cambios actuales siguiendo REVIEW.md y AGENTS.md. No implementes
arreglos todavía. Distingue bloqueantes de mejoras opcionales, comprueba el
alcance y cita archivos concretos. Verifica que HANDOFF.md y cualquier tarea
marcada reflejen el estado real.
```

## Implementar una corrección aprobada

```text
Implementa únicamente los bloqueantes aprobados de la revisión anterior.
Conserva los cambios existentes, ejecuta las validaciones relevantes y
actualiza HANDOFF.md. No empieces otra tarea del tasklist.
```

## Preparar el cambio de agente

```text
No programes nada más. Actualiza HANDOFF.md para que otro agente pueda continuar
sin contexto previo: tarea, progreso exacto, archivos, comandos, pruebas,
errores, cambios sin commit y próxima acción.
```

## Ejecutar la prueba final

```text
Ejecuta únicamente P041 o P042 según corresponda. Usa PROTOTYPE_RESULTS.md,
controla el nivel antes de comparar y escucha también después de NAM y cab/IR.
No concluyas que suena mejor solo por mayor volumen.
```
