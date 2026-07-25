# trackpad-haptic

When an agent finishes, my trackpad taps me. I don't have to watch the window.

## Why I built this

I keep several coding agents running at once: Cursor, Claude Code, Codex. Each one sits in its own tab. If I look away for a few minutes, I lose the plot. Which ones are still thinking? Which ones are stuck waiting on me?

I tried the usual signals first. Notification sounds disappear into a laptop day. Dock badges sit unread until I happen to glance at them. The cue I don't miss is a Force Touch pulse under my fingers. When a turn ends, taps hit the trackpad. I can stay in a paper or another editor and still know something needs a reply.

## Setup

macOS with a Force Touch trackpad. MIT licensed.

1. Clone this repo (or open it in your coding agent).
2. Open [HOOKS.md](HOOKS.md), copy everything **below the line**, and paste it into Cursor / Claude Code / Codex on this Mac.

The agent interviews you for a few choices, builds and installs the binary, and wires your stop hooks. You should not need to edit JSON or run `make` yourself.

## Try it (optional)

After setup, or if you only want a manual buzz:

```bash
trackpad-haptic tap        # three strong taps
trackpad-haptic tap 2 1    # one strong click
trackpad-haptic tap 3 1    # one buzz
```

```text
trackpad-haptic tap [waveform] [count] [interval_ms]
```

Defaults: waveform `6`, count `3`, interval `400` ms. Waveforms worth trying: `1` weak · `2` strong click · `3` buzz · `6` strong tap · `15` soft thud · `16` strong thud.
