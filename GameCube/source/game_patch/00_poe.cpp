#include "game_patch/game_patch.h"
#include "main.h"
#include "tp/d_com_inf_game.h"
#include "events.h"
#include "data/flags.h"

namespace mod::game_patch
{
    KEEP_FUNC void _00_handle_poeItem()
    {
        using namespace libtp::data::flags;

        uint8_t* poeCountPtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_collect.poe_count;
        rando::Randomizer* randoPtr = rando::gRandomizer;

        // Do not increment the poe count if it is already at 60
        const uint32_t currentPoeCount = *poeCountPtr;
        if (currentPoeCount >= 60)
        {
            return;
        }

        // Increment the poe count
        *poeCountPtr = static_cast<uint8_t>(currentPoeCount + 1);

        // Check if the HC barrier requires poes and if we have enough poe souls to set the flag.
        randoPtr->checkSetHCBarrierFlag(rando::HC_Poe_Souls, *poeCountPtr);

        // Check if the HC bk check requires poes and if we have enough poe souls to unlock the check.
        randoPtr->checkSetHCBkFlag(rando::HC_BK_Poe_Souls, *poeCountPtr);
    }
} // namespace mod::game_patch