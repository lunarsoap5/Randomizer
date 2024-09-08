/**	@file data.h
 *  @brief Contains structs for different types of checks.
 *
 *  StageIDX are different size on purpose for alignment/padding, they should never exceed 0xFF.
 *
 *	@author AECX
 *	@bug No known bugs.
 */
#ifndef RANDO_DATA_H
#define RANDO_DATA_H

#include "gc_wii/card.h"
#include "data/items.h"
#include <cstdint>

namespace mod::rando
{
    class BGMReplacement
    {
       public:
        BGMReplacement() {}
        ~BGMReplacement() {}

        uint8_t getOriginalBgmTrack() const { return this->originalBgmTrack; }
        uint8_t getReplacementBgmTrack() const { return this->replacementBgmTrack; }
        uint8_t getReplacementBgmWave() const { return this->replacementBgmWave; }

       private:
        uint8_t originalBgmTrack;
        uint8_t replacementBgmTrack;
        uint8_t replacementBgmWave;
        uint8_t padding;
    } __attribute__((__packed__));

    class RegionFlag
    {
       public:
        RegionFlag() {}
        ~RegionFlag() {}

        uint8_t getRegionId() const { return this->region_id; }
        uint8_t getBitId() const { return this->bit_id; }

       private:
        uint8_t region_id;
        uint8_t bit_id;
    } __attribute__((__packed__));

    class EventFlag
    {
       public:
        EventFlag() {}
        ~EventFlag() {}

        uint8_t getOffset() const { return this->offset; }
        uint8_t getFlag() const { return this->flag; }

       private:
        uint8_t offset;
        uint8_t flag;
    } __attribute__((__packed__));

    class DZXCheck
    {
       public:
        DZXCheck() {}
        ~DZXCheck() {}

        uint16_t getHash() const { return this->hash; }
        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getMagicByte() const { return this->magicByte; }

        uint8_t getData(uint32_t index) const
        {
            // Make sure the index is valid
            if (index >= sizeof(this->data))
            {
                return 0;
            }
            return this->data[index];
        }

       private:
        uint16_t hash;
        uint8_t stageIDX;
        uint8_t magicByte; // ignore this byte in data[]
        uint8_t data[0x20];
    } __attribute__((__packed__));

    class ShopCheck
    {
       public:
        ShopCheck() {}
        ~ShopCheck() {}

        uint8_t getShopItemID() const { return this->shopItemID; }
        uint8_t getReplacementItemID() const { return this->replacementItemID; }

       private:
        uint8_t shopItemID;        // target item id
        uint8_t replacementItemID; // replacement item id
        uint8_t padding[2];
    } __attribute__((__packed__));

    class RELCheck
    {
       public:
        RELCheck() {}
        ~RELCheck() {}

        uint16_t getReplacementType() const { return this->replacementType; }
        uint16_t getStageIDX() const { return this->stageIDX; }
        uint32_t getModuleID() const { return this->moduleID; }
        uint32_t getOffset() const { return this->offset; }
        uint32_t getOverride() const { return this->override; }

       private:
        uint16_t replacementType;
        uint16_t stageIDX;
        uint32_t moduleID;
        uint32_t offset;
        uint32_t override;
    } __attribute__((__packed__));

    class PoeCheck
    {
       public:
        PoeCheck() {}
        ~PoeCheck() {}

        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getFlag() const { return this->flag; }
        uint8_t getItem() const { return this->item; }

       private:
        uint8_t stageIDX;
        uint8_t flag; // Flag used for identification
        uint8_t item; // New item id
        uint8_t padding;
    } __attribute__((__packed__));

    enum class FileDirectory : uint8_t
    {
        Room = 0x0,
        Message = 0x1,
        Object = 0x2,
        Stage = 0x3,
    };

    enum class ReplacementType : uint8_t
    {
        Item = 0x0,            // Standard item replacement
        HiddenSkill = 0x1,     // Hidden Skill checks check for the room last loaded into.
        ItemMessage = 0x2,     // Replaces messages for item IDs
        Instruction = 0x3,     // Replaces a u32 instruction
        AlwaysLoaded = 0x4,    // Replaces values specifically in the bmgres archive which is always loaded.
        MessageResource = 0x5, // Replaces values in the MESG section of a bmgres archive file.
    };

    class ARCReplacement
    {
       public:
        ARCReplacement() {}
        ~ARCReplacement() {}

        int32_t getOffset() const { return this->offset; }
        uint32_t getReplacementValue() const { return this->replacementValue; }
        FileDirectory getDirectory() const { return this->directory; }
        ReplacementType getReplacementType() const { return this->replacementType; }
        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getRoomID() const { return this->roomID; }

       private:
        int32_t offset;                  // The offset where the item is stored from the message flow header.
        uint32_t replacementValue;       // Used to be item, but can be more now.
        FileDirectory directory;         // The type of directory where the check is stored.
        ReplacementType replacementType; // The type of replacement that is taking place.
        uint8_t stageIDX;                // The name of the file where the check is stored
        uint8_t roomID;                  // The room number for chests/room based dzr checks.
    } __attribute__((__packed__));

    class ObjectArchiveReplacement
    {
       public:
        ObjectArchiveReplacement() {}
        ~ObjectArchiveReplacement() {}

        int32_t getOffset() const { return this->offset; }
        uint32_t getReplacementValue() const { return this->replacementValue; }
        const char* getFileNamePtr() const { return &this->fileName[0]; }
        uint8_t getStageIDX() const { return this->stageIDX; }

       private:
        int32_t offset;            // The offset where the item is stored from the message flow header.
        uint32_t replacementValue; // Used to be item, but can be more now.
        char fileName[15];
        uint8_t stageIDX; // The name of the file where the check is stored
    } __attribute__((__packed__));

    class BossCheck
    {
       public:
        BossCheck() {}
        ~BossCheck() {}

        uint16_t getStageIDX() const { return this->stageIDX; }
        uint8_t getItem() const { return this->item; }

       private:
        uint16_t stageIDX; // The stage where the replacement is taking place.
        uint8_t item;      // New item id
        uint8_t padding;
    } __attribute__((__packed__));

    class HiddenSkillCheck
    {
       public:
        HiddenSkillCheck() {}
        ~HiddenSkillCheck() {}

        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getRoomID() const { return this->roomID; }
        uint8_t getItemID() const { return this->itemID; }

       private:
        uint8_t stageIDX; // The ID of the stage that Golden Wolf was located in
        uint8_t roomID;   // The room of the stage that the Golden Wolf was located in.
        uint8_t itemID;   // The item to be given when in the above stage and room.
        uint8_t padding;
    } __attribute__((__packed__));

    class BugReward
    {
       public:
        BugReward() {}
        ~BugReward() {}

        uint16_t getBugId() const { return this->bugID; }
        uint8_t getItemId() const { return this->itemID; }

       private:
        uint16_t bugID; // The bug that link is showing to Agitha
        uint8_t itemID; // The item that Agitha will give Link.
        uint8_t padding;
    } __attribute__((__packed__));

    class SkyCharacter
    {
       public:
        SkyCharacter() {}
        ~SkyCharacter() {}

        uint16_t getStageIDX() const { return this->stageIDX; }
        uint8_t getItemId() const { return this->itemID; }
        uint8_t getRoomID() const { return this->roomID; }

       private:
        uint16_t stageIDX; // The stage that the Owl Statue is located.
        uint8_t itemID;    // The item to be given.
        uint8_t roomID;    // The room that the Owl Statue is located in.
    } __attribute__((__packed__));

    // These items are given either during cutscenes or at a specific time.
    class EventItem
    {
       public:
        EventItem() {}
        ~EventItem() {}

        uint8_t getItemID() const { return this->itemID; }
        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getRoomID() const { return this->roomID; }
        uint8_t getFlag() const { return this->flag; }

       private:
        uint8_t itemID;   // The item to be given.
        uint8_t stageIDX; // The stage that the event is in.
        uint8_t roomID;   // The room that the event is in.
        uint8_t flag;     // The unique identifier used to disinguish between checks in the same room.
    } __attribute__((__packed__));

    class CustomMessageHeaderInfo
    {
       public:
        CustomMessageHeaderInfo() {}
        ~CustomMessageHeaderInfo() {}

        uint16_t getHeaderSize() const { return this->headerSize; }
        uint16_t getTotalEntries() const { return this->totalEntries; }
        uint32_t getMsgTableSize() const { return this->msgTableSize; }
        uint32_t getMsgIdTableOffset() const { return this->msgIdTableOffset; }

       private:
        uint16_t headerSize;
        uint16_t totalEntries;
        uint32_t msgTableSize;
        uint32_t msgIdTableOffset;
    } __attribute__((__packed__));

    class CustomMessageData
    {
       public:
        CustomMessageData() {}
        ~CustomMessageData() {}

        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getRoomIDX() const { return this->roomIDX; }
        uint16_t getMsgID() const { return this->msgID; }

       private:
        uint8_t stageIDX;
        uint8_t roomIDX;
        uint16_t msgID;
    } __attribute__((__packed__));

    class CLR0Header
    {
       public:
        CLR0Header() {}
        ~CLR0Header() {}

        uint32_t getTotalByteLength() const { return this->totalByteLength; }
        uint32_t getNumBmdEntries() const { return this->numBmdEntries; }
        uint16_t getBmdEntriesOffset() const { return this->bmdEntriesOffset; }
        uint16_t getRawRGBOffset() const { return this->rawRGBOffset; }

       private:
        /* 0x00 */ uint32_t totalByteLength;  // Total byte size of the entire CLR0 chunk
        /* 0x04 */ uint32_t numBmdEntries;    // Total number of bmd replacement entries.
        /* 0x08 */ uint16_t bmdEntriesOffset; // Offset to the list of bmd entries
        /* 0x0A */ uint16_t rawRGBOffset;     // Offset to the list of raw RGB entries
    } __attribute__((__packed__));

    enum BMDDirectory : uint8_t
    {
        BMWE = 0,
        BMDR = 1,
        BMWR = 2,
    };

    class BMDEntry
    {
       public:
        BMDEntry() {}
        ~BMDEntry() {}

        uint8_t getRecolorType() const { return this->recolorType; }
        uint8_t getArchiveIndex() const { return this->archiveIndex; }
        uint16_t getNumTextures() const { return this->numTextures; }
        uint16_t getTextureListOffset() const { return this->textureListOffset; }
        const char* getBmdResPtr() const { return &this->bmdRes[0]; }
        BMDDirectory getDirectory() const { return this->directory; }

       private:
        /* 0x00 */ uint8_t recolorType;        // 0: CMPR, 1: MAT, etc.
        /* 0x01 */ uint8_t archiveIndex;       // The index of the archive used to load the texture replacement.
        /* 0x02 */ BMDDirectory directory;     // The directory type to use, BMDR/BMWR/etc.
        /* 0x03 */ uint8_t numTextures;        // number of textures that are being recolored in this bmd file.
        /* 0x04 */ uint16_t textureListOffset; // offset to the list of textures being recolored.
        /* 0x06 */ char bmdRes[0x12];          // names are of varying size, but I haven't seen one over 0x10 yet.
    } __attribute__((__packed__));

    class CMPRTextureEntry
    {
       public:
        CMPRTextureEntry() {}
        ~CMPRTextureEntry() {}

        const uint8_t* getRGBAPtr() const { return reinterpret_cast<const uint8_t*>(&this->rgba); }
        const char* getTextureNamePtr() const { return &this->textureName[0]; }

       private:
        /* 0x00 */ uint32_t rgba;
        /* 0x04 */ char textureName[0xC];
    } __attribute__((__packed__));

    enum RecolorType : uint8_t
    {
        CMPR = 0,
        MAT = 1,
        Invalid = 0xFF,
    };

    enum RawRGBId : uint8_t
    {
        LanternGlow,
        Hearts,
        ABtn,
        BBtn,
        XBtn,
        YBtn,
        ZBtn
    };

    enum CastleEntryRequirements : uint8_t
    {
        HC_Open = 0,
        HC_Fused_Shadows = 1,
        HC_Mirror_Shards,
        HC_All_Dungeons,
        HC_Vanilla
    };

    enum PalaceEntryRequirements : uint8_t
    {
        PoT_Open = 0,
        PoT_Fused_Shadows = 1,
        PoT_Mirror_Shards,
        PoT_Vanilla
    };

    enum StartingTimeOfDay : uint8_t
    {
        Morning = 0,
        Noon = 1,
        Evening = 2,
        Night = 3
    };

    class RawRGBTable
    {
       public:
        RawRGBTable() {}
        ~RawRGBTable() {}

        const uint8_t* getLanternColorPtr() const { return reinterpret_cast<const uint8_t*>(&this->lanternColor); }

        const uint8_t* getWolfDomeAttackColorPtr() const
        {
            return reinterpret_cast<const uint8_t*>(&this->wolfDomeAttackColor);
        }

        uint32_t getLanternColor() const { return this->lanternColor; }
        uint32_t getHeartColor() const { return this->heartColor; }
        uint32_t getAButtonColor() const { return this->aButtonColor; }
        uint32_t getBButtonColor() const { return this->bButtonColor; }
        uint32_t getXButtonColor() const { return this->xButtonColor; }
        uint32_t getYButtonColor() const { return this->yButtonColor; }
        uint32_t getZButtonColor() const { return this->zButtonColor; }
        uint32_t getWolfDomeAttackColor() const { return this->wolfDomeAttackColor; }

       private:
        uint32_t lanternColor;
        uint32_t heartColor;
        uint32_t aButtonColor;
        uint32_t bButtonColor;
        uint32_t xButtonColor;
        uint32_t yButtonColor;
        uint32_t zButtonColor;
        uint32_t wolfDomeAttackColor;
    } __attribute__((__packed__));

    enum DvdEntryNumId : uint8_t
    {
        // DO NOT set any of these enums to a specific value. The exact values and the order are irrelevant (other than
        // DvdEntryNumIdSize which must go last).
        ResObjectKmdl,   // Link wearing Hero's Clothes
        ResObjectZmdl,   // Link wearing Zora Armor
        ResObjectOgZORA, // Zora Armor Get Item
        ResObjectAlink,  // Link's Equipment
                         // ResObjectWmdl,      // Wolf Link and Midna on back
                         // ResObjectCWShd,     // Ordon Shield
                         // ResObjectSWShd,     // Wooden Shield
                         // ResObjectHyShd,     // Hylian Shield

        DvdEntryNumIdSize,
        // DvdEntryNumIdSize MUST GO LAST. When adding a new enum, put it above this one and don't forget to actually add the
        // lookup in the dvdentrynum.cpp file!
    };

    class GoldenWolfItemReplacement
    {
       public:
        GoldenWolfItemReplacement() {}
        ~GoldenWolfItemReplacement() {}

        int32_t getItemActorId() const { return this->itemActorId; }
        int16_t getFlag() const { return this->flag; }
        uint8_t getMarkerFlag() const { return this->markerFlag; }

        void setItemActorId(int32_t id) { this->itemActorId = id; }
        void setFlag(int16_t value) { this->flag = value; }
        void setMarkerFlag(uint8_t value) { this->markerFlag = value; }

       private:
        int32_t itemActorId; // Global actor id for the spawned item
        int16_t flag;        // Flag associated with the current golden wolf
        uint8_t markerFlag;  // Flag associated with the current golden wolf's marker on the map
    };

    class ShuffledEntrance
    {
       public:
        ShuffledEntrance() {}
        ~ShuffledEntrance() {}

        uint8_t getOrigStageIDX() const { return this->origStageIDX; }
        uint8_t getOrigRoomIDX() const { return this->origRoomIDX; }
        uint8_t getOrigSpawn() const { return this->origSpawn; }
        int8_t getOrigState() const { return this->origState; }
        uint8_t getNewStageIDX() const { return this->newStageIDX; }
        uint8_t getNewRoomIDX() const { return this->newRoomIDX; }
        uint8_t getNewSpawn() const { return this->newSpawn; }
        int8_t getNewState() const { return this->newState; }

       private:
        uint8_t origStageIDX;
        uint8_t origRoomIDX;
        uint8_t origSpawn;
        int8_t origState;
        uint8_t newStageIDX;
        uint8_t newRoomIDX;
        uint8_t newSpawn;
        int8_t newState;
    } __attribute__((__packed__));
} // namespace mod::rando
#endif