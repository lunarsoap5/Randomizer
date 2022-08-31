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
    // Randomizer::Randomizer( SeedInfo* seedInfo, uint8_t selectedSeed )
    Randomizer::Randomizer()
    {
        getConsole() << "Rando loading...\n";
        // loadSeed( seedInfo, selectedSeed );
        loadSeed();
    }

    Randomizer::~Randomizer( void )
    {
        getConsole() << "Rando unloading...\n";

        // Clear Seed
        delete m_Seed;
    }

    // void Randomizer::loadSeed( SeedInfo* seedInfo, uint8_t selectedSeed )
    void Randomizer::loadSeed()
    {
        SeedListEntry* activeEntry = seedList2.getActiveEntry();

        // if ( seedInfo->fileIndex == 0xFF )
        if ( activeEntry == nullptr )
        {
            // getConsole() << "<Randomizer> Error: No such seed (0xFF)\n";
            getConsole() << "<Randomizer> Error: Valid seed not selected.\n";
        }
        else
        {
            // getConsole() << "Seed: " << seedInfo->header.seed << "\n";
            getConsole() << "Seed: " << activeEntry->playthroughName() << "\n";
            // Load the seed
            // m_SeedInfo = seedInfo;
            seedList2.setCurrentEntryToActive();

            // Align to void*, as pointers use the largest variable type in the Seed class
            // m_Seed = new ( sizeof( void* ) ) Seed( CARD_SLOT_A, seedInfo );
            m_Seed = new ( sizeof( void* ) ) Seed( CARD_SLOT_A, seedList2.getActiveEntry() );

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

    // void Randomizer::changeSeed( SeedInfo* seedInfo, uint8_t newSeed )
    void Randomizer::changeSeed()
    {
        getConsole() << "Seed unloading...\n";
        delete m_Seed;
        m_Seed = nullptr;
        m_SeedInit = false;

        getConsole() << "Seed Loading...\n";
        // loadSeed( seedInfo, newSeed );
        loadSeed();
    }
}     // namespace mod::rando