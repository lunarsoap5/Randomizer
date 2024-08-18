#include "main.h"
#include "item_wheel_menu.h"
#include "patch.h"

#include "game_patch/game_patch.h"
#include "customMessages.h"
#include "cxx.h"
#include "tp/d_stage.h"
#include "tp/d_com_inf_game.h"
#include "tp/f_ap_game.h"
#include "tp/f_op_actor_mng.h"
#include "tp/d_menu_ring.h"
#include "tp/d_item.h"
#include "tp/d_msg_class.h"
#include "tp/resource.h"
#include "tp/d_msg_flow.h"
#include "tp/d_a_npc.h"
#include "tp/d_menu_window.h"
#include "Z2AudioLib/Z2AudioMgr.h"
#include "tp/d_s_logo.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "Z2AudioLib/Z2SoundMgr.h"
#include "tp/dynamic_link.h"
#include "tp/d_a_itembase.h"
#include "tp/JKRMemArchive.h"
#include "tp/m_Do_dvd_thread.h"
#include "gc_wii/dvdfs.h"
#include "gc_wii/OSCache.h"
#include "codehandler.h"
#include "memory.h"
#include "gc_wii/OSInterrupt.h"
#include "gc_wii/vi.h"
#include "asm.h"
#include "tp/d_file_select.h"
#include "functionHooks.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>

namespace mod
{
    void main()
    {
        // Set up the console
        // Align to uint8_t, as that's the largest variable type in the Console class
        gConsole = new (sizeof(uint8_t)) libtp::display::Console(CONSOLE_PROTECTED_LINES);

        // Initialize the randomizer
        // Align to 0x20 to be safe, as there are multiple classes in the Randomizer class with varying sizes
        rando::Randomizer* randoPtr = new (0x20) rando::Randomizer;
        rando::Seed* seedPtr = randoPtr->getSeedPtr();

        // Check if the randomizer was successfully initialized
        if (!seedPtr)
        {
            // Free the memory used by gRandomizer
            delete randoPtr;

            // Hook fapGm_Execute with the function for when a seed fails to be loaded
            gReturn_fapGm_Execute =
                libtp::patch::hookFunction(libtp::tp::f_ap_game::fapGm_Execute, handle_fapGm_Execute_FailLoadSeed);

            // Print error text to the console
            getConsole() << "FATAL: The seed could not be loaded!\n"
                         << "Press R + Z to close the console.\n\n";

            // Force the console to be displayed
            setConsoleScreen(true);

            // Exit now, so that the game will play vanilla aside from being able to open/close the console
            return;
        }

        // Everything should be ready to go, so set gRandomizer
        rando::gRandomizer = randoPtr;

        // Handle the main function hooks
        hookFunctions();

        // Set up the codehandler
        // writeCodehandlerToMemory();

        // Run game patches
        game_patch::_00_poe();
        game_patch::_02_modifyItemData();
        game_patch::_03_increaseClimbSpeed();
        game_patch::_06_writeASMPatches();

        // Load custom messages
        customMessages::createMsgTable();
        customMessages::setDungeonItemAreaColorIndex();

        // Load item wheel menu strings
        customMessages::createItemWheelMenuData();

        // Initialize the table of archive file entries that are used for texture recoloring.
        initArcLookupTable();

        // Display some info
        getConsole() << "Seed: " << seedPtr->getHeaderPtr()->getSeedNamePtr() << " successfully applied.\n"
                     << "Press R + Z to close the console.\n\n";

        // Hide the console
        setConsoleScreen(false);
    }

    void exit() {}

    void hookFunctions()
    {
        using namespace libtp;
        using namespace libtp::tp::d_stage;
        using namespace libtp::tp::d_com_inf_game;

        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();

        // Hook functions
        patch::writeBranch(snprintf, assembly::asm_handle_snprintf);
        patch::writeBranch(vsnprintf, assembly::asm_handle_vsnprintf);
        gReturn_fapGm_Execute = patch::hookFunction(libtp::tp::f_ap_game::fapGm_Execute, handle_fapGm_Execute);

        // DMC
        const uint32_t do_link_address = reinterpret_cast<uint32_t>(libtp::tp::dynamic_link::do_link);
        libtp::patch::writeStandardBranches(do_link_address + 0x250, assembly::asmDoLinkHookStart, assembly::asmDoLinkHookEnd);

        gReturn_do_unlink = patch::hookFunction(libtp::tp::dynamic_link::do_unlink, handle_do_unlink);

        // DZX
        gReturn_actorInit = patch::hookFunction(actorInit, handle_actorInit);
        gReturn_actorInit_always = patch::hookFunction(actorInit_always, handle_actorInit_always);
        gReturn_actorCommonLayerInit = patch::hookFunction(actorCommonLayerInit, handle_actorCommonLayerInit);
        gReturn_tgscInfoInit = patch::hookFunction(tgscInfoInit, handle_tgscInfoInit);
        gReturn_roomLoader = patch::hookFunction(libtp::tp::d_stage::roomLoader, handle_roomLoader);
        // gReturn_stageLoader = patch::hookFunction( libtp::tp::d_stage::stageLoader, handle_stageLoader );
        gReturn_dStage_playerInit = patch::hookFunction(libtp::tp::d_stage::dStage_playerInit, handle_dStage_playerInit);

        gReturn_dComIfGp_setNextStage =
            patch::hookFunction(libtp::tp::d_com_inf_game::dComIfGp_setNextStage, handle_dComIfGp_setNextStage);

        // Custom States
        gReturn_getLayerNo_common_common = patch::hookFunction(getLayerNo_common_common, game_patch::_01_getLayerNo);

        // Item Creation Functions
        gReturn_createItemForBoss = patch::hookFunction(libtp::tp::f_op_actor_mng::createItemForBoss, handle_createItemForBoss);

        gReturn_createItemForMidBoss =
            patch::hookFunction(libtp::tp::f_op_actor_mng::createItemForMidBoss, handle_createItemForMidBoss);

        gReturn_createItemForPresentDemo =
            patch::hookFunction(libtp::tp::f_op_actor_mng::createItemForPresentDemo, handle_createItemForPresentDemo);

        gReturn_createItemForTrBoxDemo =
            patch::hookFunction(libtp::tp::f_op_actor_mng::createItemForTrBoxDemo, handle_createItemForTrBoxDemo);

        gReturn_CheckFieldItemCreateHeap =
            patch::hookFunction(libtp::tp::d_a_itembase::CheckFieldItemCreateHeap, handle_CheckFieldItemCreateHeap);

        // Item Wheel functions
        gReturn_setLineUpItem = patch::hookFunction(tp::d_save::setLineUpItem, handle_setLineUpItem);

        item_wheel_menu::gReturn_dMenuRing__create =
            patch::hookFunction(libtp::tp::d_menu_ring::dMenuRing__create, item_wheel_menu::handle_dMenuRing__create);

        item_wheel_menu::gReturn_dMenuRing__delete =
            patch::hookFunction(libtp::tp::d_menu_ring::dMenuRing__delete, item_wheel_menu::handle_dMenuRing__delete);

        item_wheel_menu::gReturn_dMenuRing__draw =
            patch::hookFunction(libtp::tp::d_menu_ring::dMenuRing__draw, item_wheel_menu::handle_dMenuRing__draw);

        // ItemGet functions
        gReturn_execItemGet = patch::hookFunction(libtp::tp::d_item::execItemGet, handle_execItemGet);
        gReturn_checkItemGet = patch::hookFunction(libtp::tp::d_item::checkItemGet, handle_checkItemGet);
        gReturn_item_func_ASHS_SCRIBBLING =
            patch::hookFunction(libtp::tp::d_item::item_func_ASHS_SCRIBBLING, handle_item_func_ASHS_SCRIBBLING);

        // Message Functions
        gReturn_setMessageCode_inSequence =
            patch::hookFunction(libtp::tp::control::setMessageCode_inSequence, handle_setMessageCode_inSequence);

        gReturn_getFontCCColorTable = patch::hookFunction(tp::d_msg_class::getFontCCColorTable, handle_getFontCCColorTable);

        gReturn_getFontGCColorTable = patch::hookFunction(tp::d_msg_class::getFontGCColorTable, handle_getFontGCColorTable);

        // Query/EventFunctions
        gReturn_query001 = patch::hookFunction(libtp::tp::d_msg_flow::query001, handle_query001);
        gReturn_query022 = patch::hookFunction(libtp::tp::d_msg_flow::query022, handle_query022);
        gReturn_query023 = patch::hookFunction(libtp::tp::d_msg_flow::query023, handle_query023);
        gReturn_query025 = patch::hookFunction(libtp::tp::d_msg_flow::query025, handle_query025);
        gReturn_checkEmptyBottle = patch::hookFunction(libtp::tp::d_save::checkEmptyBottle, handle_checkEmptyBottle);
        gReturn_query037 = patch::hookFunction(libtp::tp::d_msg_flow::query037, handle_query037);
        gReturn_query049 = patch::hookFunction(libtp::tp::d_msg_flow::query049, handle_query049);
        gReturn_query042 = patch::hookFunction(libtp::tp::d_msg_flow::query042, handle_query042);
        // gReturn_event000 = patch::hookFunction( libtp::tp::d_msg_flow::event000, handle_event000 );
        gReturn_event017 = patch::hookFunction(libtp::tp::d_msg_flow::event017, handle_event017);
        gReturn_doFlow = patch::hookFunction(libtp::tp::d_msg_flow::doFlow, handle_doFlow);
        gReturn_setNormalMsg = patch::hookFunction(libtp::tp::d_msg_flow::setNormalMsg, handle_setNormalMsg);

        // Save flag functions
        gReturn_isDungeonItem = patch::hookFunction(tp::d_save::isDungeonItem, handle_isDungeonItem);
        gReturn_onDungeonItem = patch::hookFunction(tp::d_save::onDungeonItem, handle_onDungeonItem);
        gReturn_daNpcT_chkEvtBit = patch::hookFunction(libtp::tp::d_a_npc::daNpcT_chkEvtBit, handle_daNpcT_chkEvtBit);
        gReturn_isEventBit = patch::hookFunction(libtp::tp::d_save::isEventBit, handle_isEventBit);
        gReturn_onEventBit = patch::hookFunction(libtp::tp::d_save::onEventBit, handle_onEventBit);

        gReturn_isSwitch_dSv_memBit = patch::hookFunction(libtp::tp::d_save::isSwitch_dSv_memBit, handle_isSwitch_dSv_memBit);

        gReturn_onSwitch_dSv_memBit = patch::hookFunction(libtp::tp::d_save::onSwitch_dSv_memBit, handle_onSwitch_dSv_memBit);

        gReturn_checkTreasureRupeeReturn =
            patch::hookFunction(tp::d_a_alink::checkTreasureRupeeReturn, handle_checkTreasureRupeeReturn);

        gReturn_isDarkClearLV = patch::hookFunction(tp::d_save::isDarkClearLV, handle_isDarkClearLV);

        // Pause Menu Functions
        gReturn_collect_save_open_init =
            patch::hookFunction(tp::d_menu_window::collect_save_open_init, handle_collect_save_open_init);

        // Link functions
        gReturn_checkBootsMoveAnime =
            patch::hookFunction(libtp::tp::d_a_alink::checkBootsMoveAnime, handle_checkBootsMoveAnime);

        gReturn_setGetItemFace = patch::hookFunction(libtp::tp::d_a_alink::setGetItemFace, handle_setGetItemFace);

        gReturn_setWolfLockDomeModel =
            patch::hookFunction(libtp::tp::d_a_alink::setWolfLockDomeModel, handle_setWolfLockDomeModel);

        gReturn_procFrontRollCrashInit =
            patch::hookFunction(libtp::tp::d_a_alink::procFrontRollCrashInit, handle_procFrontRollCrashInit);

        gReturn_procWolfDashReverseInit =
            patch::hookFunction(libtp::tp::d_a_alink::procWolfDashReverseInit, handle_procWolfDashReverseInit);

        gReturn_procWolfAttackReverseInit =
            patch::hookFunction(libtp::tp::d_a_alink::procWolfAttackReverseInit, handle_procWolfAttackReverseInit);

        gReturn_searchBouDoor = patch::hookFunction(libtp::tp::d_a_alink::searchBouDoor, handle_searchBouDoor);
        gReturn_checkCastleTownUseItem =
            patch::hookFunction(libtp::tp::d_a_alink::checkCastleTownUseItem, handle_checkCastleTownUseItem);

        gReturn_damageMagnification =
            patch::hookFunction(libtp::tp::d_a_alink::damageMagnification, handle_damageMagnification);

        gReturn_procCoGetItemInit = patch::hookFunction(libtp::tp::d_a_alink::procCoGetItemInit, handle_procCoGetItemInit);

        // Audio functions
        // Only hook sceneChange if there is at least one replacement audio
        if (seedPtr->getBgmTablePtr())
        {
            gReturn_sceneChange = patch::hookFunction(libtp::z2audiolib::z2scenemgr::sceneChange, handle_sceneChange);
        }

        // Only hook startSound if there is at least one replacement audio
        if (seedPtr->getFanfareTablePtr())
        {
            gReturn_startSound = patch::hookFunction(libtp::z2audiolib::z2soundmgr::startSound, handle_startSound);
        }

        gReturn_loadSeWave = patch::hookFunction(libtp::z2audiolib::z2scenemgr::loadSeWave, handle_loadSeWave);

        gReturn_checkBgmIDPlaying =
            patch::hookFunction(libtp::z2audiolib::z2seqmgr::checkBgmIDPlaying, handle_checkBgmIDPlaying);

        // Title Screen functions
        gReturn_dScnLogo_c_dt = patch::hookFunction(libtp::tp::d_s_logo::dScnLogo_c_dt, handle_dScnLogo_c_dt);

        // Archive/Resource functions
        gReturn_getResInfo = patch::hookFunction(libtp::tp::d_resource::getResInfo, handle_getResInfo);

        gReturn_mountArchive__execute =
            patch::hookFunction(libtp::tp::m_Do_dvd_thread::mountArchive__execute, handle_mountArchive__execute);

        // d_meter functions
        gReturn_resetMiniGameItem = patch::hookFunction(libtp::tp::d_meter2_info::resetMiniGameItem, handle_resetMiniGameItem);

        // Game Over functions
        gReturn_dispWait_init = patch::hookFunction(libtp::tp::d_gameover::dispWait_init, handle_dispWait_init);

        // Shop Functions
        gReturn_seq_decide_yes = patch::hookFunction(libtp::tp::d_shop_system::seq_decide_yes, handle_seq_decide_yes);

        // Title Screen functions
        gReturn_dFile_select_c___create =
            patch::hookFunction(libtp::tp::d_file_select::dFile_select_c___create, resetQueueOnFileSelectScreen);
    }

    void initArcLookupTable()
    {
        using namespace rando;
        using namespace libtp::gc_wii::dvdfs;

        rando::Randomizer* randoPtr = rando::gRandomizer;

        // Hero's Clothes
        randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/Kmdl.arc"), DvdEntryNumId::ResObjectKmdl);

        // Zora Armor
        randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/Zmdl.arc"), DvdEntryNumId::ResObjectZmdl);

        // Zora Armor - Get Item
        randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/O_gD_zora.arc"), DvdEntryNumId::ResObjectOgZORA);

        // randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/Wmdl.arc"), DvdEntryNumId::ResObjectWmdl);
        // randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/CWShd.arc"), DvdEntryNumId::ResObjectCWShd);
        // randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/SWShd.arc"), DvdEntryNumId::ResObjectSWShd);
        // randoPtr->setDvdEntryNum(DVDConvertPathToEntrynum("/res/Object/HyShd.arc"), DvdEntryNumId::ResObjectHyShd);
    }

    void writeCodehandlerToMemory()
    {
        // Clear the cache for the codehandler flag and source address to be safe
        uint8_t* codehandlerIsWrittenAddress = reinterpret_cast<uint8_t*>(0x800013FF);
        libtp::gc_wii::os_cache::DCFlushRange(codehandlerIsWrittenAddress, sizeof(uint8_t));

        // Only the first 4 bytes of the codehandler source address's cache needs to be cleared, as we only need to check the
        // first 4 bytes to determine if anything else should be done
        uint32_t* dst = reinterpret_cast<uint32_t*>(0x80001800);
        libtp::memory::clear_DC_IC_Cache(dst, sizeof(uint32_t));

        // If something is already at 0x80001800, then assume a codehandler is already in place
        bool codehandlerIsWritten = static_cast<bool>(*codehandlerIsWrittenAddress);
        if (dst[0] != 0)
        {
            // If the codehandler has not been manually written, then assume an external codehandler is being used
            if (!codehandlerIsWritten)
            {
                return;
            }
        }
        else if (codehandlerIsWritten)
        {
            // Somehow nothing is at 0x80001800 when the codehandler was previously written
            return;
        }

        // Disable interrupts to be safe
        bool enable = libtp::gc_wii::os_interrupt::OSDisableInterrupts();

        // Write the codehandler to memory if necessary
        if (!codehandlerIsWritten)
        {
            // Perform a safety clear before writing the codehandler
            const uint32_t size = codehandler::codehandlerSize;
            libtp::memory::clearMemory(dst, size);

            // Copy the codehandler to 0x80001800
            memcpy(dst, reinterpret_cast<const void*>(codehandler::codehandler), size);

            // Copy the game id, disc number, and version number to 0x80001800
            // These use a total of 8 bytes, so handle as uint32_t variables for simplicity
            uint32_t* gameInfo = reinterpret_cast<uint32_t*>(0x80000000);
            dst[0] = gameInfo[0];
            dst[1] = gameInfo[1];

            // Uncomment out the next line to enable pause at boot
            // *reinterpret_cast<uint32_t*>( 0x80002774 ) = 1;

            // Clear the cache for the codehandler
            libtp::memory::clear_DC_IC_Cache(dst, size);

            // Set the flag for the codehandler being manually written and clear the cache for it
            *codehandlerIsWrittenAddress = 1;
            libtp::gc_wii::os_cache::DCFlushRange(codehandlerIsWrittenAddress, sizeof(uint8_t));
        }

        // Hook VISetNextFrameBuffer to branch to the codehandler
        uint32_t VISetNextFrameBufferAddress = reinterpret_cast<uint32_t>(libtp::gc_wii::vi::VISetNextFrameBuffer);
        libtp::patch::writeBranchBL(VISetNextFrameBufferAddress + 0x44, assembly::asmCallCodehandler);

        // Restore interrupts
        libtp::gc_wii::os_interrupt::OSRestoreInterrupts(enable);
    }
} // namespace mod