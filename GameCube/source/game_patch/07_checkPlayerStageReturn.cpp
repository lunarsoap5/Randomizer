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
            uint8_t spawnPoint = static_cast<uint8_t>(startPoint);

            if (startPoint == -0x4) // Portal
            {
                // Since you can't load a save from a portal spawn, use the vanilla spawns.
                switch (rando::gRandomizer->getSeedPtr()->getStageIDX())
                {
                    case StageIDs::Death_Mountain:
                    case StageIDs::Kakariko_Village:
                    case StageIDs::Snowpeak:
                        spawnPoint = 5;
                        break;
                    case StageIDs::Mirror_Chamber:
                        spawnPoint = 2; // Position without save prompt to avoid any potential ER impacts
                        break;
                    case StageIDs::Outside_Castle_Town:
                        spawnPoint = 7;
                        break;
                    case StageIDs::Gerudo_Desert:
                        spawnPoint = 0xB;
                        break;
                    case StageIDs::Ordon_Spring:
                        spawnPoint = 0x1E;
                        break;
                    case StageIDs::Upper_Zoras_River:
                        spawnPoint = 0x63;
                        break;
                    case StageIDs::Lake_Hylia:
                        spawnPoint = 0x85;
                        break;
                    case StageIDs::Sacred_Grove:
                        spawnPoint = 0xFE;
                        break;
                    case StageIDs::Faron_Woods:
                    {
                        if (playerReturnPlacePtr->link_room_id == 0)
                            spawnPoint = 0; // S FW
                        else
                            spawnPoint = 0xFE; // N FW
                        break;
                    }
                    case StageIDs::Hyrule_Field:
                    {
                        if (playerReturnPlacePtr->link_room_id == 0)
                            spawnPoint = 0xF; // Bridge of Eldin
                        else
                            spawnPoint = 6; // KG
                        break;
                    }
                    default:
                        spawnPoint = 0; // Other portals use 0.
                        break;
                }
            }

            playerReturnPlacePtr->link_spawn_point_id = spawnPoint;

            strncpy(playerReturnPlacePtr->link_current_stage,
                    startStgPtr->mStage,
                    sizeof(playerReturnPlacePtr->link_current_stage) - 1);

            playerReturnPlacePtr->link_room_id = startStgPtr->mRoomNo;
            playerReturnPlacePtr->unk10 = startStgPtr->mLayer;
        }
    }
} // namespace mod::game_patch
