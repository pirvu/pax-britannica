/* Minimal replacement for dokidoki-support/glu.c on the web.
 *
 * Emscripten has no GLU, and the game only ever calls gluBuild2DMipmaps (from
 * dokidoki/graphics.lua). Since every texture is created with GL_NEAREST for
 * both the min and mag filter, mipmaps are never sampled, so uploading level 0
 * with glTexImage2D is equivalent. The Lua-visible module name and function
 * signature match the real binding so graphics.lua needs no change.
 */

#include <GLES2/gl2.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static const void *checkpointer(lua_State *L, int num)
{
  if(lua_islightuserdata(L, num))
    return lua_touserdata(L, num);
  else if(lua_isstring(L, num))
    return lua_tostring(L, num);
  luaL_argerror(L, num, "expected lightuserdata or string");
  return NULL;
}

static int glu__gluBuild2DMipmaps(lua_State *L)
{
  GLenum target = (GLenum)luaL_checknumber(L, 1);
  GLint internal_format = (GLint)luaL_checknumber(L, 2);
  GLsizei width = (GLsizei)luaL_checknumber(L, 3);
  GLsizei height = (GLsizei)luaL_checknumber(L, 4);
  GLenum format = (GLenum)luaL_checknumber(L, 5);
  GLenum type = (GLenum)luaL_checknumber(L, 6);
  const void *data = checkpointer(L, 7);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(target, 0, internal_format, width, height, 0, format, type,
               data);

  lua_pushnumber(L, 0); /* GLU_OK */
  return 1;
}

static const luaL_Reg functions[] =
{
  {"gluBuild2DMipmaps", glu__gluBuild2DMipmaps},
  {NULL, NULL}
};

int luaopen_glu(lua_State *L)
{
  lua_newtable(L);
  luaL_register(L, NULL, functions);
  return 1;
}
