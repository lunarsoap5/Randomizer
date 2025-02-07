#include <cstring>

#include "game_patch/game_patch.h"
#include "data/items.h"
#include "data/stages.h"
#include "events.h"
#include "tp/d_a_alink.h"
#include "tp/d_com_inf_game.h"

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
        if (!checkDungeon() && !checkBossRoom() && !checkStageName(stage::allStages[stage::StageIDs::Cave_of_Ordeals]))
        {
            d_save::dSv_player_c* playerPtr = &d_com_inf_game::dComIfG_gameInfo.save.save_file.player;
            d_save::dSv_player_return_place_c* playerReturnPlacePtr = &playerPtr->player_return_place;
            d_stage::dStage_startStage* startStgPtr = &d_com_inf_game::dComIfG_gameInfo.play.mStartStage;
            uint16_t startPoint = d_com_inf_game::dComIfG_gameInfo.save.mRestart.mStartPoint;

            if (startPoint == 0xFFFC) // Portal
            {
                playerReturnPlacePtr->link_spawn_point_id = 0; // Just set the spawn to 0 so that the player has a valid spawn
                                                               // location since you can't load a save from a portal spawn.
            }
            else
            {
                playerReturnPlacePtr->link_spawn_point_id = startPoint;
            }

            strncpy(playerReturnPlacePtr->link_current_stage,
                    startStgPtr->mStage,
                    sizeof(playerReturnPlacePtr->link_current_stage) - 1);

            playerReturnPlacePtr->link_room_id = startStgPtr->mRoomNo;
        }
    }
} // namespace mod::game_patch
