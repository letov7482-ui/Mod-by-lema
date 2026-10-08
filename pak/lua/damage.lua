local Config = require("config")

local function HookDamage()
    if not Config.Damage.Enabled then return end
    
    local original_CalculateDamage = CalculateDamage
    if original_CalculateDamage then
        CalculateDamage = function(...)
            local damage = original_CalculateDamage(...)
            if damage then
                damage = damage * Config.Damage.Multiplier
            end
            return damage
        end
    end
end

HookDamage()
