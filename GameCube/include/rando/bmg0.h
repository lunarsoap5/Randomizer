/**	@file bmg0.h
 *  @brief Contains classes for BMG and message flows.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_BMG0_H
#define RANDO_BMG0_H

#include "tp/d_msg_flow.h"

#include <cstdint>

namespace mod::rando
{
    enum EntityInfoIdx : uint8_t
    {
        NODE_REMAP = 0,
        BRANCH_PATCH = 1,
        BRANCH_NEXT_NODE = 2,
        EVENT_PATCH = 3,
        EVENT_NEXT_NODE = 4,
        STRING_REPLACEMENT = 5,
    };

    class InfRemap
    {
       public:
        InfRemap() {}
        ~InfRemap() {}

        uint16_t getBitMask() const { return this->bitMask; }
        uint16_t getFLIValue() const { return this->fliValue; }
        uint16_t getFLWIndex() const { return this->flwIndex; }
        uint16_t getNewINFIndex() const { return this->newInfIndex; }

       private:
        uint16_t bitMask;
        uint16_t fliValue;
        uint16_t flwIndex;
        uint16_t newInfIndex;
    } __attribute__((__packed__));

    class EntityInfo
    {
       public:
        EntityInfo() {}
        ~EntityInfo() {}

        /* 0x00 */ int16_t ctxCompAdjustment;
        /* 0x02 */ int16_t basicCompAdjustment;
        /* 0x04 */ uint8_t tableSliceInfoStartIdx;
        /* 0x05 */ uint8_t bmgLookupBytes[9];
        // Size: 0x1E bytes
    } __attribute__((__packed__));

    class TableSliceInfo
    {
       public:
        TableSliceInfo() {}
        ~TableSliceInfo() {}

       public:
        uint16_t startIdx;
        uint16_t len;
    } __attribute__((__packed__));

    class BMG0Section
    {
       public:
        BMG0Section() {}
        ~BMG0Section() {}

        const uint16_t* getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                               uint16_t flwIndex,
                                               uint16_t flowContext) const;
        uint16_t getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, bool isSelectOptionsNode) const;
        const char* getReplacementStr(uint8_t bgmNumber, uint16_t context, uint16_t infIndex) const;
        const uint16_t* getCustomBranchResultNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                                  uint16_t context,
                                                  uint16_t branchProcResult) const;
        const uint8_t* getBranchNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const;
        const uint8_t* getEventNodeReplacement(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const;

       private:
        void getTableSliceInfos(const EntityInfo* entityInfo, uint8_t bmgNumber, TableSliceInfo* outTableSliceInfos) const;
        const uint16_t* getBranchEditData(uint16_t context, uint16_t flwIndex) const;
        const uint16_t* getEventEditData(uint16_t context, uint16_t flwIndex) const;

        /* 0x00 */ char magic[4];
        /* 0x04 */ uint16_t signToInitFliOffset;
        /* 0x06 */ uint16_t numSignToInitFliOffsetEntries;
        /* 0x08 */ uint16_t entityInfoTableOffset;
        /* 0x0A */ uint16_t tableSliceInfosOffset;
        /* 0x0C */ uint16_t wordCompValsOffset;
        /* 0x0E */ uint16_t shortCompValsOffset;
        /* 0x10 */ uint16_t nodeRemapTableOffset;
        /* 0x12 */ uint16_t branchPatchTableOffset;
        /* 0x14 */ uint16_t branchNextNodeBaseIdxTableOffset;
        /* 0x16 */ uint16_t branchNextNodeTableOffset;
        /* 0x18 */ uint16_t eventPatchTableOffset;
        /* 0x1A */ uint16_t strOffsetTableOffset;
        /* 0x1C */ uint16_t strTableOffset;
        /* 0x1E */ uint16_t strTableLen;
    } __attribute__((__packed__));
} // namespace mod::rando
#endif