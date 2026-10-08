local Config = require("config")

local function HookSpeed()
    if not Config.Speed.Enabled then return end
    
    local original_GetWalkSpeed = GetWalkSpeed
    if original_GetWalkSpeed then
        GetWalkSpeed = function(...)
            return original_GetWalkSpeed(...) * Config.Speed.WalkMultiplier
        end
    end
    
    local original_GetSprintSpeed = GetSprintSpeed
    if original_GetSprintSpeed then
        GetSprintSpeed = function(...)
            return original_GetSprintSpeed(...) * Config.Speed.SprintMultiplier
        end
    end
end

HookSpeed()
