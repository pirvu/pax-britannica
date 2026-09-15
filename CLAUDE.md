# Pax Britannica — repository guide

One-button underwater RTS from GAMMA IV (2009). A thin C layer hosts Lua 5.1;
all gameplay is Lua. The `wasm-port` branch adds a browser build.

## Branch layout — read this before any git work

This repository holds **three unrelated histories**. `git merge-base` finds no
common ancestor between any pair; each has its own initial commit.

| branch | what |
|---|---|
| `master` | the game (267 commits, ends 2011-05-13) |
| `dokidoki` | the Lua framework, upstream's tip |
| `dokidoki-support` | the C layer, upstream's tip |
| `dokidoki-wasm` / `dokidoki-support-wasm` | the two above, plus one browser-support commit each |
| `wasm-port` | the game plus `web/`, gitlinks pointing at the two `-wasm` branches |

`.gitmodules` points both submodules at `url = .` — the superproject itself. So
`git clone` gets all three histories and the submodule step just materialises
two of them into subdirectories.

**Cloning needs a flag.** Since git 2.38 (CVE-2022-39253) a submodule may not be
cloned over the file transport, and `.` is a local path:

    git -c protocol.file.allow=always submodule update --init

A plain `git submodule update --init` fails with `transport 'file' not allowed`.
This is the single most likely thing to waste your time here.

A gitlink pins a SHA, not a branch. The `-wasm` branches exist only to keep
those commits reachable; the originals stay untouched as upstream mirrors.
Changing engine code therefore means **two commits in two places**: commit on
the `-wasm` branch, push it, then bump the gitlink on `wasm-port`.

## Building

Desktop is unchanged — `make linux` (see `compiling.txt`). Browser:

    source ~/emsdk/emsdk_env.sh      # emsdk is NOT on PATH by default
    make -C web                      # -> web/dist
    make -C web serve                # -> http://localhost:8173

`web/README.md` covers the port in detail. `web/` is self-contained; the desktop
build was deliberately not turned into a fourth platform in
`dokidoki-support/Makefile`.

## Deployment

`https://pax.pirvu.ro`, on `v1.pirvu.ro`, from `ghcr.io/pirvu/pax-britannica`.
CI builds and publishes on every push to `wasm-port`. To ship:

    ssh root@v1.pirvu.ro 'cd /opt/compose/pax && docker compose pull && docker compose up -d'

## What bit us, so it doesn't bite you again

**Emscripten's immediate-mode GL emulation packs one interleaved vertex buffer
per `glBegin`/`glEnd` and infers the layout from what each vertex supplies.**
Desktop GL treats `glColor` as persistent state; here, a colour set once and
then omitted misaligns every vertex after it. The radial menus and particle
quads drew as long bands across the screen. Every vertex now repeats its
attributes, which desktop GL does not mind. If new drawing code misbehaves
geometrically, check this first.

**The Lua main loop cannot block.** `kernel.lua` used to spin on the clock,
which freezes a browser tab. It now waits on `requestAnimationFrame` via the
`web` module (`web/src/web.c`), which needs `-sASYNCIFY` to unwind the C stack
from inside the interpreter.

**Audio format negotiation is a trap.** `SDL_OpenAudio(&want, &have)` means
"give me whatever the hardware runs at" — on a 48kHz device that is not the
44100Hz the mixer produces, and there is no resampler. Use
`SDL_OpenAudioDevice(..., allowed_changes = 0)` so SDL converts. A 48kHz device
took two debugging rounds to find because the failure looked like a black canvas.

**Failures during startup are silent by default.** The engine logged to a file
in the browser's in-memory filesystem, and emscripten's `glfwOpenWindow` reports
success even when no WebGL context was created. Both are fixed — logs go to the
console, and the context is checked explicitly — but if a change ever makes the
canvas go black with a clean console, suspect something before the first draw.

**`scripts/production.lua` is the one CRLF file in the repo.** Editors and
scripts that normalise line endings turn a three-line change into 213. Check
`git diff --stat` before committing.

**nginx `gzip_proxied` defaults to `off`**, and it counts a request as proxied
when it carries a `Via` header, which the Caddy edge adds. Local testing hits
the container directly and compresses fine; only the deployment reveals it.

## Conventions

Commit messages: imperative subject, body explaining *why* rather than what;
they end with a `Co-Authored-By: Claude` line. Prose in docs and comments is
British-flavoured and avoids em dashes in favour of commas or parentheses.
