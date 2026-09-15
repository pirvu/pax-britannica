/* The handful of entry points emscripten's GL and GLFW emulation is missing.
 *
 * gl.c and luaglfw.c bind hundreds of desktop functions, but the game only
 * calls about forty of them, and all but these four exist in emscripten. The
 * link is built with ERROR_ON_UNDEFINED_SYMBOLS=0, so the rest stay unresolved
 * and abort loudly if anything ever reaches them.
 */

#include <GL/gl.h>
#include <GL/glfw.h>

/* Legacy GL emulation implements the float variants only, and these two are
 * what every sprite and particle quad is built from. */

GLAPI void GLAPIENTRY glVertex2d( GLdouble x, GLdouble y )
{
  glVertex2f( (GLfloat)x, (GLfloat)y );
}

GLAPI void GLAPIENTRY glTexCoord2d( GLdouble s, GLdouble t )
{
  glTexCoord2f( (GLfloat)s, (GLfloat)t );
}

/* A canvas has no video mode list to enumerate; the kernel only logs these.
 * glfwGetDesktopMode does exist in emscripten and reports the canvas size. */

GLFWAPI int GLFWAPIENTRY glfwGetVideoModes( GLFWvidmode *list, int maxcount )
{
  (void)list; (void)maxcount;
  return 0;
}

/* No joystick support yet: reporting zero buttons makes the per-frame poll in
 * components/the_one_button.lua fall back to the keyboard. Wiring this to the
 * Gamepad API would be the natural place to add controller support. */

GLFWAPI int GLFWAPIENTRY glfwGetJoystickParam( int joy, int param )
{
  (void)joy; (void)param;
  return 0;
}

GLFWAPI int GLFWAPIENTRY glfwGetJoystickPos( int joy, float *pos, int numaxes )
{
  (void)joy; (void)pos; (void)numaxes;
  return 0;
}

GLFWAPI int GLFWAPIENTRY glfwGetJoystickButtons( int joy, unsigned char *buttons,
                                                 int numbuttons )
{
  (void)joy; (void)buttons; (void)numbuttons;
  return 0;
}
