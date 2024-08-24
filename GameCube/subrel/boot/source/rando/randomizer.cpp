/**	@file randomizer.cpp
 *  @brief Randomizer main class
 *
 *	@author AECX
 *	@bug No known bugs.
 */
#include "rando/randomizer.h"
#include "cxx.h"
#include "gc_wii/card.h"
#include "main.h"
#include "rando/data.h"
#include "rando/seed.h"
#include "memory.h"
#include "gc_wii/OSTime.h"

#ifdef TP_EU
#include "tp/d_s_logo.h"
#endif

namespace mod::rando
{
    Randomizer::Randomizer()
    {
        // Initialize gRandomizer
        gRandomizer = this;

        // getConsole() << "Rando loading...\n";

        // Align to void*, as pointers use the largest variable type in the Seed class
#ifdef DVD
        Seed* seedPtr = new (sizeof(void*)) Seed;
#else
        Seed* seedPtr = new (sizeof(void*)) Seed(CARD_SLOT_A);
#endif

        if (seedPtr->seedIsLoaded())
        {
            // Initialize variables
            this->m_Seed = seedPtr;
            this->m_GoldenWolfItemReplacement.setItemActorId(-1);
            this->m_GameState = GameState::GAME_BOOT;
            this->m_GiveItemToPlayerStatus = EventItemStatus::QUEUE_EMPTY;
            this->initRandState();

#ifdef TP_EU
            this->m_CurrentLanguage = libtp::tp::d_s_logo::getPalLanguage2(nullptr);
#endif
            // Generate the BGM/Fanfare table data
            seedPtr->loadBgmData();
            seedPtr->loadShuffledEntrances();

            // getConsole() << "Applying one-time patches:\n";
            seedPtr->applyOneTimePatches();
        }
        else
        {
            // The seed failed to load, so clear seedPtr
            delete seedPtr;
        }
    }

    Randomizer::~Randomizer()
    {
        // getConsole() << "Rando unloading...\n";

        // Clear Seed
        Seed* seedPtr = this->m_Seed;
        if (seedPtr)
        {
            delete seedPtr;
        }

        // Free the memory used by dynamic stuff
        if (this->m_MsgTableInfo)
        {
            // m_MsgTableInfo should have been allocated via a uint8_t array, which does not store an array count at the start
            // of the array, so use delete instead of delete[]
            delete this->m_MsgTableInfo;
        }

        if (this->m_HintMsgTableInfo)
        {
            // m_HintMsgTableInfo should have been allocated via a uint8_t array, which does not store an array count at the
            // start of the array, so use delete instead of delete[]
            delete this->m_HintMsgTableInfo;
        }

        // Done, so clear gRandomizer
        gRandomizer = nullptr;
    }

    void Randomizer::initRandState()
    {
        // m_RandState is being used with xorshift32, which requires the state to not be 0. OSGetTick may return 0, so keep
        // calling it until it does not return 0.
        uint32_t state;

        do
        {
            state = libtp::gc_wii::os_time::OSGetTick();
        } while (state == 0);

        this->m_RandState = state;
    }
} // namespace mod::rando