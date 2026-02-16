#include <cstring>

#include "game_patch/game_patch.h"
#include "data/flags.h"
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

        // Update our save warp to be our latest valid entrance.

        libtp::tp::d_stage::dStage_startStage* lastSavableStartPtr = rando::gRandomizer->getLastSavableStart();

        int8_t roomNo = lastSavableStartPtr->mRoomNo;
        int16_t point = lastSavableStartPtr->mPoint;

        // If S+Q would put us in first LBT room, check if flag is set which should make us spawn on land. Wait to
        // update until here so saves correct value even if you just found Shadow Crystal and immediately S+Q.
        if (roomNo == 0 &&
            !strncmp(lastSavableStartPtr->mStage,
                     libtp::data::stage::allStages[libtp::data::stage::StageIDs::Lakebed_Temple],
                     sizeof(lastSavableStartPtr->mStage) - 1) &&
            libtp::tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::TRANSFORMING_UNLOCKED))
        {
            point = 2;
        }

        d_save::dSv_player_return_place_c* playerReturnPlacePtr =
            &d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_return_place;

        strncpy(playerReturnPlacePtr->link_current_stage,
                lastSavableStartPtr->mStage,
                sizeof(playerReturnPlacePtr->link_current_stage) - 1);

        playerReturnPlacePtr->link_spawn_point_id = point;
        playerReturnPlacePtr->link_room_id = roomNo;
        playerReturnPlacePtr->unk10 = lastSavableStartPtr->mLayer;
    }
} // namespace mod::game_patch
