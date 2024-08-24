/**	@file seed.cpp
 *  @brief Seed class to access seed-data
 *
 *	@author AECX
 *	@bug No known bugs.
 */

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>

#ifdef DVD
#include "gc_wii/dvd.h"
#else
#include "gc_wii/card.h"
#endif

#include "rando/seed.h"
#include "cxx.h"
#include "game_patch/game_patch.h"
#include "main.h"
#include "rando/data.h"
#include "tools.h"
#include "tp/d_a_shop_item_static.h"
#include "tp/d_item_data.h"
#include "user_patch/user_patch.h"

namespace mod::rando
{
#ifdef DVD
    Seed::Seed()
#else
    Seed::Seed(int32_t chan)
#endif
    {
        // Initialize variables
        this->m_StageIDX = 0xFF;

        getConsole() << "Loading seed...\n";

        uint8_t* data;
#ifdef DVD
        // Allocate the memory to the back of the heap to avoid possible fragmentation
        const int32_t fileSize = libtp::tools::readFile("/mod/seed.bin", false, &data);
#else
        // The memory card should already be mounted
        // Allocate the memory to the back of the heap to avoid possible fragmentation
        const int32_t fileSize = libtp::tools::readFileFromGCI(chan, SEED_GCI_ID, false, true, &data);
#endif
        // Make sure the file was successfully read
        if (fileSize <= 0)
        {
            return;
        }

        // Get the header data
        // Align to uint32_t, as that's the largest variable type in the Header class
        const Header* headerPtr = new (sizeof(uint32_t)) Header(data);

        // Make sure the file is a proper seed
        if (!headerPtr->magicIsValid())
        {
            delete[] data;
            delete headerPtr;
            return;
        }

        // Make sure the seed version is supported
        if (!headerPtr->versionIsValid())
        {
            delete[] data;
            delete headerPtr;
            return;
        }

        // Seed should be valid, so assign the header pointer now
        this->m_Header = headerPtr;

        // Get the main seed data
        // Align to 0x20 for safety, since some functions cast parts of it to classes/structs/arrays/etc.
        const uint32_t dataSize = headerPtr->getDataSize();
        const uint8_t* gciDataPtr = new (0x20) uint8_t[dataSize];
        this->m_GCIData = gciDataPtr;

        memcpy(const_cast<uint8_t*>(gciDataPtr), &data[headerPtr->getHeaderSize()], dataSize);

        const uint32_t clr0Offset = headerPtr->getClr0Offset();
        const CLR0Header* cLR0Ptr = reinterpret_cast<const CLR0Header*>(gciDataPtr + clr0Offset);
        this->m_CLR0 = cLR0Ptr;

        this->m_RawRGBTable = reinterpret_cast<const RawRGBTable*>(gciDataPtr + clr0Offset + cLR0Ptr->getRawRGBOffset());
        this->m_BmdEntries = reinterpret_cast<const BMDEntry*>(gciDataPtr + clr0Offset + cLR0Ptr->getBmdEntriesOffset());

        // Load the custom text data
        this->loadCustomText(data);

        // Set the static pointers for the Seed Header and Data. These are used by TPO
        const void** ptrTable = reinterpret_cast<const void**>(0x800042BC);
        ptrTable[0] = headerPtr;
        ptrTable[1] = gciDataPtr;

        delete[] data;
    }

    Seed::~Seed()
    {
        // Clear the static pointers for the Seed Header and Data.  These are used by TPO
        const void** ptrTable = reinterpret_cast<const void**>(0x800042BC);
        ptrTable[0] = nullptr;
        ptrTable[1] = nullptr;

        // Free the memory used by dynamic stuff
        if (this->m_Header)
        {
            delete this->m_Header;
        }

        if (this->m_GCIData)
        {
            delete[] this->m_GCIData;
        }

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

    void Seed::applyOneTimePatches()
    {
        const EntryInfo* oneTimePatchInfoPtr = this->getHeaderPtr()->getOneTimePatchInfoPtr();
        const uint32_t num_bytes = oneTimePatchInfoPtr->getNumEntries();
        const uint32_t gci_offset = oneTimePatchInfoPtr->getDataOffset();
        const uint32_t* bitfieldPtr = reinterpret_cast<const uint32_t*>(&this->m_GCIData[gci_offset]);

        constexpr uint32_t totalOneTimePatches = sizeof(user_patch::oneTimePatches) / sizeof(user_patch::oneTimePatches[0]);
        uint32_t patchesApplied = this->m_PatchesApplied;

        for (uint32_t i = 0; i < totalOneTimePatches; i++)
        {
            if (flagIsEnabled(bitfieldPtr, num_bytes, i))
            {
                user_patch::oneTimePatches[i](rando::gRandomizer);
                patchesApplied++;
            }
        }

        this->m_PatchesApplied = static_cast<uint16_t>(patchesApplied);
    }

    void Seed::loadBgmData()
    {
        const Header* headerPtr = this->m_Header;

        // BGM replacement data
        const EntryInfo* shuffledBgmInfoPtr = headerPtr->getBgmTableInfoPtr();
        uint32_t num_shuffledTracks = shuffledBgmInfoPtr->getNumEntries();
        uint32_t gci_offset = shuffledBgmInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        const uint8_t* gciDataPtr = &this->m_GCIData[0];
        this->m_BgmTable = reinterpret_cast<const BGMReplacement*>(&gciDataPtr[gci_offset]);
        this->m_BgmTableEntries = static_cast<uint16_t>(num_shuffledTracks);

        // Fanfare replacement data
        shuffledBgmInfoPtr = headerPtr->getFanfareTableInfoPtr();
        num_shuffledTracks = shuffledBgmInfoPtr->getNumEntries();
        gci_offset = shuffledBgmInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        this->m_FanfareTable = reinterpret_cast<const BGMReplacement*>(&gciDataPtr[gci_offset]);
        this->m_FanfareTableEntries = static_cast<uint16_t>(num_shuffledTracks);
    }

    void Seed::loadShuffledEntrances()
    {
        const EntryInfo* shuffledEntranceInfoPtr = this->m_Header->getEntranceTableCheckInfoPtr();
        const uint32_t num_shuffledEntrances = shuffledEntranceInfoPtr->getNumEntries();
        const uint32_t gci_offset = shuffledEntranceInfoPtr->getDataOffset();

        // Set the pointer as offset into our buffer
        this->m_ShuffledEntrances = reinterpret_cast<const ShuffledEntrance*>(&this->m_GCIData[gci_offset]);
        this->m_NumShuffledEntrances = static_cast<uint16_t>(num_shuffledEntrances);
    }

    void Seed::loadCustomText(const uint8_t* data)
    {
        const Header* headerPtr = this->m_Header;
        const uint32_t headerOffset = headerPtr->getHeaderSize() + headerPtr->getCustomTextHeaderOffset();

        // Get the custom message header
        const CustomMessageHeaderInfo* customMessageHeader =
            reinterpret_cast<const CustomMessageHeaderInfo*>(&data[headerOffset]);

        // Allocate memory for the ids, message offsets, and messages
        const uint32_t totalHintMsgEntries = customMessageHeader->getTotalEntries();
        rando::Randomizer* randoPtr = rando::gRandomizer;
        randoPtr->setTotalHintMsgEntries(static_cast<uint16_t>(totalHintMsgEntries));

        uint32_t msgIdTableSize = totalHintMsgEntries * sizeof(CustomMessageData);
        const uint32_t msgOffsetTableSize = totalHintMsgEntries * sizeof(uint32_t);

        // Round msgIdTableSize up to the size of the offsets to make sure the offsets are properly aligned
        msgIdTableSize = (msgIdTableSize + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1);
        const uint32_t msgTableInfoSize = msgIdTableSize + msgOffsetTableSize + customMessageHeader->getMsgTableSize();

        // Align to uint16_t, as thagt's the largest variable type in CustomMessageData
        uint8_t* hintMsgTableInfoPtr = new (sizeof(uint16_t)) uint8_t[msgTableInfoSize];
        randoPtr->setHintMsgTableInfoPtr(hintMsgTableInfoPtr);

        // When calculating the offset the the message table information, we are assuming that the message header is
        // followed by the entry information for all of the languages in the seed data.
        const uint32_t offset = headerOffset + customMessageHeader->getMsgIdTableOffset();

        // Copy the data to the pointers
        memcpy(hintMsgTableInfoPtr, &data[offset], msgTableInfoSize);

        for (uint32_t i = msgIdTableSize + msgOffsetTableSize; i < msgTableInfoSize; i++)
        {
            hintMsgTableInfoPtr[i] = ~hintMsgTableInfoPtr[i];
        }
    }

} // namespace mod::rando