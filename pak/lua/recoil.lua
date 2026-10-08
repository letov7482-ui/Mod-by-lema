-- Отключение отдачи
local Config = require("config")

local function HookRecoil()
    if not Config.Recoil.Enabled then return end
    
    local original_CalculateRecoil = CalculateRecoil
    if original_CalculateRecoil then
        CalculateRecoil = function(...)
            local recoil = original_CalculateRecoil(...)
            if recoil then
                recoil.Vertical = recoil.Vertical * Config.Recoil.VerticalMultiplier
                recoil.Horizontal = recoil.Horizontal * Config.Recoil.HorizontalMultiplier
            end
            return recoil
        end
    end
end

HookRecoil()
