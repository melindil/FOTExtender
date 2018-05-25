function OnStart ()

    logger:log("Entered OnStart")

    newperk = {name = "hulkSmash",
               minlevel = 16,
               maxperktaken = 1,
               requiredstat1 = "doctor",
               requiredamt1 = 50,
               requiredEN = 5
              }
    hookexecutor:ReplacePerk(newperk, 18)
end

function OnRadiated(e)
    logger:log("Radiated: " .. e["name"])
    if e:HasPerk("hulkSmash") then
      logger:log("Radiated and has perk: " .. e["name"])
      if e:GetTempPerkValue("hulkSmash") == 0 then
        logger:log(e["name"] .. " gains hulkSmash")
        e:DisplayMessage("<Cg>HULK SMASH!")
        bonus = {strength=2, radiationResist=75}
        e:ApplyTempBonus(bonus)
      end
      e:SetTempPerkValue("hulkSmash",6)	-- # of big ticks to keep buff
    end
end

function OnLongTick(e)
    curr = e:GetTempPerkValue("hulkSmash")
    if curr > 0 then
      curr = curr - 1
      if curr == 0 then
        logger:log(e["name"] .. " hulkSmash expired")
        e:DisplayMessage("Normal")
        bonus = {strength=2, radiationResist=75}
        e:RemoveTempBonus(bonus)
      end
      e:SetTempPerkValue("hulkSmash",curr)
    end
end