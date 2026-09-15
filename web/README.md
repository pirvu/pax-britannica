# Pax Britannica in the browser

A WebAssembly build of the game, via emscripten. The desktop build is untouched:
everything here lives in `web/`, and the handful of changes outside it are
guarded so `make linux` still works exactly as before.

## Building

The engine lives in two submodules, which are branches of this same repository.
Getting them needs a config flag, for the reason explained in `compiling.txt`:

    git -c protocol.file.allow=always submodule update --init

Needs [emsdk](https://emscripten.org/docs/getting_started/downloads.html) and
`curl`. Lua 5.1.5 is downloaded from lua.org on the first build (checksummed;
the game needs 5.1 specifically, since dokidoki's component system is built on
`setfenv`/`getfenv`).

    source /path/to/emsdk/emsdk_env.sh
    make -C web          # builds web/dist/
    make -C web serve    # and serves it on http://localhost:8173

`dist/` must be served over HTTP, not opened as a `file://` URL — the game data
is fetched as `index.data`.

## How it fits together

| | |
|---|---|
| `Makefile` | the whole build; deliberately separate from `dokidoki-support/Makefile` |
| `src/shell.html` | the page: scaled canvas, click-to-play overlay, controls |
| `src/web.c` | Lua module `web`: frame pacing and a WebGL context check |
| `src/glu_web.c` | stands in for GLU, which emscripten does not have |
| `src/compat.c` | the four GL/GLFW entry points emscripten is missing |
| `src/web_loaders.h` | registers the `particles` and `web` Lua modules |

### The parts that needed real work

**The main loop.** `kernel.lua` ran a `while true` that never returned and
ended its frame wait in a spin loop — which in a browser is a hung tab. It now
waits for `requestAnimationFrame` instead, through `web.next_frame()`. Returning
to the event loop from inside the Lua interpreter is what `-sASYNCIFY` is for.

**Immediate-mode OpenGL.** `-sLEGACY_GL_EMULATION` covers `glBegin`/`glEnd` and
the matrix stack. One catch: the emulation packs a single interleaved vertex
buffer and infers the layout from what each vertex supplies, so `glColor` set
once per primitive — persistent state in desktop GL — misaligns every vertex
after the first. The draw code repeats the colour per vertex, which desktop GL
does not mind.

**Audio.** `mixer.c` gained an SDL2 backend feeding the existing `mix_into()`
from an audio callback, in place of the ALSA thread. The device is opened with
`allowed_changes = 0` so SDL resamples to the 44100Hz stereo the mixer produces,
rather than handing back whatever the hardware runs at.

**Undefined symbols are warnings.** `gl.c` and `luaglfw.c` bind hundreds of
desktop entry points the game never calls. Only the ones it does call have to
exist; anything else aborts loudly if ever reached.

## Known limitations

- No gamepad. `glfwGetJoystickButtons` is stubbed to report nothing, so the
  keyboard is the only input. Wiring it to the Gamepad API is the obvious next
  step; `src/compat.c` is where it goes.
- The wasm is ~1.2MB, most of it Asyncify instrumentation — Lua is full of
  indirect calls, so the transform is conservative. `ASYNCIFY_ONLY` would trim
  it considerably.
- Emscripten's SDL still uses a `ScriptProcessorNode`, which runs on the main
  thread alongside the game loop. Hence the 1024-frame buffer.
