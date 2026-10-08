local Config = require("config")

if Config.Spread.Enabled then
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
