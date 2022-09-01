/**	@file seedlist2.h
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
#include "rando/seedlist2.h"

namespace mod::rando
{
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

    SeedListEntry& SeedListEntry::operator=( SeedListEntry other )
    {
        copyFrom( *this, other );
        return *this;
    }

    void SeedListEntry::updateFromDirectoryEntry( libtp::util::card::DirectoryEntry& dirEntry )
    {
        // Copy filename
        std::memset( m_filename, 0, 33 );
        std::memcpy( m_filename, &dirEntry.filename, 32 );

        // Create playthroughName
        char* playthroughNamePtr = &dirEntry.filename[12];

        std::memset( m_playthroughName, 0, 25 );

        uint8_t nameAdjNounLength = 20;
        for ( uint8_t i = 0; i < 20; i++ )
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
                // For example, maxFullySupportedVer is 12.5 and the seed is v12.8.
                m_status = SeedListEntryStatus::PARTIALLY_SUPPORTED;
            }
        }
        else
        {
            // Newer (but not too new) and not fully supported
            m_status = SeedListEntryStatus::NOT_SUPPORTED;
        }
    }

    SeedList2::SeedList2() {}

    SeedList2::~SeedList2()
    {
        delete[] entries;
    }

    void SeedList2::updateEntries( libtp::util::card::DirectoryEntry* dirEntries, int numDirEntries )
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

        for ( int i = 0; i < numDirEntries; i++ )
        {
            libtp::util::card::DirectoryEntry& dirEntry = dirEntries[i];
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

        // Update properties of instance.
        this->count = totalEntries;
        this->entries = new SeedListEntry[totalEntries];
        for ( int i = 0; i < totalEntries; i++ )
        {
            this->entries[i] = workingEntries[i];
        }

        if ( idxForPrevFilename >= 0 && idxForPrevFilename < totalEntries )
        {
            this->selectedIndex = idxForPrevFilename;
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

    void SeedList2::clearEntries() {}

    SeedListEntry* SeedList2::getSelectedEntry()
    {
        if ( selectedIndex >= 0 && selectedIndex < count )
        {
            return &( entries[selectedIndex] );
        }

        return nullptr;
    }

    void SeedList2::incrementSelectedEntry()
    {
        if ( selectedIndex < 0 )
            return;

        int8_t newIndex = selectedIndex + 1;
        selectedIndex = newIndex >= count ? 0 : newIndex;
    }

    void SeedList2::decrementSelectedEntry()
    {
        if ( selectedIndex < 0 )
            return;

        int8_t newIndex = selectedIndex - 1;
        selectedIndex = newIndex < 0 ? count - 1 : newIndex;
    }

    void SeedList2::clearActiveEntry()
    {
        if ( activeEntry != nullptr )
        {
            delete activeEntry;
            activeEntry = nullptr;
        }
    }

    void SeedList2::setCurrentEntryToActive()
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

    bool SeedList2::shouldSwapSeedToSelected()
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
    void SeedList2::handleMemCardDetach()
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
