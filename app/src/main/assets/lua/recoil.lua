local Config = require("config")

if Config.Recoil.Enabled then
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
