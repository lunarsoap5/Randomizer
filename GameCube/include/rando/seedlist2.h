/**	@file seedlist2.h
 *  @brief Stores list of seeds
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_SEEDLIST2_H
#define RANDO_SEEDLIST2_H

#include <cstdint>

#include "util/cardUtils.h"

namespace mod::rando
{
    enum SeedListEntryStatus : uint8_t
    {
        FULLY_SUPPORTED,
        PARTIALLY_SUPPORTED,
        NOT_SUPPORTED,
        VERSION_UNKNOWN
    };

    class SeedListEntry
    {
       public:
        SeedListEntry() {}
        SeedListEntry( const SeedListEntry& existingInst );
        SeedListEntry& operator=( const SeedListEntry& other );

        void updateFromDirectoryEntry( libtp::util::card::DirectoryEntry& dirEntry );

        uint16_t verMajor() { return m_verMajor; }
        uint16_t verMinor() { return m_verMinor; }
        uint16_t blockCount() { return m_fileBlockCount; }
        uint32_t commentsOffset() { return m_commentsOffset; }
        const char* filename() { return m_filename; }
        const char* playthroughName() { return m_playthroughName; }
        SeedListEntryStatus status() { return m_status; }
        bool isCompatibleWithRando() { return m_status == FULLY_SUPPORTED || m_status == PARTIALLY_SUPPORTED; }

       private:
        uint16_t m_verMajor;
        uint16_t m_verMinor;
        uint16_t m_fileBlockCount;
        uint16_t m_commentsOffset;
        char m_filename[33];
        char m_playthroughName[25];
        SeedListEntryStatus m_status;

        static void copyFrom( SeedListEntry& dest, const SeedListEntry& src );
    };

    class SeedList2
    {
       public:
        SeedList2();
        ~SeedList2( void );

        int8_t getCount() { return count; }
        int8_t getSelectedIndex() { return selectedIndex; }
        SeedListEntry* getActiveEntry() { return activeEntry; }
        void updateEntries( libtp::util::card::DirectoryEntry* dirEntries, int count );
        void clearEntries();
        SeedListEntry* getSelectedEntry();
        void incrementSelectedEntry();
        void decrementSelectedEntry();
        void setCurrentEntryToActive();
        // bool checkActiveEntryMatchesSelected();
        bool shouldSwapSeedToSelected();
        void clearActiveEntry();
        void handleMemCardDetach();

       private:
        enum PrevMemCardAction : uint8_t
        {
            NONE,
            LOADFILE,
            DETACH,
        };

        int8_t count = 0;
        int8_t selectedIndex = -1;
        PrevMemCardAction prevMemCardAction = NONE;
        SeedListEntry* entries = nullptr;
        // activeEntry points to its own copy of the entry data for that seed.
        SeedListEntry* activeEntry = nullptr;

        // SeedInfo FindSeed( uint64_t seed );

        // uint8_t m_numSeeds;
        // uint8_t m_selectedSeed;

        // SeedInfo* m_seedInfo = nullptr;
    };

    // int abc();

}     // namespace mod::rando

#endif