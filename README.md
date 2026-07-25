# trackpad-haptic

When an agent finishes, my trackpad taps me. I don't have to watch the window.

## Why I built this

I keep several coding agents running at once: Cursor, Claude Code, Codex. Each one sits in its own tab. If I look away for a few minutes, I lose the plot. Which ones are still thinking? Which ones are stuck waiting on me?

I tried the usual signals first. Notification sounds disappear into a laptop day. Dock badges sit unread until I happen to glance at them. The cue I don't miss is a Force Touch pulse under my fingers. When a turn ends, taps hit the trackpad. I can stay in a paper or another editor and still know something needs a reply.

This repo is that cue as a one-line command:

```bash
trackpad-haptic tap
```

## Install

Needs macOS with a Force Touch trackpad (MacBook or Magic Trackpad) and Xcode Command Line Tools. MIT licensed.

```bash
git clone https://github.com/sankalpsharmaa/trackpad-haptic.git
cd trackpad-haptic
make && make install   # → ~/.local/bin/trackpad-haptic
```

Add `~/.local/bin` to your `PATH`, then try:

```bash
trackpad-haptic tap        # three strong taps, 400 ms apart
trackpad-haptic tap 2 1    # one strong click
trackpad-haptic tap 3 1    # one buzz
```

## Hook it up

Do not copy-paste JSON by hand. Open [HOOKS.md](HOOKS.md), copy the prompt under the line, and paste it into a coding agent on your machine. It will find your local Cursor / Claude Code / Codex configs and wire the stop hook to `trackpad-haptic`.

## Options

```text
trackpad-haptic tap [waveform] [count] [interval_ms]
```

Defaults: waveform `6` (strong tap), count `3`, interval `400` ms.

Waveforms worth trying: `1` weak, `2` strong click, `3` buzz, `6` strong tap, `15` soft thud, `16` strong thud. Apple does not document these IDs; they can change with macOS updates. If your machine has no Force Touch actuator, the command exits with an error.
