#include <cstring>

#include "game_patch/game_patch.h"
#include "data/items.h"
#include "data/stages.h"
#include "events.h"
#include "tp/d_a_alink.h"
#include "tp/d_com_inf_game.h"
#include "rando/randomizer.h"

namespace mod::game_patch
{
    void _07_checkPlayerStageReturn()
    {
        using namespace libtp::tp::d_com_inf_game;
        using namespace libtp::tp::d_a_alink;
        using namespace libtp::tp;
        using namespace libtp::data;

        // If we are not in a dungeon, we want to set our save warp to be the last entrance we entered.
        // As we randomize Boss/midboss rooms, this will have to be adjusted accordingly.
        if ((rando::gRandomizer->getSeedPtr()->getStageIDX() > 29) &&
            !checkStageName(stage::allStages[stage::StageIDs::Cave_of_Ordeals]))
        {
            libtp::tp::d_com_inf_game::dComIfG_inf_c* gameInfoPtr = &d_com_inf_game::dComIfG_gameInfo;
            libtp::tp::d_save::dSv_info_c* savePtr = &gameInfoPtr->save;

            d_save::dSv_player_return_place_c* playerReturnPlacePtr = &savePtr->save_file.player.player_return_place;
            d_stage::dStage_startStage* startStgPtr = &gameInfoPtr->play.mStartStage;
            const int32_t startPoint = static_cast<int32_t>(savePtr->mRestart.mStartPoint);

            if (startPoint == -0x4) // Portal
            {
                switch (rando::gRandomizer->getSeedPtr()->getStageIDX())
                {
                    case StageIDs::Snowpeak:
                    {
                        playerReturnPlacePtr->link_spawn_point_id = 5; // Snowpeak doesn't have a valid spawn 0 on the mountain.
                        break;
                    }
                    default:
                    {
                        playerReturnPlacePtr->link_spawn_point_id =
                            0; // Just set the spawn to 0 so that the player has a valid spawn
                        // location since you can't load a save from a portal spawn.
                        break;
                    }
                }
            }
            else
            {
                playerReturnPlacePtr->link_spawn_point_id = static_cast<uint8_t>(startPoint);
            }

            strncpy(playerReturnPlacePtr->link_current_stage,
                    startStgPtr->mStage,
                    sizeof(playerReturnPlacePtr->link_current_stage) - 1);

            playerReturnPlacePtr->link_room_id = startStgPtr->mRoomNo;
            playerReturnPlacePtr->unk10 = startStgPtr->mLayer;
        }
    }
} // namespace mod::game_patch
