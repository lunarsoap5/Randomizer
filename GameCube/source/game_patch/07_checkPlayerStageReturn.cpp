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
                // Since you can't load a save from a portal spawn, use the vanilla spawns.
                switch (rando::gRandomizer->getSeedPtr()->getStageIDX())
                {
                    case StageIDs::Death_Mountain:
                    case StageIDs::Kakariko_Village:
                    case StageIDs::Snowpeak:
                        playerReturnPlacePtr->link_spawn_point_id = 5;
                        break;
                    case StageIDs::Mirror_Chamber:
                        playerReturnPlacePtr->link_spawn_point_id =
                            2; // Position without save prompt to avoid any potential ER impacts
                        break;
                    case StageIDs::Outside_Castle_Town:
                        playerReturnPlacePtr->link_spawn_point_id = 7;
                        break;
                    case StageIDs::Gerudo_Desert:
                        playerReturnPlacePtr->link_spawn_point_id = 0xB;
                        break;
                    case StageIDs::Ordon_Spring:
                        playerReturnPlacePtr->link_spawn_point_id = 0x1E;
                        break;
                    case StageIDs::Upper_Zoras_River:
                        playerReturnPlacePtr->link_spawn_point_id = 0x63;
                        break;
                    case StageIDs::Lake_Hylia:
                        playerReturnPlacePtr->link_spawn_point_id = 0x85;
                        break;
                    case StageIDs::Sacred_Grove:
                        playerReturnPlacePtr->link_spawn_point_id = 0xFE;
                        break;
                    case StageIDs::Faron_Woods:
                    {
                        if (playerReturnPlacePtr->link_room_id == 0)
                            playerReturnPlacePtr->link_spawn_point_id = 0; // S FW
                        else
                            playerReturnPlacePtr->link_spawn_point_id = 0xFE; // N FW
                        break;
                    }
                    case StageIDs::Hyrule_Field:
                    {
                        if (playerReturnPlacePtr->link_room_id == 0)
                            playerReturnPlacePtr->link_spawn_point_id = 0xF; // Bridge of Eldin
                        else
                            playerReturnPlacePtr->link_spawn_point_id = 6; // KG
                        break;
                    }
                    default:
                        playerReturnPlacePtr->link_spawn_point_id = 0; // Other portals use 0.
                        break;
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
