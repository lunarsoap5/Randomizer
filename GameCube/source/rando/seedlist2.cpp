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
    static const char* asdf = "0123456789abcDefghiJkLmNopQrstuVwxyzABEHR";

    uint16_t decodeU16From3Chars( char* chars )
    {
        int result = 0;

        for ( int i = 0; i < 3; i++ )
        {
            char character = chars[i];
            const char* charPtr = std::find( asdf, asdf + 41, character );
            if ( charPtr == asdf + 41 )
            {
                // Failed to map character to number.
                return VersionDecodeFailure;
            }

            result *= 41;
            result += std::distance( asdf, charPtr );
        }

        return result;
    }

    void SeedListEntry::copyFrom( SeedListEntry& dest, const SeedListEntry& src )
    {
        dest.m_verMajor = src.m_verMajor;
        dest.m_verMinor = src.m_verMinor;
        dest.m_fileBlockCount = src.m_fileBlockCount;
        dest.m_commentsOffset = src.m_commentsOffset;
        memcpy( dest.m_filename, src.m_filename, 33 );
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
        std::memset( m_filename, 0, 33 );
        std::memcpy( m_filename, &dirEntry.filename, 32 );

        uint16_t versionMajor = decodeU16From3Chars( &m_filename[2] );
        uint16_t versionMinor = decodeU16From3Chars( &m_filename[5] );

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
        if ( this->entries != nullptr )
        {
            delete[] this->entries;
        }

        SeedListEntry* workingEntries = new SeedListEntry[numDirEntries];
        int totalEntries = 0;

        for ( int i = 0; i < numDirEntries; i++ )
        {
            libtp::util::card::DirectoryEntry& dirEntry = dirEntries[i];
            if ( dirEntry.filename[0] == 's' && dirEntry.filename[1] == 'd' )
            {
                // Filename starting with "sd" indicates this is a seed file.
                workingEntries[totalEntries].updateFromDirectoryEntry( dirEntry );

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
        this->selectedIndex = totalEntries > 0 ? 0 : -1;

        delete[] workingEntries;
    }

    void SeedList2::clearEntries()
    {
        if ( entries != nullptr )
        {
            delete[] entries;
        }

        count = 0;
        selectedIndex = -1;
    }

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

}     // namespace mod::rando
