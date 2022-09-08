/**	@file seedlist.cpp
 *  @brief Stores list of seeds
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iterator>

#ifdef DVD
#include "gc_wii/dvd.h"
#else
#include "gc_wii/card.h"
#endif

#include "string.h"
#include "rando/seedlist.h"

namespace mod::rando
{
    using libtp::gc_wii::card::CARDDirEntries;
    using libtp::gc_wii::card::CARDDirEntry;

    static const uint16_t VersionDecodeFailure = 0xFFFF;

    // Converts 4 chars representing 24 bits to u16 verMajor and verMinor
    void decodeVersion( uint16_t* outVersions, const char* verStr )
    {
        uint32_t parsed24Bits = 0;
        uint8_t shiftAmt = 18;
        bool error = false;

        for ( uint8_t i = 0; i < 4; i++ )
        {
            char charVal = verStr[i];
            uint8_t numericalValue;

            if ( charVal < '0' )
            {
                error = true;
                break;
            }
            else if ( charVal <= '9' )
            {
                numericalValue = charVal - '0';
            }
            else if ( charVal < 'A' )
            {
                error = true;
                break;
            }
            else if ( charVal <= 'Z' )
            {
                numericalValue = charVal - 'A' + 10;
            }
            else if ( charVal < 'a' )
            {
                error = true;
                break;
            }
            else if ( charVal <= 'z' )
            {
                numericalValue = charVal - 'a' + 36;
            }
            else
            {
                error = true;
                break;
            }

            parsed24Bits |= ( numericalValue << shiftAmt );
            shiftAmt -= 6;
        }

        if ( error )
        {
            outVersions[0] = VersionDecodeFailure;
            outVersions[1] = VersionDecodeFailure;
            return;
        }

        // 24 bits. First 4 are a value 0 through 15. 1 plus this (1 through 16)
        // says how many bits are for the major version (follows the first 4
        // bits). The remaining bits (up to 19) are treated as a u16 and used
        // for the minor version. This lets us represent any version number we
        // might realistically see (up to version 65534.15) in 4 characters
        // instead of 6.

        // Example of versions which cause a forced major version update are
        // 32767.31, 16383.63, 8191.127, 4095.255, 2047.511, 1023.1023. As you
        // can see, we are unlikely to run into any of these limits. Since the
        // minor version resets whenever the major version increments, we would
        // have to go through thousands of major verions before we even have a
        // chance of a forced major version update (which isn't even a big deal
        // if we had one by some miracle).

        uint8_t numBitsForVerMajor = 1 + ( ( parsed24Bits >> 20 ) & 0xF );

        uint32_t verMajorBitMask = 0xFFFF0 & ( 0xFFFF0 << ( 16 - numBitsForVerMajor ) );
        uint16_t verMajor = ( parsed24Bits & verMajorBitMask ) >> ( 20 - numBitsForVerMajor );

        uint32_t verMinorBitMask = 0xFFFFF & ( 0xFFFFF >> ( numBitsForVerMajor ) );
        uint16_t verMinor = parsed24Bits & verMinorBitMask;

        outVersions[0] = verMajor;
        outVersions[1] = verMinor;
    }

    void SeedListEntry::copyFrom( SeedListEntry& dest, const SeedListEntry& src )
    {
        dest.m_verMajor = src.m_verMajor;
        dest.m_verMinor = src.m_verMinor;
        dest.m_fileBlockCount = src.m_fileBlockCount;
        dest.m_commentsOffset = src.m_commentsOffset;
        memcpy( dest.m_filename, src.m_filename, 33 );
        memcpy( dest.m_playthroughName, src.m_playthroughName, 25 );
        dest.m_status = src.m_status;
    }

    SeedListEntry::SeedListEntry( const SeedListEntry& other )
    {
        copyFrom( *this, other );
    }

    SeedListEntry& SeedListEntry::operator=( const SeedListEntry& other )
    {
        copyFrom( *this, other );
        return *this;
    }

    void SeedListEntry::updateFromDirectoryEntry( CARDDirEntry& dirEntry )
    {
        // Copy filename
        std::memset( m_filename, 0, 33 );
        std::memcpy( m_filename, &dirEntry.filename, 32 );

        // Create playthroughName
        char* playthroughNamePtr = &dirEntry.filename[12];

        std::memset( m_playthroughName, 0, 25 );

        // Adjective and Noun both have a max of 10 chars.
        uint8_t nameAdjNounLength = 20;
        for ( uint8_t i = 0; i < nameAdjNounLength; i++ )
        {
            if ( playthroughNamePtr[i] == 0 )
            {
                nameAdjNounLength = i;
                break;
            }
        }

        std::memcpy( m_playthroughName, playthroughNamePtr, nameAdjNounLength );
        m_playthroughName[nameAdjNounLength] = '_';
        std::memcpy( &m_playthroughName[nameAdjNounLength + 1], &dirEntry.filename[9], 3 );

        // Decode version numbers
        uint16_t versionNums[2];
        decodeVersion( versionNums, &dirEntry.filename[2] );

        uint16_t versionMajor = versionNums[0];
        uint16_t versionMinor = versionNums[1];

        m_verMajor = versionMajor;
        m_verMinor = versionMinor;
        m_fileBlockCount = dirEntry.numBlocks;
        m_commentsOffset = dirEntry.commentsOffset;

        if ( versionMajor == VersionDecodeFailure || versionMinor == VersionDecodeFailure )
        {
            // Incompatible. Unable to parse version numbers.
            m_status = SeedListEntryStatus::VERSION_UNKNOWN;
        }
        else if ( CHECK_MIN_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) &&
                  CHECK_MIN_SUPPORTED_SEED_DATA_VER_MINOR( versionMinor ) &&
                  CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) )
        {
            if ( CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MINOR( versionMinor ) )
            {
                m_status = SeedListEntryStatus::FULLY_SUPPORTED;
            }
            else
            {
                // For example, maxFullySupportedVer is 12.5 and the seed is
                // v12.8.
                m_status = SeedListEntryStatus::PARTIALLY_SUPPORTED;
            }
        }
        else
        {
            // Either too old or too new to support.
            m_status = SeedListEntryStatus::NOT_SUPPORTED;
        }
    }

    SeedList::~SeedList()
    {
        if ( entries != nullptr )
        {
            delete[] entries;
            entries = nullptr;
        }
    }

    void merge( SeedListEntry* seedEntries,
                uint8_t* outArr,
                uint8_t* leftArr,
                int leftCount,
                uint8_t* rightArr,
                int rightCount )
    {
        uint8_t i = 0;
        uint8_t l = 0;
        uint8_t r = 0;

        while ( l < leftCount && r < rightCount )
        {
            // Sort by playthroughName A-Z (ascending).
            if ( strcmp( seedEntries[leftArr[l]].playthroughName(), seedEntries[rightArr[r]].playthroughName() ) < 0 )
            {
                outArr[i++] = leftArr[l++];
            }
            else
            {
                outArr[i++] = rightArr[r++];
            }
        }

        while ( l < leftCount )
        {
            outArr[i++] = leftArr[l++];
        }
        while ( r < rightCount )
        {
            outArr[i++] = rightArr[r++];
        }
    }

    // Merge sort implementation to show seeds on title screen in alphabetical order.
    void mergeSortSeedEntries( SeedListEntry* seedEntries, uint8_t* outArr, int count )
    {
        if ( count < 2 )
        {
            return;
        }

        int leftArrLen = count / 2;
        int rightArrLen = count - leftArrLen;

        uint8_t* leftArr = new uint8_t[leftArrLen];
        uint8_t* rightArr = new uint8_t[rightArrLen];

        uint8_t rightArrIdx = 0;
        for ( uint8_t idx = 0; idx < count; idx++ )
        {
            if ( idx < leftArrLen )
            {
                leftArr[idx] = outArr[idx];
            }
            else
            {
                rightArr[rightArrIdx] = outArr[idx];
                rightArrIdx++;
            }
        }

        mergeSortSeedEntries( seedEntries, leftArr, leftArrLen );
        mergeSortSeedEntries( seedEntries, rightArr, rightArrLen );
        merge( seedEntries, outArr, leftArr, leftArrLen, rightArr, rightArrLen );

        delete[] leftArr;
        delete[] rightArr;
    }

    void SeedList::updateEntries( CARDDirEntries* dirEntries, int numDirEntries )
    {
        prevMemCardAction = LOADFILE;

        bool hasPrevFilename = false;
        char prevFilename[33];

        if ( this->entries != nullptr )
        {
            if ( selectedIndex >= 0 && selectedIndex < count )
            {
                hasPrevFilename = true;
                memcpy( prevFilename, this->entries[selectedIndex].filename(), 33 );
            }

            delete[] this->entries;
        }

        SeedListEntry* workingEntries = new SeedListEntry[numDirEntries];
        int totalEntries = 0;
        int8_t idxForPrevFilename = -1;

        CARDDirEntry* entries = dirEntries->entries;

        for ( int i = 0; i < numDirEntries; i++ )
        {
            CARDDirEntry& dirEntry = entries[i];
            if ( dirEntry.filename[0] == 's' && dirEntry.filename[1] == 'd' )
            {
                // Filename starting with "sd" indicates this is a seed file.
                workingEntries[totalEntries].updateFromDirectoryEntry( dirEntry );
                if ( hasPrevFilename && strcmp( prevFilename, workingEntries[totalEntries].filename() ) == 0 )
                {
                    idxForPrevFilename = totalEntries;
                }

                totalEntries += 1;
            }
        }

        // If workingEntries had playthroughNames ["b", "c", "a"], then after
        // sorting, sortedIndex would contain [2,0,1]. Meaning index 2 (for "a")
        // would come first, then index 0 ("b"), then index 1 ("c").
        uint8_t sortedIndexes[CARD_MAX_FILE];
        for ( uint8_t i = 0; i < totalEntries; i++ )
        {
            sortedIndexes[i] = i;
        }
        mergeSortSeedEntries( workingEntries, sortedIndexes, totalEntries );

        // Update properties of instance.
        this->count = totalEntries;
        this->entries = new SeedListEntry[totalEntries];
        for ( int i = 0; i < totalEntries; i++ )
        {
            // Copy from workingEntries in sorted order.
            this->entries[i] = workingEntries[sortedIndexes[i]];
        }

        if ( idxForPrevFilename >= 0 && idxForPrevFilename < totalEntries )
        {
            this->selectedIndex = 0;
            for ( uint8_t i = 0; i < totalEntries; i++ )
            {
                if ( sortedIndexes[i] == idxForPrevFilename )
                {
                    this->selectedIndex = i;
                    break;
                }
            }
        }
        else if ( totalEntries > 0 )
        {
            this->selectedIndex = 0;
        }
        else
        {
            this->selectedIndex = -1;
        }

        delete[] workingEntries;
    }

    SeedListEntry* SeedList::getSelectedEntry()
    {
        if ( selectedIndex >= 0 && selectedIndex < count )
        {
            return &( entries[selectedIndex] );
        }

        return nullptr;
    }

    void SeedList::incrementSelectedEntry()
    {
        if ( selectedIndex < 0 )
            return;

        int8_t newIndex = selectedIndex + 1;
        selectedIndex = newIndex >= count ? 0 : newIndex;
    }

    void SeedList::decrementSelectedEntry()
    {
        if ( selectedIndex < 0 )
            return;

        int8_t newIndex = selectedIndex - 1;
        selectedIndex = newIndex < 0 ? count - 1 : newIndex;
    }

    void SeedList::clearActiveEntry()
    {
        if ( activeEntry != nullptr )
        {
            delete activeEntry;
            activeEntry = nullptr;
        }
    }

    void SeedList::setCurrentEntryToActive()
    {
        if ( selectedIndex >= 0 && selectedIndex < count )
        {
            // Updating activeEntry.
            if ( activeEntry == nullptr )
            {
                activeEntry = new SeedListEntry();
            }
            *activeEntry = entries[selectedIndex];
        }
        else
        {
            clearActiveEntry();
        }
    }

    bool SeedList::shouldSwapSeedToSelected()
    {
        if ( selectedIndex < 0 || selectedIndex >= count )
        {
            // Should not swap if the current selection is invalid (no seeds in the seedList).
            return false;
        }

        // Should swap to the valid selected entry if we have no activeEntry or
        // if the activeEntry has a different filename from the selected one.
        return ( activeEntry == nullptr || strcmp( activeEntry->filename(), entries[selectedIndex].filename() ) != 0 );
    }

    // Clear all entries except the activeEntry if it exists.
    void SeedList::handleMemCardDetach()
    {
        // This can get called several times. For example, when resetting to the
        // title screen, it seems to get called every frame.
        if ( prevMemCardAction != DETACH )
        {
            prevMemCardAction = DETACH;

            if ( entries != nullptr )
            {
                delete[] entries;
            }

            if ( activeEntry != nullptr )
            {
                entries = new SeedListEntry[1];
                count = 1;
                selectedIndex = 0;
                entries[0] = *activeEntry;
            }
            else
            {
                entries = nullptr;
                count = 0;
                selectedIndex = -1;
            }
        }
    }

}     // namespace mod::rando
