# trackpad-haptic

When an agent finishes, my trackpad taps me. I don't have to watch the window.

## Why I built this

I keep several coding agents running at once: Cursor, Claude Code, Codex. Each one sits in its own tab. If I look away for a few minutes, I lose the plot. Which ones are still thinking? Which ones are stuck waiting on me?

I tried the usual signals first. Notification sounds disappear into a laptop day. Dock badges sit unread until I happen to glance at them. The cue I don't miss is a Force Touch pulse under my fingers. When a turn ends, taps hit the trackpad. I can stay in a paper or another editor and still know something needs a reply.

This repo is that cue as a one-line command. Wire it into your agent’s stop / finish hook:

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

After install, point each agent’s “turn finished” hook at `~/.local/bin/trackpad-haptic tap`. Use an absolute path in hook configs so the agent’s shell does not need your interactive `PATH`.

### Cursor

Create or edit `~/.cursor/hooks.json` (or the project’s `.cursor/hooks.json`):

```json
{
  "version": 1,
  "hooks": {
    "stop": [
      {
        "command": "$HOME/.local/bin/trackpad-haptic tap",
        "description": "Trackpad taps when the agent finishes",
        "timeout": 5
      }
    ]
  }
}
```

`stop` runs when the agent turn ends. Restart Cursor (or reload hooks) if a new file does not pick up right away.

### Claude Code

In `~/.claude/settings.json` (or a project `.claude/settings.json`), add a `Stop` hook:

```json
{
  "hooks": {
    "Stop": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "$HOME/.local/bin/trackpad-haptic tap",
            "timeout": 8
          }
        ]
      }
    ]
  }
}
```

Merge that into your existing `hooks` object if you already have other Stop hooks. After editing hook scripts, run `/hooks` in Claude Code or restart the session so the new command is trusted and loaded.

### Codex

In `~/.codex/hooks.json` (or the project’s `.codex/hooks.json`):

```json
{
  "hooks": {
    "Stop": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "$HOME/.local/bin/trackpad-haptic tap",
            "timeout": 8
          }
        ]
      }
    ]
  }
}
```

Codex may ask you to trust new hook commands the first time they run.

### Minimal shell wrapper (optional)

If you want a short name or extra args later:

```bash
# ~/.local/bin/agent-done-haptic
#!/bin/sh
exec "$HOME/.local/bin/trackpad-haptic" tap 6 3 400
```

```bash
chmod +x ~/.local/bin/agent-done-haptic
```

Point the hooks above at `$HOME/.local/bin/agent-done-haptic` instead.

### Multiple agents at once

A bare `tap` on every Stop fires whenever *that* agent finishes, even if another is still working. That is usually what you want. If you only want a buzz when the *last* busy agent goes idle, you need a small lease / refcount layer in front of this binary (each start acquires, each stop releases, tap only at zero). This repo ships the buzz itself; keep the counting in your own hooks if you need it.

## Options

```text
trackpad-haptic tap [waveform] [count] [interval_ms]
```

Defaults: waveform `6` (strong tap), count `3`, interval `400` ms.

Waveforms worth trying: `1` weak, `2` strong click, `3` buzz, `6` strong tap, `15` soft thud, `16` strong thud. Apple does not document these IDs; they can change with macOS updates. If your machine has no Force Touch actuator, the command exits with an error.
