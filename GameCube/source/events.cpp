#include <cstring>

#include "events.h"
#include "asm.h"
#include "data/items.h"
#include "data/stages.h"
#include "main.h"
#include "patch.h"
#include "rando/randomizer.h"
#include "tp/J2DTextBox.h"
#include "tp/d_a_alink.h"
#include "tp/d_a_player.h"
#include "tp/d_camera.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_item.h"
#include "tp/d_kankyo.h"
#include "tp/d_map_path_dmap.h"
#include "tp/d_meter2_info.h"
#include "tp/d_resource.h"
#include "tp/dzx.h"
#include "tp/m_do_ext.h"
#include "tp/rel/d_a_obj_Lv5Key.h"
#include "user_patch/03_customCosmetics.h"
#include "tp/J2DPicture.h"

namespace mod::events
{
    // REL patching trampolines
    void ( *return_daObjLv5Key_c__Wait )( libtp::tp::rel::d_a_obj_Lv5Key::daObjLv5Key_c* _this ) = nullptr;
    CMEB checkNpcTransform = nullptr;

    void onLoad( rando::Randomizer* randomizer )
    {
        // Make sure the randomizer is loaded/enabled and a seed is loaded
        if ( !getCurrentSeed( randomizer ) )
        {
            return;
        }

        randomizer->onStageLoad();
    }

    void offLoad( rando::Randomizer* randomizer )
    {
        // Make sure the randomizer is loaded/enabled and a seed is loaded
        if ( !getCurrentSeed( randomizer ) )
        {
            return;
        }

        // Check if the seed is already applied to the save-file (flags etc.)
        // Try to do it otherwise
        if ( !randomizer->m_SeedInit &&
             ( strcmp( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mStage, "F_SP108" ) == 0 ) &&
             ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mRoomNo == 1 ) &&
             ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mPoint == 0x15 ) )
        {
            randomizer->initSave();
        }

        randomizer->overrideObjectARC();
        randomizer->overrideEventARC();
        user_patch::setLanternColor( randomizer );
    }

    void onRELLink( rando::Randomizer* randomizer, libtp::tp::dynamic_link::DynamicModuleControl* dmc )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            randomizer->overrideREL();
        }

        // Static REL overrides and patches
        uint32_t relPtrRaw = reinterpret_cast<uint32_t>( dmc->moduleInfo );

        switch ( dmc->moduleInfo->id )
        {
            // d_a_e_hp.rel
            // Generic Poe
            case 0x00C8:
                libtp::patch::writeBranchLR( reinterpret_cast<void*>( relPtrRaw + e_hp_ExecDead_liOffset ),
                                             reinterpret_cast<void*>( assembly::asmAdjustPoeItem ) );

                // Disable Poe increment (handled through item_get_func; see game_patches)
                *reinterpret_cast<uint32_t*>( relPtrRaw + e_hp_ExecDead_incOffset ) = 0x60000000;

                break;
            // d_a_e_po.rel
            // Arbiter's Poe
            case 0x00DD:
                libtp::patch::writeBranchLR( reinterpret_cast<void*>( relPtrRaw + e_po_ExecDead_liOffset ),
                                             reinterpret_cast<void*>( assembly::asmAdjustAGPoeItem ) );

                // Disable Poe increment (handled through item_get_func; see game_patches)
                *reinterpret_cast<uint32_t*>( relPtrRaw + e_po_ExecDead_incOffset ) = 0x60000000;
                break;
            // d_a_obj_kshutter.rel
            // Lakebed Temple Boss Door
            case 0x1FA:
                // Nop out the instruction that stores the new total small key value when the game attempts to
                // remove a small key from the inventory when opening the boss door
                if ( libtp::tools::playerIsInRoomStage(
                         2,
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Lakebed_Temple] ) )
                {
                    *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1198 ) = 0x60000000;     // Previous: 0x3803ffff
                }
                break;
            // d_a_npc_kn.rel
            // Hero's Shade
            case 0x147:
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x34D0 ),
                                             reinterpret_cast<void*>( assembly::asmAdjustHiddenSkillItem ) );
                // Give a an item based on which Golden Wolf you learned a skill from.
                break;
            // d_a_npc_ins.rel
            // Agitha
            case 0x141:
            {
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x21B8 ),
                                             reinterpret_cast<void*>( assembly::asmAdjustBugReward ) );
                break;
            }
            // d_a_mg_rod.rel
            // Fishing Hole Rod
            case 0x32:
            {
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0xB2B0 ),
                                             reinterpret_cast<void*>( libtp::tp::d_item::execItemGet ) );

                break;
            }
            // d_a_Statue_Tag.rel
            // Owl Statues
            case 0x85:     // d_a_Tag_Statue - Owl Statues
            {
                // replace sky character
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0xB7C ) = 0x48000020;     // b 0x20
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0xB9C ),
                                             reinterpret_cast<void*>( assembly::asmAdjustSkyCharacter ) );

                break;
            }
            // d_a_obj_life_container.rel
            // Heart Pieces/Containers
            case 0x35:
            {
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x1804 ),
                                             reinterpret_cast<void*>( assembly::asmAdjustFieldItemParams ) );
                break;
            }
            // d_a_obj_bosswarp.rel
            // Post-Boss Cutscene
            case 0x5B:
            {
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x1884 ),
                                             reinterpret_cast<void*>( libtp::tp::d_item::execItemGet ) );
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1888 ) = 0x480000A8;     // b 0xA8
                // Replace dungeon reward that is given after beating a boss and show the appropriate text.
                break;
            }
            // d_a_npc_bouS.rel
            // Inside Bo's House
            case 0x121:
            {
                // Prevent Bo from talking after the chest has been opened
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1A44 ) = 0x48000028;     // b 0x28
                break;
            }
            // d_a_npc_ykm.rel
            // Yeto
            case 0x17F:
            {
                // Prevent Yeto from leaving the dungeon if the player has the boss key
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1524 ) = 0x38600000;     // li r3,0
                break;
            }
            // d_a_npc_ykw.rel
            // Yeta
            case 0x180:
            {
                // Prevent Yeta from leaving the dungeon if the player has the boss key
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1038 ) = 0x38600000;     // li r3,0
                break;
            }
            // d_a_e_mk.rel
            // Ook
            case 0xD2:
            {
                // Transform back into link if you are wolf when defeating Ook
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x4A88 ),
                                             reinterpret_cast<void*>( assembly::asmTransformOokWolf ) );

                break;
            }
            // d_a_obj_swBallC.rel
            // Light Sword Cutscene
            case 0x280:
            {
                // The cutscene gives link the MS during the cutscene by default, so we just nop out the link to the function.
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0xB50 ) = 0x60000000;
                break;
            }
            // d_a_npc_rafrel.rel
            // Auru
            case 0x15F:
            {
                // Allow Auru to spawn, even if you have raised the Mirror.
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x6C4 ) =
                    0x38600131;     // set auru to check for whether he gave the player the item to spawn.
                break;
            }
            // d_a_obj_smallkey.rel
            // Freestanding Small Keys
            case 0x26D:
            {
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0xC88 ) =
                    0x48000058;     // patch instruction to prevent game from removing bulblin camp key.
                break;
            }
            // d_a_b_bq.rel
            // Diababa
            case 0x8B:
            {
                // Transform back into link if you are wolf when defeating Diababa
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x21B8 ),
                                             reinterpret_cast<void*>( assembly::asmTransformDiababaWolf ) );

                break;
            }

                // d_a_obj_Lv5Key.rel
            // Snowpeak Ruins Small Key Lock
            case 0x189:
            {
                // Prevent Snowpeak Ruins Small Key softlock
                return_daObjLv5Key_c__Wait =
                    libtp::patch::hookFunction( reinterpret_cast<void ( * )( libtp::tp::rel::d_a_obj_Lv5Key::daObjLv5Key_c* )>(
                                                    relPtrRaw + d_a_obj_Lv5Key__Wait_offset ),
                                                []( libtp::tp::rel::d_a_obj_Lv5Key::daObjLv5Key_c* lv5KeyPtr )
                                                {
                                                    float playerPos[3];
                                                    libtp::tp::d_map_path_dmap::getMapPlayerPos( playerPos );

                                                    // Will compare xPos if 0x4000 & yRot is nonzero (lock is on x-axis).
                                                    // Will compare zPos if 0x4000 & yRot is zero (lock is on z-axis).
                                                    // Will compare greater if 0x8000 & yRot is nonzero.
                                                    // This implementation reduces instruction count to 49 compared to naive
                                                    // approach's 86.

                                                    bool isCompareX = lv5KeyPtr->mCollisionRot.y & 0x4000;
                                                    bool isCompareGreater = lv5KeyPtr->mCollisionRot.y & 0x8000;

                                                    float* playerAxisPos = nullptr;
                                                    float* lockPos = nullptr;

                                                    if ( isCompareX )
                                                    {
                                                        playerAxisPos = &playerPos[0];
                                                        lockPos = &( lv5KeyPtr->mCurrent.mPosition.x );
                                                    }
                                                    else
                                                    {
                                                        playerAxisPos = &playerPos[2];
                                                        lockPos = &( lv5KeyPtr->mCurrent.mPosition.z );
                                                    }

                                                    bool swapDoorSides = false;

                                                    if ( isCompareGreater )
                                                    {
                                                        if ( *playerAxisPos > *lockPos + 17 )
                                                        {
                                                            swapDoorSides = true;
                                                            *lockPos += 34;
                                                        }
                                                    }
                                                    else if ( *playerAxisPos < *lockPos - 17 )
                                                    {
                                                        swapDoorSides = true;
                                                        *lockPos -= 34;
                                                    }

                                                    if ( swapDoorSides )
                                                    {
                                                        lv5KeyPtr->mCollisionRot.y ^= 0x8000;       // facing
                                                        lv5KeyPtr->mCurrent.mAngle.y ^= 0x8000;     // speed direction
                                                    }

                                                    // Call original function
                                                    return_daObjLv5Key_c__Wait( lv5KeyPtr );
                                                } );
                break;
            }
            // d_a_midna.rel
            // Midna
            case 0x33:
            {
                checkNpcTransform = reinterpret_cast<CMEB>( relPtrRaw + 0x8A0C );
                break;
            }
            // d_a_npc_hoz.rel
            // Iza
            case 0x13E:
            {
                // Transform Info Human After Defeating Shadow Beasts By Iza
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0xC7F8 ),
                                             reinterpret_cast<void*>( assembly::asmAdjustIzaWolf ) );
            }
            // d_a_obj_drop.rel
            // Tear of Light
            case 0x1B7:
            {
                // set wait timer to 1
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x0FCC ) = 0x38000001;     // li 0x1
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x1038 ) = 0x38000001;     // li 0x1

                // set y_pos of drop to be at ground level
                *reinterpret_cast<uint32_t*>( relPtrRaw + 0x2474 ) = 0x00000000;     // 0.0f
            }

            // d_a_kytag03.rel
            case 0x10C:
            {
                // Modify draw function to draw the Reekfish path so long as we have smelled the fish once.
                libtp::patch::writeBranchBL( reinterpret_cast<void*>( relPtrRaw + 0x66C ),
                                             reinterpret_cast<void*>( assembly::asmShowReekfishPath ) );
            }
        }
    }

    void onRELUnlink( rando::Randomizer* randomizer, libtp::tp::dynamic_link::DynamicModuleControl* dmc )
    {
        switch ( dmc->moduleInfo->id )
        {
            // d_a_obj_Lv5Key.rel
            // Snowpeak Ruins Small Key Lock
            case 0x189:
            {
                return_daObjLv5Key_c__Wait = libtp::patch::unhookFunction( return_daObjLv5Key_c__Wait );
                break;
            }
            // d_a_midna.rel
            // Midna
            case 0x33:
            {
                checkNpcTransform = nullptr;
            }
        }
    }

    void onDZX( rando::Randomizer* randomizer, libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            randomizer->overrideDZX( chunkTypeInfo );
        }
        loadCustomActors();
    }

    int32_t onPoe( rando::Randomizer* randomizer, uint8_t flag )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            return randomizer->getPoeItem( flag );
        }
        else
        {
            // Default item
            return static_cast<int32_t>( libtp::data::items::Poe_Soul );
        }
    }

    uint8_t onSkyCharacter( rando::Randomizer* randomizer )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            return randomizer->getSkyCharacter();
        }
        else
        {
            // Default item
            return static_cast<int32_t>( libtp::data::items::Ancient_Sky_Book_Partly_Filled );
        }
    }

    void onARC( rando::Randomizer* randomizer, void* data, int roomNo, rando::FileDirectory fileDirectory )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            randomizer->overrideARC( reinterpret_cast<uint32_t>( data ), fileDirectory, roomNo );
        }
    }

    void onBugReward( rando::Randomizer* randomizer, uint32_t msgEventAddress, uint8_t bugID )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            uint8_t itemID = randomizer->overrideBugReward( bugID );
            *reinterpret_cast<uint16_t*>( ( *reinterpret_cast<uint32_t*>( msgEventAddress + 0xA04 ) + 0x3580 ) + 0x6 ) =
                itemID;     // Change Big Wallet Item
            *reinterpret_cast<uint16_t*>( ( *reinterpret_cast<uint32_t*>( msgEventAddress + 0xA04 ) + 0x3628 ) + 0x6 ) =
                itemID;     // Change Giant Wallet Item
            *reinterpret_cast<uint16_t*>( ( *reinterpret_cast<uint32_t*>( msgEventAddress + 0xA04 ) + 0x35c8 ) + 0x6 ) =
                itemID;     // Change Purple Rupee Item
            *reinterpret_cast<uint16_t*>( ( *reinterpret_cast<uint32_t*>( msgEventAddress + 0xA04 ) + 0x35F0 ) + 0x6 ) =
                itemID;     // Change Orange Rupee Item
        }
    }

    void onHiddenSkill( rando::Randomizer* randomizer, uint16_t eventIndex )
    {
        if ( randoIsEnabled( randomizer ) )
        {
            libtp::tp::d_item::execItemGet( randomizer->getHiddenSkillItem( eventIndex ) );
        }
    }

    void setSaveFileEventFlag( uint16_t flag )
    {
        libtp::tp::d_save::onEventBit( &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.event_flags, flag );
    }

    void onAdjustFieldItemParams( libtp::tp::f_op_actor::fopAc_ac_c* fopAC, void* daObjLife )
    {
        using namespace libtp::data::stage;
        using namespace libtp::data::items;

        if ( libtp::tp::d_a_alink::checkStageName( allStages[stageIDs::Hyrule_Field] ) ||
             libtp::tp::d_a_alink::checkStageName( allStages[stageIDs::Upper_Zoras_River] ) ||
             libtp::tp::d_a_alink::checkStageName( allStages[stageIDs::Sacred_Grove] ) )
        {
            *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( fopAC ) + 0x962 ) =
                0x226;                  // Y Rotation Speed modifier. 0x226 is the value used when the item is on the ground.
            fopAC->mGravity = 0.0f;     // gravity
        }
        uint8_t itemID = *reinterpret_cast<uint8_t*>( reinterpret_cast<uint32_t>( fopAC ) + 0x92A );
        switch ( itemID )
        {
            case Ordon_Shield:
            case Heart_Container:
            case Piece_of_Heart:
            case Zora_Armor:
            case Arrows_10:
            case Arrows_20:
            case Arrows_30:
            {
                *reinterpret_cast<float*>( reinterpret_cast<uint32_t>( daObjLife ) + 0x7c ) = 1.0f;     // scale
                break;
            }
            default:
            {
                *reinterpret_cast<float*>( reinterpret_cast<uint32_t>( daObjLife ) + 0x7c ) = 2.0f;     // scale
                break;
            }
        }
    }

    void handleDungeonHeartContainer()
    {
        using namespace libtp::data::stage;
        const char* bossStages[8] = { allStages[stageIDs::Morpheel],
                                      allStages[stageIDs::Fyrus],
                                      allStages[stageIDs::Diababa],
                                      allStages[stageIDs::Armogohma],
                                      allStages[stageIDs::Argorok],
                                      allStages[stageIDs::Zant_Main_Room],
                                      allStages[stageIDs::Stallord],
                                      allStages[stageIDs::Blizzeta] };
        // Set the flag for the dungeon heart container if we are on a boss stage since this is the function that gets
        // called when the player picks up a heart container.
        uint32_t totalDungeonStages = sizeof( bossStages ) / sizeof( bossStages[0] );
        for ( uint32_t i = 0; i < totalDungeonStages; i++ )
        {
            if ( libtp::tp::d_a_alink::checkStageName( bossStages[i] ) )
            {
                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags.memoryFlags[0x1D] |= 0x10;
                break;
            }
        }
    }

    bool proc_query022( void* unk1, void* unk2, int32_t unk3 )
    {
        // Check to see if currently in one of the Ordon interiors
        if ( libtp::tp::d_a_alink::checkStageName(
                 libtp::data::stage::allStages[libtp::data::stage::stageIDs::Ordon_Village_Interiors] ) )
        {
            // Check to see if ckecking for the Iron Boots
            uint16_t item = *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( unk2 ) + 0x4 );

            if ( item == libtp::data::items::Iron_Boots )
            {
                // Return false so that the door in Bo's house can be opened without having the
                // Iron Boots
                return false;
            }
        }
        return mod::return_query022( unk1, unk2, unk3 );
    }

    int32_t proc_query023( void* unk1, void* unk2, int32_t unk3 )
    {
        // return the original function as we need its value
        int32_t numBombs = mod::return_query023( unk1, unk2, unk3 );
        // Check to see if currently in one of the Kakariko interiors
        if ( libtp::tools::playerIsInRoomStage(
                 1,
                 libtp::data::stage::allStages[libtp::data::stage::stageIDs::Kakariko_Village_Interiors] ) )
        {
            // If player has not bought Barnes' Bomb Bag, we want to allow them to be able to get the check.
            if ( ( !libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x908 ) ) )
            {
                return false;
            }
            // If the player has bought the bomb bag check, we won't allow them to get the check, regardless of if they
            // have bombs or not
            else
            {
                if ( numBombs == 0 )
                {
                    return 1;
                }
                else
                {
                    return mod::return_query023( unk1, unk2, unk3 );
                }
            }
        }

        // Call original function
        return mod::return_query023( unk1, unk2, unk3 );
    }

    bool proc_query042( void* unk1, void* unk2, int32_t unk3 )
    {
        // Check to see if currently in one of the Ordon interiors
        if ( randoIsEnabled( randomizer ) )
        {
            if ( randomizer->m_Seed->m_Header->transformAnywhere )
            {
                return 0;
            }
        }
        return mod::return_query042( unk1, unk2, unk3 );
    }

    bool proc_isDungeonItem( libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int memBit )
    {
        using namespace libtp::data::stage;

        switch ( memBit )
        {
            case 3:
            {
                const char* dungeonStages[] = { allStages[stageIDs::Forest_Temple],
                                                allStages[stageIDs::Ook],
                                                allStages[stageIDs::Goron_Mines],
                                                allStages[stageIDs::Dangoro],
                                                allStages[stageIDs::Lakebed_Temple],
                                                allStages[stageIDs::Deku_Toad],
                                                allStages[stageIDs::Arbiters_Grounds],
                                                allStages[stageIDs::Death_Sword],
                                                allStages[stageIDs::Snowpeak_Ruins],
                                                allStages[stageIDs::Darkhammer],
                                                allStages[stageIDs::Temple_of_Time],
                                                allStages[stageIDs::Darknut],
                                                allStages[stageIDs::City_in_the_Sky],
                                                allStages[stageIDs::Aeralfos],
                                                allStages[stageIDs::Palace_of_Twilight],
                                                allStages[stageIDs::Phantom_Zant_1],
                                                allStages[stageIDs::Phantom_Zant_2] };
                uint32_t totalDungeonStages = sizeof( dungeonStages ) / sizeof( dungeonStages[0] );
                for ( uint32_t i = 0; i < totalDungeonStages; i++ )
                {
                    if ( libtp::tp::d_a_alink::checkStageName( dungeonStages[i] ) )
                    {
                        return false;
                    }
                }
                break;
            }

            case 7:
            {
                if ( libtp::tp::d_a_alink::checkStageName( allStages[stageIDs::Forest_Temple] ) )
                {
                    const uint8_t currentRoom = libtp::tp::d_kankyo::env_light.currentRoom;

                    if ( ( ( currentRoom == 3 ) || ( currentRoom == 1 ) ) &&
                         ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mEvtManager.mRoomNo != 0 ) )
                    {
                        return false;
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }
        // Call original function
        return mod::return_isDungeonItem( memBitPtr, memBit );
    }

    void loadCustomActors()
    {
        using namespace libtp;
        if ( tp::d_a_alink::checkStageName( data::stage::allStages[data::stage::stageIDs::Faron_Woods] ) )
        {
            libtp::tp::dzx::ACTR EponaActr = { "Horse", 0x00000F0D, 0.f, 0.f, 0.f, 0, -180, 0, 0xFFFF };
            tools::SpawnActor( 0, EponaActr );
        }
    }

    void loadCustomRoomActors()
    {
        using namespace libtp;
        if ( tp::d_a_alink::checkStageName( data::stage::allStages[data::stage::stageIDs::Lake_Hylia] ) &&
             libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x3b08 ) )
        {
            libtp::tp::dzx::ACTR AuruActr =
                { "Rafrel", 0x00001D01, -116486.945f, -13860.f, 58533.0078f, 0, static_cast<int16_t>( 0xCCCD ), 0, 0xFFFF };
            tools::SpawnActor( 0, AuruActr );
        }
    }

    void loadCustomRoomSCOBs()
    {
        using namespace libtp;
        if ( tp::d_a_alink::checkStageName( data::stage::allStages[data::stage::stageIDs::Hyrule_Field] ) &&
             libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x1E08 ) )
        {
            libtp::tp::dzx::SCOB HJumpActr = { "Hjump",
                                               0x044FFF02,
                                               5600.f,
                                               -5680.f,
                                               52055.f,
                                               0,
                                               static_cast<int16_t>( 0x4000 ),
                                               0,
                                               0xFFFF,
                                               0x20,
                                               0x2D,
                                               0x2D,
                                               0xFF };
            tools::SpawnSCOB( 3, HJumpActr );
        }
    }

    bool haveItem( uint8_t item )
    {
        return libtp::tp::d_item::checkItemGet( item, 1 );
    }

    void handleQuickTransform()
    {
        libtp::tp::d_a_alink::daAlink* linkMapPtr = libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mPlayer;

        // Ensure that Link is loaded on the map.
        if ( !linkMapPtr )
        {
            return;
        }

        // Ensure that link is not in a cutscene.
        if ( libtp::tp::d_a_alink::checkEventRun( linkMapPtr ) )
        {
            return;
        }

        // Check to see if Link has the ability to transform.
        if ( !libtp::tp::d_a_alink::dComIfGs_isEventBit( 0xD04 ) )
        {
            return;
        }

        // Ensure there is a proper pointer to the Z Button Alpha.
        uint32_t zButtonAlphaPtr = reinterpret_cast<uint32_t>( libtp::tp::d_meter2_info::wZButtonPtr );
        if ( !zButtonAlphaPtr )
        {
            return;
        }

        zButtonAlphaPtr = *reinterpret_cast<uint32_t*>( zButtonAlphaPtr + 0x10C );
        if ( !zButtonAlphaPtr )
        {
            return;
        }

        // Ensure that the Z Button is not dimmed
        float zButtonAlpha = *reinterpret_cast<float*>( zButtonAlphaPtr + 0x720 );
        if ( zButtonAlpha != 1.f )
        {
            return;
        }

        // Make sure Link is not underwater or talking to someone.
        if ( libtp::tp::d_a_alink::linkStatus->status != 0x1 )
        {
            return;
        }

        // The game will crash if trying to quick transform while holding the Ball and Chain
        if ( linkMapPtr->mEquipItem == libtp::data::items::Ball_and_Chain )
        {
            return;
        }

        // Make sure Link isn't riding anything (horse, boar, etc.)
        if ( libtp::tp::d_camera::checkRide( linkMapPtr ) )
        {
            return;
        }

        if ( randoIsEnabled( randomizer ) )
        {
            if ( randomizer->m_Seed->m_Header->transformAnywhere )
            {
                // Allow transforming regardless of whether there are people around
                libtp::tp::d_a_alink::procCoMetamorphoseInit( linkMapPtr );
                return;
            }
            else
            {
                CMEB tempCMEB = checkNpcTransform;
                if ( tempCMEB )
                {
                    // Use the game's default checks for if the player can currently transform
                    if ( !tempCMEB( libtp::tp::d_a_player::m_midnaActor ) )
                    {
                        return;
                    }
                }
            }
        }

        // Check if the player has scared someone in the current area in wolf form
        if ( ( libtp::tp::d_kankyo::env_light.mEvilPacketEnabled & 0x80 ) != 0 )
        {
            return;
        }

        libtp::tp::d_a_alink::procCoMetamorphoseInit( linkMapPtr );
    }

    libtp::tp::d_resource::dRes_info_c* getObjectResInfo( const char* arcName )
    {
        return getResInfo( arcName, libtp::tp::d_com_inf_game::dComIfG_gameInfo.mResControl.mObjectInfo, 0x80 );
    }

    void handleTimeOfDayChange()
    {
        uint8_t todStages[] = { 43, 44, 41, 45, 47, 48, 46, 49, 50, 61, 52, 57, 56, 54, 55, 59, 63, 62 };
        uint8_t nonFlowStages[] = { 43, 44, 41, 48, 46, 54, 63 };
        uint32_t totaltodStages = sizeof( todStages ) / sizeof( todStages[0] );
        uint32_t totalnonFlowStages = sizeof( nonFlowStages ) / sizeof( nonFlowStages[0] );
        int32_t stageIndex = libtp::tools::getStageIndex( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mStage );

        for ( uint32_t i = 0; i < totaltodStages; i++ )
        {
            if ( todStages[i] == stageIndex )
            {
                if ( ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b.skyAngle >= 284 ) ||
                     ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b.skyAngle <= 104 ) )
                {
                    libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b.skyAngle = 105;
                }
                else
                {
                    libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b.skyAngle = 285;
                }
                for ( uint32_t j = 0; j < totalnonFlowStages; j++ )
                {
                    if ( nonFlowStages[j] == stageIndex )
                    {
                        libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mNextStage.enabled |= 0x1;
                    }
                }
            }
        }
    }

    bool checkFoolItemFreeze()
    {
        libtp::tp::d_a_alink::daAlink* linkMapPtr = libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mPlayer;

        // Ensure that Link is loaded on the map.
        if ( !linkMapPtr )
        {
            return false;
        }

        // Ensure that link is not in a cutscene.
        if ( libtp::tp::d_a_alink::checkEventRun( linkMapPtr ) )
        {
            return false;
        }

        // Make sure Link isn't riding anything (horse, boar, etc.)
        if ( libtp::tp::d_camera::checkRide( linkMapPtr ) )
        {
            return false;
        }

        // Check if Midna has actually been unlocked and is on the Z button
        // This is needed because the Z button will always be dimmed if she has not been unlocked
        if ( libtp::tp::d_a_alink::dComIfGs_isEventBit( 0xC10 ) )
        {
            // Ensure there is a proper pointer to the Z Button Alpha.
            uint32_t zButtonAlphaPtr = reinterpret_cast<uint32_t>( libtp::tp::d_meter2_info::wZButtonPtr );
            if ( !zButtonAlphaPtr )
            {
                return false;
            }

            zButtonAlphaPtr = *reinterpret_cast<uint32_t*>( zButtonAlphaPtr + 0x10C );
            if ( !zButtonAlphaPtr )
            {
                return false;
            }

            // Ensure that the Z Button is not dimmed
            float zButtonAlpha = *reinterpret_cast<float*>( zButtonAlphaPtr + 0x720 );
            if ( zButtonAlpha != 1.f )
            {
                return false;
            }
        }

        // Make sure Link is not underwater or talking to someone.
        if ( libtp::tp::d_a_alink::linkStatus->status != 0x1 )
        {
            return false;
        }

        return true;
    }

    void drawWindow( int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color )
    {
        // Make sure the background window exists
        libtp::tp::J2DPicture::J2DPicture* tempBgWindow = bgWindow;
        if ( !tempBgWindow )
        {
            return;
        }

        // Set the window color
        tempBgWindow->setWhiteColor( color );
        tempBgWindow->setBlackColor( color );

        // Convert x, y, width, and height to floats
        constexpr int32_t numValues = 4;
        int32_t values[numValues] = { x, y, width, height };
        float valuesOut[numValues];

        for ( int32_t i = 0; i < numValues; i++ )
        {
            valuesOut[i] = intToFloat( values[i] );
        }

        // Draw the window
        libtp::tp::J2DPicture::J2DPicture_draw( tempBgWindow,
                                                valuesOut[0],
                                                valuesOut[1],
                                                valuesOut[2],
                                                valuesOut[3],
                                                false,
                                                false,
                                                false );
    }

    void drawText( const char* text, int32_t x, int32_t y, uint32_t color, float textSize )
    {
        // The font takes a bit to load, so it won't be loaded immediately at boot
        void* font = libtp::tp::m_Do_ext::mDoExt_getMesgFont();
        if ( !font )
        {
            return;
        }

        using namespace libtp::tp::J2DTextBox;
        J2DTextBox tempTextBox;

        tempTextBox.setSolidColor( color );
        tempTextBox.setLineSpacing( textSize );
        tempTextBox.setFontSize( textSize, textSize );
        J2DTextBox_setFont( &tempTextBox, font );
        J2DTextBox_setString1( &tempTextBox, text );

        J2DTextBox_draw1( &tempTextBox, intToFloat( x ), intToFloat( y ) );

        // Must manually call the destructor, as it takes auto-generated parameters
        J2DTextBox_dt( &tempTextBox, static_cast<int16_t>( false ) );
    }

    int32_t getCurrentAreaNodeId()
    {
        int32_t stageIndex = libtp::tools::getStageIndex( libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mStage );
        if ( stageIndex >= 0 )
        {
            return static_cast<int32_t>( libtp::data::stage::regionID[stageIndex] );
        }
        else
        {
            return -1;
        }
    }

    uint8_t* getNodeMemoryFlags( const libtp::data::stage::AreaNodesID nodeId,
                                 const libtp::data::stage::AreaNodesID currentAreaNodeId )
    {
        using namespace libtp::data::items;
        libtp::tp::d_save::dSv_info_c* saveDataPtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save;

        uint8_t* memoryFlags;
        if ( nodeId == currentAreaNodeId )
        {
            memoryFlags = saveDataPtr->memory.temp_flags.memoryFlags;
        }
        else     // We are not in the correct node, so use the appropriate region node
        {
            memoryFlags = saveDataPtr->save_file.area_flags[static_cast<uint32_t>( nodeId )].temp_flags.memoryFlags;
        }

        return memoryFlags;
    }

    KEEP_FUNC uint16_t getPauseRupeeMax( libtp::tp::d_save::dSv_player_status_a_c* plyrStatus )
    {
        using namespace libtp::data::items;
        Wallets current_wallet;

        current_wallet = plyrStatus->currentWallet;
        if ( current_wallet < ( Wallets::BIG_WALLET | Wallets::GIANT_WALLET ) )
        {
            if ( current_wallet == Wallets::BIG_WALLET )
            {
                return 600;
            }
            if ( current_wallet == Wallets::WALLET )
            {
                return 300;
            }
            if ( current_wallet < ( Wallets::BIG_WALLET | Wallets::GIANT_WALLET ) )
            {
                return 1000;
            }
        }
        return 0;
    }

}     // namespace mod::events