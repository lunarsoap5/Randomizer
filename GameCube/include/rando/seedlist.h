/**	@file seedlist.h
 *  @brief Stores list of seeds
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_SEEDLIST_H
#define RANDO_SEEDLIST_H

#include <cstdint>

#include "gc_wii/card.h"

// Earliest SeedData version (Major.Minor) which this version of the Randomizer
// supports.
#define MIN_SUPPORTED_SEED_DATA_VER_MAJOR 1
#define MIN_SUPPORTED_SEED_DATA_VER_MINOR 0

// Final SeedData version (Major.Minor) for which this version of the Randomizer
// is guaranteed to support 100% of the features. This will change more often
// than the minSupportedVersion. Generally speaking, this should be set to the
// version of the SeedData that the website will output when this version of the
// Randomizer is released. This maxFullySupportedVersion will be used in a
// future update (pre-1.0 release) to determine if a SeedData can be played even
// if some of its non-critical features are not supported.
#define MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR 1
#define MAX_FULLY_SUPPORTED_SEED_DATA_VER_MINOR 0

// Defines to help with version comparisions, as otherwise we get: "error:
// comparison is always true due to limited range of data type
// [-Werror=type-limits]" when the macro values are 0.
#if MIN_SUPPORTED_SEED_DATA_VER_MAJOR == 0
#define CHECK_MIN_SUPPORTED_SEED_DATA_VER_MAJOR( version ) 1
#else
#define CHECK_MIN_SUPPORTED_SEED_DATA_VER_MAJOR( version ) static_cast<uint16_t>( version ) >= MIN_SUPPORTED_SEED_DATA_VER_MAJOR
#endif

#if MIN_SUPPORTED_SEED_DATA_VER_MINOR == 0
#define CHECK_MIN_SUPPORTED_SEED_DATA_VER_MINOR( version ) 1
#else
#define CHECK_MIN_SUPPORTED_SEED_DATA_VER_MINOR( version ) static_cast<uint16_t>( version ) >= MIN_SUPPORTED_SEED_DATA_VER_MINOR
#endif

#if MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR == 0
#define CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR( version ) 1
#else
#define CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR( version ) \
    static_cast<uint16_t>( version ) <= MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR
#endif

#define CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MINOR( version ) \
    static_cast<uint16_t>( version ) <= MAX_FULLY_SUPPORTED_SEED_DATA_VER_MINOR

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

        void updateFromDirectoryEntry( libtp::gc_wii::card::CARDDirEntry& dirEntry );

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

    class SeedList
    {
       public:
        SeedList() {}
        ~SeedList();

        int8_t getCount() { return count; }
        int8_t getSelectedIndex() { return selectedIndex; }
        SeedListEntry* getSelectedEntry();
        SeedListEntry* getActiveEntry() { return activeEntry; }
        void updateEntries( libtp::gc_wii::card::CARDDirEntries* dirEntries, int numDirEntries );
        void incrementSelectedEntry();
        void decrementSelectedEntry();
        void setCurrentEntryToActive();
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
    };

}     // namespace mod::rando

#endif