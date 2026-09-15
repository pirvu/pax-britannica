# Browser (WebAssembly) build of Pax Britannica.
#
# Deliberately separate from dokidoki-support/Makefile: the desktop build stays
# untouched. Run `source ~/emsdk/emsdk_env.sh` first, then `make`.
#
#   make          build dist/index.html and friends
#   make lua      build the bundled Lua 5.1.5 into a wasm static library
#   make serve    serve dist/ on http://localhost:8173
#   make clean    remove build/ and dist/ (keeps third_party/)

ROOT    := ..
SUPPORT := $(ROOT)/dokidoki-support
LUA_VERSION := 5.1.5
LUA_SHA256  := 2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333
LUA_DIR     := third_party/lua-$(LUA_VERSION)/src
BUILD   := build
DIST    := dist
DATA    := $(BUILD)/data
PORT    := 8173

#### Sources ##################################################################

# glu.c is replaced by src/glu_web.c: emscripten has no GLU.
SUPPORT_SRCS := collision.c gl.c log.c lua_stb_image.c luaglfw.c memarray.c \
                minlua.c mixer.c stb_vorbis.c

OBJS := $(SUPPORT_SRCS:%.c=$(BUILD)/%.o) \
        $(BUILD)/particles.o \
        $(BUILD)/web.o \
        $(BUILD)/glu_web.o \
        $(BUILD)/compat.o

# Lua's own sources, minus the standalone interpreter and compiler drivers.
# Listed rather than globbed because the tree does not exist until it is
# downloaded, and make expands a wildcard before that can happen.
LUA_MODULES := lapi lauxlib lbaselib lcode ldblib ldebug ldo ldump lfunc lgc \
               linit liolib llex lmathlib lmem loadlib lobject lopcodes loslib \
               lparser lstate lstring lstrlib ltable ltablib ltm lundump lvm lzio
LUA_OBJS := $(LUA_MODULES:%=$(BUILD)/lua/%.o)

#### Flags ####################################################################

# DOKIDOKI_EMSCRIPTEN is not one of the platforms minlua.c/mixer.c know about,
# so they fall through to their portable branches: no chdir, no audio device.
CFLAGS := -O2 -I$(LUA_DIR) \
          -DMEMARRAY_USE_OPENGL \
          -DDOKIDOKI_EMSCRIPTEN \
          -DEXTRA_LOADERS='"../web/src/web_loaders.h"' \
          -sUSE_GLFW=2 \
          -sUSE_SDL=2 \
          -Wno-deprecated-non-prototype -Wno-empty-body

# LEGACY_GL_EMULATION covers the glBegin/glEnd + matrix stack drawing in
# graphics.lua and particles.c. ASYNCIFY lets the Lua main loop yield to the
# browser from inside the interpreter (see src/web.c). Undefined symbols are
# warnings, not errors: gl.c and luaglfw.c bind hundreds of desktop entry
# points the game never calls.
LDFLAGS := -O2 \
           -sUSE_GLFW=2 \
           -sUSE_SDL=2 \
           -sLEGACY_GL_EMULATION \
           -sASYNCIFY \
           -sALLOW_MEMORY_GROWTH \
           -sINITIAL_MEMORY=64MB \
           -sSTACK_SIZE=4MB \
           -sERROR_ON_UNDEFINED_SYMBOLS=0 \
           -sEXIT_RUNTIME=0 \
           --shell-file src/shell.html \
           --preload-file $(DATA)@/

#### Rules ####################################################################

.PHONY: all lua data serve clean distclean

all: $(DIST)/index.html

$(DIST)/index.html: $(OBJS) $(BUILD)/liblua.a src/shell.html | data
	@mkdir -p $(DIST)
	emcc $(OBJS) $(BUILD)/liblua.a $(LDFLAGS) -o $@

$(BUILD)/%.o: $(SUPPORT)/%.c | $(LUA_DIR)/lua.h
	@mkdir -p $(BUILD)
	emcc $(CFLAGS) -c $< -o $@

$(BUILD)/particles.o: $(ROOT)/particles.c | $(LUA_DIR)/lua.h
	@mkdir -p $(BUILD)
	emcc $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.c | $(LUA_DIR)/lua.h
	@mkdir -p $(BUILD)
	emcc $(CFLAGS) -c $< -o $@

lua: $(BUILD)/liblua.a

# The order-only prerequisite downloads the sources first; the objects are then
# built by a sub-make, by which time the .c files make needs actually exist.
$(BUILD)/liblua.a: | $(LUA_DIR)/lua.h
	@$(MAKE) --no-print-directory $(LUA_OBJS)
	emar rcs $@ $(LUA_OBJS)
	emranlib $@

$(BUILD)/lua/%.o: $(LUA_DIR)/%.c
	@mkdir -p $(BUILD)/lua
	emcc -O2 -DLUA_ANSI -Wno-empty-body -c $< -o $@

# Lua 5.1 in particular: the component system in dokidoki/game.lua is built on
# setfenv/getfenv and minlua.c calls lua_open, none of which survived into 5.2.
$(LUA_DIR)/lua.h:
	@mkdir -p third_party
	@echo "fetching lua-$(LUA_VERSION) from lua.org"
	@curl -sSLo third_party/lua.tar.gz \
	  https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	@echo "$(LUA_SHA256)  third_party/lua.tar.gz" | sha256sum -c -
	@tar xzf third_party/lua.tar.gz -C third_party
	@rm -f third_party/lua.tar.gz

# The game loads everything by path relative to the working directory, so the
# preloaded filesystem has to mirror the source tree layout.
data:
	@rm -rf $(DATA)
	@mkdir -p $(DATA)/components $(DATA)/scripts $(DATA)/sprites $(DATA)/audio
	@cp $(ROOT)/*.lua $(DATA)/
	@cp $(ROOT)/components/*.lua $(DATA)/components/
	@cp $(ROOT)/scripts/*.lua $(DATA)/scripts/
	@cp $(ROOT)/sprites/*.png $(DATA)/sprites/
	@cp $(ROOT)/audio/music.ogg $(DATA)/audio/
	@mkdir -p $(DATA)/dokidoki
	@cd $(ROOT)/dokidoki && \
	  find . -name '*.lua' -not -path './examples/*' -not -path './tests/*' \
	    -exec cp --parents {} $(CURDIR)/$(DATA)/dokidoki/ \;
	@echo "staged $$(find $(DATA) -type f | wc -l) files into $(DATA)"

serve: all
	@echo "http://localhost:$(PORT)/"
	@cd $(DIST) && python3 -m http.server $(PORT)

clean:
	rm -rf $(BUILD)/*.o $(BUILD)/data $(DIST)

distclean:
	rm -rf $(BUILD) $(DIST)
