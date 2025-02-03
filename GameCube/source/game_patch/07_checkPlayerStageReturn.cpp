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
        using namespace libtp::data;

        // If we are not in a dungeon, we want to set our save warp to be the last entrance we entered.
        if (!checkDungeon() && !checkStageName(stage::allStages[stage::StageIDs::Cave_of_Ordeals]))
        {
            libtp::tp::d_save::dSv_player_c* playerPtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player;
            libtp::tp::d_save::dSv_player_return_place_c* playerReturnPlacePtr = &playerPtr->player_return_place;
            libtp::tp::d_stage::dStage_startStage* startStgPtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage;

            strncpy(playerReturnPlacePtr->link_current_stage,
                    startStgPtr->mStage,
                    sizeof(playerReturnPlacePtr->link_current_stage) - 1);

            playerReturnPlacePtr->link_spawn_point_id = startStgPtr->mPoint;
            playerReturnPlacePtr->link_room_id = startStgPtr->mRoomNo;
        }
    }
} // namespace mod::game_patch
