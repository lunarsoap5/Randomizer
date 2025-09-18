#include "main.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <math.h>

#ifdef DVD
#include "gc_wii/dvd.h"
#else
#include "gc_wii/card.h"
#endif

#include "Z2AudioLib/Z2AudioMgr.h"
#include "data/items.h"
#include "data/stages.h"
#include "game_patch/game_patch.h"
#include "memory.h"
#include "rando/data.h"
#include "rando/randomizer.h"
#include "tools.h"
#include "tp/J2DPicture.h"
#include "tp/JKRArchive.h"
#include "tp/control.h"
#include "tp/d_a_alink.h"
#include "tp/d_a_player.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_save.h"
#include "tp/dzx.h"
#include "tp/f_op_actor_mng.h"
#include "tp/f_op_actor.h"
#include "tp/f_op_scene_req.h"
#include "tp/f_op_msg_mng.h"
#include "tp/f_pc_node_req.h"
#include "tp/m_do_controller_pad.h"
#include "tp/m_do_audio.h"
#include "item_wheel_menu.h"
#include "user_patch/03_customCosmetics.h"
#include "data/flags.h"
#include "tp/JKRExpHeap.h"
#include "tp/m_do_ext.h"
#include "patch.h"
#include "asm.h"
#include "tp/d_a_itembase.h"
#include "tp/JKRMemArchive.h"
#include "tp/m_Do_dvd_thread.h"
#include "util/texture_utils.h"
#include "rando/bmg0.h"
#include "gc_wii/OSInterrupt.h"
#include "tp/d_kankyo.h"
#include "rando/customItems.h"
#include "cxx.h"
#include "tp/f_pc_executor.h"
#include "tp/d_msg_flow.h"
#include "tp/d_file_select.h"
#include "tp/dynamic_link.h"
#include "events.h"
#include "functionHooks.h"

namespace mod
{
    // Variables
    KEEP_VAR libtp::display::Console* gConsole = nullptr;
    KEEP_VAR bool gConsoleState = false;
    KEEP_VAR float rainbowPhaseAngle = 0.0f;

    void main()
    {
        // Call the boot REL
#ifdef DVD
        // The seed will be loaded in the boot REL
        libtp::tools::callRelProlog("/mod/boot.rel");
#elif defined PLATFORM_WII
        // The seed will be loaded in the boot REL
        libtp::tools::callRelProlog("rel/boot.rel");
#else
        // The seed will be loaded in the boot REL, so avoid mounting/unmounting the memory card multiple times
        libtp::tools::callRelProlog(CARD_SLOT_A, SUBREL_BOOT_ID);
#endif
    }

    void exit() {}

    void drawHeapDebugInfo()
    {
        static uint32_t archiveHeapLowestFreeSize = 0xFFFFFFFF;
        static uint32_t zeldaHeapLowestFreeSize = 0xFFFFFFFF;

        // Get the current maximum free size from any given block in the heaps
        // Archive heap
        void* heapPtr = libtp::tp::m_Do_ext::archiveHeap;
        if (heapPtr)
        {
            uint32_t currentFreeSize = libtp::tp::jkr_exp_heap::do_getFreeSize_JKRExpHeap(heapPtr);
            if (currentFreeSize < archiveHeapLowestFreeSize)
            {
                archiveHeapLowestFreeSize = currentFreeSize;
            }
        }

        // Zelda heap
        heapPtr = libtp::tp::m_Do_ext::zeldaHeap;
        if (heapPtr)
        {
            uint32_t currentFreeSize = libtp::tp::jkr_exp_heap::do_getFreeSize_JKRExpHeap(heapPtr);
            if (currentFreeSize < zeldaHeapLowestFreeSize)
            {
                zeldaHeapLowestFreeSize = currentFreeSize;
            }
        }

        // Get the values to draw
        char buf[96];
        snprintf(buf,
                 sizeof(buf),
                 "Archive: %.2fkb, Zelda: %.2fkb",
                 intToFloat(archiveHeapLowestFreeSize) / 1024.f,
                 intToFloat(zeldaHeapLowestFreeSize) / 1024.f);

        // Draw the values
        events::drawText(buf, 161, 430, 0xFFFFFFFF, true, 14.f);
    }

    KEEP_FUNC void setConsoleScreen(bool state)
    {
        gConsoleState = state;
        libtp::display::setConsole(state, 0);
    }

    void handleAnalogButtonInputs(uint32_t combo,
                                  libtp::tp::m_do_controller_pad::CPadInfo* padInfoPtr,
                                  uint32_t* buttonsPressedThisFramePtr,
                                  uint32_t* buttonsHeldPtr)
    {
        using namespace libtp::tp::m_do_controller_pad;

        // Make sure at least one of the button pointers is valid
        if (!buttonsPressedThisFramePtr && !buttonsHeldPtr)
        {
            return;
        }

        rando::Randomizer* randoPtr = rando::gRandomizer;

        // Get the threshold for the analog buttons
        constexpr float analogThreshold = 0.7f; // 70%

        // Check if L is included in the button combo
        // Analog L is currently not being used, so commented out
        /*
        if (combo & PadInputs::Button_L)
        {
            // Check if analog L is at 70% or more
            if (padInfoPtr->mTriggerLeft >= analogThreshold)
            {
                if (buttonsHeldPtr)
                {
                    *buttonsHeldPtr |= PadInputs::Button_L;
                }

                if (buttonsPressedThisFramePtr)
                {
                    // If prevFrameAnalogL is less than 70%, then 70% was reached this frame
                    if (randoPtr->getPrevFrameAnalogL() < analogThreshold)
                    {
                        *buttonsPressedThisFramePtr |= PadInputs::Button_L;
                    }
                }
            }
        }
        */

        // Check if R is included in the button combo
        if (combo & PadInputs::Button_R)
        {
            // Check if analog R is at 70% or more
            if (padInfoPtr->mTriggerRight >= analogThreshold)
            {
                if (buttonsHeldPtr)
                {
                    *buttonsHeldPtr |= PadInputs::Button_R;
                }

                if (buttonsPressedThisFramePtr)
                {
                    // If prevFrameAnalogR is less than 70%, then 70% was reached this frame
                    if (randoPtr->getPrevFrameAnalogR() < analogThreshold)
                    {
                        *buttonsPressedThisFramePtr |= PadInputs::Button_R;
                    }
                }
            }
        }
    }

    bool checkButtonsPressedThisFrame(uint32_t buttons)
    {
        using namespace libtp::tp::m_do_controller_pad;

        // Check if at least one button in the combo was pressed this frame
        return cpadInfo[PAD_1].mPressedButtonFlags & buttons;
    }

    bool checkButtonsPressedThisFrameAnalog(uint32_t buttons)
    {
        using namespace libtp::tp::m_do_controller_pad;
        CPadInfo* padInfoPtr = &cpadInfo[PAD_1];

        // Get the buttons pressed this frame
        uint32_t buttonsPressedThisFrame = padInfoPtr->mPressedButtonFlags;

        // Check if either of the analog buttons were pressed this frame past a certain threshold
        handleAnalogButtonInputs(buttons, padInfoPtr, &buttonsPressedThisFrame, nullptr);

        // Check if at least one button in the combo was pressed this frame
        return buttonsPressedThisFrame & buttons;
    }

    bool checkButtonsHeld(uint32_t buttons)
    {
        using namespace libtp::tp::m_do_controller_pad;

        // Check if the button combo is held
        return (cpadInfo[PAD_1].mButtonFlags & buttons) == buttons;
    }

    bool checkButtonsHeldAnalog(uint32_t buttons)
    {
        using namespace libtp::tp::m_do_controller_pad;
        CPadInfo* padInfoPtr = &cpadInfo[PAD_1];

        // Get the buttons held
        uint32_t buttonsHeld = padInfoPtr->mButtonFlags;

        // Check if either of the analog buttons are held past a certain threshold
        handleAnalogButtonInputs(buttons, padInfoPtr, nullptr, &buttonsHeld);

        // Check if the button combo is held
        return (buttonsHeld & buttons) == buttons;
    }

    bool checkButtonCombo(uint32_t combo)
    {
        using namespace libtp::tp::m_do_controller_pad;
        CPadInfo* padInfoPtr = &cpadInfo[PAD_1];

        // Check if the button combo is held
        if ((padInfoPtr->mButtonFlags & combo) != combo)
        {
            return false;
        }

        // Check if at least one button in the combo was pressed this frame
        return padInfoPtr->mPressedButtonFlags & combo;
    }

    bool checkButtonComboAnalog(uint32_t combo)
    {
        using namespace libtp::tp::m_do_controller_pad;
        CPadInfo* padInfoPtr = &cpadInfo[PAD_1];

        // Get the buttons pressed this frame and held
        uint32_t buttonsPressedThisFrame = padInfoPtr->mPressedButtonFlags;
        uint32_t buttonsHeld = padInfoPtr->mButtonFlags;

        // Check if either of the analog buttons are held past a certain threshold
        handleAnalogButtonInputs(combo, padInfoPtr, &buttonsPressedThisFrame, &buttonsHeld);

        // Check if the button combo is held
        if ((buttonsHeld & combo) != combo)
        {
            return false;
        }

        // Check if at least one button in the combo was pressed this frame
        return buttonsPressedThisFrame & combo;
    }

    bool handleTogglingConsole()
    {
        using namespace libtp::tp::m_do_controller_pad;

        // Check if the console should be toggled on/off
        if (checkButtonCombo(PadInputs::Button_R | PadInputs::Button_Z))
        {
            // Disable the input that was just pressed, as sometimes it could cause talking to Midna when in-game
            cpadInfo[PAD_1].mPressedButtonFlags = 0;

            // Toggle the console on/off
            setConsoleScreen(!gConsoleState);
            return true;
        }
        else
        {
            return false;
        }
    }

    KEEP_FUNC void handle_fapGm_Execute_FailLoadSeed()
    {
        // Handle toggling the console on/off
        handleTogglingConsole();

        // Call the original function
        return gReturn_fapGm_Execute();
    }

    KEEP_FUNC void handle_fapGm_Execute()
    {
        using namespace libtp;
        using namespace tp::m_do_controller_pad;
        using namespace tp::f_pc_node_req;
        using namespace tp::d_com_inf_game;

        // Uncomment out the next line to display debug heap info
        // #define DRAW_DEBUG_HEAP_INFO
#ifdef DRAW_DEBUG_HEAP_INFO
        drawHeapDebugInfo();
#undef DRAW_DEBUG_HEAP_INFO
#endif
        rando::Randomizer* randoPtr = rando::gRandomizer;

        // New frame, so the ring will be redrawn and quest items can be changed again
        randoPtr->getItemWheelMenuPtr()->resetRingDrawnThisFrame();
        randoPtr->getItemWheelMenuPtr()->changeQuestItem(true);

        dComIfG_inf_c* gameInfo = &dComIfG_gameInfo;
        CPadInfo* padInfo = &cpadInfo[PAD_1];

        // Handle game state updates
        if (l_fpcNdRq_Queue)
        {
            // Previous state
            const uint32_t prevState = randoPtr->getGameState();
            const uint32_t state = *reinterpret_cast<uint8_t*>(reinterpret_cast<uint32_t>(l_fpcNdRq_Queue) + 0x59);

            // Normal/Loading into game
            if ((prevState != GameState::GAME_ACTIVE) && (state == 11))
            {
                // Check whether we're in title screen CS
                if (0 != strcmp("S_MV000", gameInfo->play.mNextStage.mStage))
                {
                    randoPtr->setGameState(GameState::GAME_ACTIVE);
                }
            }
            else if ((prevState != GameState::GAME_TITLE) && ((state == 12) || (state == 13)))
            {
                randoPtr->setGameState(GameState::GAME_TITLE);
            }
        }
        // End of handling gameStates

        // Handle button inputs only if buttons are being held that weren't held last time
        const uint32_t currentButtons = padInfo->mButtonFlags;

        // Generic button checks need to occur outside the following conditional, so use a bool to determine if they should be
        // checked or not
        bool handledSpecialInputs = false;

        if (currentButtons != randoPtr->getLastButtonInput())
        {
            // Store before processing since we (potentially) un-set the padInfo values later
            randoPtr->setLastButtonInput(static_cast<uint16_t>(currentButtons));

            // Special combo to (de)activate the console should be handled first
            if (handleTogglingConsole())
            {
                handledSpecialInputs = true;
            }
        }

        tp::d_a_alink::daAlink* linkMapPtr = gameInfo->play.mPlayer;
        rando::Seed* seedPtr = randoPtr->getSeedPtr();

        if (!handledSpecialInputs)
        {
            // Handle generic button checks
            if (checkButtonComboAnalog(PadInputs::Button_R | PadInputs::Button_Y))
            {
                // Handle transforming
                events::handleQuickTransform(randoPtr);
            }
            else if (linkMapPtr && checkButtonsHeld(PadInputs::Button_R) && seedPtr->spinnerSpeedIsIncreased())
            {
                libtp::tp::f_op_actor::fopAc_ac_c* spinnerActor = libtp::tp::d_a_alink::getSpinnerActor(linkMapPtr);

                if (spinnerActor)
                {
                    float spinnerSpeed = spinnerActor->mSpeedF;
                    if (spinnerSpeed < 60.f)
                    {
                        spinnerSpeed += 2.f;
                        spinnerActor->mSpeedF = spinnerSpeed;
                    }
                }
            }
        }

        // Handle rando state
        if (randoPtr->getGameState() == GameState::GAME_ACTIVE)
        {
            if (!randoPtr->randomizerIsEnabled())
            {
                randoPtr->enableRandomizer();

                // Load checks for first load
                randoPtr->onStageLoad();

                // Volatile patches need to be applied whenever a file is loaded
                // getConsole() << "Applying volatile patches:\n";
                randoPtr->getSeedPtr()->applyVolatilePatches();

                // Until we implement passwords using a racetime bot, simply
                // handle decrypting hint text here.
                randoPtr->getSeedPtr()->getBMG0SectionPtr()->init();
            }
        }
        else if (randoPtr->randomizerIsEnabled())
        {
            // Temporarily disable the randomizer
            randoPtr->disableRandomizer();
        }

        // Custom events
        bool currentReloadingState;

        if (linkMapPtr)
        {
            // checkRestartRoom is needed for voiding
            currentReloadingState = tp::d_a_alink::checkRestartRoom(linkMapPtr);
        }
        else
        {
            // If linkMapPtr is not set, then assume a room is being loaded
            // In most cases this will be used for triggering onLoad
            currentReloadingState = true;
        }

        const bool prevReloadingState = randoPtr->getRoomReloadingState();

        // Custom events
        if (currentReloadingState)
        {
            if (!prevReloadingState)
            {
                // OnLoad
                events::onLoad(randoPtr);
            }
        }
        else
        {
            if (prevReloadingState)
            {
                // OffLoad
                events::offLoad(randoPtr);

                // SetHUDCosmetics
                user_patch::setHUDCosmetics(randoPtr);
            }
        }

        randoPtr->setRoomReloadingState(currentReloadingState);

        if (!libtp::tp::d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Title_Screen]))
        {
            handleFoolishItem(randoPtr);
        }

        tools::xorshift32(randoPtr->getRandStatePtr());

        if (randoPtr->getTimeChange() != rando::TimeChange::NO_CHANGE)
        {
            events::handleTimeSpeed();
        }

        if (linkMapPtr)
        {
            // Giving items at any point
            initGiveItemToPlayer(linkMapPtr, randoPtr);

            if (randoPtr->getSeedPtr()->isMidnaHairRainbow())
            {
                adjustMidnaHairColor();
            }
        }

        // End of custom events

        // Call the original function
        gReturn_fapGm_Execute();

        // Main code has ran, so update previous frame variables
        // Analog L is currently not being used, so commented out
        // randoPtr->setPrevFrameAnalogL(padInfo->mTriggerLeft);
        randoPtr->setPrevFrameAnalogR(padInfo->mTriggerRight);
    }

    void initGiveItemToPlayer(libtp::tp::d_a_alink::daAlink* linkMapPtr, rando::Randomizer* randoPtr)
    {
        using namespace libtp::tp;
        using namespace mod::rando;

        switch (linkMapPtr->mProcID)
        {
            case d_a_alink::PROC_WAIT:
            case d_a_alink::PROC_TIRED_WAIT:
            case d_a_alink::PROC_MOVE:
            case d_a_alink::PROC_WOLF_WAIT:
            case d_a_alink::PROC_WOLF_TIRED_WAIT:
            case d_a_alink::PROC_WOLF_MOVE:
            case d_a_alink::PROC_ATN_MOVE:
            case d_a_alink::PROC_WOLF_ATN_AC_MOVE:
            {
                // Check if link is currently in a cutscene
                if (d_a_alink::checkEventRun(linkMapPtr))
                {
                    break;
                }

                // Ensure that link is not currently in a message-based event.
                if (linkMapPtr->mMsgFlow.mEventId != 0)
                {
                    break;
                }

                d_com_inf_game::dComIfG_inf_c* gameInfo = &d_com_inf_game::dComIfG_gameInfo;
                uint8_t* reserveBytesPtr = &gameInfo->save.save_file.reserve.unk[0];
                uint32_t itemToGive = 0xFF;

                for (uint32_t i = 0; i < GIVE_PLAYER_ITEM_RESERVED_BYTES; i++)
                {
                    const uint32_t storedItem = reserveBytesPtr[i];

                    if (storedItem)
                    {
                        const EventItemStatus giveItemToPlayerStatus = randoPtr->getGiveItemToPlayerStatus();

                        // If we have the call to clear the queue, then we want to clear the item and break out.
                        if (giveItemToPlayerStatus == EventItemStatus::CLEAR_QUEUE)
                        {
                            reserveBytesPtr[i] = 0;
                            randoPtr->setGiveItemToPlayerStatus(EventItemStatus::QUEUE_EMPTY);
                            break;
                        }

                        // If the queue is empty and we have an item to give, update the queue state.
                        else if (giveItemToPlayerStatus == EventItemStatus::QUEUE_EMPTY)
                        {
                            randoPtr->setGiveItemToPlayerStatus(EventItemStatus::ITEM_IN_QUEUE);
                        }

                        itemToGive = game_patch::_04_verifyProgressiveItem(randoPtr, storedItem);
                        break;
                    }
                }

                // if there is no item to give, break out of the case.
                if (itemToGive == 0xFF)
                {
                    break;
                }

                libtp::tp::d_com_inf_game::dComIfG_play* playPtr = &gameInfo->play;
                playPtr->mEvent.mGtItm = static_cast<uint8_t>(itemToGive);

                // Set the process value for getting an item to start the "get item" cutscene when next available.
                linkMapPtr->mProcID = libtp::tp::d_a_alink::PROC_GET_ITEM;

                //  Get the event index for the "Get Item" event.
                const int16_t eventIdx = d_event_manager::getEventIdx3(&playPtr->mEvtManager,
                                                                       reinterpret_cast<f_op_actor::fopAc_ac_c*>(linkMapPtr),
                                                                       "DEFAULT_GETITEM",
                                                                       0xFF);

                // Finally we want to modify the event stack to prioritize our custom event so that it happens next.
                libtp::tp::f_op_actor_mng::fopAcM_orderChangeEventId(linkMapPtr, eventIdx, 1, 0xFFFF);
            }
            default:
            {
                break;
            }
        }
    }

    int32_t initCreatePlayerItem(uint32_t itemID,
                                 uint32_t flag,
                                 const float pos[3],
                                 int32_t roomNo,
                                 const int16_t rot[3],
                                 const float scale[3])
    {
        uint32_t params = 0xFF0000 | ((flag & 0xFF) << 0x8) | (itemID & 0xFF);
        return libtp::tp::f_op_actor_mng::fopAcM_create(539, params, pos, roomNo, rot, scale, -1);
    }

    KEEP_FUNC void resetQueueOnFileSelectScreen(libtp::tp::d_file_select::dFile_select_c* thisPtr)
    {
        using namespace libtp::tp::d_com_inf_game;

        // Call the original function immediately to avoid storing thisPtr on the stack
        gReturn_dFile_select_c___create(thisPtr);

        // giveItemToPlayerStatus needs to be reset to QUEUE_EMPTY upon going to the file select screen, as the player could
        // have potentially saved/died after initializing getting an item (which would set giveItemToPlayerStatus to
        // EventItemStatus::ITEM_IN_QUEUE), and then chosen to return to the title screen.
        rando::gRandomizer->setGiveItemToPlayerStatus(rando::EventItemStatus::QUEUE_EMPTY);

        // The reserved bytes that the queue uses to store the items to give are not cleared upon starting a new file, which
        // means that the player could soft reset during the process of being given item(s), and then start a new file to be
        // given those items on that new file. To avoid this, the reserved bytes need to be cleared upon going to the file
        // select screen. All of the reserved bytes excluding the ones used by the queue will also be cleared, in the event that
        // they need to be used for other stuff in the future.
        libtp::memory::clearMemory(&dComIfG_gameInfo.save.save_file.reserve, sizeof(dComIfG_gameInfo.save.save_file.reserve));
    }

    KEEP_FUNC bool handle_do_unlink(libtp::tp::dynamic_link::DynamicModuleControl* dmc)
    {
        events::onRELUnlink(rando::gRandomizer, dmc);
        return gReturn_do_unlink(dmc);
    }

    KEEP_FUNC bool handle_actorInit(void* mStatus_roomControl,
                                    libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                    int32_t unk3,
                                    void* unk4)
    {
        // Load DZX based randomizer checks that are stored in the local DZX
        events::onDZX(rando::gRandomizer, chunkTypeInfo);
        events::loadCustomActors(mStatus_roomControl);
        return gReturn_actorInit(mStatus_roomControl, chunkTypeInfo, unk3, unk4);
    }

    KEEP_FUNC bool handle_actorInit_always(void* mStatus_roomControl,
                                           libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                           int32_t unk3,
                                           void* unk4)
    {
        // Load DZX based randomizer checks that are stored in the global DZX
        events::onDZX(rando::gRandomizer, chunkTypeInfo);
        return gReturn_actorInit_always(mStatus_roomControl, chunkTypeInfo, unk3, unk4);
    }

    KEEP_FUNC bool handle_actorCommonLayerInit(void* mStatus_roomControl,
                                               libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                               int32_t unk3,
                                               void* unk4)
    {
        // Load DZX based checks that are stored in the current layer DZX
        rando::Randomizer* randoPtr = rando::gRandomizer;
        events::onDZX(randoPtr, chunkTypeInfo);
        events::loadCustomRoomActors(randoPtr);

        return gReturn_actorCommonLayerInit(mStatus_roomControl, chunkTypeInfo, unk3, unk4);
    }

    KEEP_FUNC int32_t handle_tgscInfoInit(void* stageDt, void* i_data, int32_t entryNum, void* param_3)
    {
        events::loadCustomRoomSCOBs();
        return gReturn_tgscInfoInit(stageDt, i_data, entryNum, param_3);
    }

    KEEP_FUNC void handle_dComIfGp_setNextStage(const char* stage,
                                                int16_t point,
                                                int8_t roomNo,
                                                int8_t layer,
                                                float lastSpeed,
                                                uint32_t lastMode,
                                                int32_t setPoint,
                                                int8_t wipe,
                                                int16_t lastAngle,
                                                int32_t param_9,
                                                int32_t wipSpeedT)
    {
        const int32_t stageIDX = libtp::tools::getStageIndex(stage);

        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();
        const uint32_t numShuffledEntrances = seedPtr->getNumShuffledEntrances();
        const rando::ShuffledEntrance* shuffledEntrances = seedPtr->getShuffledEntrancesPtr();

        // getConsole() << stageIDX << "," << roomNo << "," << point << "," << layer << "\n";

        if (!libtp::tp::d_a_alink::checkStageName(
                libtp::data::stage::allStages
                    [libtp::data::stage::StageIDs::Title_Screen])) // We won't want to shuffle if we are loading a save since
                                                                   // some stages use their default spawn for their entrances.
        {
            for (uint32_t i = 0; i < numShuffledEntrances; i++)
            {
                const rando::ShuffledEntrance* currentEntrance = &shuffledEntrances[i];

                if ((stageIDX == currentEntrance->getOrigStageIDX()) && (roomNo == currentEntrance->getOrigRoomIDX()) &&
                    (point == currentEntrance->getOrigSpawn()) && (layer == currentEntrance->getOrigState()))
                {
                    // getConsole() << "Shuffling Entrance\n";

                    // Note: we use 0 for lastMode so warping out with Ooccoo
                    // (normally 0xC) works correctly with ER. We can
                    // potentially add more logic here once more entrance types
                    // are randomized (especially as it relates to riding on
                    // Epona, etc.)
                    return gReturn_dComIfGp_setNextStage(libtp::data::stage::allStages[currentEntrance->getNewStageIDX()],
                                                         currentEntrance->getNewSpawn(),
                                                         currentEntrance->getNewRoomIDX(),
                                                         currentEntrance->getNewState(),
                                                         lastSpeed,
                                                         lastMode == 0xC ? 0 : lastMode,
                                                         setPoint,
                                                         wipe,
                                                         lastAngle,
                                                         param_9,
                                                         wipSpeedT);
                }
            }
        }

        // getConsole() << "No match found.\n";

        return gReturn_dComIfGp_setNextStage(stage,
                                             point,
                                             roomNo,
                                             layer,
                                             lastSpeed,
                                             lastMode,
                                             setPoint,
                                             wipe,
                                             lastAngle,
                                             param_9,
                                             wipSpeedT);
    }

    KEEP_FUNC void handle_roomLoader(void* data, void* stageDt, int32_t roomNo)
    {
        rando::Randomizer* randoPtr = rando::gRandomizer;
        events::onARC(randoPtr, data, roomNo, rando::FileDirectory::Room); // Replace room based checks.

        return gReturn_roomLoader(data, stageDt, roomNo);
    }

    KEEP_FUNC void handle_stageLoader(void* data, void* stageDt)
    {
        rando::Randomizer* randoPtr = rando::gRandomizer;
        // This function is a placeholder for now. May work with Taka on getting some ARC checks converted over to use this
        // function instead of roomLoader
        events::onARC(randoPtr, data, 0xFF, rando::FileDirectory::Stage); // Replace stage based checks.
        return gReturn_stageLoader(data, stageDt);
    }

    KEEP_FUNC int32_t handle_dStage_playerInit(void* stageDt,
                                               libtp::tp::d_stage::stage_dzr_header_entry* i_data,
                                               int32_t num,
                                               void* raw_data)
    {
        libtp::tp::d_save::dSv_player_status_a_c* playerStatusPtr =
            &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a;

        // If Link is in wolf form, then we want to change the entrance type to prevent a softlock caused by Link repeatedly
        // voiding out.

        libtp::tp::d_stage::stage_actor_data_class* allPLYR = i_data->mDzrDataPointer;

        for (int32_t i = 0; i < num; i++)
        {
            uint8_t* mParameter = reinterpret_cast<uint8_t*>(&allPLYR[i].mParameter);
            uint8_t* entranceType = &mParameter[2];

            switch (*entranceType)
            {
                // Only replace the entrance type if it is a door.
                case 0x80:
                case 0xA0:
                case 0xB0:
                {
                    if (playerStatusPtr->currentForm == 1)
                    {
                        // Change the entrance type to play the animation of walking out of the loading zone instead of entering
                        // through the door.
                        *entranceType = 0x50;
                    }
                    break;
                }

                case 0xD0:
                {
                    if (libtp::tp::d_a_alink::checkStageName(
                            libtp::data::stage::allStages[libtp::data::stage::StageIDs::Lake_Hylia]) &&
                        !libtp::tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::CLEARED_LANAYRU_TWILIGHT))
                    {
                        *entranceType = 0x50;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        return gReturn_dStage_playerInit(stageDt, i_data, num, raw_data);
    }

    KEEP_FUNC int32_t procCoGetItemInitCreateItem(const float pos[3],
                                                  int32_t item,
                                                  uint8_t unk3,
                                                  int32_t unk4,
                                                  int32_t unk5,
                                                  const float rot[3],
                                                  const float scale[3])
    {
        rando::Randomizer* randoPtr = rando::gRandomizer;
        if (randoPtr->getGiveItemToPlayerStatus() == rando::EventItemStatus::ITEM_IN_QUEUE)
        {
            randoPtr->setGiveItemToPlayerStatus(rando::EventItemStatus::CLEAR_QUEUE);
        }

        return libtp::tp::f_op_actor_mng::createItemForPresentDemo(pos, item, unk3, unk4, unk5, rot, scale);
    }

    KEEP_FUNC int32_t handle_createItemForBoss(const float pos[3],
                                               int32_t item,
                                               int32_t roomNo,
                                               const int16_t rot[3],
                                               const float scale[3],
                                               float unk6,
                                               float unk7,
                                               int32_t parameters)
    {
        (void)unk6;
        (void)unk7;

        // Spawn the appropriate item with model
        rando::Randomizer* randoPtr = rando::gRandomizer;

        uint32_t itemID = randoPtr->getBossItem();
        itemID = game_patch::_04_verifyProgressiveItem(randoPtr, itemID);

        if (item == libtp::data::items::Heart_Container) // used for Dungeon Heart Containers
        {
            parameters = 0x9F;
        }

        // If we are in hyrule field then the function is running to give us the Hot Springwater heart piece and we want it to
        // spawn on the ground.
        if (libtp::tp::d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Hyrule_Field]))
        {
            *const_cast<float*>(&pos[1]) = -190.f;
        }

        return initCreatePlayerItem(itemID, parameters & 0xFF, pos, roomNo, rot, scale);
    }

    KEEP_FUNC int32_t handle_createItemForMidBoss(const float pos[3],
                                                  int32_t item,
                                                  int32_t roomNo,
                                                  const int16_t rot[3],
                                                  const float scale[3],
                                                  int32_t unk6,
                                                  int32_t itemPickupFlag)
    {
        if (item == libtp::data::items::Boomerang)
        {
            // Spawn the appropriate item
            rando::Randomizer* randoPtr = rando::gRandomizer;

            uint32_t itemID = randoPtr->getBossItem();
            itemID = game_patch::_04_verifyProgressiveItem(randoPtr, itemID);

            return initCreatePlayerItem(itemID, 0xFF, pos, roomNo, rot, scale);
        }

        return gReturn_createItemForMidBoss(pos, item, roomNo, rot, scale, unk6, itemPickupFlag);
    }

    KEEP_FUNC int32_t handle_createItemForPresentDemo(const float pos[3],
                                                      int32_t item,
                                                      uint8_t unk3,
                                                      int32_t unk4,
                                                      int32_t unk5,
                                                      const float rot[3],
                                                      const float scale[3])
    {
        item = game_patch::_04_verifyProgressiveItem(rando::gRandomizer, item);
        return gReturn_createItemForPresentDemo(pos, item, unk3, unk4, unk5, rot, scale);
    }

    KEEP_FUNC int32_t handle_createItemForTrBoxDemo(const float pos[3],
                                                    int32_t item,
                                                    int32_t itemPickupFlag,
                                                    int32_t roomNo,
                                                    const int16_t rot[3],
                                                    const float scale[3])
    {
        item = game_patch::_04_verifyProgressiveItem(rando::gRandomizer, item);
        return gReturn_createItemForTrBoxDemo(pos, item, itemPickupFlag, roomNo, rot, scale);
    }

    KEEP_FUNC void handle_CheckFieldItemCreateHeap(libtp::tp::f_op_actor::fopAc_ac_c* actor)
    {
        libtp::tp::d_a_itembase::ItemBase* item = static_cast<libtp::tp::d_a_itembase::ItemBase*>(actor);

        // We determine whether to use the item_resource or the field_item_resource structs to spawn an item based on the item
        // being created.
        switch (item->m_itemNo)
        {
            case libtp::data::items::Empty_Bottle:
            case libtp::data::items::Sera_Bottle:
            case libtp::data::items::Jovani_Bottle:
            case libtp::data::items::Coro_Bottle:
            case libtp::data::items::Purple_Rupee_Links_House:
            case libtp::data::items::Poe_Soul:
            {
                return libtp::tp::d_a_itembase::CheckItemCreateHeap(actor);
            }
            default:
            {
                return gReturn_CheckFieldItemCreateHeap(actor);
            }
        }
    }

    KEEP_FUNC void handle_CreateInit(void* daItem)
    {
        // Modify the scale params of the rupee actor before it is created.
        gReturn_CreateInit(daItem);
        events::onAdjustCreateRupeeItemParams(daItem);
        return;
    }

    KEEP_FUNC void handle_setLineUpItem(libtp::tp::d_save::dSv_player_item_c* unk1)
    {
        (void)unk1;

        using namespace libtp::tp::d_com_inf_game;

        static const uint8_t i_item_lst[24] = {0x0A, 0x08, 0x06, 0x02, 0x09, 0x04, 0x03, 0x00, 0x01, 0x17, 0x14, 0x05,
                                               0x0F, 0x10, 0x11, 0x0B, 0x0C, 0x0D, 0x0E, 0x13, 0x12, 0x16, 0x15, 0x7};
        int32_t i1 = 0;
        int32_t i2 = 0;
        libtp::tp::d_save::dSv_player_item_c* playerItemStructPtr = &dComIfG_gameInfo.save.save_file.player.player_item;
        uint8_t* playerItemSlots = &playerItemStructPtr->item_slots[0];

        for (; i1 < 24; i1++) // Clear all of the item slots.
        {
            playerItemSlots[i1] = 0xFF;
        }

        uint8_t* playerItem = &playerItemStructPtr->item[0];
        for (i1 = 0; i1 < 24; i1++) // Fill all of the item wheel slots with their respective items if gotten.
        {
            const uint32_t currentEntry = i_item_lst[i1];
            if (playerItem[currentEntry] != 0xFF)
            {
                playerItemSlots[i2] = static_cast<uint8_t>(currentEntry);
                i2++;
            }
        }
    }

    KEEP_FUNC void handle_execItemGet(uint8_t item)
    {
        item = game_patch::_04_verifyProgressiveItem(rando::gRandomizer, item);
        return gReturn_execItemGet(item);
    }

    KEEP_FUNC int32_t handle_checkItemGet(uint8_t item, int32_t defaultValue)
    {
        using namespace libtp;
        using namespace libtp::tp;
        using namespace libtp::data;
        using namespace libtp::data::stage;

        const auto stagesPtr = &allStages[0];
        switch (item)
        {
            case items::Hylian_Shield:
            {
                // Check if we are at Kakariko Malo mart and verify that we have not bought the shield.
                if (libtp::tools::playerIsInRoomStage(3, stagesPtr[StageIDs::Kakariko_Village_Interiors]) &&
                    !tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::BOUGHT_HYLIAN_SHIELD_AT_MALO_MART))
                {
                    // Return false so we can buy the shield.
                    return 0;
                }
                break;
            }
            case items::Hawkeye:
            {
                // Check if we are at Kakariko Village and that the hawkeye is currently not for sale.
                if ((tp::d_a_alink::checkStageName(stagesPtr[StageIDs::Kakariko_Village]) &&
                     !libtp::tp::d_save::isSwitch_dSv_memBit(&d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags, 0x3E)))
                {
                    // Return false so we can buy the hawkeye.
                    return 0;
                }
                break;
            }
            case items::Ordon_Shield:
            case items::Wooden_Shield:
            {
                // Check if we are at Kakariko Malo Mart and that the Wooden Shield has not been bought.
                if (libtp::tools::playerIsInRoomStage(3, stagesPtr[StageIDs::Kakariko_Village_Interiors]) &&
                    !libtp::tp::d_save::isSwitch_dSv_memBit(&d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags, 0x5))
                {
                    // Return false so we can buy the wooden shield.
                    return 0;
                }
                break;
            }
            case items::Ordon_Pumpkin:
            case items::Ordon_Goat_Cheese:
            {
                // Check to see if currently in Snowpeak Ruins
                if (libtp::tp::d_a_alink::checkStageName(stagesPtr[StageIDs::Snowpeak_Ruins]))
                {
                    // Return false so that yeta will give the map item no matter what.
                    return 0;
                }
                break;
            }
            case items::Ball_and_Chain:
            {
                // Check to see if currently in Snowpeak Ruins
                if (libtp::tp::d_a_alink::checkStageName(stagesPtr[StageIDs::Darkhammer]))
                {
                    return libtp::tp::d_save::isSwitch_dSv_memBit(&d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags,
                                                                  0x5F); // Picked up the Ball and Chain check.
                }
                break;
            }
            default:
            {
                break;
            }
        }

        return gReturn_checkItemGet(item, defaultValue);
    }
    KEEP_FUNC void handle_item_func_ASHS_SCRIBBLING()
    {
        using namespace libtp::data::flags;
        if (!libtp::tp::d_com_inf_game::dComIfGs_isEventBit(GOT_CORAL_EARRING_FROM_RALIS))
        {
            gReturn_item_func_ASHS_SCRIBBLING();
        }
    }

    KEEP_FUNC void handle_item_func_KAKERA_HEART()
    {
        rando::Randomizer* randoPtr = rando::gRandomizer;
        // Run the vanilla function immedaitely as it updates necessary health values.
        gReturn_item_func_KAKERA_HEART();

        uint8_t maxLife = libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.maxHealth + 1;

        // Check if we have enough hearts to break the barrier.
        randoPtr->checkSetHCBarrierFlag(rando::HC_Hearts, maxLife);

        // Check if we have enough hearts to unlock the BK check.
        randoPtr->checkSetHCBkFlag(rando::HC_BK_Hearts, maxLife);
    }

    KEEP_FUNC void handle_item_func_UTUWA_HEART()
    {
        rando::Randomizer* randoPtr = rando::gRandomizer;
        // Run the vanilla function immedaitely as it updates necessary health values.
        gReturn_item_func_UTUWA_HEART();

        uint8_t maxLife = libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.maxHealth + 5;

        // Check if we have enough hearts to break the barrier.
        randoPtr->checkSetHCBarrierFlag(rando::HC_Hearts, maxLife);

        // Check if we have enough hearts to unlock the BK check.
        randoPtr->checkSetHCBkFlag(rando::HC_BK_Hearts, maxLife);
    }

    KEEP_FUNC bool handle_setMessageCode_inSequence(libtp::tp::control::TControl* control,
                                                    const void* TProcessor,
                                                    uint16_t unk3,
                                                    uint16_t msgId)
    {
        // Call the original function immediately, as a lot of stuff needs to be set before our code runs
        const bool ret = gReturn_setMessageCode_inSequence(control, TProcessor, unk3, msgId);

        // Make sure the function ran successfully
        if (ret)
        {
            game_patch::_05_setCustomItemMessage(control, TProcessor, unk3, msgId);
        }
        return ret;
    }

    KEEP_FUNC uint32_t handle_getFontCCColorTable(uint8_t colorId, uint8_t unk)
    {
        if (colorId >= 0x9)
        {
            return game_patch::_05_getCustomMsgColor(colorId);
        }
        else
        {
            return gReturn_getFontCCColorTable(colorId, unk);
        }
    }

    KEEP_FUNC uint32_t handle_getFontGCColorTable(uint8_t colorId, uint8_t unk)
    {
        if (colorId >= 0x9)
        {
            return game_patch::_05_getCustomMsgColor(colorId);
        }
        else
        {
            return gReturn_getFontCCColorTable(colorId, unk);
        }
    }

    KEEP_FUNC int32_t handle_query001(void* unk1, void* unk2, int32_t unk3)
    {
        using namespace libtp::data::flags;
        using namespace libtp::data;

        uint16_t flag = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint32_t>(unk2) + 0x4);

        switch (flag)
        {
            case 0xFA: // MDH Completed
            {
                // Check to see if currently in Jovani's house
                if (libtp::tools::playerIsInRoomStage(5, stage::allStages[stage::StageIDs::Castle_Town_Shops]))
                {
                    return 0; // Return 0 to be able to turn souls into Jovani pre MDH
                }
                break;
            }
            default:
            {
                break;
            }
        }
        return gReturn_query001(unk1, unk2, unk3);
    }

    KEEP_FUNC int32_t handle_query022(void* unk1, void* unk2, int32_t unk3)
    {
        return events::proc_query022(unk1, unk2, unk3);
    }

    KEEP_FUNC int32_t handle_query023(void* unk1, void* unk2, int32_t unk3)
    {
        return events::proc_query023(unk1, unk2, unk3);
    }

    KEEP_FUNC int32_t handle_query025(void* unk1, void* unk2, int32_t unk3)
    {
        return events::proc_query025(unk1, unk2, unk3);
    }

    KEEP_FUNC uint8_t handle_checkEmptyBottle(libtp::tp::d_save::dSv_player_item_c* playerItem)
    {
        if (libtp::tp::d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Cave_of_Ordeals]))
        {
            // Return 1 to allow the player to collect the item from the floor 50 reward, as this will make the game think that
            // the player has an empty bottle.
            return 1;
        }
        return gReturn_checkEmptyBottle(playerItem);
    }

    KEEP_FUNC int32_t handle_query037(void* unk1, void* unk2, int32_t unk3)
    {
        // Call the original function immediately as we need its output
        const int32_t menuType = gReturn_query037(unk1, unk2, unk3);

        // TODO: disabling this time of day change
        // if ((menuType == 0x2) && (reinterpret_cast<int32_t>(libtp::tp::d_a_player::m_midnaActor) ==
        //                           libtp::tp::f_op_actor_mng::fopAcM_getTalkEventPartner(nullptr)))
        // {
        //     events::handleTimeOfDayChange();
        // }

        return menuType;
    }

    KEEP_FUNC int32_t handle_query049(void* unk1, void* unk2, int32_t unk3)
    {
        const int32_t poeFlag = gReturn_query049(unk1, unk2, unk3);

        if ((poeFlag == 4) && !libtp::tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::GOT_BOTTLE_FROM_JOVANI))
        {
            return 3;
        }

        return poeFlag;
    }

    KEEP_FUNC int32_t handle_query042(void* unk1, void* unk2, int32_t unk3)
    {
        return events::proc_query042(unk1, unk2, unk3);
    }

    /*
    KEEP_FUNC int32_t handle_event000( void* messageFlow, void* nodeEvent, void* actrPtr )
    {
        // Prevent the hidden skill CS from setting the proper flags
        if ( libtp::tp::d_a_alink::checkStageName( libtp::data::stage::allStages[libtp::data::stage::StageIDs::Hidden_Skill] ) )
        {
            *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( nodeEvent ) + 4 ) = 0x0000;
        }
        return gReturn_event000( messageFlow, nodeEvent, actrPtr );
    }
    */

    KEEP_FUNC int32_t handle_event017(void* messageFlow, void* nodeEvent, void* actrPtr)
    {
        const uint32_t messageParam = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint32_t>(nodeEvent) + 4);

        // Prevent Gor Liggs from setting the third key shard flag
        if (messageParam == 0x00FB)
        {
            *reinterpret_cast<uint16_t*>(reinterpret_cast<uint32_t>(nodeEvent) + 4) = 0x0000;
        }

        return gReturn_event017(messageFlow, nodeEvent, actrPtr);
    }

    KEEP_FUNC int32_t handle_doFlow(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                    libtp::tp::f_op_actor::fopAc_ac_c* actrPtr,
                                    libtp::tp::f_op_actor::fopAc_ac_c** actrValue,
                                    int32_t i_flow)
    {
        using namespace libtp::data::stage;

        // uint16_t customInitNode = rando::gRandomizer->getCustomInitNodeIndex(msgFlow);
        // if (customInitNode != 0xFFFF)
        // {
        //     // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we
        //     // need to unset it.
        //     msgFlow->field_0x26 = 0;
        //     msgFlow->field_0x10 = customInitNode; // TODO: testing adjust for zel_00.bmg
        //     // Was defaulting to 0x24 (36) at the moment.
        // }

        // if (msgFlow->mFlow == 0x7FFF) // Check if it equals our custom flow value
        // {
        // if (msgFlow->mMsg == 0xFFFFFFFF)
        // {
        //     uint16_t customInitNode = rando::gRandomizer->getCustomInitNodeIndex(msgFlow->mFlow);
        //     if (customInitNode != 0xFFFF)
        //     {
        //         // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we
        //         // need to unset it.
        //         msgFlow->field_0x26 = 0;
        //         msgFlow->field_0x10 = customInitNode; // TODO: testing adjust for zel_00.bmg
        //         // Was defaulting to 0x24 (36) at the moment.
        //     }

        //     // This seems to be the correct check in order to initialize the
        //     // FLW entry to the correct value we are abusing, and then have
        //     // it play out normally. That is all we do in this function: if
        //     // the mFlow should map to an abusable one and mMsg is -1, then
        //     // we set field_0x26 to 0 and set the initial node to the
        //     // abusable one. Mapping the abusableNode+flow to a 0x1360 value
        //     // is done in setNormalMsg.

        //     // TODO: it seems like we specifically do not want to set offset
        //     // 0x20 (mMsg) to 0 right now since we are not overwriting it
        //     // later. Can see once we add more code for the signs and Midna,
        //     // but for now where are simply using an existing node, setting
        //     // this to 0 crashes the game while leaving it at -1 works
        //     // great. It gets set to -1 by the code in setInitValue inside
        //     // dMsgFlow_c::init (because r6 param_2 is hardcoded to 0 by
        //     // sign actor), so we want to leave this alone for the code to
        //     // function normally. It will get set inside `setNormalMsg` as
        //     // appropriate I believe.

        //     // Clear the invalid msg value since it will be set by the game once our text is loaded.
        //     // msgFlow->mMsg = 0;

        //     // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we need
        //     // to unset it.
        //     // msgFlow->field_0x26 = 0;

        //     // if (libtp::tp::d_a_alink::checkStageName(allStages[StageIDs::Hyrule_Field]) ||
        //     //     libtp::tp::d_a_alink::checkStageName(allStages[StageIDs::Outside_Castle_Town]) ||
        //     //     libtp::tp::d_a_alink::checkStageName(allStages[StageIDs::Lake_Hylia]))
        //     // {
        //     //     // Hyrule Field and outside Lake Hylia do not have a valid flow node for node 0 so we want it to use its
        //     //     // native node (8)
        //     //     msgFlow->field_0x10 = 0x8;
        //     // }
        //     // else if (libtp::tp::d_a_alink::checkStageName(allStages[StageIDs::Castle_Town]))
        //     // {
        //     //     // For Castle Town, both 1 and 2 seem to work at the very
        //     //     // least. If you use 4, you will also get shiny shoes.
        //     //     msgFlow->field_0x10 = 0x2;
        //     // }
        //     // else if (libtp::tp::d_a_alink::checkStageName(allStages[StageIDs::Death_Mountain]))
        //     // {
        //     //     // Death Mountain does not have a valid flow node for node 0 so we want it to use its
        //     //     // native node (4)
        //     //     msgFlow->field_0x10 = 0x4;
        //     // }
        //     // else
        //     // {
        //     //     // Sets the flow to use the same flow grouping as the standard flow that getItem text uses.
        //     //     msgFlow->field_0x10 = 0;
        //     // }

        //     // Right now, 23 is using the node for "If you want to warp from
        //     // here, you hvae to find a portal in this area first!" which
        //     // does not have anything special.

        //     // msgFlow->field_0x10 = 23; // TODO: testing adjust for zel_00.bmg
        //     // msgFlow->field_0x10 = 22; // TODO: testing adjust for zel_00.bmg
        //     // msgFlow->field_0x10 = 36; // TODO: testing adjust for zel_00.bmg
        // }
        // }

        // rando::gRandomizer->setLatestCustomINFIndex(msgFlow->mFlow);

        int32_t ret = gReturn_doFlow(msgFlow, actrPtr, actrValue, i_flow);

        // rando::gRandomizer->setLatestCustomINFIndex(0xFFFF);

        return ret;

        // return gReturn_doFlow(msgFlow, actrPtr, actrValue, i_flow);
    }

    KEEP_FUNC void handle_setNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                       uint16_t flwIndex,
                                       libtp::tp::f_op_actor::fopAc_ac_c* actrPtr)
    {
        // try to remap here
        // const uint16_t customInitNode =
        //     rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr()->getCustomInitNodeIndex(msgFlow, flwIndex);
        // if (customInitNode != 0xFFFF)
        // {
        //     flwIndex = customInitNode;
        //     // // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we
        //     // // need to unset it.
        //     // msgFlow->field_0x26 = 0;
        //     // msgFlow->field_0x10 = customInitNode; // TODO: testing adjust for zel_00.bmg
        //     // // Was defaulting to 0x24 (36) at the moment.
        // }

        // TODO: update
        rando::gRandomizer->checkResetFlowContext(msgFlow);

        // const rando::FlwIdxRemap* remapEntry =
        //     rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr()->getCustomInitNodeIndex(msgFlow, flwIndex);
        // if (remapEntry != nullptr)
        // {
        //     flwIndex = remapEntry->getNewInitFLWIndex();
        //     const uint16_t newContext = remapEntry->getNewContext();
        //     rando::gRandomizer->setFlowContext(msgFlow, newContext);
        //     // // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we
        //     // // need to unset it.
        //     // msgFlow->field_0x26 = 0;
        //     // msgFlow->field_0x10 = customInitNode; // TODO: testing adjust for zel_00.bmg
        //     // // Was defaulting to 0x24 (36) at the moment.
        // }

        if (msgFlow != nullptr)
        {
            uint16_t flowContext = rando::gRandomizer->getFlowContext();

            const uint16_t* remapEntry =
                rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr()->getNodeRemapData(msgFlow, flwIndex, flowContext);
            if (remapEntry != nullptr)
            {
                flwIndex = remapEntry[0];
                const uint16_t newContext = remapEntry[1];
                rando::gRandomizer->setFlowContext(msgFlow, newContext);
                // // When this byte is set, the current event is aborted. With unused nodes, it is set to 1 by default so we
                // // need to unset it.
                // msgFlow->field_0x26 = 0;
                // msgFlow->field_0x10 = customInitNode; // TODO: testing adjust for zel_00.bmg
                // // Was defaulting to 0x24 (36) at the moment.
            }
            else if (flowContext == 0 && msgFlow->mFlow >= 0x7000)
            {
                // If we do not find a starting node for an FLI in the 0x7000's
                // (meaning a custom sign), then set to FLW index 0x28 (any msg
                // node with next node idx 0xFFFF is fine) with reserved context
                // value 1 for the "No hints were placed here." fallback. Check
                // that flowContext is 0 to avoid infinite msg loops.
                flwIndex = 0x28;
                rando::gRandomizer->setFlowContext(msgFlow, 1);
            }
        }

        gReturn_setNodeIndex(msgFlow, flwIndex, actrPtr);

        // TODO: what if we use a custom eventNode to set the context? That
        // might make things more difficult / way bigger in data size in the
        // GCI.

        // Let's say we have something like this:
        // - FLI value, currentContext(or any okay), newNodeIndex, newContext

        // We definitely need to patch nodeProc's switch to read the correct
        // value in the case that we are remapping the node

        // Actually, let's assume we cannot remap a node's type. We can only
        // remap its data as used by the branch, msg, and event procs.

        // TODO: first thing to do is assume we have a specific context value.
        // Need to remap the text for the "What is it, Link?" and the
        // "Transform/Talk To Midna" to say "Testing" and "Hints/Change time of
        // day".
    }

    KEEP_FUNC int32_t handle_setSelectMsg(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                          void* bodyMsgFlowNode,
                                          void* optionsMsgFlowNode,
                                          libtp::tp::f_op_actor::fopAc_ac_c* actrPtr)
    {
        uint8_t bodyFlowNodeCopy[8];
        uint8_t optionsFlowNodeCopy[8];

        if (msgFlow != nullptr)
        {
            memcpy(&bodyFlowNodeCopy, bodyMsgFlowNode, 8);
            memcpy(&optionsFlowNodeCopy, optionsMsgFlowNode, 8);

            // TODO: we should manually get the flwIndexes in question by using the same
            // offset 0x10 on the msgFlow* which is the bodyMsgFlowNode index.

            // We don't need to do anything about fetching the nodes since we
            // literally already have them.

            // Then we read the u16 at offset 0x4 on the bodyNode to get the
            // flwIndex for the options node.

            // TODO: can uncomment these and pass as a param if needed.
            // uint16_t bodyFlwIndex = msgFlow->field_0x10; // current FLW index
            // uint16_t optionsFlwIndex = reinterpret_cast<const uint16_t*>(bodyMsgFlowNode)[2];

            // const uint16_t customINFIndex =
            // rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr()->getCustomINFIndex(msgFlow);
            // if (customINFIndex != 0xFFFF)
            // {
            //     uint16_t* u16Arr = reinterpret_cast<uint16_t*>(flowNodeCopy);
            //     u16Arr[1] = customINFIndex;
            //     // rando::gRandomizer->setLatestCustomINFIndex(customINFIndex);
            // }

            const rando::BMG0Section* bmgSection = rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr();

            const uint16_t bodyCustomInfIdx = bmgSection->getCustomINFIndex(msgFlow, false);
            if (bodyCustomInfIdx != 0xFFFF)
            {
                uint16_t* u16Arr = reinterpret_cast<uint16_t*>(bodyFlowNodeCopy);
                u16Arr[1] = bodyCustomInfIdx;
            }

            const uint16_t optionsCustomInfIdx = bmgSection->getCustomINFIndex(msgFlow, true);
            if (optionsCustomInfIdx != 0xFFFF)
            {
                uint16_t* u16Arr = reinterpret_cast<uint16_t*>(optionsFlowNodeCopy);
                u16Arr[1] = optionsCustomInfIdx;
            }
        }

        return gReturn_setSelectMsg(msgFlow, bodyFlowNodeCopy, optionsFlowNodeCopy, actrPtr);
    }

    KEEP_FUNC int32_t handle_setNormalMsg(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                          void* flowNode,
                                          libtp::tp::f_op_actor::fopAc_ac_c* actrPtr)
    {
        using mod::rando::EntryInfo;

        // We need to be able to map a hint sign to an FLI value.

        // We need to be able to map a FLI value to a start node. (and this
        // includes 0x5670, 0x5671, 0x5672, etc.). 0xbb8 however does NOT remap
        // the start value. So maybe we include a byte in the data that says
        // whether or not to remap the start node.

        // Function call: getCustomInitNodeIndex(FLI value); returns u16, -1 if nothing found.
        // Internally each entry has a bool which defines if it remaps the initNode.

        // We need to be able to map a FLI value + a FLW index => 0x136d

        uint8_t flowNodeCopy[8];
        memcpy(&flowNodeCopy, flowNode, 8);

        // uint8_t i = flowNodeCopy[1] + 1;
        // uint32_t* a = game_patch::customEventFunctions[i];
        // if (i > 0)
        // {
        //     flowNodeCopy[1] = a[2] & 0xFF;
        //     // a(NULL, NULL, NULL);
        // }

        // uint16_t customINFIndex = rando::gRandomizer->getCustomINFIndex(msgFlow);
        const uint16_t customINFIndex =
            rando::gRandomizer->getSeedPtr()->getBMG0SectionPtr()->getCustomINFIndex(msgFlow, false);
        if (customINFIndex != 0xFFFF)
        {
            uint16_t* u16Arr = reinterpret_cast<uint16_t*>(flowNodeCopy);
            u16Arr[1] = customINFIndex;
            // rando::gRandomizer->setLatestCustomINFIndex(customINFIndex);
        }

        // uint16_t customInitNode = rando::gRandomizer->getSeedPtr()->getCustomInitNodeIndex(flowNode);
        // if (customInitNode != 0xFFFF)
        // {

        // }

        // const EntryInfo* msgRemapInfoPtr = rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getMsgRemapInfoPtr();
        // const uint32_t numEntries = msgRemapInfoPtr->getNumEntries();
        // const uint32_t gciOffset = msgRemapInfoPtr->getDataOffset();

        // if (msgFlow->mFlow == 0xbb8)
        // {
        //     uint16_t* u16Arr = reinterpret_cast<uint16_t*>(flowNodeCopy);
        //     u16Arr[1] = 0x136d;

        //     // TODO: currently this shows "No hints were placed here" (or the
        //     // hints depending on the room) for each text box. Shows 5 text
        //     // boxes since we are replacing the start position to the first of
        //     // the 5 msg nodes.

        //     // TODO: the exact start node is dynamic based on how many text
        //     // boxes we need (9 hint would require 3 total FLW msg nodes for
        //     // example).

        //     // TODO: need to take input which says "map this FLI + this msgNode
        //     // from offset 0x10 of r3 this and use this replacement 0x136d value
        //     // instead.
        // }

        // the first param (the flowNode) is a pointer to the FLWMsgEntry.

        // We can use our custom FLI u16 value (0x7fff or others) + the
        // flowNode's index (which is available on the dMsgFlow object) in order
        // to generate a custom MID1 index (instead of using a hard-coded
        // 0x1360). We can read what this should translate to by using a lookup
        // table in the seed data. Can be an array of [u16 FLI val (ex:0x5670,
        // 0x5671, 0x5672, 0x5673, 0x5680, 0x5681, 0x5682, 0x5683, 0xbb8(?)),
        // u16 FLW index we are abusing, (ex: 36 for "The yellow arrow..."), u16
        // new val to use as offset 0x2 in our custom flowNode (ex: 0x1369);
        // this is the INF1 entry index!!! (not the 0x1360 val which we might
        // not need in general?), u16 padding]

        // TODO: in the event that we have hints on Midna, we need to patch the
        // Midna code in case 9 of eventNodeProc to always go to 0xbb8
        // regardless of what is returned based on the stayNo. If Midna does not
        // have hints, then we should not apply this change there so we do not
        // impact the vanilla text. So we would use the `flagsBitfieldArray` in
        // the CPP code probably to have a bit for this. And we would also have
        // a patch to the FLI section which updates 0xbb8 to instead point
        // toward the correct new FLW entry index (ex: "The yellow arrow...").
        // With that in place, we do not have to do anything for Midna in doFlow
        // since it will be functioning normally. We will however need to still
        // handle flow_0xbb8+flowNodeIndex to map to a different 0x1369 value.

        // if (msgFlow->mFlow == 0x7FFF) // Check if it equals our custom flow value
        // {
        //     // Maybe the not super hacky way of doing this is to pass a local
        //     // flowNode to the function when we need it to lead to a custom INF1
        //     // index. We copy the bytes from the param flowNode into our local
        //     // flowNode, then overwrite the u16 at offset 0x2 for the INF1 index.

        //     // For Midna, maybe what we should do to avoid crazy CPP changes is
        //     // in the event that there are hints on Midna, we tell it if that
        //     // the 0xbb8 FLI starts not on FLW entry 143 (0xDF), but on 36, 37,
        //     // 38, 39, or 40 which are in subgraph 19.

        //     // Then if the mFlow is 0xbb8 and the FLW entry index is between 36
        //     // and 40, we instead make a temp FLW entry and give it a custom
        //     // INF1 index.

        //     // We also need to be able to remap the CoO fairy one given specific
        //     // FLI values. For example, if the mFlow is 0x7890 and the current

        //     // Scratch that. Instead, what if instead we make the u16 flowId
        //     // that we put on the sign indicate also which node to start on? For
        //     // example, 0x7802 would match our special value of starting with
        //     // 0x78, and then the 0x02 (0x01/0x03, etc.) would tell us which FLW
        //     // entry to start on. When we are spawning in the sign actor, it
        //     // should be able to say "I am this sign, and I see that I am
        //     // supposed to start here, so I will generate my FLI as such". In
        //     // fact, we should just store an array of u16s for the FLI values
        //     // for the signs, and each sign should know its index in that array
        //     // to look for, and it should use that value for its param.

        //     // Then once we are in this file, we can

        //     // Set the msg id in the node to that of our specified message.
        //     // const uint32_t msg = libtp::tp::f_op_msg_mng::fopMsgM_messageSet(0x1360, 1000);
        //     const uint32_t msg = libtp::tp::f_op_msg_mng::fopMsgM_messageSet(0x1364, 1000);

        //     msgFlow->mMsg = msg;
        //     return 1;
        // }

        // return gReturn_setNormalMsg(msgFlow, flowNode, actrPtr);
        return gReturn_setNormalMsg(msgFlow, flowNodeCopy, actrPtr);
    }

    KEEP_FUNC int32_t handle_messageNodeProc(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                             libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_1,
                                             libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_2)
    {
        // rando::gRandomizer->setLatestCustomINFIndex(msgFlow->mFlow);

        int32_t ret = gReturn_messageNodeProc(msgFlow, actrPtr_1, actrPtr_2);

        // rando::gRandomizer->setLatestCustomINFIndex(0xFFFF);

        return ret;
    }

    KEEP_FUNC int32_t handle_branchNodeProc(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                            libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_1,
                                            libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_2)
    {
        uint8_t mutNodeCopy[8];

        uint8_t* flowNodeTable = reinterpret_cast<uint8_t*>(msgFlow->mFlowNodeTBL);
        uint16_t flowNodeIdx = msgFlow->field_0x10;
        uint8_t* nodeSrc = flowNodeTable + flowNodeIdx * 8;

        memcpy(mutNodeCopy, nodeSrc, 8);

        // Store pointer to copy on randomizer
        rando::gRandomizer->setMutFlowNodePtr(mutNodeCopy);

        int32_t ret = gReturn_branchNodeProc(msgFlow, actrPtr_1, actrPtr_2);

        // Set ptr on randomizer to be nullptr
        rando::gRandomizer->setMutFlowNodePtr(nullptr);

        return ret;
    }

    KEEP_FUNC int32_t handle_eventNodeProc(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                           libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_1,
                                           libtp::tp::f_op_actor::fopAc_ac_c* actrPtr_2)
    {
        uint8_t mutNodeCopy[8];

        uint8_t* flowNodeTable = reinterpret_cast<uint8_t*>(msgFlow->mFlowNodeTBL);
        uint16_t flowNodeIdx = msgFlow->field_0x10;
        uint8_t* nodeSrc = flowNodeTable + flowNodeIdx * 8;

        memcpy(mutNodeCopy, nodeSrc, 8);

        // Store pointer to copy on randomizer
        rando::gRandomizer->setMutFlowNodePtr(mutNodeCopy);

        int32_t ret = gReturn_eventNodeProc(msgFlow, actrPtr_1, actrPtr_2);

        // Set ptr on randomizer to be nullptr
        rando::gRandomizer->setMutFlowNodePtr(nullptr);

        return ret;
    }

    KEEP_FUNC void handle_endFlowGroup()
    {
        // Force reset flowContext info when flow group ends.
        rando::gRandomizer->checkResetFlowContext(nullptr);

        // Call original function
        gReturn_endFlowGroup();
    }

    KEEP_FUNC void handle_talkEnd(void* eventPtr)
    {
        // Call original function
        gReturn_talkEnd(eventPtr);

        // We handle a pending ToD change from talking to Midna once this
        // function has run after the conversation ends so that the Midna actor
        // has been updated to know the conversation has ended. This avoids
        // having the Midna conversation pop back up after selecting "Change
        // ToD" when talking to Midna as wolf.
        if (rando::gRandomizer->getHasPendingTodChange())
        {
            rando::gRandomizer->setHasPendingTodChange(false);
            events::handleTimeOfDayChange();
        }
    }

    KEEP_FUNC void handle_jmessage_tSequenceProcessor__do_begin(void* seqProcessor, const void* unk2, const char* text)
    {
        // Call the original function immediately as it sets necessary values needed later on.
        gReturn_jmessage_tSequenceProcessor__do_begin(seqProcessor, unk2, text);

        *reinterpret_cast<uint8_t*>(reinterpret_cast<uint32_t>(seqProcessor) + 0xB2) = 0x1;
    }

    KEEP_FUNC bool handle_jmessage_tSequenceProcessor__do_tag(void* seqProcessor,
                                                              uint32_t unk2,
                                                              const void* currentText,
                                                              uint32_t unk4)
    {
        // Call the original function immediately as it sets necessary values needed later on.
        const bool result = gReturn_jmessage_tSequenceProcessor__do_tag(seqProcessor, unk2, currentText, unk4);

        *reinterpret_cast<uint8_t*>(reinterpret_cast<uint32_t>(seqProcessor) + 0xB2) = 0x1;
#ifdef TP_JP
        constexpr uint32_t offset = 0x5A6;
#else
        constexpr uint32_t offset = 0x5D6;
#endif
        uint32_t tReferencePtr = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(seqProcessor) + 0x4);
        *reinterpret_cast<int16_t*>(tReferencePtr + offset) = 0;

        return result;
    }

    KEEP_FUNC bool handle_isDungeonItem(libtp::tp::d_save::dSv_memBit_c* membitPtr, const int32_t memBit)
    {
        return events::proc_isDungeonItem(membitPtr, memBit);
    }

    KEEP_FUNC void handle_onDungeonItem(libtp::tp::d_save::dSv_memBit_c* membitPtr, const int32_t memBit)
    {
        return events::proc_onDungeonItem(membitPtr, memBit);
    }

    KEEP_FUNC bool handle_daNpcT_chkEvtBit(int16_t flag)
    {
        const auto stagesPtr = &libtp::data::stage::allStages[0];
        switch (flag)
        {
            case 0x153: // Checking if the player has Ending Blow
            {
                if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Hidden_Skill]))
                {
                    return true;
                }
                break;
            }

            case 0x40: // Checking if the player has completed Goron Mines
            {
                if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Kakariko_Village_Interiors]))
                {
                    return true; // Return true so Barnes will sell bombs no matter what
                }
                break;
            }
            default:
            {
                break;
            }
        }
        return gReturn_daNpcT_chkEvtBit(flag);
    }

    KEEP_FUNC bool handle_isEventBit(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag)
    {
        using namespace libtp::tp::d_a_alink;
        using namespace libtp::tp::d_com_inf_game;
        using namespace libtp::data::stage;
        using namespace libtp::data::flags;

        const auto stagesPtr = &allStages[0];

        switch (flag)
        {
            case ENDING_BLOW_UNLOCKED: // Checking for ending blow.
            {
                if (checkStageName(stagesPtr[StageIDs::Hidden_Skill]))
                {
                    return true; // If we don't have the flag, the game sends us to Faron by default. Which we don't
                                 // want.
                }
                break;
            }

            case GREAT_SPIN_UNLOCKED:
            {
                if (checkStageName(stagesPtr[StageIDs::Hidden_Skill]))
                {
                    return false; // Tell the game we don't have great spin to
                                  // not softlock in hidden skill training.
                }
                break;
            }

            case BO_TALKED_TO_YOU_AFTER_OPENING_IRON_BOOTS_CHEST: // Has Bo been defeated in wrestling
            {
                if (checkStageName(stagesPtr[StageIDs::Ordon_Village_Interiors]))
                {
                    if (dComIfGs_isEventBit(
                            libtp::data::flags::HEARD_BO_TEXT_AFTER_SUMO_FIGHT)) // Talked to Bo after chest is spawned
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                break;
            }

            case GAVE_ILIA_HER_CHARM:    // Gave Ilia the charm
            case CITY_OOCCOO_CS_WATCHED: // CiTS Intro CS watched
            {
                if (checkStageName(stagesPtr[StageIDs::Hidden_Village]))
                {
                    if (!dComIfGs_isEventBit(libtp::data::flags::GOT_ILIAS_CHARM))
                    {
                        return false; // If we haven't gotten the item from Impaz then we need to return false or it
                                      // will break her dialogue.
                    }
                }
                break;
            }

            case GORON_MINES_CLEARED: // Goron Mines Story Flag
            {
                if (checkStageName(stagesPtr[StageIDs::Goron_Mines]) ||
                    checkStageName(stagesPtr[StageIDs::Death_Mountain_Interiors]))
                {
                    return false; // The gorons will not act properly if the flag is set.
                }
                break;
            }

            case ZORA_ESCORT_CLEARED: // Escort Completed
            {
                if (checkStageName(stagesPtr[StageIDs::Castle_Town]))
                {
                    return true; // If flag isn't set, the player will be thrown into escort when they open the
                                 // door.
                }
                else if (libtp::tools::playerIsInRoomStage(
                             0,
                             stagesPtr[StageIDs::Kakariko_Village_Interiors])) // Return true to prevent Renado/Illia
                                                                               // crash after ToT
                {
                    return true;
                }
                break;
            }

            case SNOWPEAK_RUINS_CLEARED: // Snowpeak Ruins Story flag
            {
                if (checkStageName(stagesPtr[StageIDs::Kakariko_Graveyard]))
                {
                    return false; // If the flag is set, Ralis will no longer spawn in the graveyard.
                }
                break;
            }

            case CITY_IN_THE_SKY_CLEARED: // City in the Sky Story flag
            {
                if (checkStageName(stagesPtr[StageIDs::Mirror_Chamber]))
                {
                    if (!libtp::tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::FIXED_THE_MIRROR_OF_TWILIGHT))
                    {
                        if (rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getPalaceRequirements() != 3)
                        {
                            return false;
                        }
                    }
                }
                break;
            }
            case HOWLED_AT_SNOWPEAK_STONE:
            {
                if (checkStageName(stagesPtr[StageIDs::Snowpeak]))
                {
                    return false; // Return false so the player can howl at the stone multiple times to remove map glitch
                }
                break;
            }

            case WATCHED_CUTSCENE_AFTER_GOATS_2:
            {
                if (libtp::tools::playerIsInRoomStage(1, stagesPtr[StageIDs::Ordon_Village_Interiors]))
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(SERAS_CAT_RETURNED_TO_SHOP))
                    {
                        return false; // Return false so Sera will give the milk item to the player once they help the cat.
                    }
                    else
                    {
                        return true; // Return true so the player can always access the shop, even if the cat has not
                                     // returned.
                    }
                }
                break;
            }

            case FIXED_THE_MIRROR_OF_TWILIGHT:
            {
                if (checkStageName(stagesPtr[StageIDs::Palace_of_Twilight]))
                {
                    return true; // If the flag is not set, the player cannot leave PoT from the inside.
                }
                break;
            }

            default:
            {
                break;
            }
        }
        return gReturn_isEventBit(eventPtr, flag);
    }

    KEEP_FUNC void handle_onEventBit(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag)
    {
        using namespace libtp::tp::d_a_alink;
        using namespace libtp::tp::d_com_inf_game;
        using namespace libtp::data::stage;
        using namespace libtp::data::flags;

        libtp::tp::d_save::dSv_save_c* saveFilePtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file;
        if (eventPtr == &saveFilePtr->mEvent)
        {
            libtp::tp::d_save::dSv_player_c* playerPtr = &saveFilePtr->player;
            libtp::tp::d_save::dSv_player_status_a_c* playerStatusAPtr = &playerPtr->player_status_a;
            libtp::tp::d_save::dSv_player_status_b_c* playerStatusBPtr = &playerPtr->player_status_b;

            const uint32_t darkClearLevelFlag = playerStatusBPtr->dark_clear_level_flag;

            switch (flag)
            {
                // Case block for Wolf -> Human crash patches/bug fixes. Some cutscenes/events either crash or act weird if Link
                // is Human but needs to be Wolf and the game no longer attempts to auto-transform Link once the Shadow Crystal
                // has been obtained.
                case ENTERED_ORDON_SPRING_DAY_3:
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(TRANSFORMING_UNLOCKED))
                    {
                        // Set player to Human as the game will not do so if Shadow Crystal has been obtained.
                        playerStatusAPtr->currentForm = 0;
                    }
                    break;
                }

                // Case block for Human -> Wolf crash patches/bug fixes. Some cutscenes/events either crash or act weird if Link
                // is Wolf but needs to be human and the game no longer attempts to auto-transform Link once the Shadow Crystal
                // has been obtained.
                case WATCHED_CUTSCENE_AFTER_BEING_CAPTURED_IN_FARON_TWILIGHT:
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(TRANSFORMING_UNLOCKED))
                    {
                        // Set player to Wolf as the game will not do so if Shadow Crystal has been obtained.
                        playerStatusAPtr->currentForm = 1;
                    }
                    break;
                }

                case MIDNAS_DESPERATE_HOUR_COMPLETED: // MDH Completed
                {
                    playerStatusBPtr->dark_clear_level_flag |= 0x8;
                    break;
                }

                case CLEARED_FARON_TWILIGHT: // Cleared Faron Twilight
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(MIDNAS_DESPERATE_HOUR_COMPLETED))
                    {
                        if (darkClearLevelFlag == 0x6)
                        {
                            playerStatusBPtr->transform_level_flag = 0x8; // Set the flag for the last transformed twilight.
                                                                          // Also puts Midna on the player's back

                            playerStatusBPtr->dark_clear_level_flag |= 0x8;
                        }
                    }
                    break;
                }

                case CLEARED_ELDIN_TWILIGHT: // Cleared Eldin Twilight
                {
                    events::setSaveFileEventFlag(MAP_WARPING_UNLOCKED); // in glitched Logic, you can skip the gorge bridge.
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(MIDNAS_DESPERATE_HOUR_COMPLETED))
                    {
                        if (darkClearLevelFlag == 0x7)
                        {
                            playerStatusBPtr->transform_level_flag |= 0x8; // Set the flag for the last transformed twilight.
                                                                           // Also puts Midna on the player's back

                            playerStatusBPtr->dark_clear_level_flag |= 0x8;
                        }
                    }

                    break;
                }

                case CLEARED_LANAYRU_TWILIGHT: // Cleared Lanayru Twilight
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(MIDNAS_DESPERATE_HOUR_COMPLETED))
                    {
                        if (darkClearLevelFlag == 0x7) // All twilights completed
                        {
                            playerStatusBPtr->transform_level_flag |= 0x8; // Set the flag for the last transformed twilight.
                                                                           // Also puts Midna on the player's back

                            playerStatusBPtr->dark_clear_level_flag |= 0x8;
                        }
                    }

                    break;
                }

                case PALACE_OF_TWILIGHT_CLEARED:
                {
                    if (rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getCastleRequirements() ==
                        rando::CastleEntryRequirements::HC_Vanilla) // Vanilla
                    {
                        events::setSaveFileEventFlag(libtp::data::flags::BARRIER_GONE);
                        return gReturn_onEventBit(eventPtr, flag); // set PoT story flag
                    }
                    break;
                }

                case REMOVE_SWORD_SHIELD_FROM_WOLF_BACK:
                {
                    if (!libtp::tp::d_com_inf_game::dComIfGs_isEventBit(CLEARED_FARON_TWILIGHT))
                    {
                        playerStatusBPtr->transform_level_flag |= 0x1; // Set the last transformed twilight to include Faron
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }
        }

        return gReturn_onEventBit(eventPtr, flag);
    }

    KEEP_FUNC bool handle_isSwitch_dSv_memBit(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag)
    {
        const auto stagesPtr = &libtp::data::stage::allStages[0];

        if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Kakariko_Graveyard]))
        {
            if (flag == 0x66) // Check for escort completed flag
            {
                if (!libtp::tp::d_com_inf_game::dComIfGs_isEventBit(
                        libtp::data::flags::GOT_ZORA_ARMOR_FROM_RUTELA)) // return false if we haven't gotten the item
                                                                         // from Rutella.
                {
                    return false;
                }
            }
        }
        else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Hidden_Village_Interiors]))
        {
            if (flag == 0x61) // Is Impaz in her house
            {
                return true;
            }
        }

        else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Ordon_Village]))
        {
            if ((flag == 0x21) &&
                !libtp::tp::d_kankyo::dKy_daynight_check()) // Midna jumps to shield house are active and it is daytime
            {
                return false;
            }
        }
        return gReturn_isSwitch_dSv_memBit(memoryBit, flag);
    }

    KEEP_FUNC void handle_onSwitch_dSv_memBit(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag)
    {
        libtp::tp::d_save::dSv_info_c* savePtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save;

        const auto stagesPtr = &libtp::data::stage::allStages[0];

        if (memoryBit == &savePtr->memory.temp_flags)
        {
            if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Forest_Temple]))
            {
                if (flag == 0x52)
                {
                    // Don't set the flag for all monkeys freed in the lobby of Forest Temple
                    return;
                }
            }
            else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Arbiters_Grounds]))
            {
                if (flag == 0x26) // Poe flame CS trigger
                {
                    // Open the Poe gate
                    libtp::tp::d_save::offSwitch_dSv_memBit(memoryBit, 0x45);

                    return;
                }
            }
            else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Lake_Hylia]))
            {
                if (flag == 0xD) // Lanayru Twilight End CS trigger.
                {
                    if (libtp::tp::d_com_inf_game::dComIfGs_isEventBit(libtp::data::flags::TRANSFORMING_UNLOCKED))
                    {
                        // Set player to Human as the game will not do so if Shadow Crystal has been obtained.
                        savePtr->save_file.player.player_status_a.currentForm = 0;
                    }
                }
            }
            else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Kakariko_Village]))
            {
                if (flag == 0x3E) // Hawkeye is for sell.
                {
                    // Remove the coming soon sign so the hawkeye can be bought.
                    libtp::tp::d_save::offSwitch_dSv_memBit(memoryBit, 0xB);
                }
            }
            else if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Hyrule_Field]))
            {
                if (flag == 0x11) // Destroyed North Eldin rocks barrier
                {
                    // Unlock Eldin Province on map. We do this manually rather than calling `onRegionBit` since that
                    // function would see that the rocks are not yet broken and would skip enabling the region.
                    libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_last_stay_info
                        .player_last_region |= 0x08;
                }
            }
        }

        if (memoryBit == &savePtr->save_file.mSave[6].temp_flags)
        {
            if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Kakariko_Village_Interiors]))
            {
                if (flag == 0x1B) // Repair Castle Town Bridge
                {
                    *reinterpret_cast<uint16_t*>(&savePtr->save_file.mEvent.mEvent[0xF9]) =
                        rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getMaloShopDonationAmount();
                }
            }
        }

        return gReturn_onSwitch_dSv_memBit(memoryBit, flag);
    }

    KEEP_FUNC void handle_onSwitch_dSv_info(libtp::tp::d_save::dSv_info_c* saveInfo, int32_t flag, int32_t roomNo)
    {
        const auto stagesPtr = &libtp::data::stage::allStages[0];

        if (libtp::tp::d_a_alink::checkStageName(stagesPtr[libtp::data::stage::StageIDs::Sacred_Grove]))
        {
            if (flag == 0xEE) // Struck Master Sword in pedestal
            {
                // Custom Flag to remove statue in front of door
                return gReturn_onSwitch_dSv_info(saveInfo, 0x63, roomNo);
            }
        }

        return gReturn_onSwitch_dSv_info(saveInfo, flag, roomNo);
    }

    KEEP_FUNC bool handle_isDarkClearLV(void* playerStatusPtr, int32_t twilightNode)
    {
        if ((twilightNode == 0) && libtp::tools::playerIsInRoomStage(
                                       1,
                                       libtp::data::stage::allStages[libtp::data::stage::StageIDs::Ordon_Village_Interiors]))

        {
            return false; // Return false so Sera will give us the bottle if we have rescued the cat.
        }

        return gReturn_isDarkClearLV(playerStatusPtr, twilightNode);
    }

    KEEP_FUNC void handle_setWarashibeItem(libtp::tp::d_save::dSv_player_item_c* playerItemPtr, uint8_t itemID)
    {
        // If the trade item slot would be emptied (such as after showing the
        // Invoice) and the player already has the HorseCall, update the trade
        // item slot to be the HorseCall instead of removing the slot.
        if (itemID == libtp::data::items::NullItem &&
            libtp::tp::d_com_inf_game::dComIfGs_isItemFirstBit(libtp::data::items::Horse_Call))
        {
            itemID = libtp::data::items::Horse_Call;
        }
        gReturn_setWarashibeItem(playerItemPtr, itemID);
    }

    KEEP_FUNC void handle_onRegionBit(libtp::tp::d_save::dSv_player_field_last_stay_info_c* lastStayInfoPtr, int32_t i_region)
    {
        using namespace libtp::tp;

        // Make barriers between Lanayru/Eldin (North Eldin rocks and CT bridge) and Lanayru/Faron (gate keys)
        // work both ways. Prevents barely walking into other Provinces from giving you full warp access when
        // you otherwise have no way to proceed on foot.
        if (i_region == 3) // Eldin Province
        {
            bool checkNorthEldinRocks = false;

            if (d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Hidden_Village]))
            {
                checkNorthEldinRocks = true;
            }
            else if (d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Hyrule_Field]))
            {
                int32_t currentRoom = libtp::tools::getCurrentRoomNo();
                if (currentRoom == 7) // Outside Hidden Village
                {
                    checkNorthEldinRocks = true;
                }
                else if (currentRoom == 0) // Big Eldin Field room
                {
                    int32_t previousRoom = libtp::tools::getPreviousRoomNo();
                    if (previousRoom == 7)
                    {
                        // Walked to big Eldin Field room from outside HV
                        checkNorthEldinRocks = true;
                    }
                    else if (previousRoom == -1)
                    {
                        // Here we are freshly loading into room 0 rather than walking to it.
                        int16_t startStagePoint = d_com_inf_game::dComIfG_gameInfo.play.mStartStage.mPoint;
                        if (startStagePoint == -1)
                        {
                            // Never unlock Eldin Province when entering the big EF room by voiding/gameOver. Returning
                            // from a void is based on position and not spawn, so any spawnPoint references are either
                            // -1 or, in the case of approaching from the north, outdated and variable. This prevents
                            // different workarounds where you void or die to unlock the region when respawning.
                            return;
                        }
                        else if (startStagePoint == 7 &&
                                 !d_save::isSwitch_dSv_memBit(&d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags, 0x1B))
                        {
                            // If we are entering from the CT side of the bridge, skip unlocking if the bridge
                            // has not yet been repaired.
                            return;
                        }
                    }
                }
            }

            if (checkNorthEldinRocks &&
                !d_save::isSwitch_dSv_memBit(&d_com_inf_game::dComIfG_gameInfo.save.save_file.mSave[6].temp_flags, 0x11))
            {
                // If Eldin/Lanayru barrier rocks are not broken, then return without unlocking Eldin Province.
                // Breaking the rocks will immediately unlock Eldin Province (handled by other code).
                return;
            }
        }
        else if (i_region == 2) // Faron Province
        {
            if (d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Hyrule_Field]) &&
                libtp::tools::getCurrentRoomNo() == 15)
            {
                // The Lanayru/Faron gate which requires gate keys is in the middle of this room. Prevent this
                // room from unlocking Faron province so you do not get free access from the Lanayru side.
                return;
            }
        }

        // Otherwise call the function normally.
        gReturn_onRegionBit(lastStayInfoPtr, i_region);
    }

    KEEP_FUNC void handle_collect_save_open_init(uint8_t param_1)
    {
        game_patch::_07_checkPlayerStageReturn();
        return gReturn_collect_save_open_init(param_1);
    }

    KEEP_FUNC void handle_resetMiniGameItem(libtp::tp::d_meter2_info::G_Meter2_Info* gMeter2InfoPtr, bool minigameFlag)
    {
        using namespace libtp::tp;

        // If we are in Iza's hut and we have the bow, we want to update the save file bow item stored in g_meter2_info just in
        // case the player started the minigame without it and somehow broke out of the minigame.
        if (d_a_alink::checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Zoras_River]))
        {
            if (events::haveItem(libtp::data::items::Heros_Bow))
            {
                gMeter2InfoPtr->mSaveBowItem = libtp::data::items::Heros_Bow;

                // We want to keep track of the number of arrows we have as well because the game
                // will set our arrow count to this variable upon save warp if the minigame is not completed.
                gMeter2InfoPtr->mSaveArrowNum =
                    d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_item_record.bow_ammo;
            }
        }

        // Run the original function as we want to clear the other minigame flags
        gReturn_resetMiniGameItem(gMeter2InfoPtr, minigameFlag);
    }

    KEEP_FUNC bool handle_checkBootsMoveAnime(libtp::tp::d_a_alink::daAlink* d_a_alink, int32_t param_1)
    {
        if (libtp::tp::d_a_alink::ironBootsVars.heavyStateSpeed == 1.f)
        {
            return false;
        }
        return gReturn_checkBootsMoveAnime(d_a_alink, param_1);
    }

    KEEP_FUNC void handle_setGetItemFace(libtp::tp::d_a_alink::daAlink* linkMapPtr, uint16_t itemID)
    {
        using namespace libtp::data::items;
        using namespace rando::customItems;

        switch (itemID)
        {
            // Only the first foolish item should need to be checked, but check all to be safe
            case Foolish_Item:
            {
                itemID = Ordon_Pumpkin;
                break;
            }

            case Wooden_Sword:
            case Ordon_Sword:
            case Ordon_Shield:
            case Master_Sword:
            case Master_Sword_Light:
            case Shadow_Crystal:
            {
                itemID = Clawshot;
                break;
            }

            default:
            {
                break;
            }
        }
        return gReturn_setGetItemFace(linkMapPtr, itemID);
    }

    KEEP_FUNC void handle_setWolfLockDomeModel(libtp::tp::d_a_alink::daAlink* linkActrPtr)
    {
        // Call the original function immediately, as certain values need to be set in the Link Actor struct.
        gReturn_setWolfLockDomeModel(linkActrPtr);

        rando::gRandomizer->replaceWolfLockDomeColor(linkActrPtr);
        return;
    }

    KEEP_FUNC bool handle_procFrontRollCrashInit(libtp::tp::d_a_alink::daAlink* linkActrPtr)
    {
        handleBonkDamage();
        return gReturn_procFrontRollCrashInit(linkActrPtr);
    }

    KEEP_FUNC bool handle_procWolfDashReverseInit(libtp::tp::d_a_alink::daAlink* linkActrPtr, bool param_1)
    {
        handleBonkDamage();
        return gReturn_procWolfDashReverseInit(linkActrPtr, param_1);
    }

    KEEP_FUNC bool handle_procWolfAttackReverseInit(libtp::tp::d_a_alink::daAlink* linkActrPtr)
    {
        handleBonkDamage();
        return gReturn_procWolfAttackReverseInit(linkActrPtr);
    }

    KEEP_FUNC libtp::tp::f_op_actor::fopAc_ac_c* handle_searchBouDoor(libtp::tp::f_op_actor::fopAc_ac_c* actrPtr)
    {
        if (rando::gRandomizer->getSeedPtr()->getStageIDX() == libtp::data::stage::StageIDs::Ordon_Village)
        {
            return nullptr;
        }

        return gReturn_searchBouDoor(actrPtr);
    }

    KEEP_FUNC float handle_damageMagnification(libtp::tp::d_a_alink::daAlink* linkActrPtr, int32_t param_1, int32_t param_2)
    {
        // Call the original function immediately, as we only need the current damage magnification value.
        float ret = gReturn_damageMagnification(linkActrPtr, param_1, param_2);

        ret *= intToFloat(static_cast<int32_t>(rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getDamageMagnification()));
        return ret;
    }

    KEEP_FUNC bool handle_checkCastleTownUseItem(uint16_t item_id)
    {
        using namespace libtp::data::items;
        using namespace libtp::tp::d_a_alink;
        using namespace libtp::tp::d_com_inf_game;

        const int32_t roomID = libtp::tools::getCurrentRoomNo();

        // Check if the player is in past area
        if (checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Sacred_Grove]) && roomID == 0x2)
        {
            if (item_id == Ooccoo_Jr)
            {
                return false; // remove the ability to use ooccoo in past area
            }
        }
        else if (checkStageName(libtp::data::stage::allStages[libtp::data::stage::StageIDs::Temple_of_Time]) && roomID == 0x0)
        {
            switch (item_id)
            {
                case Ooccoo_Jr:
                case Ooccoo_FT:
                case Ooccoo_Dungeon:
                {
                    // Check if the player hasn't taken ooccoo at tot entrance
                    if (!libtp::tp::d_save::isDungeonItem(&libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags,
                                                          0x6))
                    {
                        return false;
                    }
                    break;
                }
            }
        }

        return gReturn_checkCastleTownUseItem(item_id);
    }

    KEEP_FUNC void handle_loadSeWave(void* z2SceneMgr, uint32_t waveID)
    {
        rando::gRandomizer->setZ2ScenePtr(z2SceneMgr);
        return gReturn_loadSeWave(z2SceneMgr, waveID);
    }

    KEEP_FUNC bool handle_checkBgmIDPlaying(libtp::z2audiolib::z2seqmgr::Z2SeqMgr* seqMgr, uint32_t sfx_id)
    {
        // Call original function immediately as it sets necessary values.
        const bool ret = gReturn_checkBgmIDPlaying(seqMgr, sfx_id);

        // Game Over sfx
        if (sfx_id == 0x01000013)
        {
            return false;
        }

        return ret;
    }

    KEEP_FUNC void handle_dispWait_init(libtp::tp::d_gameover::dGameOver* ptr)
    {
        // Set the timer
        ptr->mTimer = 0;
        return gReturn_dispWait_init(ptr);
    }

    KEEP_FUNC int32_t handle_seq_decide_yes(libtp::tp::d_shop_system::dShopSystem* shopPtr,
                                            libtp::tp::f_op_actor::fopAc_ac_c* actor,
                                            void* msgFlow)
    {
        using namespace libtp::data::stage;

        const auto stagesPtr = &allStages[0];
        if (libtp::tools::playerIsInRoomStage(3, stagesPtr[StageIDs::Kakariko_Village_Interiors]))
        {
            // We want the shop item to have its flag updated no matter what in kak malo mart
            libtp::tp::d_shop_system::setSoldOutFlag(shopPtr);
        }

        return gReturn_seq_decide_yes(shopPtr, actor, msgFlow);
    }

    KEEP_FUNC int32_t handle_procCoGetItemInit(libtp::tp::d_a_alink::daAlink* linkActrPtr)
    {
        // If we are giving a custom item, we want to set mParam0 to 0x100 so that instead of trying to search for an item actor
        // that doesnt exist we want the game to create one using the item id in mGtItm.
        if (rando::gRandomizer->getGiveItemToPlayerStatus() == rando::EventItemStatus::ITEM_IN_QUEUE)
        {
            libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mPlayer->mDemo.mParam0 = 0x100;
        }

        return gReturn_procCoGetItemInit(linkActrPtr);
    }

    KEEP_FUNC void* handle_dScnLogo_c_dt(void* dScnLogo_c, int16_t bFreeThis)
    {
        // Call the original function immediately, as certain values need to be set first
        void* ret = gReturn_dScnLogo_c_dt(dScnLogo_c, bFreeThis);

        // Initialize m_BgWindow since mMain2DArchive should now be set
        rando::Randomizer* randoPtr = rando::gRandomizer;
        if (!randoPtr->getBgWindowPtr())
        {
            void* main2DArchive = libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mMain2DArchive;
            if (main2DArchive)
            {
                // Get the image to use for the background window
                void* bg = libtp::tp::JKRArchive::JKRArchive_getResource2(main2DArchive,
                                                                          0x54494D47, // TIMG
                                                                          "tt_block_grade.bti");

                if (bg)
                {
                    randoPtr->setBgWindowPtr(new libtp::tp::J2DPicture::J2DPicture(bg));
                }
            }
        }

        return ret;
    }

    KEEP_FUNC void handleFoolishItem(rando::Randomizer* randoPtr)
    {
        using namespace libtp::z2audiolib;
        using namespace libtp::z2audiolib::z2scenemgr;
        using namespace libtp::tp;

        rando::customItems::FoolishItems* foolishItemsPtr = randoPtr->getFoolishItemsPtr();
        const uint8_t currentDamageMultiplier = randoPtr->getSeedPtr()->getHeaderPtr()->getDamageMagnification();

        // Get the trigger count
        uint32_t count = foolishItemsPtr->getTriggerCount();
        if (count == 0)
        {
            return;
        }

        if (!events::checkFoolItemFreeze())
        {
            return;
        }

        // Failsafe: Make sure the count does not somehow exceed 100
        if (count > 100)
        {
            count = 100;
        }

        // Reset trigger counter to 0
        foolishItemsPtr->resetTriggerCount();

        /* Store the currently loaded sound wave to local variables as we will need to load them back later.
         * We use this method because if we just loaded the sound waves every time the item was gotten, we'd
         * eventually run out of memory so it is safer to unload everything and load it back in. */
        z2scenemgr::Z2SceneMgr* sceneMgrPtr = &z2audiomgr::g_mDoAud_zelAudio.mSceneMgr;
        const uint32_t seWave1 = sceneMgrPtr->SeWaveToErase_1;
        const uint32_t seWave2 = sceneMgrPtr->SeWaveToErase_2;

        void* scenePtr = randoPtr->getZ2ScenePtr();
        eraseSeWave(scenePtr, seWave1);
        eraseSeWave(scenePtr, seWave2);
        loadSeWave(scenePtr, 0x46);
        m_Do_Audio::mDoAud_seStartLevel(0x10040, nullptr, 0, 0);
        loadSeWave(scenePtr, seWave1);
        loadSeWave(scenePtr, seWave2);

        d_com_inf_game::dComIfG_inf_c* gameInfoPtr = &d_com_inf_game::dComIfG_gameInfo;
        libtp::tp::d_save::dSv_player_status_a_c* playerStatusPtr = &gameInfoPtr->save.save_file.player.player_status_a;
        d_a_alink::daAlink* linkMapPtr = gameInfoPtr->play.mPlayer;
        int32_t newHealthValue;

        if (playerStatusPtr->currentForm == 1)
        {
            newHealthValue = playerStatusPtr->currentHealth - ((2 * count) * currentDamageMultiplier);
            d_a_alink::procWolfDamageInit(linkMapPtr, nullptr);
        }
        else
        {
            newHealthValue = playerStatusPtr->currentHealth - (count * currentDamageMultiplier);
            d_a_alink::procDamageInit(linkMapPtr, nullptr, 0);
        }

        // Make sure an underflow doesn't occur
        if (newHealthValue < 0)
        {
            newHealthValue = 0;
        }

        playerStatusPtr->currentHealth = static_cast<uint16_t>(newHealthValue);
    }

    KEEP_FUNC void handleBonkDamage()
    {
        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();
        if (!seedPtr->bonksDoDamage())
        {
            return;
        }

        libtp::tp::d_save::dSv_player_status_a_c* playerStatusPtr =
            &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a;

        const uint8_t currentDamageMultiplier = seedPtr->getHeaderPtr()->getDamageMagnification();
        int32_t newHealthValue;

        if (playerStatusPtr->currentForm == 1)
        {
            newHealthValue = playerStatusPtr->currentHealth - (2 * currentDamageMultiplier);
        }
        else
        {
            // Damage multiplier is 1 by default
            newHealthValue = playerStatusPtr->currentHealth - currentDamageMultiplier;
        }

        // Make sure an underflow doesn't occur
        if (newHealthValue < 0)
        {
            newHealthValue = 0;
        }

        playerStatusPtr->currentHealth = static_cast<uint16_t>(newHealthValue);
    }

    KEEP_FUNC libtp::tp::d_resource::dRes_info_c* handle_getResInfo(const char* arcName,
                                                                    libtp::tp::d_resource::dRes_info_c* objectInfo,
                                                                    int32_t size)
    {
        libtp::tp::d_resource::dRes_info_c* resourcePtr = gReturn_getResInfo(arcName, objectInfo, size);

        // Make sure the randomizer is enabled, as otherwise some arcs seem to get modifed that shouldn't. One example of this
        // causing problems is when starting a file, and once you have control of Link, reset to go back to the title screen.
        // Doing so will cause the game to crash if this code runs when the randomizer is disabled.
        rando::Randomizer* randoPtr = rando::gRandomizer;
        if (randoPtr->randomizerIsEnabled() && resourcePtr)
        {
            randoPtr->overrideObjectARC(resourcePtr, arcName);
        }

        return resourcePtr;
    }

    KEEP_FUNC void handle_dMenuOption__tv_open1_move(void* thisPtr)
    {
        using namespace libtp::data;
        using namespace libtp::tp;

        // Clear the lastMode value in case the player was previously riding Epona or swimming.
        d_com_inf_game::dComIfG_inf_c* gameInfoPtr = &d_com_inf_game::dComIfG_gameInfo;
        libtp::tp::d_save::dSv_info_c* savePtr = &gameInfoPtr->save;
        savePtr->mRestart.mLastMode = 0;

        // If a player hasn't completed a twilight/MDH, we want to unset the transform flag so they aren't forced to be wolf
        // un-necessarily.
        libtp::tp::d_save::dSv_save_c* saveFilePtr = &savePtr->save_file;
        libtp::tp::d_save::dSv_player_status_b_c* playerStatusBPtr = &saveFilePtr->player.player_status_b;
        uint8_t* memoryFlagsPtr = &saveFilePtr->mSave[4].temp_flags.memoryFlags[0xA];

        for (int32_t i = 0; i < 4; i++)
        {
            if (!d_save::isDarkClearLV(static_cast<void*>(playerStatusBPtr), i))
            {
                playerStatusBPtr->transform_level_flag &= ~(1 << i);

                if (i == 0x3) // MDH
                {
                    // Unset the flag that starts MDH
                    *memoryFlagsPtr &= ~0x40;
                    d_save::offEventBit(&saveFilePtr->mEvent, flags::MIDNAS_DESPERATE_HOUR_STARTED);
                }
            }
        }

        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();
        const rando::ShuffledEntrance* shuffledEntrances = seedPtr->getShuffledEntrancesPtr();

        // The very first entry of the shuffledEntrances table is always the spawn entrance.
        const rando::ShuffledEntrance* currentEntrance = &shuffledEntrances[0];

        libtp::tp::d_stage::dStage_nextStage* nextStagePtr = &gameInfoPtr->play.mNextStage;

        strncpy(nextStagePtr->mStage,
                libtp::data::stage::allStages[currentEntrance->getNewStageIDX()],
                sizeof(nextStagePtr->mStage) - 1);

        nextStagePtr->mRoomNo = currentEntrance->getNewRoomIDX();
        nextStagePtr->mPoint = currentEntrance->getNewSpawn();
        savePtr->mRestart.mStartPoint = currentEntrance->getNewSpawn();
        nextStagePtr->mLayer = currentEntrance->getNewState();
        nextStagePtr->enabled |= 0x1;

        return gReturn_dMenuOption__tv_open1_move(thisPtr);
    }

    KEEP_FUNC bool handleAdjustToTSwordReq()
    {
        using namespace libtp::data;
        using namespace libtp::tp;

        const int32_t roomID = libtp::tools::getCurrentRoomNo();
        libtp::tp::d_save::dSv_info_c* savePtr = &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save;

        // If we've already struck the pedestal in grove, we don't want to strike again since that would cause a softlock. We
        // don't need to check the stage because the code that this handle is injected into only runs in Sacred Grove.
        if ((roomID == 0x1) && d_save::isSwitch_dSv_memBit(&savePtr->memory.temp_flags, 0x63))
        {
            return false;
        }

        // Make sure we at least have a sword in our hand
        const uint8_t equippedSword = savePtr->save_file.player.player_status_a.equipment[1];
        if (equippedSword != 0xFF)
        {
            // If wooden sword isn't enough, then we can do a simple >= check for the rest of the swords
            const uint8_t neededSword = rando::gRandomizer->getSeedPtr()->getHeaderPtr()->getToTSwordRequirement();
            if (neededSword != items::Wooden_Sword)
            {
                // If the sword we have equipped is better or equal to the sword we need, allow it to be used.
                if ((equippedSword >= neededSword) && (equippedSword != items::Wooden_Sword))
                {
                    return true;
                }
            }
            else
            {
                // If we only need wooden sword, then any sword is good enough to satify the condition
                return true;
            }
        }

        return false;
    }

    KEEP_FUNC void adjustMidnaHairColor()
    {
        // Rainbow Calculations
        float angleIncrement = 1.0f;
        rainbowPhaseAngle += angleIncrement;
        if (rainbowPhaseAngle >= 360.0f)
            rainbowPhaseAngle -= 360.0f;
        float phase_rad = rainbowPhaseAngle * (float)(M_PI / 180.0);

        const float amplitude = 127.5f;
        uint16_t r_val = static_cast<uint16_t>(amplitude * (sinf(phase_rad) + 1.0f) + 0.5f);
        uint16_t g_val = static_cast<uint16_t>(amplitude * (sinf(phase_rad + 2.0f * M_PI / 3.0f) + 1.0f) + 0.5f);
        uint16_t b_val = static_cast<uint16_t>(amplitude * (sinf(phase_rad + 4.0f * M_PI / 3.0f) + 1.0f) + 0.5f);

        // Apply to Midna's Hair
        libtp::tp::d_a_player::daMidna_c* midnaPtr = libtp::tp::d_a_player::m_midnaActor;

        if (midnaPtr != nullptr)
        {
            const uint8_t tip_color = 200;

            midnaPtr->field_0x6e0.r = static_cast<int16_t>(r_val);
            midnaPtr->field_0x6e0.g = static_cast<int16_t>(g_val);
            midnaPtr->field_0x6e0.b = static_cast<int16_t>(b_val);
            midnaPtr->field_0x6e0.a = 0;

            midnaPtr->field_0x6e8.r = static_cast<int8_t>(r_val / 10.0f);
            midnaPtr->field_0x6e8.g = static_cast<int8_t>(g_val / 10.0f);
            midnaPtr->field_0x6e8.b = static_cast<int8_t>(b_val / 10.0f);
            midnaPtr->field_0x6e8.a = 0;

            midnaPtr->field_0x6ec.r = tip_color;
            midnaPtr->field_0x6ec.g = tip_color;
            midnaPtr->field_0x6ec.b = tip_color;
            midnaPtr->field_0x6ec.a = 0;
        }
    }

    // This is called in the NON-MAIN thread which is loading the archive where `mountArchive->mIsDone = true;` would be called
    // normally (this is the last thing that gets called before the archive is considered loaded). The archive is no longer
    // automatically marked as loaded, so we need to do this ourselves when we are done. (This indicates that the archive is
    // loaded, and whatever was waiting on it will see this byte change the next time it polls the completion status (polling
    // happens once per frame?))
    KEEP_FUNC bool handle_mountArchive__execute(libtp::tp::m_Do_dvd_thread::mDoDvdThd_mountArchive_c* mountArchive)
    {
        const bool ret = gReturn_mountArchive__execute(mountArchive);

        if (ret)
        {
            rando::gRandomizer->recolorArchiveTextures(mountArchive);
        }

        // Need to mark the archive as loaded once we are done modifying its
        // contents.
        mountArchive->mIsDone = true;

        return ret;
    };

    float __attribute__((noinline)) intToFloat(int32_t value)
    {
        return static_cast<float>(value);
    }
} // namespace mod