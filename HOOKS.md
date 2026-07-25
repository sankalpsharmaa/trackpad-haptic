# Wire trackpad-haptic into your agent hooks

Copy everything below the line into a coding agent (Cursor, Claude Code, Codex, etc.). It will inspect *your* machine and edit the right config files — do not hand-merge JSON from a blog post.

---

Wire [trackpad-haptic](https://github.com/sankalpsharmaa/trackpad-haptic) so my trackpad buzzes when an agent turn finishes.

## Goal

When a coding agent finishes a turn, run:

```bash
"$HOME/.local/bin/trackpad-haptic" tap
```

(or whatever path `which trackpad-haptic` / `command -v trackpad-haptic` returns after install). Use an absolute path in hook configs. Do not assume my interactive `PATH` is available inside the hook runner.

## What you should do

1. Confirm the binary exists and works: run `trackpad-haptic tap 2 1` (one click). If missing, build/install from this repo (`make && make install`).
2. Discover which agents I actually use on this machine. Check for existing hook configs, for example:
   - Cursor: `~/.cursor/hooks.json`, project `.cursor/hooks.json`
   - Claude Code: `~/.claude/settings.json`, project `.claude/settings.json` / `settings.local.json`
   - Codex: `~/.codex/hooks.json`, project `.codex/hooks.json`, and any trust/hash entries Codex requires for new commands
3. Read those files first. Merge a finish/stop hook that calls the absolute `trackpad-haptic tap` command. Preserve every existing hook. Do not replace whole hook arrays.
4. Prefer the platform’s “turn finished” event:
   - Cursor: `stop`
   - Claude Code: `Stop`
   - Codex: `Stop`
5. Set a generous timeout (about 5–8 seconds) so a 3-tap default can finish.
6. After edits, tell me how to reload (restart app, `/hooks`, re-trust command, etc.) for each tool you touched.
7. Smoke-test: trigger a short agent turn or run the hook command manually and confirm I feel the taps.

## Constraints

- macOS + Force Touch only.
- Minimal change: add the haptic call; do not refactor my hook system.
- If I already have a status/lease wrapper that calls haptics, reuse it instead of double-firing.
- If several agents share one “last agent idle” refcount, keep that behavior and only ensure this binary is what fires the buzz.
- Ask me before creating brand-new global config files if none exist for a tool I never use.
