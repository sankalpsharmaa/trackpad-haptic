# trackpad-haptic

Trigger Force Touch trackpad haptics from the command line on macOS.

Uses private `MultitouchSupport` APIs (same family as Apple’s trackpad “tap” feedback). Useful for agent “done” signals, scripts, or anything that should buzz the trackpad without looking at the screen.

## Requirements

- macOS with a **Force Touch** trackpad (MacBook, or Magic Trackpad with Force Touch)
- Xcode Command Line Tools (`xcode-select --install`)

No Homebrew deps. The Multitouch framework is loaded at runtime via `dlopen`.

## Install

```bash
git clone https://github.com/sankalpsharmaa/trackpad-haptic.git
cd trackpad-haptic
make
make install   # → ~/.local/bin/trackpad-haptic
```

Ensure `~/.local/bin` is on your `PATH`.

Or build in place without installing:

```bash
make
./trackpad-haptic tap
```

## Usage

```text
trackpad-haptic tap [waveform] [count] [interval_ms]
```

| Arg | Default | Range | Meaning |
|-|-|-|-|
| `waveform` | `6` | 1–32 | Haptic pattern ID |
| `count` | `3` | 1–20 | Number of pulses |
| `interval_ms` | `400` | 50–2000 | Gap between pulses |

### Examples

```bash
# Default: three strong taps, 400ms apart
trackpad-haptic tap

# Single strong click
trackpad-haptic tap 2 1

# Soft triple thud
trackpad-haptic tap 15 3 300

# Buzz once
trackpad-haptic tap 3 1
```

### Waveforms (empirical)

These IDs are not documented by Apple; they can change across macOS versions. On recent Apple Silicon Macs they roughly map as:

| ID | Feel |
|-|-|
| 1 | Weak |
| 2 | Strong click |
| 3 | Buzz |
| 4 | Light |
| 5 | Medium |
| 6 | Strong tap (default) |
| 15 | Soft thud |
| 16 | Strong thud |

Try `trackpad-haptic tap N 1` for N in 1–16 to feel what your machine does.

## How it works

1. `dlopen`s `/System/Library/PrivateFrameworks/MultitouchSupport.framework`
2. Lists multitouch devices and finds one whose actuator opens successfully
3. For each pulse: create actuator → open → actuate → close (handles are treated as single-shot)

Linked frameworks: `CoreFoundation`, `IOKit`. MultitouchSupport is not linked at build time.

## Caveats

- **Private API** — can break or change behavior on macOS updates
- **No Force Touch** → exits with `No Force Touch trackpad actuator found`
- May need Accessibility / Input Monitoring in some edge cases; usually works for a normal user session without extra TCC prompts
- Not for App Store distribution

## Uninstall

```bash
rm -f ~/.local/bin/trackpad-haptic
```

## License

MIT
