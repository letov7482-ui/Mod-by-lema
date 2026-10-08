-- Основной конфиг мода
-- Загружается при старте игры

local Config = {}

Config.Recoil = {
    Enabled = true,
    VerticalMultiplier = 0.0,
    HorizontalMultiplier = 0.0,
}

Config.Spread = {
    Enabled = true,
    Multiplier = 0.0,
}

Config.Damage = {
    Enabled = true,
    Multiplier = 2.5,
}

Config.Speed = {
    Enabled = true,
    WalkMultiplier = 1.8,
    SprintMultiplier = 2.2,
}

Config.Aimbot = {
    Enabled = true,
    FOV = 120.0,
    Smooth = 0.3,
    TargetBone = "head",
}

Config.ESP = {
    Enabled = true,
    Box = true,
    Name = true,
    Distance = true,
    Health = true,
    Skeleton = false,
}

return Config
