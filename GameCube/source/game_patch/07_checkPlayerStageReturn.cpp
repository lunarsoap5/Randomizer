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

        // If we are not in a dungeon, we want to set our save warp to be our last valid entrance.
        // As we randomize Boss/midboss rooms and caves, this will have to be adjusted accordingly.
        // if ((rando::gRandomizer->getSeedPtr()->getStageIDX() > 29) &&
        //     !checkStageName(stage::allStages[stage::StageIDs::Cave_of_Ordeals]))
        // {
        libtp::tp::d_save::dSv_info_c* savePtr = &d_com_inf_game::dComIfG_gameInfo.save;
        d_save::dSv_player_return_place_c* playerReturnPlacePtr = &savePtr->save_file.player.player_return_place;

        libtp::tp::d_stage::dStage_startStage* lastSavableStartPtr = rando::gRandomizer->getLastSavableStart();

        strncpy(playerReturnPlacePtr->link_current_stage,
                lastSavableStartPtr->mStage,
                sizeof(playerReturnPlacePtr->link_current_stage) - 1);

        playerReturnPlacePtr->link_spawn_point_id = lastSavableStartPtr->mPoint;
        playerReturnPlacePtr->link_room_id = lastSavableStartPtr->mRoomNo;
        playerReturnPlacePtr->unk10 = lastSavableStartPtr->mLayer;
        // }
    }
} // namespace mod::game_patch
