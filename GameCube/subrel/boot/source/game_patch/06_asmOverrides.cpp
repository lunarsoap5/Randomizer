#include <cstdint>

#include "main.h"
#include "game_patch/game_patch.h"
#include "asm_templates.h"
#include "tp/d_item.h"
#include "tp/d_msg_flow.h"
#include "tp/d_a_alink.h"
#include "data/items.h"
#include "tp/d_menu_collect.h"
#include "patch.h"
#include "events.h"
#include "tp/d_kankyo_rain.h"
#include "asm.h"
#include "tp/d_com_inf_game.h"
#include "tp/m_Do_dvd_thread.h"
#include "Z2AudioLib/Z2SceneMgr.h"
#include "tp/d_meter2_draw.h"
#include "functionHooks.h"
#include "tp/d_menu_ring.h"
#include "tp/d_a_obj_item.h"
#include "tp/d_s_play.h"
#include "tp/d_menu_fmap.h"

namespace mod::game_patch
{
    void _06_writeASMPatches()
    {
        // Get the addresses to overwrite
#ifdef TP_US
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>(0x8000B8A4);
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>(0x80238F58);
#elif defined TP_EU
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>(0x8000B878);
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>(0x802395D8);
#elif defined TP_JP
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>(0x8000B8A4);
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>(0x802398E0);
#endif
        // Perform the overwrites

        /* If the address is loaded into the cache before the overwrite is made,
        then the cache will need to be cleared after the overwrite */

        // Enable the crash screen
        *enableCrashScreen = ASM_BRANCH(0x14);

        // Nop out the instruction that causes a miscalculation in message resources.
        *patchMessageCalculation = ASM_NOP;

        // Force checkTreasureRupeeReturn to return false by overwriting the first two instructions in it
        uint32_t checkTreasureRupeeReturnAddress = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::checkTreasureRupeeReturn);
        *reinterpret_cast<uint32_t*>(checkTreasureRupeeReturnAddress) = ASM_LOAD_IMMEDIATE(3, 0);       // Previous 0x9421fff0
        *reinterpret_cast<uint32_t*>(checkTreasureRupeeReturnAddress + 0x4) = ASM_BRANCH_LINK_REGISTER; // Previous 0x7c0802a6

        // Modify the Wooden Sword function to not set a region flag by default by nopping out the function call to isSwitch
        uint32_t woodenSwordFunctionAddress = reinterpret_cast<uint32_t>(libtp::tp::d_item::item_func_WOOD_STICK);
        *reinterpret_cast<uint32_t*>(woodenSwordFunctionAddress + 0x40) = ASM_NOP; // Previous 0x4bf9cafd

        // Modify the Heart Container function to not set the dungeon flag for the heart container upon collection
        uint32_t heartContainerFunctionAddress = reinterpret_cast<uint32_t>(libtp::tp::d_item::item_func_UTUWA_HEART);
        *reinterpret_cast<uint32_t*>(heartContainerFunctionAddress + 0x7C) = ASM_NOP; // Previous 0x4bf9c5e9

        // Modify event035 to not remove Auru's Memo from inventory after talking to Fyer.
        uint32_t event035MemoAddress = reinterpret_cast<uint32_t>(libtp::tp::d_msg_flow::event035);

        *reinterpret_cast<uint32_t*>(event035MemoAddress + 0x40) =
            ASM_COMPARE_WORD_IMMEDIATE(4, libtp::data::items::Asheis_Sketch); // Previous 0x2c040090

        // Modify procCoGetItem to display the 20 and 60 poe messages when the player currently has 19 and 59 respectively, as
        // this project changes the poe count to increment after the message is displayed instead of before
        uint32_t procCoGetItemAddress = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::procCoGetItem);
        *reinterpret_cast<uint32_t*>(procCoGetItemAddress + 0x56C) = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE(0, 19);
        *reinterpret_cast<uint32_t*>(procCoGetItemAddress + 0x580) = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE(0, 59);

        // Prevent onStageBossEnemyAddress from setting the Ooccoo flag when defeating a boss.
        uint32_t onStageBossEnemyAddress = reinterpret_cast<uint32_t>(libtp::tp::d_com_inf_game::dComIfGs_onStageBossEnemy);
        *reinterpret_cast<uint32_t*>(onStageBossEnemyAddress + 0x60) = ASM_NOP; // Previous 480070e9
        *reinterpret_cast<uint32_t*>(onStageBossEnemyAddress + 0x8C) = ASM_NOP; // Previous 480070bd

        // Patch setSceneName so that certain Boss Music plays even if flags are set.
        uint32_t setSceneNameAddress = reinterpret_cast<uint32_t>(libtp::z2audiolib::z2scenemgr::setSceneName);
        // Morpheel
        *reinterpret_cast<uint32_t*>(setSceneNameAddress + 0x216C) = ASM_BRANCH(0x28); // Previous beq- 0x28
        // Stallord
        *reinterpret_cast<uint32_t*>(setSceneNameAddress + 0x22E8) = ASM_BRANCH(0x24); // Previous beq- 0x24
        // Armogohma
        *reinterpret_cast<uint32_t*>(setSceneNameAddress + 0x254C) = ASM_BRANCH(0x24); // Previous beq- 0x24
        // Diababa
        *reinterpret_cast<uint32_t*>(setSceneNameAddress + 0x1F50) = ASM_BRANCH(0x24); // Previous beq- 0x24

        uint32_t mDoDvdThd_mountArchive_c__execute =
            reinterpret_cast<uint32_t>(libtp::tp::m_Do_dvd_thread::mountArchive__execute);

        events::performStaticASMReplacement(mDoDvdThd_mountArchive_c__execute + 0x200, ASM_NOP);

        // Modify dKyr_odour_draw to draw the Reekfish path so long as we have smelled the fish once.
        uint32_t odourDrawAddress = reinterpret_cast<uint32_t>(libtp::tp::d_kankyo_rain::dKyr_odour_draw);
        libtp::patch::writeBranchBL(odourDrawAddress + 0xBC, assembly::asmShowReekfishPath);

        // Modify getRupeeMax calls in screenSet to display the proper wallet in the pause menu
        uint32_t screenSetAddress = reinterpret_cast<uint32_t>(libtp::tp::d_menu_collect::dMenuCollect_screenSet);
        libtp::patch::writeBranchBL(screenSetAddress + 0xDCC, events::getPauseRupeeMax);
        libtp::patch::writeBranchBL(screenSetAddress + 0xDF0, events::getPauseRupeeMax);

        // Modify drawKanteraScreen to change the lantern meter color to match lantern light color from seed.
        uint32_t drawKanteraAddress = reinterpret_cast<uint32_t>(libtp::tp::d_meter2_draw::drawKanteraScreen);
        libtp::patch::writeBranchBL(drawKanteraAddress + 0xE4, events::modifyLanternMeterColor);

        uint32_t procCoGetItemInitAddress = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::procCoGetItemInit);
        libtp::patch::writeBranchBL(procCoGetItemInitAddress + 0x17C, procCoGetItemInitCreateItem);

        // Modify the item wheel constructor to allow equipping of items, even as wolf
        uint32_t itemWheelConstructorAddress = reinterpret_cast<uint32_t>(libtp::tp::d_menu_ring::dMenuRing_ct);
        *reinterpret_cast<uint32_t*>(itemWheelConstructorAddress + 0x15C) = ASM_LOAD_IMMEDIATE(0, 0);

        // Modify the checkStatus Function to show us the current equips, even as wolf
        const uint32_t checkStatus_address = reinterpret_cast<uint32_t>(libtp::tp::d_meter2::checkStatus);
        libtp::patch::writeBranchBL(checkStatus_address + 0x3C, assembly::asmManageEquippedItemsAsWolf);

        const uint32_t decideDoStatus_address = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::decideDoStatus);
        libtp::patch::writeBranchBL(decideDoStatus_address + 0x4D4, handleAdjustToTSwordReq);

        // give all items that have an item ID of 0x13 or higher
        const uint32_t itemGetAddr = reinterpret_cast<uint32_t>(libtp::tp::d_a_obj_item::itemGet);
        *reinterpret_cast<uint32_t*>(itemGetAddr + 0x54) = ASM_NOP;

        // All non rupee/ammo items use procInitSimpleDemo and itemGet
        const uint32_t itemGetNextExecuteAddr = reinterpret_cast<uint32_t>(libtp::tp::d_a_obj_item::itemGetNextExecute);
        *reinterpret_cast<uint32_t*>(itemGetNextExecuteAddr + 0x74) = ASM_BRANCH(0x70);

        // prevent boomerang from being given on room load
        const uint32_t createInitAddr = reinterpret_cast<uint32_t>(libtp::tp::d_a_obj_item::CreateInit);
        *reinterpret_cast<uint32_t*>(createInitAddr + 0x264) = ASM_BRANCH(0x10);

        // Allow boomerang to rotate
        const uint32_t modeWaitAddr = reinterpret_cast<uint32_t>(libtp::tp::d_a_obj_item::mode_wait);
        *reinterpret_cast<uint32_t*>(modeWaitAddr + 0x78) = ASM_NOP;

        // Modify checkGroundSpecialMode to patch twilight fog transforms
        const uint32_t checkGroundAddress = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::checkGroundSpecialMode);
        libtp::patch::writeBranchBL(checkGroundAddress + 0x4C, events::checkValidGroundTransform);

        // Modify d_s_play::phase1 to not give the horse call during the cutscene and instead give our custom item.
        const uint32_t dScnPlayPhase1Addr = reinterpret_cast<uint32_t>(libtp::tp::d_s_play::dScnPlay_phase_1);
        *reinterpret_cast<uint32_t*>(dScnPlayPhase1Addr + 0x234) = ASM_NOP;
        libtp::patch::writeBranchBL(dScnPlayPhase1Addr + 0x24C, events::replaceHorseCallItem);

        // const uint32_t dScnPlayPhase1Addr = reinterpret_cast<uint32_t>(0x801d2508); // func

        // Modify dMenu_Fmap2DBack_c::isShowRegion so region you are currently in does not automatically show.
        const uint32_t isShowRegionAddr = reinterpret_cast<uint32_t>(libtp::tp::d_menu_fmap2D::isShowRegion);
        *reinterpret_cast<uint32_t*>(isShowRegionAddr + 0x130) = ASM_LOAD_IMMEDIATE(3, 0);

        // Modify dMenu_Fmap_c::region_map_proc so Z button press does not show
        // portals when zoomed on region if that region is not unlocked/showing.
        const uint32_t regionMapProcAddr = reinterpret_cast<uint32_t>(libtp::tp::d_menu_fmap::region_map_proc);
        libtp::patch::writeBranchBL(regionMapProcAddr + 0xE0, assembly::asmAdjustFMapShowRegionPortals);

#ifdef TP_JP
        uint32_t checkWarpStartAddress = reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::checkWarpStart);

        // Patch checkWarpStart to remove the instruction that updates the map glitch value later
        *reinterpret_cast<uint32_t*>(checkWarpStartAddress + 0x15C) = ASM_NOP; // Previous 0x901e0570

        // Patch checkWarpStart to allow map glitch to work
        libtp::patch::writeStandardBranches(checkWarpStartAddress + 0x64,
                                            assembly::asmUnpatchMapGlitchStart,
                                            assembly::asmUnpatchMapGlitchEnd);
#endif
    }
} // namespace mod::game_patch