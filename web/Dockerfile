# Builds the browser version and serves it from nginx.
#
#   docker build -f web/Dockerfile -t pax-britannica .
#   docker run --rm -p 8080:80 pax-britannica
#
# The build context is the repository root, not web/: the game's Lua, sprites
# and audio live there, and so do the two engine submodules. Initialise those
# first or the build fails on a missing dokidoki/:
#
#   git -c protocol.file.allow=always submodule update --init

# Pinned to match the toolchain the port was developed against.
FROM emscripten/emsdk:6.0.9 AS build

WORKDIR /src
COPY . .

# Downloads and checksums Lua 5.1.5, then builds web/dist.
RUN make -C web

FROM nginx:1.27-alpine

COPY web/docker/nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /src/web/dist /usr/share/nginx/html

EXPOSE 80
