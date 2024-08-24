/**	@file seed.cpp
 *  @brief Seed class to access seed-data
 *
 *	@author AECX
 *	@bug No known bugs.
 */

#include "rando/seed.h"

#include <cstdint>
#include <cstring>

#include "rando/seed.h"
#include "data/stages.h"
#include "gc_wii/card.h"
#include "main.h"
#include "rando/data.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_item.h"
#include "user_patch/user_patch.h"
#include "tp/d_item_data.h"
#include "tp/d_a_shop_item_static.h"
#include "game_patch/game_patch.h"
#include "rando/customItems.h"
#include "cxx.h"

namespace mod::rando
{
    KEEP_FUNC bool flagIsEnabled(const uint32_t* bitfieldPtr, uint32_t totalFlags, uint32_t flag)
    {
        // Make sure the flag is valid
        if (flag >= totalFlags)
        {
            return false;
        }

        constexpr uint32_t bitsPerWord = sizeof(uint32_t) * 8;
        return (bitfieldPtr[flag / bitsPerWord] >> (flag % bitsPerWord)) & 1U;
    }

    KEEP_FUNC bool Seed::flagBitfieldFlagIsEnabled(uint32_t flag) const
    {
        const EntryInfo* flagBitfieldPtr = this->getHeaderPtr()->getFlagBitfieldPtr();
        const uint32_t num_bytes = flagBitfieldPtr->getNumEntries();
        const uint32_t gci_offset = flagBitfieldPtr->getDataOffset();

        const uint32_t* bitfieldPtr = reinterpret_cast<const uint32_t*>(&this->m_GCIData[gci_offset]);
        return flagIsEnabled(bitfieldPtr, num_bytes, flag);
    }

    bool Seed::InitSeed(void)
    {
        // (Re)set counters & status
        this->m_AreaFlagsModified = 0;
        this->m_EventFlagsModified = 0;
        this->m_PatchesApplied = 0;

        // getConsole() << "Setting Event Flags... \n";
        this->applyEventFlags();

        // getConsole() << "Setting Region Flags... \n";
        this->applyRegionFlags();

        this->giveStartingItems();
        getConsole() << "Seed Applied Successfully.\n";
        return true;
    }

    void Seed::applyEventFlags()
    {
        using namespace libtp;

        const EntryInfo* eventFlagsInfoPtr = this->m_Header->getEventFlagsInfoPtr();
        const uint32_t num_eventFlags = eventFlagsInfoPtr->getNumEntries();
        const uint32_t gci_offset = eventFlagsInfoPtr->getDataOffset();

        if (num_eventFlags == 0)
        {
            return;
        }

        // Set the pointer as offset into our buffer
        const EventFlag* eventFlags = reinterpret_cast<const EventFlag*>(&this->m_GCIData[gci_offset]);
        uint8_t* mEventPtr = &tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.mEvent.mEvent[0];
        uint32_t eventFlagsModified = this->m_EventFlagsModified;

        for (uint32_t i = 0; i < num_eventFlags; i++)
        {
            const uint32_t offset = eventFlags[i].getOffset();
            const uint8_t flag = eventFlags[i].getFlag();

            mEventPtr[offset] |= flag;
            eventFlagsModified++;
        }

        this->m_EventFlagsModified = static_cast<uint16_t>(eventFlagsModified);
    }

    void Seed::applyRegionFlags()
    {
        using namespace libtp;

        const EntryInfo* regionFlagsInfoPtr = this->m_Header->getRegionFlagsInfoPtr();
        const uint32_t num_regionFlags = regionFlagsInfoPtr->getNumEntries();
        const uint32_t gci_offset = regionFlagsInfoPtr->getDataOffset();

        if (num_regionFlags == 0)
        {
            return;
        }

        tp::d_save::dSv_info_c* SaveInfo = &tp::d_com_inf_game::dComIfG_gameInfo.save;
        // tp::d_com_inf_game::dComIfG_inf_c* gameInfo = &tp::d_com_inf_game::dComIfG_gameInfo;

        // Set the pointer as offset into our buffer
        const RegionFlag* regionFlags = reinterpret_cast<const RegionFlag*>(&this->m_GCIData[gci_offset]);

        const uint8_t* regionIDPtr = &data::stage::regions[0];
        uint32_t areaFlagsModified = this->m_AreaFlagsModified;
        uint8_t* memoryFlagsPtr = &SaveInfo->memory.temp_flags.memoryFlags[0];

        // Loop through all regions
        for (uint32_t i = 0; i < sizeof(data::stage::regions); i++)
        {
            const uint8_t regionID = regionIDPtr[i];
            tp::d_save::getSave(SaveInfo, static_cast<int32_t>(regionID));

            // Loop through region-flags from GCI
            for (uint32_t j = 0; j < num_regionFlags; j++)
            {
                const RegionFlag* currentRegion = &regionFlags[j];
                if (currentRegion->getRegionId() == regionID)
                {
                    // The n'th bit that we want to set TRUE
                    const int32_t bit = static_cast<int32_t>(currentRegion->getBitId());

                    const int32_t offset = bit / 8;
                    const int32_t shift = bit % 8;

                    // Failsafe; localAreaNode size is 0x20
                    if (offset < 0x20)
                    {
                        memoryFlagsPtr[offset] |= (0x80 >> shift);
                        areaFlagsModified++;
                    }
                }
            }

            // Save the modifications
            tp::d_save::putSave(SaveInfo, regionID);
        }

        this->m_AreaFlagsModified = static_cast<uint16_t>(areaFlagsModified);
        tp::d_save::getSave(SaveInfo, data::stage::regions[2]);
    }

    void Seed::giveStartingItems()
    {
        using namespace libtp;

        const EntryInfo* startingItemInfoPtr = this->m_Header->getStartingItemCheckInfoPtr();
        const uint32_t num_startingItems = startingItemInfoPtr->getNumEntries();
        const uint32_t gci_offset = startingItemInfoPtr->getDataOffset();

        if (num_startingItems == 0)
        {
            return;
        }

        // Set the pointer as offset into our buffer
        const uint8_t* startingItems = &this->m_GCIData[gci_offset];

        for (uint32_t i = 0; i < num_startingItems; i++)
        {
            libtp::tp::d_item::execItemGet(startingItems[i]);
        }
    }

    bool Seed::LoadChecks(const char* stage)
    {
        using namespace libtp;

        // Find the index of this stage
        uint8_t stageIDX;
        const auto stagesPtr = &data::stage::allStages[0];
        constexpr uint32_t stageCount = sizeof(data::stage::allStages) / sizeof(data::stage::allStages[0]);

        for (stageIDX = 0; stageIDX < stageCount; stageIDX++)
        {
            if (!strcmp(stage, stagesPtr[stageIDX]))
            {
                break;
            }
        }

        // Don't run if this isn't actually a new stage
        const bool result = stageIDX != this->m_StageIDX;
        if (result)
        {
            this->clearChecks();

            this->LoadDZX(stageIDX);
            this->LoadREL(stageIDX);
            this->LoadPOE(stageIDX);
            this->LoadBOSS(stageIDX);
            this->LoadBugReward();
            this->LoadSkyCharacter(stageIDX);
            this->LoadHiddenSkill();
            this->LoadEventChecks(stageIDX);

            // Save current stageIDX for next time
            this->m_StageIDX = stageIDX;
        }

        return result;
    }

    void Seed::clearChecks()
    {
        this->m_NumLoadedDZXChecks = 0;
        this->m_NumLoadedRELChecks = 0;
        this->m_NumLoadedPOEChecks = 0;
        this->m_NumBugRewardChecks = 0;
        this->m_NumSkyBookChecks = 0;
        this->m_NumHiddenSkillChecks = 0;
        this->m_NumLoadedEventChecks = 0;

        if (this->m_DZXChecks)
        {
            delete[] this->m_DZXChecks;
        }

        if (this->m_RELChecks)
        {
            delete[] this->m_RELChecks;
        }

        if (this->m_PoeChecks)
        {
            delete[] this->m_PoeChecks;
        }

        if (this->m_BossChecks)
        {
            delete[] this->m_BossChecks;
        }

        if (this->m_BugRewardChecks)
        {
            delete[] this->m_BugRewardChecks;
        }

        if (this->m_SkyBookChecks)
        {
            delete[] this->m_SkyBookChecks;
        }

        if (this->m_HiddenSkillChecks)
        {
            delete[] this->m_HiddenSkillChecks;
        }

        if (this->m_EventChecks)
        {
            delete[] this->m_EventChecks;
        }
    }

    void Seed::LoadDZX(uint8_t stageIDX)
    {
        using namespace libtp;

        const EntryInfo* dzxCheckInfoPtr = this->m_Header->getDzxCheckInfoPtr();
        const uint32_t num_dzxchecks = dzxCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = dzxCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const DZXCheck* allDZX = reinterpret_cast<const DZXCheck*>(&this->m_GCIData[gci_offset]);

        uint32_t numLoadedDZXChecks = this->m_NumLoadedDZXChecks;
        for (uint32_t i = 0; i < num_dzxchecks; i++)
        {
            if (allDZX[i].getStageIDX() == stageIDX)
            {
                numLoadedDZXChecks++;
            }
        }
        this->m_NumLoadedDZXChecks = static_cast<uint16_t>(numLoadedDZXChecks);

        // Allocate memory the actual DZXChecks
        // We do NOT have to clear the previous buffer as that's already done in "LoadChecks()"
        DZXCheck* dzxChecksPtr = new DZXCheck[numLoadedDZXChecks];
        this->m_DZXChecks = dzxChecksPtr;

        // offset into m_DZXChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_dzxchecks; i++)
        {
            const DZXCheck* currentCheck = &allDZX[i];
            DZXCheck* globalCheck = &dzxChecksPtr[j];

            if (currentCheck->getStageIDX() == stageIDX)
            {
                // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
                memcpy(globalCheck, currentCheck, sizeof(DZXCheck));
                j++;
            }
        }
    }

    void Seed::LoadREL(uint8_t stageIDX)
    {
        using namespace libtp;

        const EntryInfo* relCheckInfoPtr = this->m_Header->getRelCheckInfoPtr();
        const uint32_t num_relchecks = relCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = relCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const RELCheck* allREL = reinterpret_cast<const RELCheck*>(&this->m_GCIData[gci_offset]);

        uint32_t numLoadedRELChecks = this->m_NumLoadedRELChecks;
        for (uint32_t i = 0; i < num_relchecks; i++)
        {
            const uint32_t currentStageIdx = allREL[i].getStageIDX();

            // Check if we are on the current stage, or if the rel override needs to be applied to every stage
            if ((currentStageIdx == stageIDX) || (currentStageIdx == 0xFF))
            {
                numLoadedRELChecks++;
            }
        }
        this->m_NumLoadedRELChecks = static_cast<uint16_t>(numLoadedRELChecks);

        // Allocate memory to the actual RELChecks
        // Do NOT need to clear the previous buffer as that's taken care of by LoadChecks()
        RELCheck* relChecksPtr = new RELCheck[numLoadedRELChecks];
        this->m_RELChecks = relChecksPtr;

        // offset into m_RELChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_relchecks; i++)
        {
            const RELCheck* currentRel = &allREL[i];
            RELCheck* globalRel = &relChecksPtr[j];
            const uint32_t currentStageIdx = currentRel->getStageIDX();

            // Check if we are on the current stage, or if the rel override needs to be applied to every stage
            if ((currentStageIdx == stageIDX) || (currentStageIdx == 0xFF))
            {
                // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
                memcpy(globalRel, currentRel, sizeof(RELCheck));
                j++;
            }
        }
    }

    void Seed::LoadPOE(uint8_t stageIDX)
    {
        using namespace libtp;

        const EntryInfo* poeCheckInfoPtr = this->m_Header->getPoeCheckInfoPtr();
        const uint32_t num_poechecks = poeCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = poeCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const PoeCheck* allPOE = reinterpret_cast<const PoeCheck*>(&this->m_GCIData[gci_offset]);

        uint32_t numLoadedPOEChecks = this->m_NumLoadedPOEChecks;
        for (uint32_t i = 0; i < num_poechecks; i++)
        {
            if (allPOE[i].getStageIDX() == stageIDX)
            {
                numLoadedPOEChecks++;
            }
        }
        this->m_NumLoadedPOEChecks = static_cast<uint16_t>(numLoadedPOEChecks);

        // Allocate memory to the actual POEChecks
        // Do NOT need to clear the previous buffer as that's taken care of by LoadChecks()
        PoeCheck* poeChecksPtr = new PoeCheck[numLoadedPOEChecks];
        this->m_PoeChecks = poeChecksPtr;

        // offset into m_PoeChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_poechecks; i++)
        {
            const PoeCheck* currentPoeCheck = &allPOE[i];
            PoeCheck* globalPoeCheck = &poeChecksPtr[j];

            if (currentPoeCheck->getStageIDX() == stageIDX)
            {
                // Store the i'th POE check into the j'th Loaded POEcheck that's relevant to our current stage
                memcpy(globalPoeCheck, currentPoeCheck, sizeof(PoeCheck));
                j++;
            }
        }
    }

    void Seed::LoadHiddenSkill()
    {
        using namespace libtp;

        const EntryInfo* hiddenSkillCheckInfoPtr = this->m_Header->getHiddenSkillCheckInfoPtr();
        const uint32_t num_hiddenSkills = hiddenSkillCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = hiddenSkillCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        HiddenSkillCheck* hiddenSkillChecksPtr = new HiddenSkillCheck[num_hiddenSkills];
        this->m_HiddenSkillChecks = hiddenSkillChecksPtr;
        const HiddenSkillCheck* allSKILL = reinterpret_cast<const HiddenSkillCheck*>(&this->m_GCIData[gci_offset]);

        // Offset into m_HiddenSkillChecks
        uint32_t j = 0;

        uint32_t numHiddenSkillChecks = this->m_NumHiddenSkillChecks;
        for (uint32_t i = 0; i < num_hiddenSkills; i++)
        {
            const HiddenSkillCheck* currentSkillCheck = &allSKILL[i];
            HiddenSkillCheck* globalSkillCheck = &hiddenSkillChecksPtr[j];

            // Store the i'th DZX check into the j'th Loaded DZX check that's relevant to our current stage
            memcpy(globalSkillCheck, currentSkillCheck, sizeof(HiddenSkillCheck));
            j++;
            numHiddenSkillChecks++;
        }
        this->m_NumHiddenSkillChecks = static_cast<uint16_t>(numHiddenSkillChecks);
    }

    void Seed::LoadBugReward()
    {
        using namespace libtp;

        const EntryInfo* bugRewardCheckInfoPtr = this->m_Header->getBugRewardCheckInfoPtr();
        const uint32_t num_bugRewards = bugRewardCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = bugRewardCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const BugReward* allBUG = reinterpret_cast<const BugReward*>(&this->m_GCIData[gci_offset]);

        uint32_t numBugRewardChecks = this->m_NumBugRewardChecks;
        for (uint32_t i = 0; i < num_bugRewards; i++)
        {
            numBugRewardChecks++;
        }
        this->m_NumBugRewardChecks = static_cast<uint16_t>(numBugRewardChecks);

        // Allocate memory to the actual POEChecks
        // Do NOT need to clear the previous buffer as that's taken care of by LoadChecks()
        BugReward* bugRewardChecksPtr = new BugReward[numBugRewardChecks];
        this->m_BugRewardChecks = bugRewardChecksPtr;

        // offset into m_BugRewardChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_bugRewards; i++)
        {
            const BugReward* currentBugCheck = &allBUG[i];
            BugReward* globalBugCheck = &bugRewardChecksPtr[j];

            memcpy(globalBugCheck, currentBugCheck, sizeof(BugReward));
            j++;
        }
    }

    void Seed::LoadSkyCharacter(uint8_t stageIDX)
    {
        const EntryInfo* skyCharacterCheckInfoPtr = this->m_Header->getSkyCharacterCheckInfoPtr();
        const uint32_t num_skycharacters = skyCharacterCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = skyCharacterCheckInfoPtr->getDataOffset();

        SkyCharacter* skyBookChecksPtr = new SkyCharacter[num_skycharacters];
        this->m_SkyBookChecks = skyBookChecksPtr;
        const SkyCharacter* allCHAR = reinterpret_cast<const SkyCharacter*>(&this->m_GCIData[gci_offset]);

        // offset into m_SkyBookChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_skycharacters; i++)
        {
            const SkyCharacter* currentCharCheck = &allCHAR[i];
            SkyCharacter* globalCharCheck = &skyBookChecksPtr[j];

            uint32_t numSkyBookChecks = this->m_NumSkyBookChecks;
            if ((currentCharCheck->getStageIDX() == stageIDX))
            {
                memcpy(globalCharCheck, currentCharCheck, sizeof(SkyCharacter));
                numSkyBookChecks++;
                j++;
            }
            this->m_NumSkyBookChecks = static_cast<uint16_t>(numSkyBookChecks);
        }
    }

    void Seed::LoadBOSS(uint8_t stageIDX)
    {
        using namespace libtp;

        const EntryInfo* bossCheckInfoPtr = this->m_Header->getBossCheckInfoPtr();
        const uint32_t num_bossChecks = bossCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = bossCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        BossCheck* bossChecksPtr = new BossCheck[1];
        this->m_BossChecks = bossChecksPtr;
        const BossCheck* allBOSS = reinterpret_cast<const BossCheck*>(&this->m_GCIData[gci_offset]);

        // There is only one BOSS check per stage. Once we have a match, we are done.
        for (uint32_t i = 0; i < num_bossChecks; i++)
        {
            const BossCheck* currentBossCheck = &allBOSS[i];
            if (currentBossCheck->getStageIDX() == stageIDX)
            {
                memcpy(bossChecksPtr, currentBossCheck, sizeof(BossCheck));
                break;
            }
        }
    }

    void Seed::LoadEventChecks(uint8_t stageIDX)
    {
        using namespace libtp;

        const EntryInfo* eventItemCheckInfoPtr = this->m_Header->getEventItemCheckInfoPtr();
        const uint32_t num_eventchecks = eventItemCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = eventItemCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        EventItem* eventChecksPtr = new EventItem[num_eventchecks];
        this->m_EventChecks = eventChecksPtr;
        const EventItem* allEvent = reinterpret_cast<const EventItem*>(&this->m_GCIData[gci_offset]);

        // offset into m_EventChecks
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_eventchecks; i++)
        {
            const EventItem* currentEventCheck = &allEvent[i];
            EventItem* globalEventCheck = &eventChecksPtr[j];

            uint32_t numEventChecks = this->m_NumLoadedEventChecks;
            if ((currentEventCheck->getStageIDX() == stageIDX))
            {
                memcpy(globalEventCheck, currentEventCheck, sizeof(EventItem));
                numEventChecks++;
                j++;
            }
            this->m_NumLoadedEventChecks = static_cast<uint16_t>(numEventChecks);
        }
    }

    void Seed::LoadARCChecks(uint8_t stageIDX, FileDirectory fileDirectory, int32_t roomNo)
    {
        using namespace libtp;

        // Until a better way is found, we are going to clear the buffer here just to be safe
        if (this->m_ArcReplacements)
        {
            delete[] this->m_ArcReplacements;
        }

        const EntryInfo* arcCheckInfoPtr = this->m_Header->getArcCheckInfoPtr();
        const uint32_t num_arcchecks = arcCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = arcCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const ARCReplacement* allARC = reinterpret_cast<const ARCReplacement*>(&this->m_GCIData[gci_offset]);

        uint32_t numLoadedArcReplacements = 0;
        for (uint32_t i = 0; i < num_arcchecks; i++)
        {
            const ARCReplacement* currentArcCheck = &allARC[i];
            if ((currentArcCheck->getStageIDX() == stageIDX) && (currentArcCheck->getDirectory() == fileDirectory))
            {
                numLoadedArcReplacements++;
            }
        }
        this->m_NumLoadedArcReplacements = static_cast<uint16_t>(numLoadedArcReplacements);

        // Allocate memory to the actual ARCChecks
        ARCReplacement* arcReplacementsPtr = new ARCReplacement[numLoadedArcReplacements];
        this->m_ArcReplacements = arcReplacementsPtr;

        // offset into m_ArcReplacements
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_arcchecks; i++)
        {
            const ARCReplacement* currentArcCheck = &allARC[i];
            ARCReplacement* globalArcCheck = &arcReplacementsPtr[j];

            switch (fileDirectory)
            {
                case rando::FileDirectory::Room:
                {
                    if ((currentArcCheck->getStageIDX() == stageIDX) && (currentArcCheck->getDirectory() == fileDirectory) &&
                        (currentArcCheck->getRoomID() == roomNo))
                    {
                        memcpy(globalArcCheck, currentArcCheck, sizeof(ARCReplacement));
                        j++;
                    }
                    break;
                }

                default:
                {
                    if ((currentArcCheck->getStageIDX() == stageIDX) && (currentArcCheck->getDirectory() == fileDirectory))
                    {
                        memcpy(globalArcCheck, currentArcCheck, sizeof(ARCReplacement));
                        j++;
                    }
                    break;
                }
            }
        }
    }

    void Seed::LoadObjectARCChecks()
    {
        using namespace libtp;

        // Until a better way is found, we are going to clear the buffer here just to be safe
        if (this->m_ObjectArcReplacements)
        {
            delete[] this->m_ObjectArcReplacements;
        }

        const EntryInfo* objectArcCheckInfoPtr = this->m_Header->getObjectArcCheckInfoPtr();
        const uint32_t num_objarcchecks = objectArcCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = objectArcCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const ObjectArchiveReplacement* allARC =
            reinterpret_cast<const ObjectArchiveReplacement*>(&this->m_GCIData[gci_offset]);

        const uint32_t stageIdx = this->m_StageIDX;

        uint32_t numLoadedObjectArcReplacements = 0;
        for (uint32_t i = 0; i < num_objarcchecks; i++)
        {
            if ((allARC[i].getStageIDX() == stageIdx))
            {
                numLoadedObjectArcReplacements++;
            }
        }
        this->m_NumLoadedObjectArcReplacements = static_cast<uint16_t>(numLoadedObjectArcReplacements);

        // Allocate memory to the actual ARCChecks
        ObjectArchiveReplacement* objectArcReplacementsPtr = new ObjectArchiveReplacement[numLoadedObjectArcReplacements];
        this->m_ObjectArcReplacements = objectArcReplacementsPtr;

        // offset into m_ArcReplacements
        uint32_t j = 0;

        for (uint32_t i = 0; i < num_objarcchecks; i++)
        {
            const ObjectArchiveReplacement* currentObjectArcReplacement = &allARC[i];
            ObjectArchiveReplacement* globalObjectArcReplacement = &objectArcReplacementsPtr[j];

            if ((currentObjectArcReplacement->getStageIDX() == stageIdx))
            {
                memcpy(globalObjectArcReplacement, currentObjectArcReplacement, sizeof(ObjectArchiveReplacement));
                j++;
            }
        }
    }

    void Seed::applyVolatilePatches()
    {
        const EntryInfo* volatilePatchInfoPtr = this->getHeaderPtr()->getVolatilePatchInfoPtr();
        const uint32_t num_bytes = volatilePatchInfoPtr->getNumEntries();
        const uint32_t gci_offset = volatilePatchInfoPtr->getDataOffset();
        const uint32_t* bitfieldPtr = reinterpret_cast<const uint32_t*>(&this->m_GCIData[gci_offset]);

        constexpr uint32_t totalVolatilePatches = sizeof(user_patch::volatilePatches) / sizeof(user_patch::volatilePatches[0]);
        uint32_t patchesApplied = this->m_PatchesApplied;

        for (uint32_t i = 0; i < totalVolatilePatches; i++)
        {
            if (flagIsEnabled(bitfieldPtr, num_bytes, i))
            {
                user_patch::volatilePatches[i](rando::gRandomizer);
                patchesApplied++;
            }
        }

        this->m_PatchesApplied = static_cast<uint16_t>(patchesApplied);
    }

    void Seed::loadShopModels()
    {
        using namespace libtp::tp;
        using namespace libtp::data;
        using namespace libtp::tp::d_a_shop_item_static;

        const EntryInfo* shopItemCheckInfoPtr = this->m_Header->getShopItemCheckInfoPtr();
        const uint32_t num_shopItems = shopItemCheckInfoPtr->getNumEntries();
        const uint32_t gci_offset = shopItemCheckInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const ShopCheck* allSHOP = reinterpret_cast<const ShopCheck*>(&this->m_GCIData[gci_offset]);

        d_item_data::ItemResource* itemResourcePtr = &d_item_data::item_resource[0];

        uint8_t foolishModelIndexes[MAX_SPAWNED_FOOLISH_ITEMS];
        uint32_t foolishModelCounter = 0;
        const uint32_t stageIdx = this->m_StageIDX;

        for (uint32_t i = 0; i < num_shopItems; i++)
        {
            const ShopCheck* currentShopCheck = &allSHOP[i];

            const uint32_t replacementItem =
                game_patch::_04_verifyProgressiveItem(gRandomizer, currentShopCheck->getReplacementItemID());

            const uint32_t shopItem = currentShopCheck->getShopItemID();
            ShopItemData* currentShopItemDataPtr = &shopItemData[shopItem];

            switch (replacementItem)
            {
                // Only the first foolish item should need to be checked, but check all to be safe
                case rando::customItems::Foolish_Item_1:
                case rando::customItems::Foolish_Item_2:
                case rando::customItems::Foolish_Item_3:
                case rando::customItems::Foolish_Item_4:
                case rando::customItems::Foolish_Item_5:
                case rando::customItems::Foolish_Item_6:
                {
                    game_patch::_02_modifyFoolishShopModel(foolishModelIndexes, foolishModelCounter, shopItem);
                    foolishModelCounter++;

                    // If the end of the array is reached, then reset the counter, as there should only ever be a certain amount
                    // of ice traps in any given shop
                    if (foolishModelCounter >= sizeof(foolishModelIndexes))
                    {
                        foolishModelCounter = 0;
                    }
                    break;
                }
                default:
                {
                    d_item_data::ItemResource* currentItemResourcePtr = &itemResourcePtr[replacementItem];

                    currentShopItemDataPtr->arcName = currentItemResourcePtr->arcName;
                    currentShopItemDataPtr->modelResIdx = currentItemResourcePtr->modelResIdx;
                    currentShopItemDataPtr->wBtkResIdx = currentItemResourcePtr->btkResIdx;
                    currentShopItemDataPtr->wBckResIdx = currentItemResourcePtr->bckResIdx;
                    currentShopItemDataPtr->wBrkResIdx = currentItemResourcePtr->brkResIdx;
                    currentShopItemDataPtr->wBtpResIdx = currentItemResourcePtr->btpResIdx;
                    currentShopItemDataPtr->tevFrm = currentItemResourcePtr->tevFrm;

                    break;
                }
            }

            if (shopItem == 0x13) // Magic Armor
            {
                switch (replacementItem)
                {
                    case items::Green_Rupee:
                    case items::Blue_Rupee:
                    case items::Yellow_Rupee:
                    case items::Red_Rupee:
                    case items::Purple_Rupee:
                    case items::Orange_Rupee:
                    case items::Silver_Rupee:
                    case items::Purple_Rupee_Links_House:
                    {
                        currentShopItemDataPtr->posY = 65.f;
                        break;
                    }
                    default:
                    {
                        currentShopItemDataPtr->posY = 60.f;
                        break;
                    }
                }
            }
            else if (shopItem == 0x2) // Red Potion
            {
                if (stageIdx != libtp::data::stage::Kakariko_Village_Interiors)
                {
                    d_item_data::ItemResource* currentItemResourcePtr = &itemResourcePtr[libtp::data::items::Red_Potion_Shop];

                    currentShopItemDataPtr->arcName = currentItemResourcePtr->arcName;
                    currentShopItemDataPtr->modelResIdx = currentItemResourcePtr->modelResIdx;
                    currentShopItemDataPtr->wBtkResIdx = currentItemResourcePtr->btkResIdx;
                    currentShopItemDataPtr->wBckResIdx = currentItemResourcePtr->bckResIdx;
                    currentShopItemDataPtr->wBrkResIdx = currentItemResourcePtr->brkResIdx;
                    currentShopItemDataPtr->wBtpResIdx = currentItemResourcePtr->btpResIdx;
                    currentShopItemDataPtr->tevFrm = currentItemResourcePtr->tevFrm;
                    currentShopItemDataPtr->posY = 15.f;
                }
            }
            else if (shopItem == 0x6) // Wooden Shield
            {
                if (stageIdx != libtp::data::stage::Kakariko_Village_Interiors)
                {
                    d_item_data::ItemResource* currentItemResourcePtr = &itemResourcePtr[libtp::data::items::Wooden_Shield];

                    currentShopItemDataPtr->arcName = currentItemResourcePtr->arcName;
                    currentShopItemDataPtr->modelResIdx = currentItemResourcePtr->modelResIdx;
                    currentShopItemDataPtr->wBtkResIdx = currentItemResourcePtr->btkResIdx;
                    currentShopItemDataPtr->wBckResIdx = currentItemResourcePtr->bckResIdx;
                    currentShopItemDataPtr->wBrkResIdx = currentItemResourcePtr->brkResIdx;
                    currentShopItemDataPtr->wBtpResIdx = currentItemResourcePtr->btpResIdx;
                    currentShopItemDataPtr->tevFrm = currentItemResourcePtr->tevFrm;
                    currentShopItemDataPtr->posY = 15.f;
                }
            }
            else
            {
                currentShopItemDataPtr->posY = 15.f;
            }

            currentShopItemDataPtr->btpFrm = 0xFF;
            currentShopItemDataPtr->mFlags = 0xFFFFFFFF;

            game_patch::_02_modifyShopModelScale(shopItem, replacementItem);

            // Clear the cache for the modified values
            libtp::gc_wii::os_cache::DCFlushRange(currentShopItemDataPtr,
                                                  sizeof(libtp::tp::d_a_shop_item_static::ShopItemData));
        }
    }

} // namespace mod::rando