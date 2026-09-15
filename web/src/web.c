/* Lua module 'web': the few browser services the engine needs.
 *
 * The desktop kernel paces itself by sleeping until the next frame is due and
 * then spinning on the clock. Neither works in a browser: there is no thread to
 * block, and a spin loop freezes the tab. Instead the kernel waits for the next
 * animation frame, which is the browser's own idea of when a frame is due.
 *
 * Waiting means returning to the JS event loop from the middle of the Lua
 * interpreter, so this needs ASYNCIFY to unwind and rewind the C stack.
 */

#include <emscripten.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

EM_ASYNC_JS(void, web_next_frame, (), {
  await new Promise(function (resolve) { requestAnimationFrame(resolve); });
});

/* Emscripten's GLFW2 shim returns success from glfwOpenWindow even when the
 * WebGL context could not be created, which otherwise shows up only as a black
 * canvas. Module.ctx is what Browser.createContext stores on success. */
EM_JS(int, web_has_gl_context, (), {
  return Module['ctx'] ? 1 : 0;
});

/* The on-screen pads in shell.html. The game is one button per player, so a
 * touch device needs nothing more than four booleans; the shell keeps them in
 * Module.paxButtons and components/the_one_button.lua ORs them into the
 * keyboard state. Out-of-range or missing means "not held". */
EM_JS(int, web_button_held, (int player), {
  var b = Module['paxButtons'];
  return (b && b[player]) ? 1 : 0;
});

static int web__button_held(lua_State *L)
{
  int player = luaL_checkint(L, 1);
  lua_pushboolean(L, player >= 1 && player <= 4 && web_button_held(player - 1));
  return 1;
}

static int web__has_gl_context(lua_State *L)
{
  lua_pushboolean(L, web_has_gl_context());
  return 1;
}

static int web__next_frame(lua_State *L)
{
  (void)L;
  web_next_frame();
  return 0;
}

static const luaL_Reg functions[] =
{
  {"next_frame", web__next_frame},
  {"has_gl_context", web__has_gl_context},
  {"button_held", web__button_held},
  {NULL, NULL}
};

int luaopen_web(lua_State *L)
{
  lua_newtable(L);
  luaL_register(L, NULL, functions);
  return 1;
}
