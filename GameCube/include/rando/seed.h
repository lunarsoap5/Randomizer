/**	@file seed.h
 *  @brief Seed class to access seed-data
 *
 *	@author AECX
 *	@bug No known bugs.
 */
#ifndef RANDO_SEED_H
#define RANDO_SEED_H

#include <cstdint>
#include <cstring>

#ifdef DVD
#include "gc_wii/dvd.h"
#else
#include "gc_wii/card.h"
#endif

#include "rando/data.h"
#include "tools.h"
#include "data.h"

#define SEED_MAGIC_STRING "TPR"

namespace mod::rando
{
    enum SeedEnabledFlag
    {
        TRANSFORM_ANYWHERE = 0,
        QUICK_TRANSFORM,
        INCREASE_SPINNER_SPEED,
        BONKS_DO_DAMAGE,
        INCREASE_WALLETS,
        MODIFY_SHOP_MODELS,
    };

    // Function for checking if specific bits in various bitfields are enabled. Currently used for volatilePatchInfo,
    // oneTimePatchInfo, and flagBitfieldInfo.
    bool flagIsEnabled(const uint32_t* bitfieldPtr, uint32_t totalFlags, uint32_t flag);

    /**
     *  @brief Optional functions that have to be executed once and patch/modify the game code
     */
    class EntryInfo
    {
       public:
        EntryInfo() {}
        ~EntryInfo() {}

        uint16_t getNumEntries() const { return this->numEntries; }
        uint16_t getDataOffset() const { return this->dataOffset; }

       private:
        uint16_t numEntries;
        uint16_t dataOffset;
    } __attribute__((__packed__));

    class Header
    {
       public:
        Header(const void* fileData) { memcpy(this, fileData, sizeof(Header)); }
        ~Header() {}

        bool magicIsValid() const
        {
            // Use memcmp just in case the magic gets updated to have null character(s) in it for some reason
            return memcmp(&this->magic[0], SEED_MAGIC_STRING, sizeof(this->magic)) == 0;
        }

        bool versionIsValid() const
        {
            // The major and minor seed versions use 2 bytes each, so merge both into a single 4 byte variable
            constexpr uint32_t supportedSeedVersion = (_VERSION_MAJOR << 16) | _VERSION_MINOR;
            return this->version == supportedSeedVersion;
        }

        const char* getSeedNamePtr() const { return &this->seedName[0]; }
        uint16_t getHeaderSize() const { return this->headerSize; }
        uint16_t getDataSize() const { return this->dataSize; }
        uint32_t getTotalSize() const { return this->totalSize; }

        uint16_t getClr0Offset() const { return this->clr0Offset; }
        uint16_t getCustomTextHeaderSize() const { return this->customTextHeaderSize; }
        uint16_t getCustomTextHeaderOffset() const { return this->customTextHeaderOffset; }

        uint8_t getCastleRequirements() const { return this->castleRequirements; }
        uint8_t getPalaceRequirements() const { return this->palaceRequirements; }
        uint8_t getMapClearBits() const { return this->mapClearBits; }
        uint8_t getDamageMagnification() const { return this->damageMagnification; }
        uint8_t getStartingTimeOfDay() const { return this->startingTimeOfDay; }
        uint8_t getToTSwordRequirement() const { return this->totSwordRequirement; }

        const EntryInfo* getVolatilePatchInfoPtr() const { return &this->volatilePatchInfo; }
        const EntryInfo* getOneTimePatchInfoPtr() const { return &this->oneTimePatchInfo; }
        const EntryInfo* getFlagBitfieldPtr() const { return &this->flagBitfieldInfo; }
        const EntryInfo* getEventFlagsInfoPtr() const { return &this->eventFlagsInfo; }
        const EntryInfo* getRegionFlagsInfoPtr() const { return &this->regionFlagsInfo; }
        const EntryInfo* getDzxCheckInfoPtr() const { return &this->dzxCheckInfo; }
        const EntryInfo* getRelCheckInfoPtr() const { return &this->relCheckInfo; }
        const EntryInfo* getPoeCheckInfoPtr() const { return &this->poeCheckInfo; }
        const EntryInfo* getArcCheckInfoPtr() const { return &this->arcCheckInfo; }
        const EntryInfo* getObjectArcCheckInfoPtr() const { return &this->objectArcCheckInfo; }
        const EntryInfo* getBossCheckInfoPtr() const { return &this->bossCheckInfo; }
        const EntryInfo* getHiddenSkillCheckInfoPtr() const { return &this->hiddenSkillCheckInfo; }
        const EntryInfo* getBugRewardCheckInfoPtr() const { return &this->bugRewardCheckInfo; }
        const EntryInfo* getSkyCharacterCheckInfoPtr() const { return &this->skyCharacterCheckInfo; }
        const EntryInfo* getShopItemCheckInfoPtr() const { return &this->shopItemCheckInfo; }
        const EntryInfo* getEventItemCheckInfoPtr() const { return &this->eventItemCheckInfo; }
        const EntryInfo* getStartingItemCheckInfoPtr() const { return &this->startingItemInfo; }
        const EntryInfo* getEntranceTableCheckInfoPtr() const { return &this->entranceTableInfo; }
        const EntryInfo* getBgmTableInfoPtr() const { return &this->bgmTableInfo; }
        const EntryInfo* getFanfareTableInfoPtr() const { return &this->fanfareTableInfo; }

       private:
        /* 0x00 */ char magic[3]; // Not null terminated, should always be TPR
        /* 0x03 */ char seedName[33];

        // SeedData version major and minor; uint16_t for each. Need to handle as a single variable to get around a compiler
        // warning about comparing an unsigned value to 0
        /* 0x24 */ uint32_t version;

        /* 0x28 */ uint16_t headerSize; // Total size of the header in bytes
        /* 0x2A */ uint16_t dataSize;   // Total number of bytes of seed data
        /* 0x2C */ uint32_t totalSize;  // Total number of bytes in the GCI

        // BitArray where each bit represents a patch/modification to be applied for this playthrough; these
        // patchs/modifications must be applied every time a file is loaded
        /* 0x30 */ EntryInfo volatilePatchInfo;

        // BitArray where each bit represents a patch/modification to be applied for this playthrough; these
        // patchs/modifications must be applied only when a seed is loaded
        /* 0x34 */ EntryInfo oneTimePatchInfo;

        // BitArray where each bit represents an arbitrary flag indicated by the SeedEnabledFlag enum
        /* 0x38 */ EntryInfo flagBitfieldInfo;

        /* 0x3C */ EntryInfo eventFlagsInfo;  // eventFlags that need to be set for this seed
        /* 0x40 */ EntryInfo regionFlagsInfo; // regionFlags that need to be set, alternating
        /* 0x44 */ EntryInfo dzxCheckInfo;
        /* 0x48 */ EntryInfo relCheckInfo;
        /* 0x4C */ EntryInfo poeCheckInfo;
        /* 0x50 */ EntryInfo arcCheckInfo;
        /* 0x54 */ EntryInfo objectArcCheckInfo;
        /* 0x58 */ EntryInfo bossCheckInfo;
        /* 0x5C */ EntryInfo hiddenSkillCheckInfo;
        /* 0x60 */ EntryInfo bugRewardCheckInfo;
        /* 0x64 */ EntryInfo skyCharacterCheckInfo;
        /* 0x68 */ EntryInfo shopItemCheckInfo;
        /* 0x6C */ EntryInfo eventItemCheckInfo;
        /* 0x70 */ EntryInfo startingItemInfo;
        /* 0x74 */ EntryInfo entranceTableInfo;
        /* 0x78 */ EntryInfo bgmTableInfo;
        /* 0x7C */ EntryInfo fanfareTableInfo;
        /* 0x80 */ EntryInfo sfxTableInfo;
        /* 0x84 */ uint16_t clr0Offset;
        /* 0x86 */ uint16_t customTextHeaderSize;
        /* 0x88 */ uint16_t customTextHeaderOffset;
        /* 0x8A */ uint8_t castleRequirements;
        /* 0x8B */ uint8_t palaceRequirements;
        /* 0x8C */ uint8_t mapClearBits;
        /* 0x8D */ uint8_t damageMagnification;
        /* 0x8E */ uint8_t startingTimeOfDay;
        /* 0x8F */ uint8_t totSwordRequirement;
    } __attribute__((__packed__));

    class Seed
    {
       public:
        /**
         *  @brief Class to dynamically load required data from a given seed
         *
         *  @param chan The memory card slot to load from
         */
#ifdef DVD
        Seed();
#else
        Seed(int32_t chan);
#endif

        ~Seed();

        /**
         *  @brief Applies patches, event & region flags according from this seed to the current savefile
         */
        bool InitSeed();

        /**
         *  @brief Load check data for a given stage & stores them in a temp. (smaller) buffer
         *
         *  @return True if checks have been updated (new stage) otherwise we are still on the same stage
         */
        bool LoadChecks(const char* stage);

        void LoadARCChecks(uint8_t stageIDX, FileDirectory fileDirectory, int32_t roomNo);
        void LoadObjectARCChecks();

        /**
         *  @brief Manages game_patches from the seed that must occur every time a file is loaded
         */
        void applyVolatilePatches();

        /**
         *  @brief Manages one-time game_patches from the seed
         */
        void applyOneTimePatches();

        const Header* getHeaderPtr() const { return this->m_Header; }
        bool seedIsLoaded() const { return this->m_GCIData; }

        const DZXCheck* getDZXChecksPtr() const { return this->m_DZXChecks; }
        const RELCheck* getRELChecksPtr() const { return this->m_RELChecks; }
        const PoeCheck* getPoeChecksPtr() const { return this->m_PoeChecks; }
        const ARCReplacement* getArcReplacementsPtr() const { return this->m_ArcReplacements; }
        const BossCheck* getBossChecksPtr() const { return this->m_BossChecks; }
        const HiddenSkillCheck* getHiddenSkillChecksPtr() const { return this->m_HiddenSkillChecks; }
        const BugReward* getBugRewardChecksPtr() const { return this->m_BugRewardChecks; }
        const SkyCharacter* getSkyBookChecksPtr() const { return this->m_SkyBookChecks; }
        const ObjectArchiveReplacement* getObjectArcReplacementsPtr() const { return this->m_ObjectArcReplacements; }
        const EventItem* getEventChecksPtr() const { return this->m_EventChecks; }

        const ShuffledEntrance* getShuffledEntrancesPtr() const { return this->m_ShuffledEntrances; }
        const BGMReplacement* getBgmTablePtr() const { return this->m_BgmTable; }
        const BGMReplacement* getFanfareTablePtr() const { return this->m_FanfareTable; }
        const CLR0Header* getCLR0Ptr() const { return this->m_CLR0; }
        const RawRGBTable* getRawRGBTablePtr() const { return this->m_RawRGBTable; }
        const BMDEntry* getBmdEntriesPtr() const { return this->m_BmdEntries; }

        uint16_t getNumLoadedDZXChecks() const { return this->m_NumLoadedDZXChecks; }
        uint16_t getNumLoadedRELChecks() const { return this->m_NumLoadedRELChecks; }
        uint16_t getNumLoadedPOEChecks() const { return this->m_NumLoadedPOEChecks; }
        uint16_t getNumLoadedArcReplacements() const { return this->m_NumLoadedArcReplacements; }
        uint16_t getNumHiddenSkillChecks() const { return this->m_NumHiddenSkillChecks; }
        uint16_t getNumBugRewardChecks() const { return this->m_NumBugRewardChecks; }
        uint16_t getNumSkyBookChecks() const { return this->m_NumSkyBookChecks; }
        uint16_t getNumLoadedEventChecks() const { return this->m_NumLoadedEventChecks; }
        uint16_t getNumLoadedObjectArcReplacements() const { return this->m_NumLoadedObjectArcReplacements; }
        uint16_t getNumShuffledEntrances() const { return this->m_NumShuffledEntrances; }
        uint16_t getNumShuffledBgmTracks() const { return this->m_BgmTableEntries; }
        uint16_t getNumShuffledFanfares() const { return this->m_FanfareTableEntries; }

        uint8_t getStageIDX() const { return this->m_StageIDX; }

        bool flagBitfieldFlagIsEnabled(uint32_t flag) const;

        bool canTransformAnywhere() const { return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::TRANSFORM_ANYWHERE); }
        bool canQuickTransform() const { return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::QUICK_TRANSFORM); }
        bool bonksDoDamage() const { return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::BONKS_DO_DAMAGE); }
        bool walletsAreIncreased() const { return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::INCREASE_WALLETS); }
        bool shopModelsAreModified() const { return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::MODIFY_SHOP_MODELS); }

        bool spinnerSpeedIsIncreased() const
        {
            return this->flagBitfieldFlagIsEnabled(SeedEnabledFlag::INCREASE_SPINNER_SPEED);
        }

        void loadShopModels();
        void loadShuffledEntrances();
        void clearChecks();
        void loadBgmData();

       private:
        void applyEventFlags();
        void applyRegionFlags();
        void giveStartingItems();

        void LoadDZX(uint8_t stageIDX);
        void LoadREL(uint8_t stageIDX);
        void LoadPOE(uint8_t stageIDX);
        void LoadBOSS(uint8_t stageIDX);
        void LoadSkyCharacter(uint8_t stageIDX);
        void LoadHiddenSkill();
        void LoadBugReward();
        void LoadEventChecks(uint8_t stageIDX);
        void loadCustomText(const uint8_t* data);

        const Header* m_Header;
        const uint8_t* m_GCIData;

        const DZXCheck* m_DZXChecks;             // DZX replacement checks for current stage
        const RELCheck* m_RELChecks;             // REL Modifications for current stage
        const PoeCheck* m_PoeChecks;             // Poe Checks for current stage
        const ARCReplacement* m_ArcReplacements; // Checks for the currently loaded .arc file
        const BossCheck* m_BossChecks;
        const HiddenSkillCheck* m_HiddenSkillChecks;
        const BugReward* m_BugRewardChecks;
        const SkyCharacter* m_SkyBookChecks;
        const ObjectArchiveReplacement* m_ObjectArcReplacements;
        const EventItem* m_EventChecks;

        const BGMReplacement* m_BgmTable;     // Bgm replacement data
        const BGMReplacement* m_FanfareTable; // Fanfare replacement data
        const ShuffledEntrance* m_ShuffledEntrances;
        const CLR0Header* m_CLR0;
        const RawRGBTable* m_RawRGBTable;
        const BMDEntry* m_BmdEntries;

        uint16_t m_NumLoadedDZXChecks;       // Number of currently loaded DZXCheck
        uint16_t m_NumLoadedRELChecks;       // Number of currently loaded RELCheck
        uint16_t m_NumLoadedPOEChecks;       // Number of currently loaded POEChecks
        uint16_t m_NumLoadedArcReplacements; // Number of currently loaded ArcChecks
        uint16_t m_NumHiddenSkillChecks;
        uint16_t m_NumBugRewardChecks;
        uint16_t m_NumSkyBookChecks;
        uint16_t m_NumLoadedEventChecks;
        uint16_t m_NumLoadedObjectArcReplacements;
        uint16_t m_NumShuffledEntrances;
        uint16_t m_BgmTableEntries;
        uint16_t m_FanfareTableEntries;

        uint16_t m_PatchesApplied;
        uint16_t m_EventFlagsModified;
        uint16_t m_AreaFlagsModified;

        uint8_t m_StageIDX; // StageIDX from last Checkload
    };
} // namespace mod::rando
#endif