#pragma once

namespace mod_settings
{
class BooleanSetting;
}
// Game-thread API shared by the recovery shortcut and the forthcoming settings UI.
mod_settings::BooleanSetting& FleetCommanderConfirmationSetting();
void                          InvalidateFleetCommanderConfirmationSession();

// One-way recovery action, called on the game thread by the hotkey dispatcher.
void EnableFleetCommanderAbilityConfirmation();
