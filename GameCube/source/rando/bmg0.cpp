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

    uint8_t getCurrentBmgNumber(libtp::tp::d_msg_flow::dMsgFlow* msgFlow)
    {
        if (msgFlow->mFlow_p != getZel00BmgFlw())
        {
            return libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStageData.mStagInfo->mMsgGroup;
        }
        return 0;
    }

    uint16_t BMG0Section::getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, bool isSelectOptionsNode) const
    {
        // Use simple implementation for now. This is only used by custom hint
        // signs at the moment, so it makes more sense to not encode a fancy
        // format in the seed data which would only achieve what we have here in
        // many more steps.
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

    void BMG0Section::getTableSliceInfos(const EntityInfo* entityInfo,
                                         uint8_t bmgNumber,
                                         TableSliceInfo* outTableSliceInfos) const
    {
        if (bmgNumber >= 9)
        {
            outTableSliceInfos[0].len = 0;
            outTableSliceInfos[1].len = 0;
            return;
        }

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const TableSliceInfo* tableSliceInfoTable =
            reinterpret_cast<const TableSliceInfo*>(headerPtr + this->tableSliceInfosOffset);

        uint8_t rawBmgByte = entityInfo->bmgLookupBytes[bmgNumber];
        uint16_t baseIdx = entityInfo->tableSliceInfoStartIdx;

        for (int i = 0; i < 2; i++)
        {
            uint8_t offset = (rawBmgByte >> (i * 4)) & 0x0F;
            if (offset < 9)
            {
                uint16_t idx = baseIdx + offset;
                outTableSliceInfos[i].startIdx = tableSliceInfoTable[idx].startIdx;
                outTableSliceInfos[i].len = tableSliceInfoTable[idx].len;
            }
            else
            {
                outTableSliceInfos[i].len = 0;
            }
        }
    }

    template<typename T>
    int searchCompTable(TableSliceInfo* tableSliceInfo, const T* compTable, T compVal, int16_t idxAdjustment)
    {
        if (tableSliceInfo->len > 0)
        {
            int foundIdx = binarySearch<const T>(compTable, tableSliceInfo->startIdx, tableSliceInfo->len, compVal);

            if (foundIdx >= 0)
            {
                // Adjust from relative to absolute index
                return foundIdx + idxAdjustment;
            }
        }
        return -1;
    }

    int BMG0Section::doNormalEntitySearch(uint8_t bmgNumber,
                                          uint16_t context,
                                          uint16_t idxInBlock,
                                          EntityInfoIdx entityInfoIdx) const
    {
        // Note: idxInBlock is either a FLW index or an INF index depending on
        // the entity type.
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const EntityInfo* entityInfoTable = reinterpret_cast<const EntityInfo*>(headerPtr + this->entityInfoTableOffset);

        const EntityInfo* strReplEntityInfo = &(entityInfoTable[entityInfoIdx]);
        TableSliceInfo tableSliceInfos[2];
        getTableSliceInfos(strReplEntityInfo, bmgNumber, tableSliceInfos);

        // Context compare
        if (context > 0)
        {
            const uint32_t* wordCompTable = reinterpret_cast<const uint32_t*>(headerPtr + this->wordCompValsOffset);
            const uint32_t lookupVal = (context << 16) + idxInBlock;
            int idx = searchCompTable(tableSliceInfos, wordCompTable, lookupVal, strReplEntityInfo->ctxCompAdjustment);
            if (idx >= 0)
                return idx;
        }

        // Basic compare
        const uint16_t* shortCompTable = reinterpret_cast<const uint16_t*>(headerPtr + this->shortCompValsOffset);
        int idx = searchCompTable(tableSliceInfos, shortCompTable, idxInBlock, strReplEntityInfo->basicCompAdjustment);
        if (idx >= 0)
            return idx;

        return -1;
    }

    int BMG0Section::doNodeRemapEntitySearch(uint8_t bmgNumber, uint16_t context, uint16_t flwIndex, uint16_t fliValue) const
    {
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const EntityInfo* entityInfoTable = reinterpret_cast<const EntityInfo*>(headerPtr + this->entityInfoTableOffset);
        const EntityInfo* entityInfo = &(entityInfoTable[EntityInfoIdx::NODE_REMAP]);
        TableSliceInfo tableSliceInfos[2];
        getTableSliceInfos(entityInfo, bmgNumber, tableSliceInfos);

        const uint32_t* wordCompTable = reinterpret_cast<const uint32_t*>(headerPtr + this->wordCompValsOffset);

        // Context compare
        if (context > 0)
        {
            const uint32_t ctxLookupVal = (context << 16) + flwIndex;
            int idx = searchCompTable(tableSliceInfos, wordCompTable, ctxLookupVal, entityInfo->ctxCompAdjustment);
            if (idx >= 0)
                return idx;
        }

        // FLI compare
        if (flwIndex != 0xFFFF || context == 0)
        {
            // Only allow remapping 0xFFFF using an FLI value when we do not
            // have a flowContext. This is to help avoid infinite loops caused
            // by developer error.
            const uint32_t basicLookupVal = (fliValue << 0x10) + flwIndex;
            int idx = searchCompTable(&(tableSliceInfos[1]), wordCompTable, basicLookupVal, entityInfo->basicCompAdjustment);
            if (idx >= 0)
                return idx;
        }

        return -1;
    }

    const uint16_t* BMG0Section::getNodeRemapData(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                                  uint16_t flwIndex,
                                                  uint16_t context) const
    {
        if (msgFlow == nullptr)
            return nullptr;

        uint8_t bmgNumber = getCurrentBmgNumber(msgFlow);

        int foundIdx = doNodeRemapEntitySearch(bmgNumber, context, flwIndex, msgFlow->mFlow);
        if (foundIdx >= 0)
        {
            // Get from results table
            const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
            const uint32_t* nodeRemapTable = reinterpret_cast<const uint32_t*>(headerPtr + this->nodeRemapTableOffset);

            const uint32_t* ptr = &(nodeRemapTable[foundIdx]);
            return reinterpret_cast<const uint16_t*>(ptr);
        }

        return nullptr;
    }

    const char* BMG0Section::getReplacementStr(uint8_t bmgNumber, uint16_t context, uint16_t infIndex) const
    {
        int foundIdx = doNormalEntitySearch(bmgNumber, context, infIndex, EntityInfoIdx::STRING_REPLACEMENT);
        if (foundIdx >= 0)
        {
            const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
            const uint16_t* strOffsetTable = reinterpret_cast<const uint16_t*>(headerPtr + this->strOffsetTableOffset);
            const char* strTable = reinterpret_cast<const char*>(headerPtr + this->strTableOffset);

            uint16_t strOffset = strOffsetTable[foundIdx];
            return &(strTable[strOffset]);
        }
        return nullptr;
    }

    // const uint16_t* BMG0Section::getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
    //                                                        uint16_t context,
    //                                                        uint16_t branchProcResult) const
    const uint16_t* BMG0Section::getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                                           uint16_t context,
                                                           uint16_t branchProcResult) const
    {
        uint8_t bmgNumber = getCurrentBmgNumber(msgFlow);
        uint16_t flwIdx = msgFlow->field_0x10;

        int foundIdx = doNormalEntitySearch(bmgNumber, context, flwIdx, EntityInfoIdx::BRANCH_NEXT_NODE);
        if (foundIdx >= 0)
        {
            const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
            const uint16_t* baseIdxTable =
                reinterpret_cast<const uint16_t*>(headerPtr + this->branchNextNodeBaseIdxTableOffset);

            const uint16_t baseIdx = baseIdxTable[foundIdx];
            const uint16_t finalIdx = baseIdx + branchProcResult;

            const uint16_t* nextNodeTable = reinterpret_cast<const uint16_t*>(headerPtr + this->branchNextNodeTableOffset);
            return &(nextNodeTable[finalIdx]);
        }
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

    void BMG0Section::tryPatchFlowNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                       uint16_t context,
                                       uint8_t* mutFlowNode,
                                       const uint8_t* patchTable,
                                       EntityInfoIdx entityInfoIdx) const
    {
        if (msgFlow == nullptr || mutFlowNode == nullptr)
            return;

        uint8_t bmgNumber = getCurrentBmgNumber(msgFlow);
        uint16_t currFlwIdx = msgFlow->field_0x10;

        int foundIdx = doNormalEntitySearch(bmgNumber, context, currFlwIdx, entityInfoIdx);
        if (foundIdx >= 0)
        {
            const uint8_t* patchData = patchTable + foundIdx * 8;

            uint8_t magicByte = patchData[0];

            for (int i = 1; i < 8; i++)
            {
                uint8_t patchByte = patchData[i];
                if (patchByte != magicByte)
                {
                    mutFlowNode[i] = patchByte;
                }
            }
        }
    }

    void BMG0Section::tryPatchBranchNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context, uint8_t* mutFlowNode) const
    {
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const uint8_t* branchPatchTable = reinterpret_cast<const uint8_t*>(headerPtr + this->branchPatchTableOffset);

        tryPatchFlowNode(msgFlow, context, mutFlowNode, branchPatchTable, EntityInfoIdx::BRANCH_PATCH);
    }

    void BMG0Section::tryPatchEventNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context, uint8_t* mutFlowNode) const
    {
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const uint8_t* eventPatchTable = reinterpret_cast<const uint8_t*>(headerPtr + this->eventPatchTableOffset);

        tryPatchFlowNode(msgFlow, context, mutFlowNode, eventPatchTable, EntityInfoIdx::EVENT_PATCH);
    }

} // namespace mod::rando
