local Config = require("config")

local function HookSpread()
    if not Config.Spread.Enabled then return end
    
    local original_CalculateSpread = CalculateSpread
    if original_CalculateSpread then
        CalculateSpread = function(...)
            local spread = original_CalculateSpread(...)
            if spread then
                spread = spread * Config.Spread.Multiplier
            end
            return spread
        end
    end
end

HookSpread()
