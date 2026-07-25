# trackpad-haptic

Your trackpad buzzes when an agent finishes — so you don’t have to stare at the screen.

## Why

I run several coding agents at once (Cursor, Claude Code, Codex, …). Tabs pile up. I look away, come back, and have no idea which ones are still thinking and which are waiting on me.

Sound notifications get ignored. Dock badges get ignored. What actually works: a physical tap under my fingers. When the last busy agent goes idle, the trackpad buzzes. I can keep reading a paper or writing elsewhere and still know something needs my attention.

That’s it. A tiny CLI that fires Force Touch haptics so “agent done” is something you *feel*.

Wire it into whatever hooks / scripts you already use when a turn ends. Example: three strong taps when work finishes.

```bash
trackpad-haptic tap
```

## Install

macOS with a Force Touch trackpad (MacBook or Magic Trackpad). Needs Xcode Command Line Tools.

```bash
git clone https://github.com/sankalpsharmaa/trackpad-haptic.git
cd trackpad-haptic
make && make install   # installs to ~/.local/bin
```

Put `~/.local/bin` on your `PATH`, then try:

```bash
trackpad-haptic tap        # 3 taps (default)
trackpad-haptic tap 2 1    # one strong click
trackpad-haptic tap 3 1    # one buzz
```

## Options

```text
trackpad-haptic tap [waveform] [count] [interval_ms]
```

Defaults: waveform `6` (strong tap), count `3`, interval `400` ms.

Handy waveforms to try: `1` weak · `2` strong click · `3` buzz · `6` strong tap · `15` soft thud · `16` strong thud.

## Notes

- Uses Apple’s private trackpad haptic APIs — fine for personal tooling, can break on OS updates.
- No Force Touch hardware → it just errors out politely.
- MIT licensed.
