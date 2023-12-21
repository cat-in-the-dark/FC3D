print("Hello world!")

local timer = 0
local idx = 0

local n_models = 3;

function Update()
    timer = timer + dt
    if timer > 2 then
        idx = (idx + 1) % n_models
        timer = 0
    end
end

function Draw()
    -- mdl(3, 0,0,0, 1)
    mdl(idx + 1, 0,0,0, 1)
end