# DI Rescue

Open-source DI conditioner for electric guitar and bass, designed to run immediately before [Neural Amp Modeler](https://www.neuralampmodeler.com/).

> Status: prototype implementation under DSP and listening validation. It is not yet released.

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

The prototype plan and acceptance gate are documented in [`docs/CONTEXT_PROTOTYPE.md`](docs/CONTEXT_PROTOTYPE.md) and [`docs/tasks_prototype.md`](docs/tasks_prototype.md). The evidence behind the detector, transient and harmonic decisions is recorded in [`docs/LITERATURE_AND_DSP.md`](docs/LITERATURE_AND_DSP.md).

## Build status

The previous DSP revision built as AU/VST3 and passed `auval` on macOS. The literature-backed DSP correction on the current branch still requires a fresh macOS Debug/Release build, `auval`, aliasing measurements and the level-matched listening test described in `PROTOTYPE_RESULTS.md`.

## Working with Codex and Devin

This repository includes shared context so either agent can continue without
reconstructing prior decisions:

- [`AGENTS.md`](AGENTS.md): mandatory workflow and real-time DSP rules;
- [`HANDOFF.md`](HANDOFF.md): current state, evidence and next action;
- [`DECISIONS.md`](DECISIONS.md): confirmed scope decisions;
- [`PROMPTS.md`](PROMPTS.md): ready-to-use prompts for starting, continuing and
  reviewing work;
- [`REVIEW.md`](REVIEW.md): review gates for DSP, UI and scope;
- [`PROTOTYPE_BUILD.md`](PROTOTYPE_BUILD.md): verified build record;
- [`PROTOTYPE_RESULTS.md`](PROTOTYPE_RESULTS.md): level-matched listening and
  validation record;
- [`docs/LITERATURE_AND_DSP.md`](docs/LITERATURE_AND_DSP.md): primary sources,
  DSP implications, implemented formulas and parameters still awaiting calibration.

Start a coding session with:

```text
Read AGENTS.md and every file it references. Inspect the repository state and
execute only the first genuinely pending task in docs/tasks_prototype.md.
Verify its acceptance criteria and update HANDOFF.md before stopping.
```

Do not run Codex and Devin simultaneously against the same checkout.

## License

DI Rescue is licensed under the GNU Affero General Public License v3.0. See [`LICENSE`](LICENSE).
