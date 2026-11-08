# AnimeTune — low-latency VST3 vocal tuner / anime voice shaper

AnimeTune is a JUCE/C++20 VST3 prototype designed for Windows x86-64 and live vocal monitoring in FL Studio.

## What is included

- YIN-style monophonic F0 detection with confidence gating and parabolic refinement.
- Short 1024-sample FFT processing path for low algorithmic buffering.
- Pitch-shift control separated from formant/timbre control.
- Retune-speed smoothing, correction strength, key selector, bypass, and pitch display.
- Seven general presets and three male→female presets.
- VST3 state persistence through `AudioProcessorValueTreeState`.
- Minimal black/white UI with cyan accent and resizable editor.

## Important engineering note

This repository is intentionally a **source prototype**, not a claim of studio-grade Auto-Tune equivalence. The DSP path is designed to be small and CPU-friendly, but the spectral remapping is an approximation. A production release should be validated with null tests, sustained-vowel tests, consonant/transient tests, low male voices, vibrato, and pluginval/FL Studio host testing.

The current 1024-sample analysis window implies roughly 21.3 ms at 48 kHz (23.2 ms at 44.1 kHz) of algorithmic windowing before the host's own ASIO buffer/driver latency. The actual microphone-to-speaker latency therefore depends on FL Studio's buffer size, ASIO driver and audio device. Do not assume a guaranteed total latency below 30 ms.

## Presets

1. Natural Tune — subtle correction, +12% formant.
2. Anime Soft — moderate correction, +30% formant.
3. Anime Bright — brighter, +45% formant.
4. Chibi / Cute — +58% formant and a small pitch lift.
5. Cyber Idol — tighter correction and synthetic brightness.
6. Hard Tune (Anime Robot) — fast correction for an obvious effect.
7. Whisper Anime — softer correction, brighter formant character.
8. Male→Female Natural — +5 semitones, +35% formant.
9. Male→Female Airy — +6 semitones, +42% formant.
10. Male→Female Idol — +6 semitones, +47% formant and faster retune.

Presets 8–10 are intended for a male input. On a female input they can sound over-shifted; reduce Formant Shift and Pitch Shift if needed.

## Build on Windows

Requirements:

- Windows 10/11 x64
- Visual Studio 2022/2026 with Desktop C++ workload
- CMake 3.24+
- Git

JUCE is fetched by CMake at configure time. The project currently pins JUCE 9.0.5. The official JUCE repository currently lists 9.0.5 as the latest release. 

PowerShell:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The VST3 will normally be placed under:

`build/AnimeTune_artefacts/Release/VST3/AnimeTune.vst3`

Copy the complete `AnimeTune.vst3` folder to:

`C:\Program Files\Common Files\VST3\`

Then restart/rescan plugins in FL Studio.

## One-command form

If Visual Studio is the default CMake generator:

```powershell
cmake -B build
cmake --build build --config Release
```

## Why not Rubber Band / RVC in the main path?

Rubber Band is excellent for quality, but the requested live-monitoring target favors a shorter internal window. RVC/ONNX voice conversion is deliberately not part of this prototype's realtime path because model inference and buffering can push latency much higher and add deployment complexity. A future optional offline/low-priority mode could host a neural converter separately.

## Reference projects studied

- QPitch: JUCE pitch correction with pitch detection, target-note mapping and formant preservation.
- stftPitchShiftPlugin: real-time STFT pitch/timbre shifting with configurable low-latency block sizes.
- OpenVoiceChanger: model-free DSP mode plus a wider real-time voice-conversion architecture.

These projects were used as architectural references; their source code is not copied into AnimeTune.

## Known limitations / next production steps

- Current spectral pitch/formant remapping is intentionally lightweight and should be treated as a prototype.
- The `Key` control is exposed but note-selection logic should be extended from chromatic mapping to scale-aware target selection before a release build.
- De-esser is currently represented as a stateful parameter and should be implemented as a dedicated split-band detector/filter in the next DSP pass.
- A true pitch-synchronous PSOLA engine would improve sustained-vowel quality and vibrato preservation.
- Add automated audio regression tests and pluginval testing.
- Add a proper input/output peak meter and oversampling-aware limiter for production.

## License

The project source written for AnimeTune is provided as-is for development. JUCE remains under its own license; see the fetched JUCE repository for its licensing terms.

## Cloud build and one-click installer (no local developer tools)

The repository includes a GitHub Actions workflow that builds the Windows x64 VST3 and packages it into `AnimeTune-Installer.exe`. The target computer does **not** need CMake, Git, JUCE, or Visual Studio.

### Create the cloud build

1. Create a GitHub account if you do not already have one.
2. On GitHub, create a new **private** repository named `AnimeTune` (public is also possible if you intend to publish the source).
3. Upload the contents of this project folder to the repository. The `.github` folder must be included; make sure `.github/workflows/windows-build.yml` is present.
4. Open the repository's **Actions** tab.
5. Select **Build AnimeTune for Windows** and click **Run workflow**.
6. Wait for the run to finish. A green check means the workflow completed successfully. If it fails, open the run and inspect the first red step and its log.
7. At the bottom of the successful run, download the artifact named **AnimeTune-Windows-x64**.

The artifact contains:
- `AnimeTune-Installer.exe` — run this on Windows, approve the administrator prompt, and it copies the VST3 bundle into `C:\Program Files\Common Files\VST3\AnimeTune.vst3`.
- `AnimeTune-VST3-Windows-x64.zip` — portable copy of the plugin bundle for manual installation.

After installation, restart FL Studio and run **Plugin manager → Find installed plugins** (or rescan VST3 plugins). The installer copies the plugin; it does not configure the FL Studio audio driver or microphone monitoring.

### Notes

- A successful compile does not prove that the DSP is production-grade or that every preset sounds natural. Test the plugin in FL Studio before relying on it for a recording session.
- The installer is not code-signed. Windows SmartScreen may show an unknown-publisher warning. Only run it if you trust the source repository and have inspected the workflow/build.
- JUCE's licensing terms apply to JUCE itself and to distribution of binaries built with it. Check the current JUCE licensing options before public or commercial distribution.
