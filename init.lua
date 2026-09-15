dokidoki_disable_debug = true
require 'dokidoki.module' [[]]

local kernel = require 'dokidoki.kernel'

local the_game = require 'the_game'

local args = {}
for _, a in ipairs(arg) do
  args[a] = true
end

kernel.set_ratio(4/3)

-- The browser build always runs in the canvas: real fullscreen needs a user
-- gesture, so it is offered from the page instead.
local on_web = pcall(require, 'web')

if on_web or args['--windowed'] then
  kernel.set_video_mode(1024, 768)
else
  kernel.set_fullscreen(true)
end

kernel.start_main_loop(the_game.make())
