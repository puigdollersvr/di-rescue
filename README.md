# DI Rescue

Open-source DI conditioner for electric guitar and bass, designed to run immediately before [Neural Amp Modeler](https://www.neuralampmodeler.com/).

> Status: prototype scaffold. The DSP and interface are not yet validated or released.

## Goal

Improve a clean DI that is intact but somewhat dull by applying a conservative combination of:

- adaptive spectral restoration;
- transient recovery;
- subtle parallel harmonic enrichment;
- deterministic level compensation for honest A/B testing before NAM.

DI Rescue is not an amp simulator, cabinet simulator, creative EQ, noise gate, normalizer, or replacement for NAM input calibration.

## Planned signal chain

```text
DI → DI Rescue → Neural Amp Modeler → cab/IR → mix
```

## Prototype controls

- **Instrument:** Guitar / Bass
- **Restore:** 0–100%
- **Bypass**
- **IN/OUT meters** with clipping indicators

## Prototype target

- macOS Apple Silicon
- Audio Unit for Logic Pro
- VST3 when it does not complicate the prototype
- C++17, JUCE and CMake
- zero or correctly reported latency

The prototype plan and acceptance gate are documented in [`docs/CONTEXT_PROTOTYPE.md`](docs/CONTEXT_PROTOTYPE.md) and [`docs/tasks_prototype.md`](docs/tasks_prototype.md).

## Build status

The repository is being initialized. Build commands will be verified on macOS before they are presented as supported.

## License

DI Rescue is licensed under the GNU Affero General Public License v3.0. See [`LICENSE`](LICENSE).
