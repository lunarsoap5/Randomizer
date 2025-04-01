/**	@file bmg0.cpp
 *  @brief BMG0 section in seed data
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#include "rando/bmg0.h"

namespace mod::rando
{
    FlwIdxRemap* BMG0Section::getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t flwIndex) const
    {
        if (msgFlow == nullptr)
            return nullptr;
        // return -1;

        // Only allow remapping 0xFFFF if the msgFlow itself is being
        // initialized. Otherwise you can get stuck in a loop of messages when
        // it tries to exit normally using 0xFFFF.
        if (flwIndex == 0xFFFF && msgFlow->mMsg != 0xFFFFFFFF)
            return nullptr;
        // return -1;

        const uint16_t targetFLIValue = msgFlow->mFlow;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const FlwIdxRemap* entries = reinterpret_cast<const FlwIdxRemap*>(headerPtr + this->flwIdxRemapOffset);
        const uint16_t num_entries = this->numFlwIdxRemapEntries;

        for (uint32_t i = 0; i < num_entries; i++)
        {
            if (entries[i].getFLIValue() == targetFLIValue && entries[i].getOldInitFLWIndex() == flwIndex)
            {
                uint32_t u32Addr = reinterpret_cast<uint32_t>(entries) + i * sizeof(FlwIdxRemap);
                return reinterpret_cast<FlwIdxRemap*>(u32Addr);
                // return &(entries[i]);
                // return entries[i].getNewInitFLWIndex();
            }
        }
        return nullptr;
    }

    uint16_t BMG0Section::getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow) const
    {
        if (msgFlow == nullptr)
            return -1;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const InfRemap* entries = reinterpret_cast<const InfRemap*>(headerPtr + this->infRemapOffset);
        const uint16_t num_entries = this->numInfRemapEntries;

        const uint16_t targetFLIValue = msgFlow->mFlow;
        const uint16_t targetFLWIndex = msgFlow->field_0x10;

        for (uint32_t i = 0; i < num_entries; i++)
        {
            const uint16_t bitMask = entries[i].getBitMask();
            const uint16_t maskedFliValue = bitMask & targetFLIValue;

            if (entries[i].getFLIValue() == maskedFliValue && entries[i].getFLWIndex() == targetFLWIndex)
            {
                return entries[i].getNewINFIndex();
            }
        }
        return -1;
    }

    char* BMG0Section::getReplacementStr(uint16_t context, uint16_t infIndex) const
    {
        if (context == 0)
            return nullptr;

        const uint16_t numEntries = this->numStrRemapEntries;
        if (numEntries == 0)
            return nullptr;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->strRemapLookupOffset);

        uint32_t lookupVal = (context << 16) + infIndex;
        for (int i = 0; i < numEntries; i++)
        {
            if (lookupTable[i] == lookupVal)
            {
                const uint16_t* offsetsTable = reinterpret_cast<const uint16_t*>(headerPtr + this->strRemapOffsetsOffset);
                const uint16_t offsetInStrTable = offsetsTable[i];

                uint32_t strAddr = reinterpret_cast<uint32_t>(headerPtr) + this->strTableOffset + offsetInStrTable;
                return reinterpret_cast<char*>(strAddr);
            }
        }

        return nullptr;
    }

    const uint16_t* BMG0Section::getBranchEditData(uint16_t context, uint16_t flwIndex) const
    {
        if (context == 0)
            return nullptr;

        const uint16_t numEntries = this->numBranchEditLookups;
        if (numEntries == 0)
            return nullptr;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->branchEditLookupsOffset);

        uint32_t lookupVal = (flwIndex << 16) + context;
        for (int i = 0; i < numEntries; i++)
        {
            if (lookupTable[i] == lookupVal)
            {
                return reinterpret_cast<const uint16_t*>(&(lookupTable[i + 1]));
            }
        }

        return nullptr;
    }

    const uint16_t* BMG0Section::getEventEditData(uint16_t context, uint16_t flwIndex) const
    {
        if (context == 0)
            return nullptr;

        const uint16_t numEntries = this->numEventEditLookups;
        if (numEntries == 0)
            return nullptr;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->eventEditLookupsOffset);

        uint32_t lookupVal = (flwIndex << 16) + context;
        for (int i = 0; i < numEntries; i++)
        {
            if (lookupTable[i] == lookupVal)
            {
                return reinterpret_cast<const uint16_t*>(&(lookupTable[i + 1]));
            }
        }

        return nullptr;
    }

    const uint16_t* BMG0Section::getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                                           uint16_t context,
                                                           uint16_t branchProcResult) const
    {
        if (msgFlow == nullptr)
            return nullptr;

        const uint16_t* branchEditData = getBranchEditData(context, msgFlow->field_0x10);
        if (branchEditData == nullptr)
            return nullptr;

        const uint16_t baseTableIndex = branchEditData[1];
        const uint16_t finalTableIndex = baseTableIndex + branchProcResult;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const uint16_t* branchProcResultsTable = reinterpret_cast<const uint16_t*>(headerPtr + this->nextFlwTableOffset);

        return &(branchProcResultsTable[finalTableIndex]);
    }

    const uint8_t* BMG0Section::getEventNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const
    {
        if (msgFlow == nullptr)
            return nullptr;

        const uint16_t* branchEditData = getEventEditData(context, msgFlow->field_0x10);
        if (branchEditData == nullptr)
            return nullptr;

        const uint16_t tableIndex = branchEditData[0];

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const uint8_t* branchProcResultsTable = reinterpret_cast<const uint8_t*>(headerPtr + this->eventNodesOffset);

        return &(branchProcResultsTable[tableIndex * 8]);
    }

} // namespace mod::rando
