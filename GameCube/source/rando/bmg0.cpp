/**	@file bmg0.cpp
 *  @brief BMG0 section in seed data
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#include "rando/bmg0.h"
#include "tp/d_com_inf_game.h"
#include "tp/JKRArchivePub.h"

namespace mod::rando
{
    template<typename T>
    // int binarySearch(T arr[], uint32_t low, uint32_t high, T target)
    int binarySearch(T arr[], int low, int len, T target)
    {
        int high = low + len;
        while (low <= high)
        {
            int mid = low + (high - low) / 2;

            // Check if target is present at mid
            if (arr[mid] == target)
                return mid;

            // Adjust to left or right half
            if (arr[mid] < target)
                low = mid + 1;
            else
                high = mid - 1;
        }
        return -1;
    }

    void* getZel00BmgFlw()
    {
        uint32_t infPtrRaw = reinterpret_cast<uint32_t>(libtp::tp::JKRArchivePub::JKRArchivePub_getGlbResource(
            0x524F4F54, // ROOT
            "zel_00.bmg",
            libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mMsgDtArchive[0]));

        // All zel_xx.bmg files have the offet to the FLW1 block relative to the
        // start of the file (MESGbmg1) stored as a word at offset 0x8.
        uint32_t offsetToFlwBlock = reinterpret_cast<uint32_t*>(infPtrRaw)[2];
        uint32_t flwBlockAddr = infPtrRaw + offsetToFlwBlock;
        return reinterpret_cast<void*>(flwBlockAddr);
    }

    // Function to get a u16,u16,u16,u16 tableSlice data given an EntityInfo

    // const uint16_t* BMG0Section::getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
    //                                                     uint16_t flwIndex,
    //                                                     uint16_t flowContext) const
    const uint16_t* BMG0Section::getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow*, uint16_t, uint16_t) const
    {
        // if (msgFlow == nullptr)
        //     return nullptr;
        // // return -1;

        // // // Only allow remapping 0xFFFF if the msgFlow itself is being
        // // // initialized. Otherwise you can get stuck in a loop of messages when
        // // // it tries to exit normally using 0xFFFF.
        // // if (flwIndex == 0xFFFF && msgFlow->mMsg != 0xFFFFFFFF)
        // //     return nullptr;
        // // // return -1;

        // // binary search

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->nodeRemapCompsOffset);

        // for (uint32_t i = 0; i < 2; i++)
        // {
        //     int startIdx;
        //     int endIndex;
        //     uint32_t lookupVal;

        //     if (i == 0)
        //     {
        //         // Context compare
        //         if (flowContext == 0)
        //             continue;

        //         startIdx = 0;
        //         endIndex = this->nodeRemapContextCompsLength;
        //         lookupVal = (flowContext << 0x10) + flwIndex;
        //     }
        //     else
        //     {
        //         // FLI compare
        //         if (flwIndex == 0xFFFF && flowContext != 0)
        //         {
        //             // Disallow remapping 0xFFFF using an FLI value when we have
        //             // a flowContext. This is to help avoid infinite loops
        //             // caused by developer error.
        //             continue;
        //         }

        //         uint8_t bmgNumber = 0;
        //         if (msgFlow->mFlow_p != getZel00BmgFlw())
        //         {
        //             bmgNumber = libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStageData.mStagInfo->mMsgGroup;
        //         }

        //         const BmgStrCompData* bmgDataTable =
        //             reinterpret_cast<const BmgStrCompData*>(headerPtr + this->bmgStrCompsTableOffset);

        //         startIdx = bmgDataTable[bmgNumber].nodeRemapContextCompStartIndex;
        //         endIndex = startIdx + bmgDataTable[bmgNumber].nodeRemapContextCompLength;
        //         lookupVal = (msgFlow->mFlow << 0x10) + flwIndex;
        //     }

        //     int foundIdx = binarySearch<const uint32_t>(lookupTable, startIdx, endIndex, lookupVal);
        //     if (foundIdx >= 0)
        //     {
        //         // Get from results table
        //         const uint32_t* resultsTable = reinterpret_cast<const uint32_t*>(headerPtr + this->nodeRemapResultsOffset);

        //         const uint32_t* ptr = &(resultsTable[foundIdx]);
        //         return reinterpret_cast<const uint16_t*>(ptr);
        //     }
        // }

        return nullptr;
    }

    uint16_t BMG0Section::getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, bool isSelectOptionsNode) const
    {
        if (msgFlow != nullptr)
        {
            if (msgFlow->mFlow >= 0x7000)
            {
                if (isSelectOptionsNode)
                    return 0x136a;
                else
                    return 0x1369;
            }
        }
        return -1;
    }

    // void BMG0Section::getTableSliceInfos(const EntityInfo* entityInfo, TableSliceInfo* outTableSliceInfos) const
    void BMG0Section::getTableSliceInfos(const EntityInfo* entityInfo,
                                         uint8_t bmgNumber,
                                         TableSliceInfo* outTableSliceInfos) const
    {
        if (bmgNumber >= 9)
            return;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const TableSliceInfo* tableSliceInfoTable =
            reinterpret_cast<const TableSliceInfo*>(headerPtr + this->tableSliceInfosOffset);

        uint8_t rawBmgByte = entityInfo->bmgLookupBytes[bmgNumber];

        bool hasCtxComp = (rawBmgByte >> 7) & 0x01;
        bool hasBasicComp = (rawBmgByte >> 6) & 0x01;
        uint8_t offsetForBmg = rawBmgByte & 0x3F;

        uint16_t currIdx = entityInfo->tableSliceInfoStartIdx + offsetForBmg;

        // Fill out param with 0s
        for (int i = 0; i < 8; i++)
        {
            reinterpret_cast<uint8_t*>(outTableSliceInfos)[i] = 0;
        }

        if (hasCtxComp)
        {
            outTableSliceInfos[0].startIdx = tableSliceInfoTable[currIdx].startIdx;
            outTableSliceInfos[0].len = tableSliceInfoTable[currIdx].len;
            currIdx++;
        }

        if (hasBasicComp)
        {
            outTableSliceInfos[1].startIdx = tableSliceInfoTable[currIdx].startIdx;
            outTableSliceInfos[1].len = tableSliceInfoTable[currIdx].len;
        }
    }

    const char* BMG0Section::getReplacementStr(uint8_t bmgNumber, uint16_t context, uint16_t infIndex) const
    {
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const EntityInfo* entityInfoTable = reinterpret_cast<const EntityInfo*>(headerPtr + this->entityInfoTableOffset);
        const uint16_t* strOffsetTable = reinterpret_cast<const uint16_t*>(headerPtr + this->strOffsetTableOffset);
        const char* strTable = reinterpret_cast<const char*>(headerPtr + this->strTableOffset);

        const EntityInfo* strReplEntityInfo = &(entityInfoTable[EntityInfoIdx::STRING_REPLACEMENT]);
        TableSliceInfo tableSliceInfos[2];
        getTableSliceInfos(strReplEntityInfo, bmgNumber, tableSliceInfos);

        // Context compare
        if (tableSliceInfos[0].len > 0)
        {
            const uint32_t* wordCompTable = reinterpret_cast<const uint32_t*>(headerPtr + this->wordCompValsOffset);

            const uint32_t lookupVal = (context << 16) + infIndex;
            int foundIdx =
                binarySearch<const uint32_t>(wordCompTable, tableSliceInfos[0].startIdx, tableSliceInfos[0].len, lookupVal);

            if (foundIdx >= 0)
            {
                // Adjust from relative to absolute index
                foundIdx += strReplEntityInfo->ctxCompAdjustment;

                uint16_t strOffset = strOffsetTable[foundIdx];
                return &(strTable[strOffset]);
            }
        }

        // Basic compare
        if (tableSliceInfos[1].len > 0)
        {
            const uint16_t* shortCompTable = reinterpret_cast<const uint16_t*>(headerPtr + this->shortCompValsOffset);

            int foundIdx =
                binarySearch<const uint16_t>(shortCompTable, tableSliceInfos[1].startIdx, tableSliceInfos[1].len, infIndex);

            if (foundIdx >= 0)
            {
                // Adjust from relative to absolute index
                foundIdx += strReplEntityInfo->basicCompAdjustment;

                uint16_t strOffset = strOffsetTable[foundIdx];
                return &(strTable[strOffset]);
            }
        }

        //

        // const uint16_t numBmgEntries = this->bmgStrCompsTableNumEntries;
        // if (bmgNumber >= numBmgEntries)
        // {
        //     // Bail if bmgNumber is out of range (not 0 through 8)
        //     return nullptr;
        // }

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const BmgStrCompData* bmgDataTable = reinterpret_cast<const BmgStrCompData*>(headerPtr +
        // this->bmgStrCompsTableOffset); const uint16_t* offsetsTable = reinterpret_cast<const uint16_t*>(headerPtr +
        // this->strOffsetsTableOffset);

        // // Try to find in contextComps table
        // if (context != 0)
        // {
        //     uint16_t contextCompsLength = bmgDataTable[bmgNumber].contextCompsLength;
        //     if (contextCompsLength > 0)
        //     {
        //         const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->contextCompValsOffset);

        //         const int startIdx = bmgDataTable[bmgNumber].contextCompsStartIndex;
        //         const int endIndex = startIdx + contextCompsLength;

        //         const uint32_t lookupVal = (context << 16) + infIndex;
        //         int foundIdx = binarySearch<const uint32_t>(lookupTable, startIdx, endIndex, lookupVal);
        //         if (foundIdx >= 0)
        //         {
        //             // Find in str offsets
        //             const uint16_t offsetInStrTable = offsetsTable[foundIdx];

        //             uint32_t strAddr = reinterpret_cast<uint32_t>(headerPtr) + this->strTableOffsetNew + offsetInStrTable;
        //             return reinterpret_cast<char*>(strAddr);
        //         }
        //     }
        // }

        // // Try to find in basicComps table
        // uint16_t basicCompsLength = bmgDataTable[bmgNumber].basicCompsLength;
        // if (basicCompsLength > 0)
        // {
        //     const uint16_t* lookupTable = reinterpret_cast<const uint16_t*>(headerPtr + this->basicCompValsOffset);

        //     const int startIdx = bmgDataTable[bmgNumber].basicCompsStartIndex;
        //     const int endIndex = startIdx + basicCompsLength;

        //     int foundIdx = binarySearch<const uint16_t>(lookupTable, startIdx, endIndex, infIndex);
        //     if (foundIdx >= 0)
        //     {
        //         // Find in str offsets
        //         const uint16_t offsetInStrTable = offsetsTable[this->numContextCompStrOffsets + foundIdx];

        //         uint32_t strAddr = reinterpret_cast<uint32_t>(headerPtr) + this->strTableOffsetNew + offsetInStrTable;
        //         return reinterpret_cast<char*>(strAddr);
        //     }
        // }

        return nullptr;
    }

    // const uint16_t* BMG0Section::getBranchEditData(uint16_t context, uint16_t flwIndex) const
    const uint16_t* BMG0Section::getBranchEditData(uint16_t, uint16_t) const
    {
        // if (context == 0)
        //     return nullptr;

        // const uint16_t numEntries = this->numBranchEditLookups;
        // if (numEntries == 0)
        //     return nullptr;

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->branchEditLookupsOffset);

        // uint32_t lookupVal = (flwIndex << 16) + context;
        // for (int i = 0; i < numEntries; i++)
        // {
        //     int checkIndex = i * 2;
        //     if (lookupTable[checkIndex] == lookupVal)
        //     {
        //         return reinterpret_cast<const uint16_t*>(&(lookupTable[checkIndex + 1]));
        //     }
        // }

        return nullptr;
    }

    // const uint16_t* BMG0Section::getEventEditData(uint16_t context, uint16_t flwIndex) const
    const uint16_t* BMG0Section::getEventEditData(uint16_t, uint16_t) const
    {
        // if (context == 0)
        //     return nullptr;

        // const uint16_t numEntries = this->numEventEditLookups;
        // if (numEntries == 0)
        //     return nullptr;

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint32_t* lookupTable = reinterpret_cast<const uint32_t*>(headerPtr + this->eventEditLookupsOffset);

        // uint32_t lookupVal = (flwIndex << 16) + context;
        // for (int i = 0; i < numEntries; i++)
        // {
        //     int checkIndex = i * 2;
        //     if (lookupTable[checkIndex] == lookupVal)
        //     {
        //         return reinterpret_cast<const uint16_t*>(&(lookupTable[checkIndex + 1]));
        //     }
        // }

        return nullptr;
    }

    // const uint16_t* BMG0Section::getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
    //                                                        uint16_t context,
    //                                                        uint16_t branchProcResult) const
    const uint16_t* BMG0Section::getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow*, uint16_t, uint16_t) const
    {
        return nullptr;

        // if (msgFlow == nullptr)
        //     return nullptr;

        // const uint16_t* branchEditData = getBranchEditData(context, msgFlow->field_0x10);
        // if (branchEditData == nullptr)
        //     return nullptr;

        // const uint16_t baseTableIndex = branchEditData[1];
        // if (baseTableIndex == 0xFFFF)
        // {
        //     // No result remapping
        //     return nullptr;
        // }

        // const uint16_t finalTableIndex = baseTableIndex + branchProcResult;

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint16_t* branchProcResultsTable = reinterpret_cast<const uint16_t*>(headerPtr + this->nextFlwTableOffset);

        // return &(branchProcResultsTable[finalTableIndex]);
    }

    // const uint8_t* BMG0Section::getBranchNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const
    const uint8_t* BMG0Section::getBranchNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow*, uint16_t) const
    {
        return nullptr;

        // if (msgFlow == nullptr)
        //     return nullptr;

        // const uint16_t* branchEditData = getBranchEditData(context, msgFlow->field_0x10);
        // if (branchEditData == nullptr)
        //     return nullptr;

        // const uint16_t tableIndex = branchEditData[0];

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint8_t* branchProcResultsTable = reinterpret_cast<const uint8_t*>(headerPtr + this->branchNodesOffset);

        // return &(branchProcResultsTable[tableIndex * 8]);
    }

    // const uint8_t* BMG0Section::getEventNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const
    const uint8_t* BMG0Section::getEventNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow*, uint16_t) const
    {
        return nullptr;

        // if (msgFlow == nullptr)
        //     return nullptr;

        // const uint16_t* branchEditData = getEventEditData(context, msgFlow->field_0x10);
        // if (branchEditData == nullptr)
        //     return nullptr;

        // const uint16_t tableIndex = branchEditData[0];

        // const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        // const uint8_t* branchProcResultsTable = reinterpret_cast<const uint8_t*>(headerPtr + this->eventNodesOffset);

        // return &(branchProcResultsTable[tableIndex * 8]);
    }

} // namespace mod::rando
