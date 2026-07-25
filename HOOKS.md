# Wire trackpad-haptic into your agent hooks

1. Open this repo in a coding agent on your Mac (or paste [HOOKS.md](HOOKS.md) from GitHub).
2. Copy **everything below the line** into that agent.
3. Answer the interview questions. The agent installs the binary and wires your hooks — you should not run `make` or edit JSON yourself.

---

Wire [trackpad-haptic](https://github.com/sankalpsharmaa/trackpad-haptic) on this Mac so my Force Touch trackpad buzzes when a coding agent finishes a turn.

You own install **and** hooks. Clone/build/install the binary if needed, then merge stop hooks into my real configs. Do not replace hook arrays. Do not invent files for tools I never use. Do not ask me to run `make` or hand-edit JSON unless something is blocked (permissions, missing Xcode CLT, no Force Touch).

## How to work with me

**Interview before you edit.** If you have AskUserQuestion, `/interview`, or any structured multiple-choice tool, use it. Do not guess scope.

Interview rules:

- Ask **exactly 3 questions per round**.
- Each question: one sentence on why it matters, then concrete options, plus **Other / let me explain**.
- After each round, restate what you learned in one line per answer, then continue.
- Skip questions my files already answer (e.g. I already have a haptic wrapper).
- When choices are settled, implement. Show diffs. Tell me how to reload and verify.

If you have **no** interview tool, ask the same questions in chat as `1 / 2 / 3` choices and wait for answers before writing files.

## Interview question map

Work top to bottom. Drop any item already settled by inspection.

### Round A — who and where

1. **Which agents should buzz?**  
   Why: only those tools get hook edits.  
   Options: Cursor only · Claude Code only · Codex only · Cursor + Claude · all three I actually have configs for · Other

2. **Global or project hooks?**  
   Why: decides `~/.cursor` vs repo `.cursor`, and the Claude/Codex equivalents.  
   Options: user-global only (every project) · this project only · match whatever I already use · Other

3. **I already have a status/haptic wrapper — what should you do?**  
   Why: avoids double taps. Search for `trackpad-haptic`, `haptic`, `agent-status` first; then ask if unsure.  
   Options: reuse existing wrapper only · add raw `trackpad-haptic tap` on Stop · show me what you found and I’ll pick · Other

### Round B — feel and timing

4. **Buzz pattern on finish?**  
   Why: sets the command args and hook timeout.  
   Options: default triple tap (`tap`) · one strong click (`tap 2 1`) · one buzz (`tap 3 1`) · soft triple thud (`tap 15 3 300`) · Other

5. **When several agents run at once?**  
   Why: changes whether you need leases.  
   Options: tap whenever *that* agent stops (simple) · tap only when the *last* busy agent goes idle (needs refcount) · I already have last-idle logic — keep it · Other

6. **If last-idle and I have no wrapper yet?**  
   Why: only ask if they chose last-idle.  
   Options: smallest lease dir you can add · skip last-idle, use per-stop taps · Other

### Round C — safety and finish

7. **May you create a missing global hooks file?**  
   Why: some people have no `~/.cursor/hooks.json` on purpose.  
   Options: yes, create minimal file · no, only edit files that already exist · ask per tool · Other

8. **Git?**  
   Why: dotfiles repos are easy to commit by accident.  
   Options: edit only, no commit · commit if the file is in a git repo · Other

9. **Ready to implement from these answers?**  
   Options: yes · one more thing (I’ll explain) · show the plan first, then wait

After round C (or sooner if everything is clear), implement.

## Implementation playbook

Follow only after the interview (or after I paste answers).

### Binary

1. Resolve path, in order: `$HOME/.local/bin/trackpad-haptic` → `command -v trackpad-haptic` → build from this repo with `make && make install` (you run this; do not make me run it).
2. Feel-test before config edits:

   ```bash
   /absolute/path/to/trackpad-haptic tap 2 1
   ```

   If stderr reports no Force Touch actuator, stop. Hooks cannot fix that.
3. In configs, use the **absolute** path. Hook runners often lack my interactive `PATH`.
4. Hook `timeout`: at least **5** seconds; prefer **8** for the default 3×400 ms taps.

### Discover

Read before writing:

| Tool | Look here |
|-|-|
| Cursor | `~/.cursor/hooks.json`, `<project>/.cursor/hooks.json` |
| Claude Code | `~/.claude/settings.json`, `settings.local.json`, project `.claude/` copies |
| Codex | `~/.codex/hooks.json`, project `.codex/hooks.json`, trust/hashes in `~/.codex/config.toml` |

Search home and this machine for existing callers of `trackpad-haptic`. If a Stop/stop hook already buzzes, reuse that path.

### Merge rules

- Append one hook entry. Never wipe an event array.
- Keep valid JSON. Cursor: keep `"version": 1` when present.
- Edit symlink targets (real files under a dotfiles repo), not the symlink path if the editor would refuse.
- Show a unified diff per file.
- No drive-by refactors. No commit/push unless I said so in the interview.

### Cursor

- Event: `stop` ([docs](https://cursor.com/docs/hooks)).
- Example entry:

  ```json
  {
    "command": "/Users/<me>/.local/bin/trackpad-haptic tap",
    "description": "Trackpad haptic when the agent finishes",
    "timeout": 8
  }
  ```

- User-global and project hooks can both run — wire one place only.
- If Output → Hooks complains about stdout JSON, wrap:

  ```bash
  #!/bin/zsh
  /Users/<me>/.local/bin/trackpad-haptic tap
  print -r -- '{}'
  ```

- Reload: restart Cursor; debug via **View → Output → Hooks**.

### Claude Code

- Event: `Stop` (PascalCase).
- Command hook under `hooks.Stop` with `"type": "command"`, absolute command, `timeout` 8.
- Preserve `PreToolUse`, `UserPromptSubmit`, permissions, etc.
- Reload: `/hooks` or restart. Warn me if you edit a live hook script (UPS can drop until reload).

### Codex

- Event: `Stop`, same command-hook shape as Claude Code.
- Update trust/hashes if Codex requires them for new command strings; a skipped untrusted hook looks like “not wired.”
- Reload: restart Codex / approve trust prompts.

### Multi-agent

- **Per-stop** (default from interview): each tool’s Stop calls `trackpad-haptic` with the chosen args.
- **Last-idle**: only if I chose it — reuse my wrapper, or add the smallest lease acquire/release and tap at zero.
- Never double-fire wrapper + raw `tap` on the same Stop.

### Verify

Report pass/fail:

1. Manual feel: `tap 2 1`.
2. Clean env: `env -i HOME="$HOME" USER="$USER" /absolute/path/trackpad-haptic tap 2 1`
3. JSON still parses.
4. `rg -n 'trackpad-haptic' ~/.cursor ~/.claude ~/.codex` (plus project paths you touched) — expected entries, no duplicates.
5. One short live turn per wired tool → buzz at the end.
6. On failure: read that tool’s hook log / trust errors and fix.

End with: what changed, how to reload each tool, and what I should feel on the next finished turn.
