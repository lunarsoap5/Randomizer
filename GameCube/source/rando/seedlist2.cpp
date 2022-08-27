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

    void SeedListEntry::updateFromDirectoryEntry( libtp::util::card::DirectoryEntry& dirEntry )
    {
        std::memset( m_filename, 0, 33 );
        std::memcpy( m_filename, &dirEntry.filename, 32 );

        uint16_t versionMajor = decodeU16From3Chars( &m_filename[2] );
        uint16_t versionMinor = decodeU16From3Chars( &m_filename[5] );

        m_verMajor = versionMajor;
        m_verMinor = versionMinor;

        if ( versionMajor == VersionDecodeFailure || versionMinor == VersionDecodeFailure )
        {
            // Incompatible. Unable to parse version numbers.
            m_status = SeedListEntryStatus::VERSION_UNKNOWN;
        }
        else if ( !( CHECK_MIN_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) ) ||
                  !( CHECK_MIN_SUPPORTED_SEED_DATA_VER_MINOR( header.versionMinor ) ) )
        {
            // Incompatible. Too old, no longer supported
            m_status = SeedListEntryStatus::NOT_SUPPORTED;
        }
        else if ( !( CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) ) ||
                  !( CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MINOR( versionMinor ) ) )
        {
            // Newer and not fully supported
            m_status = SeedListEntryStatus::PARTIALLY_SUPPORTED;
        }
        else
        {
            // Fully supported
            m_status = SeedListEntryStatus::FULLY_SUPPORTED;
        }
    }

    SeedList2::SeedList2( SeedListEntry* entries, int count )
    {
        this->count = count;
        this->entries = new SeedListEntry[count];

        for ( int i = 0; i < count; i++ )
        {
            this->entries[i] = entries[i];
        }
    }

    SeedList2::~SeedList2()
    {
        delete[] entries;
    }

    SeedList2* SeedList2::fromDirectoryEntries( libtp::util::card::DirectoryEntry* dirEntries, int count )
    {
        SeedListEntry* workingEntries = new SeedListEntry[count];
        int totalEntries = 0;

        for ( int i = 0; i < count; i++ )
        {
            libtp::util::card::DirectoryEntry& dirEntry = dirEntries[i];
            if ( dirEntry.filename[0] == 's' && dirEntry.filename[1] == 'd' )
            {
                // Filename starting with "sd" indicates this is a seed file.
                workingEntries[totalEntries].updateFromDirectoryEntry( dirEntry );

                totalEntries += 1;
            }
        }

        SeedList2* seedlist2 = new SeedList2( workingEntries, totalEntries );

        delete[] workingEntries;

        return seedlist2;
    }

}     // namespace mod::rando
