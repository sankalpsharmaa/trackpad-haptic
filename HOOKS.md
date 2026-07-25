# Wire trackpad-haptic into your agent hooks

1. Install the binary (`make && make install` in this repo).
2. Copy **everything below the horizontal line** into a coding agent on *this* Mac.
3. Let the agent inspect your local configs and merge the hooks. Do not hand-edit JSON from memory.

---

Wire [trackpad-haptic](https://github.com/sankalpsharmaa/trackpad-haptic) so my Force Touch trackpad buzzes when a coding-agent turn finishes on this machine.

You are setting this up on **my** Mac. Inspect real files. Merge carefully. Do not invent configs for tools I do not use.

## Success criteria

- `trackpad-haptic` is installed and a manual `tap` is felt.
- Every agent I actually use fires a haptic on turn-finished / Stop.
- Existing hooks are preserved (append/merge only).
- No double-buzz from two hooks calling the binary on the same stop.
- You tell me exactly how to reload each tool and how to verify.

## Phase 0 — binary

1. Resolve the binary path (prefer the first that exists and is executable):
   - `"$HOME/.local/bin/trackpad-haptic"`
   - `command -v trackpad-haptic`
   - build from this repo if missing: `make && make install`
2. Smoke-test **before** editing configs:

   ```bash
   /path/to/trackpad-haptic tap 2 1
   ```

   I should feel one strong click. If stderr says no Force Touch actuator, stop and report — hooks cannot fix missing hardware.
3. In every hook config, store an **absolute** path (expand `$HOME` to `/Users/...`). Hook runners often have a minimal `PATH` and will not see `~/.local/bin`.
4. Default finish command:

   ```bash
   /Users/<me>/.local/bin/trackpad-haptic tap
   ```

   That is waveform `6`, count `3`, interval `400` ms (~1.2s of buzzing). Set hook `timeout` to **at least 5** (prefer **8**) seconds so the process is not killed mid-tap.

## Phase 1 — discover what I use

Check which of these exist **before** writing anything:

| Tool | Config locations to read |
|-|-|
| Cursor | `~/.cursor/hooks.json`, `<project>/.cursor/hooks.json` |
| Claude Code | `~/.claude/settings.json`, `~/.claude/settings.local.json`, project `.claude/settings.json` / `settings.local.json` |
| Codex | `~/.codex/hooks.json`, project `.codex/hooks.json`, plus `~/.codex/config.toml` if Codex stores hook trust/hashes there |

Also search my home / this repo for existing haptic wrappers, e.g. scripts whose names mention `haptic`, `trackpad`, `agent-status`, `agent-done`. If something already calls `trackpad-haptic` on stop, **reuse it** — do not add a second Stop hook that also taps.

Ask me before creating a brand-new global hooks file for a tool that has **no** config and that I clearly do not run.

## Phase 2 — wire each tool (merge only)

### Shared rules

- Read the full file first. Parse JSON. Append one new hook entry; never replace an entire `hooks` / event array.
- Keep valid JSON (Cursor wants `"version": 1` at the top level when present).
- Prefer a tiny shell wrapper only if the platform needs stdout JSON or argument folding — see below.
- After edits, show me a unified diff of each file you changed.

### Cursor

- Event: **`stop`** (turn finished). Docs: [Cursor hooks](https://cursor.com/docs/hooks).
- File shape:

  ```json
  {
    "version": 1,
    "hooks": {
      "stop": [
        {
          "command": "/Users/<me>/.local/bin/trackpad-haptic tap",
          "description": "Trackpad haptic when the agent finishes",
          "timeout": 8
        }
      ]
    }
  }
  ```

- Merge into the existing `hooks.stop` array if it already exists.
- Cursor may send JSON on stdin and expect JSON on stdout. A bare `trackpad-haptic` ignores stdin and prints nothing — that is usually fine (`{}` / empty). If Cursor’s Hooks output panel shows failures about invalid JSON / exit codes, wrap:

  ```bash
  # ~/.local/bin/trackpad-haptic-cursor-stop
  #!/bin/zsh
  /Users/<me>/.local/bin/trackpad-haptic tap
  print -r -- '{}'
  ```

  `chmod +x` it and point `command` at that wrapper instead.
- User-global and project hooks can **both** run. Do not add the same tap in both places.
- Reload: quit/reopen Cursor, or otherwise reload hooks; check **View → Output → Hooks** if it does not fire.

### Claude Code

- Event: **`Stop`** (PascalCase — not Cursor’s `stop`).
- Typical location: `~/.claude/settings.json` under `hooks.Stop`.
- Shape (command hook):

  ```json
  {
    "hooks": {
      "Stop": [
        {
          "matcher": "",
          "hooks": [
            {
              "type": "command",
              "command": "/Users/<me>/.local/bin/trackpad-haptic tap",
              "timeout": 8
            }
          ]
        }
      ]
    }
  }
  ```

- Merge into existing `Stop` entries; do not delete `PreToolUse` / `UserPromptSubmit` / permission hooks.
- If settings are symlinked into a git-managed dotfiles repo, edit the **real** file (some editors refuse writes through symlinks).
- After changing hook scripts or commands, Claude Code often needs **`/hooks`** or a full restart so the new command is trusted/loaded. Say so explicitly.
- Known footgun: editing a hook script mid-session can drop UserPromptSubmit until reload — warn me if you touch live hook scripts.

### Codex

- Event: **`Stop`** (same family as Claude Code).
- File: `~/.codex/hooks.json` (or project `.codex/hooks.json`).
- Shape is usually the Claude-like `type: command` list under `hooks.Stop`.
- Codex may require **trusting** new hook command strings (hashes in `~/.codex/config.toml` or a first-run prompt). After adding the command, check whether trust must be updated; do not leave me with a silently skipped hook.
- Reload: restart Codex / re-approve the hook if prompted.

## Phase 3 — multi-agent behavior

Default: **tap on every Stop** for that tool. Fine when one agent finishes while another is still working.

Only if I already have a shared lease/refcount “agent status” wrapper:

- Do **not** add a raw second `trackpad-haptic` Stop hook.
- Ensure that wrapper’s idle/done path calls the absolute `trackpad-haptic` path.
- Optionally keep busy-start hooks (`beforeSubmitPrompt` / `UserPromptSubmit` / `PreToolUse`) as they are — this prompt is about the finish buzz.

If I ask for last-idle-only and I have no wrapper yet, implement the smallest possible lease dir (acquire on start, release on stop, tap only when count hits zero). Do not build that unless I ask.

## Phase 4 — verify

Run through this checklist and report pass/fail:

1. Manual: `trackpad-haptic tap 2 1` → felt.
2. Manual absolute path from a clean env:

   ```bash
   env -i HOME="$HOME" USER="$USER" /Users/<me>/.local/bin/trackpad-haptic tap 2 1
   ```

3. Config: `jq`/Python parse of each edited JSON file succeeds.
4. `rg -n 'trackpad-haptic' ~/.cursor ~/.claude ~/.codex` (and project copies you edited) shows the expected Stop/stop entries and **no accidental duplicates**.
5. Live: one short agent turn in each wired tool → taps at the end.
6. If live turn fails: paste/read the tool’s hook log / Output → Hooks / Codex trust errors and fix.

## Constraints

- macOS + Force Touch only.
- Minimal diff. No drive-by refactors of my hook system.
- No committing or pushing unless I ask.
- Prefer user-global hooks for “always on every project,” project hooks only if that is already how I organize things.
- If anything is ambiguous (two Claude settings files, symlink farms, conflicting wrappers), stop and ask rather than guessing.
