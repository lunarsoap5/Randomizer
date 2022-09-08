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

namespace mod::rando
{
    Randomizer::Randomizer()
    {
        getConsole() << "Rando loading...\n";
        loadSeed();
    }

    Randomizer::~Randomizer( void )
    {
        getConsole() << "Rando unloading...\n";

        // Clear Seed
        delete m_Seed;
    }

    void Randomizer::loadSeed()
    {
        SeedListEntry* activeEntry = seedList.getActiveEntry();

        if ( activeEntry == nullptr )
        {
            getConsole() << "<Randomizer> Error: Valid seed not selected.\n";
        }
        else
        {
            getConsole() << "Seed: " << activeEntry->playthroughName() << "\n";
            // Load the seed
            seedList.setCurrentEntryToActive();

            // Align to void*, as pointers use the largest variable type in the Seed class
            m_Seed = new ( sizeof( void* ) ) Seed( CARD_SLOT_A, seedList.getActiveEntry() );

            if ( m_Seed->checkIfSeedLoaded() )
            {
                // Load checks for first load
                onStageLoad();
            }
            else
            {
                // The seed failed to load, so clear the seed
                delete m_Seed;
                m_Seed = nullptr;
            }
        }
    }

    void Randomizer::changeSeed()
    {
        getConsole() << "Seed unloading...\n";
        delete m_Seed;
        m_Seed = nullptr;
        m_SeedInit = false;

        getConsole() << "Seed Loading...\n";
        loadSeed();
    }
}     // namespace mod::rando