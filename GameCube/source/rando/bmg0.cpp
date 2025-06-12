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
        int high = low + len - 1;
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

    void BMG0Section::init()
    {
        if (!isDecrypted)
        {
            isDecrypted = true;
            decryptStrings();
        }
    }

    // XTEA
    void decipher(uint32_t numRounds, uint32_t* blockPtr, uint32_t* key)
    {
        uint32_t v0 = blockPtr[0];
        uint32_t v1 = blockPtr[1];
        uint32_t delta = 0x9E3779B9;
        uint32_t sum = delta * numRounds;

        for (uint32_t i = 0; i < numRounds; i++)
        {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(int)((sum >> 11) & 3)]);
            sum -= delta;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[(int)(sum & 3)]);
        }

        blockPtr[0] = v0;
        blockPtr[1] = v1;
    }

    void BMG0Section::decryptStrings()
    {
        uint8_t* headerPtr = reinterpret_cast<uint8_t*>(&this->magic);
        char* strTable = reinterpret_cast<char*>(headerPtr + this->strTableOffset);
        uint32_t* blockPtr = reinterpret_cast<uint32_t*>(strTable + this->strTableEncodedStart);

        // Init prevEncodedBlock to IV for first iteration
        uint32_t prevEncodedBlock[2];
        prevEncodedBlock[0] = this->encryptionKey[0];
        prevEncodedBlock[1] = this->encryptionKey[1];

        uint32_t numBlocks = this->encodedStrTableNumBlocks;
        for (uint32_t blockIdx = 0; blockIdx < numBlocks; blockIdx++)
        {
            uint32_t tempBlock0 = blockPtr[0];
            uint32_t tempBlock1 = blockPtr[1];

            decipher(2, blockPtr, this->encryptionKey);
            blockPtr[0] ^= prevEncodedBlock[0];
            blockPtr[1] ^= prevEncodedBlock[1];

            prevEncodedBlock[0] = tempBlock0;
            prevEncodedBlock[1] = tempBlock1;
            blockPtr += 2;
        }
    }

    uint8_t getCurrentBmgNumber(libtp::tp::d_msg_flow::dMsgFlow* msgFlow)
    {
        // FLI value of 3000 (0xbb8) or more indicates to use the global bmg
        // zel_00 and not the stage's bmg. This is vanilla behavior seen in
        // dMsgObject_c::changeFlowGroupLocal. Note that the FLI value is
        // treated as an s16 by dMsgObject_c which is why our max custom FLI is
        // only 0x7FFF and not 0xFFFF.
        if (msgFlow->mFlow >= 3000)
        {
            return 0;
        }
        return libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mStageData.mStagInfo->mMsgGroup;
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

    const TableSliceInfo* BMG0Section::getTableSliceInfo(const EntityInfo* lookup, uint8_t bmgNumber) const
    {
        static const uint8_t NIBBLE_LOOKUP[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

        uint32_t bmgLookup = lookup->bmgLookup;

        uint32_t hasDataMask = 1 << bmgNumber;
        if ((bmgLookup & hasDataMask) == 0)
            return nullptr;

        uint32_t relativeVal = 0;
        if (bmgNumber > 0)
        {
            // Count how many entries are defined for earlier bmgNumbers.
            uint32_t prevBits = bmgLookup & (hasDataMask - 1); // Max result is 0xFF.
            relativeVal = NIBBLE_LOOKUP[prevBits & 0x0F] + NIBBLE_LOOKUP[prevBits >> 4];
        }

        uint32_t baseOffset = (bmgLookup >> 9) & 0x7F;
        uint32_t finalOffset = baseOffset + relativeVal;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const TableSliceInfo* tableSliceInfoTable =
            reinterpret_cast<const TableSliceInfo*>(headerPtr + this->tableSliceInfosOffset);

        return &(tableSliceInfoTable[finalOffset]);
    }

    template<typename T>
    int searchCompTable(const TableSliceInfo* tableSliceInfo, const T* compTable, T compVal, int16_t idxAdjustment)
    {
        if (tableSliceInfo != nullptr && tableSliceInfo->len > 0)
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
        // Note: param `idxInBlock` is either a FLW index or an INF index
        // depending on the entity type.
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const EntityInfo* entityInfoTable = reinterpret_cast<const EntityInfo*>(headerPtr + this->entityInfoTableOffset);

        const EntityInfo* entityInfo = &(entityInfoTable[entityInfoIdx * 2]);
        const TableSliceInfo* tableSliceInfo = getTableSliceInfo(entityInfo, bmgNumber);

        // Context compare
        if (context > 0)
        {
            const uint32_t* wordCompTable = reinterpret_cast<const uint32_t*>(headerPtr + this->wordCompValsOffset);
            const uint32_t lookupVal = (context << 16) + idxInBlock;
            int idx = searchCompTable(tableSliceInfo, wordCompTable, lookupVal, entityInfo->idxAdjustment);
            if (idx >= 0)
                return idx;
        }

        entityInfo += 1; // Point to next entry for basic lookup
        const TableSliceInfo* tableSliceInfoBasic = getTableSliceInfo(entityInfo, bmgNumber);

        // Basic compare
        const uint16_t* shortCompTable = reinterpret_cast<const uint16_t*>(headerPtr + this->shortCompValsOffset);
        int idx = searchCompTable(tableSliceInfoBasic, shortCompTable, idxInBlock, entityInfo->idxAdjustment);
        return idx; // Returns index or -1 if not found
    }

    int BMG0Section::doNodeRemapEntitySearch(uint8_t bmgNumber, uint16_t context, uint16_t flwIndex, uint16_t fliValue) const
    {
        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
        const EntityInfo* entityInfoTable = reinterpret_cast<const EntityInfo*>(headerPtr + this->entityInfoTableOffset);
        const EntityInfo* entityInfo = &(entityInfoTable[EntityInfoIdx::NODE_REMAP * 2]);
        const TableSliceInfo* tableSliceInfo = getTableSliceInfo(entityInfo, bmgNumber);

        const uint32_t* wordCompTable = reinterpret_cast<const uint32_t*>(headerPtr + this->wordCompValsOffset);

        // Context compare
        if (context > 0)
        {
            const uint32_t ctxLookupVal = (context << 16) + flwIndex;
            int idx = searchCompTable(tableSliceInfo, wordCompTable, ctxLookupVal, entityInfo->idxAdjustment);
            if (idx >= 0)
                return idx;
        }

        // FLI compare
        if (flwIndex != 0xFFFF || context == 0)
        {
            entityInfo += 1; // Point to next entry for basic lookup
            const TableSliceInfo* tableSliceInfoBasic = getTableSliceInfo(entityInfo, bmgNumber);

            // Only allow remapping 0xFFFF using an FLI value when we do not
            // have a flowContext. This is to help avoid infinite loops caused
            // by developer error.
            const uint32_t basicLookupVal = (fliValue << 0x10) + flwIndex;
            int idx = searchCompTable(tableSliceInfoBasic, wordCompTable, basicLookupVal, entityInfo->idxAdjustment);
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
            if (isDecrypted || strOffset < this->strTableEncodedStart)
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
    }

    const uint16_t* BMG0Section::getCustomEventNextNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const
    {
        uint8_t bmgNumber = getCurrentBmgNumber(msgFlow);
        uint16_t flwIdx = msgFlow->field_0x10;

        int foundIdx = doNormalEntitySearch(bmgNumber, context, flwIdx, EntityInfoIdx::EVENT_NEXT_NODE);
        if (foundIdx >= 0)
        {
            const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->magic);
            const uint16_t* nextNodeTable = reinterpret_cast<const uint16_t*>(headerPtr + this->eventNextNodeTableOffset);

            return &(nextNodeTable[foundIdx]);
        }
        return nullptr;
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
