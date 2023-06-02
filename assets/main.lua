print("Hello world!")

local timer = 0
local idx = 0

function Update()
    timer = timer + dt
    if timer > 2 then
        idx = (idx + 1) % 2
        timer = 0
    end
end

function Draw()
    mdl(idx + 1, 0,0,0, 1)
end