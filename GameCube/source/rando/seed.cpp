/**	@file seed.cpp
 *  @brief Seed class to access seed-data
 *
 *	@author AECX
 *	@bug No known bugs.
 */

#include "rando/seed.h"

#include <string.h>

#include <cstdint>
#include <cstdio>

#include "data/stages.h"
#include "gc/card.h"
#include "gc/dvdfs.h"
#include "main.h"
#include "rando/data.h"
#include "tools.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_save.h"
#include "user_patch/user_patch.h"

namespace mod::rando
{
    Seed::Seed( int32_t chan, SeedInfo* seedInfo ): m_CardSlot( chan )
    {
        m_Header = &seedInfo->header;

        // Loading seed rando-dataX '<seed>'...

        // Store our filename index
        m_fileIndex = seedInfo->fileIndex;

        mod::console << "Loading seed " << m_fileIndex << ": '" << m_Header->seed << "'...\n";

        // Load the whole gci locally to reduce number of reads (memcard)
        uint32_t length = m_Header->fileSize;
        char fileName[12] = "rando-data\0";

        fileName[10] = static_cast<char>( '0' + m_fileIndex );

        m_GCIData = new uint8_t[length];

        m_CARDResult = libtp::tools::ReadGCI( m_CardSlot, fileName, length, 0x00, m_GCIData );
    }

    Seed::~Seed()
    {
        // Make sure to delete tempcheck buffers
        this->ClearChecks();

        // Only work with m_GCIData if the buffer is populated
        if ( m_GCIData )
        {
            this->applyPatches( false );

            // Last clear gcibuffer as other functions before rely on it
            delete[] m_GCIData;
        }
    }

    bool Seed::InitSeed( void )
    {
        // (Re)set counters & status
        m_AreaFlagsModified = 0;
        m_EventFlagsModified = 0;
        m_PatchesApplied = 0;

        if ( m_CARDResult == CARD_RESULT_READY )
        {
            mod::console << "Patching game:\n";

            this->applyPatches( true );

            mod::console << "Setting Event Flags... \n";
            this->applyEventFlags();

            mod::console << "Setting Region Flags... \n";
            this->applyRegionFlags();
            return true;
        }
        else
        {
            mod::console << "FATAL: Couldn't read Seed #" << m_fileIndex << "\n";
            mod::console << "ERROR: " << m_CARDResult << "\n";
            return false;
        }
    }

    bool Seed::LoadChecks( const char* stage )
    {
        using namespace libtp;

        // Find the index of this stage
        uint8_t stageIDX;
        for ( stageIDX = 0; stageIDX < sizeof( data::stage::allStages ) / sizeof( data::stage::allStages[0] ); stageIDX++ )
        {
            if ( !strcmp( stage, data::stage::allStages[stageIDX] ) )
            {
                break;
            }
        }

        // Don't run if this isn't actually a new stage
        bool result = stageIDX != m_StageIDX;
        if ( result )
        {
            this->ClearChecks();

            this->LoadDZX( stageIDX );
            this->LoadREL( stageIDX );
            this->LoadPOE( stageIDX );
            this->LoadBOSS( stageIDX );

            // Save current stageIDX for next time
            m_StageIDX = stageIDX;
        }

        return result;
    }

    void Seed::ClearChecks()
    {
        m_numLoadedDZXChecks = 0;
        m_numLoadedRELChecks = 0;
        m_numLoadedPOEChecks = 0;

        delete[] m_DZXChecks;
        delete[] m_RELChecks;
        delete[] m_POEChecks;
    }

    void Seed::applyPatches( bool set )
    {
        using namespace libtp;

        uint32_t num_bytes = m_Header->patchInfo.numEntries;
        uint32_t gci_offset = m_Header->patchInfo.dataOffset;

        // Don't bother to patch anything if there's nothing to patch
        if ( num_bytes > 0 )
        {
            // Set the pointer as offset into our buffer
            uint8_t* patch_config = &m_GCIData[gci_offset];

            for ( uint32_t i = 0; i < num_bytes; i++ )
            {
                uint8_t byte = patch_config[i];

                for ( uint32_t b = 0; b < 8; b++ )
                {
                    if ( ( byte << b ) & 0x80 )
                    {
                        // run the patch function for this bit index
                        uint32_t index = i * 8 + b;

                        if ( index < sizeof( user_patch::patches ) / sizeof( user_patch::patches[0] ) )
                        {
                            user_patch::patches[index]( mod::randomizer, set );
                            m_PatchesApplied++;
                        }
                    }
                }
            }
        }
    }

    void Seed::applyEventFlags()
    {
        using namespace libtp;

        uint32_t num_eventFlags = m_Header->eventFlagsInfo.numEntries;
        uint32_t gci_offset = m_Header->eventFlagsInfo.dataOffset;

        if ( num_eventFlags > 0 )
        {
            tp::d_com_inf_game::dComIfG_inf_c* gameInfo = &tp::d_com_inf_game::dComIfG_gameInfo;

            // Set the pointer as offset into our buffer
            eventFlag* eventFlags = reinterpret_cast<eventFlag*>( &m_GCIData[gci_offset] );

            for ( uint32_t i = 0; i < num_eventFlags; i++ )
            {
                uint8_t offset = eventFlags[i].offset;
                uint8_t flag = eventFlags[i].flag;

                gameInfo->save.events.event_flags[offset] |= flag;
                m_EventFlagsModified++;
            }
        }
    }

    void Seed::applyRegionFlags()
    {
        using namespace libtp;

        uint32_t num_regionFlags = m_Header->regionFlagsInfo.numEntries;
        uint32_t gci_offset = m_Header->regionFlagsInfo.dataOffset;

        if ( num_regionFlags > 0 )
        {
            tp::d_save::dSv_info_c* SaveInfo = &tp::d_com_inf_game::dComIfG_gameInfo.save;
            tp::d_com_inf_game::dComIfG_inf_c* gameInfo = &tp::d_com_inf_game::dComIfG_gameInfo;

            // Set the pointer as offset into our buffer
            regionFlag* regionFlags = reinterpret_cast<regionFlag*>( &m_GCIData[gci_offset] );

            uint8_t regionID = data::stage::regions[0];
            // Loop through all regions
            for ( uint32_t i = 0; i < sizeof( data::stage::regions ); i++ )
            {
                regionID = data::stage::regions[i];

                tp::d_save::getSave( SaveInfo, regionID );

                // Loop through region-flags from gci
                for ( uint32_t j = 0; j < num_regionFlags; j++ )
                {
                    if ( regionFlags[j].region_id == regionID )
                    {
                        // The n'th bit that we want to set TRUE
                        int32_t bit = regionFlags[j].bit_id;

                        int32_t offset = bit / 8;
                        int32_t shift = bit % 8;

                        // Failsafe; localAreaNode size is 0x20
                        if ( offset < 0x20 )
                        {
                            SaveInfo->memory.temp_flags.memoryFlags[offset] |=
                                ( 0x80 >>
                                  shift );     // This needs some work and will need to be transferred to the new structure.
                            m_AreaFlagsModified++;
                        }
                    }
                }

                // Save the modifications
                tp::d_save::putSave( SaveInfo, regionID );
            }

            // Reset to previous region ID
            uint8_t i = 0;

            while ( !strcmp( data::stage::allStages[i], gameInfo->play.mStartStage.mStage ) )
            {
                i++;
            }

            tp::d_save::getSave( SaveInfo, data::stage::regions[i] );
        }
    }

    void Seed::LoadDZX( uint8_t stageIDX )
    {
        using namespace libtp;

        uint32_t num_dzxchecks = m_Header->dzxCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->dzxCheckInfo.dataOffset;

        // Set the pointer as offset into our buffer
        dzxCheck* allDZX = reinterpret_cast<dzxCheck*>( &m_GCIData[gci_offset] );

        for ( uint32_t i = 0; i < num_dzxchecks; i++ )
        {
            if ( allDZX[i].stageIDX == stageIDX )
            {
                m_numLoadedDZXChecks++;
            }
        }

        // Allocate memory the actual RELChecks
        // We do NOT have to clear the previous buffer as that's already done in "LoadChecks()"
        m_DZXChecks = new dzxCheck[m_numLoadedDZXChecks];

        // offset into m_DZXChecks
        uint32_t j = 0;

        for ( uint32_t i = 0; i < num_dzxchecks; i++ )
        {
            if ( allDZX[i].stageIDX == stageIDX )
            {
                // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
                memcpy( &m_DZXChecks[j], &allDZX[i], sizeof( dzxCheck ) );
                j++;
            }
        }
    }

    void Seed::LoadREL( uint8_t stageIDX )
    {
        using namespace libtp;

        uint32_t num_relchecks = m_Header->relCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->relCheckInfo.dataOffset;

        // Set the pointer as offset into our buffer
        RELCheck* allREL = reinterpret_cast<RELCheck*>( &m_GCIData[gci_offset] );

        for ( uint32_t i = 0; i < num_relchecks; i++ )
        {
            if ( allREL[i].stageIDX == stageIDX )
            {
                m_numLoadedRELChecks++;
            }
        }

        // Allocate memory to the actual RELChecks
        // Do NOT need to clear the previous buffer as that's taken care of by LoadChecks()
        m_RELChecks = new RELCheck[m_numLoadedRELChecks];

        // offset into m_RELChecks
        uint32_t j = 0;

        for ( uint32_t i = 0; i < num_relchecks; i++ )
        {
            if ( allREL[i].stageIDX == stageIDX )
            {
                // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
                memcpy( &m_RELChecks[j], &allREL[i], sizeof( RELCheck ) );
                j++;
            }
        }
    }

    void Seed::LoadPOE( uint8_t stageIDX )
    {
        using namespace libtp;

        uint32_t num_poechecks = m_Header->poeCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->poeCheckInfo.dataOffset;

        // Set the pointer as offset into our buffer
        POECheck* allPOE = reinterpret_cast<POECheck*>( &m_GCIData[gci_offset] );

        for ( uint32_t i = 0; i < num_poechecks; i++ )
        {
            if ( allPOE[i].stageIDX == stageIDX )
            {
                m_numLoadedPOEChecks++;
            }
        }

        // Allocate memory to the actual POEChecks
        // Do NOT need to clear the previous buffer as that's taken care of by LoadChecks()
        m_POEChecks = new POECheck[m_numLoadedPOEChecks];

        // offset into m_POEChecks
        uint32_t j = 0;

        for ( uint32_t i = 0; i < num_poechecks; i++ )
        {
            if ( allPOE[i].stageIDX == stageIDX )
            {
                // Store the i'th POE check into the j'th Loaded POEcheck that's relevant to our current stage
                memcpy( &m_POEChecks[j], &allPOE[i], sizeof( POECheck ) );
                j++;
            }
        }
    }

    void Seed::LoadHiddenSkill()
    {
        using namespace libtp;

        uint32_t num_hiddenSkillChecks = m_Header->hiddenSkillCheckInfo.numEntries;
        for ( uint32_t i = 0; i < num_hiddenSkillChecks; i++ )
        {
            m_numHiddenSkillChecks++;
        }
    }

    void Seed::LoadBugReward()
    {
        using namespace libtp;

        uint32_t num_bugRewardChecks = m_Header->bugRewardCheckInfo.numEntries;
        for ( uint32_t i = 0; i < num_bugRewardChecks; i++ )
        {
            m_numBugRewardChecks++;
        }
    }

    void Seed::LoadBOSS( uint8_t stageIDX )
    {
        using namespace libtp;

        uint32_t num_bossChecks = m_Header->bossCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->bossCheckInfo.dataOffset;

        // Set the pointer as offset into our buffer
        BOSSCheck* allBOSS = reinterpret_cast<BOSSCheck*>( &m_GCIData[gci_offset] );
        m_BossChecks = nullptr;
        m_BossChecks = new BOSSCheck[1];

        // There is only one BOSS check per stage. Once we have a match, we are done.
        for ( uint32_t i = 0; i < num_bossChecks; i++ )
        {
            if ( allBOSS[i].stageIDX == stageIDX )
            {
                memcpy( &m_BossChecks[0], &allBOSS[i], sizeof( BOSSCheck ) );
                break;
            }
        }
    }

    void Seed::LoadARCChecks( int32_t fileIndex )
    {
        using namespace libtp;

        uint32_t num_arcchecks = m_Header->arcCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->arcCheckInfo.dataOffset;

        // Set the pointer as offset into our buffer
        ARCReplacement* allARC = reinterpret_cast<ARCReplacement*>( &m_GCIData[gci_offset] );
        // Until a better way is found, we are going to clear the buffer here just to be safe
        m_ArcReplacements = nullptr;
        m_numLoadedArcReplacements = 0;

        for ( uint32_t i = 0; i < num_arcchecks; i++ )
        {
            if ( allARC[i].arcFileIndex == fileIndex )
            {
                m_numLoadedArcReplacements++;
            }
        }

        // Allocate memory to the actual ARCChecks
        m_ArcReplacements = new ARCReplacement[m_numLoadedArcReplacements];

        // offset into m_DZXChecks
        uint32_t j = 0;

        for ( uint32_t i = 0; i < num_arcchecks; i++ )
        {
            if ( allARC[i].arcFileIndex == fileIndex )
            {
                // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
                memcpy( &m_ArcReplacements[j], &allARC[i], sizeof( ARCReplacement ) );
                j++;
            }
        }
    }

    void Seed::setArcIndex()
    {
        // Local vars
        uint32_t numReplacements = m_Header->arcCheckInfo.numEntries;
        uint32_t gci_offset = m_Header->arcCheckInfo.dataOffset;
        char filePath[32];
        int32_t len = 0;

        // Set the pointer as offset into our buffer
        ARCReplacement* allARC = reinterpret_cast<ARCReplacement*>( &m_GCIData[gci_offset] );
        uint32_t j = 0;

        // Loop through all ArcChecks and set their corresponding file index
        for ( uint32_t i = 0; i < numReplacements; i++ )
        {
            switch ( allARC[i].directory )
            {
                case rando::FileDirectory::Stage:
                {
                    len = snprintf( filePath, sizeof( filePath ), "res/Stage/%s", allARC[i].fileName );
                    break;
                }
                case rando::FileDirectory::Message:
                {
#ifdef TP_US
                    len = snprintf( filePath, sizeof( filePath ), "res/Msgus/%s", allARC[i].fileName );
#elif defined TP_JP
                    len = snprintf( filePath, sizeof( filePath ), "res/Msgjp/%s", allARC[i].fileName );
#elif defined TP_EU
                    // PAL uses a different file for each language
                    libtp::tp::d_s_logo::Languages lang = tp::d_s_logo::getPalLanguage2( nullptr );
                    if ( ( lang < tp::d_s_logo::Languages::uk ) || ( lang > tp::d_s_logo::Languages::it ) )
                    {
                        // The language is invalid/unsupported, so the game defaults to English
                        lang = tp::d_s_logo::Languages::uk;
                    }

                    static const char* langStrings[] = { "uk", "de", "fr", "sp", "it" };
                    len = snprintf( filePath,
                                    sizeof( filePath ),
                                    "res/Msg%s/%s",
                                    langStrings[static_cast<int32_t>( lang )],
                                    m_Seed->allARC[i].fileName );
#endif
                    break;
                }
                default:
                {
                    len = snprintf( filePath, sizeof( filePath ), "%s", allARC[i].fileName );
                }
            }

            if ( ( len >= 0 ) && ( len < static_cast<int32_t>( sizeof( filePath ) ) ) )
            {
                allARC[i].arcFileIndex = libtp::gc::dvdfs::DVDConvertPathToEntrynum( filePath );
            }
            else     // Failsafe in case we did not get a valid result.
            {
                allARC[i].arcFileIndex = -1;
            }
            memcpy( &m_ArcReplacements[j], &allARC[i], sizeof( ARCReplacement ) );
            j++;
        }
    }

}     // namespace mod::rando