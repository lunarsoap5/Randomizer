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
    class BMG0Section
    {
       public:
        BMG0Section() {}
        ~BMG0Section() {}

        void init();
        const uint16_t* getNodeRemapData(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                         uint16_t flwIndex,
                                         uint16_t flowContext) const;
        uint16_t getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, bool isSelectOptionsNode) const;
        const char* getReplacementStr(uint8_t bmgNumber, uint16_t context, uint16_t infIndex) const;
        const uint16_t* getCustomBranchNextNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                                uint16_t context,
                                                uint16_t queryResult) const;
        const uint16_t* getCustomEventNextNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context) const;
        void tryPatchBranchNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context, uint8_t* mutFlowNode) const;
        void tryPatchEventNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t context, uint8_t* mutFlowNode) const;

       private:
        enum EntityInfoIdx : uint8_t
        {
            NODE_REMAP = 0,
            BRANCH_PATCH = 1,
            BRANCH_NEXT_NODE = 2,
            EVENT_PATCH = 3,
            EVENT_NEXT_NODE = 4,
            STRING_REPLACEMENT = 5,
        };

        void decryptStrings();
        int doNormalEntitySearch(uint8_t bmgNumber, uint16_t context, uint16_t idxInBlock, EntityInfoIdx entityInfoIdx) const;
        int doNodeRemapEntitySearch(uint8_t bmgNumber, uint16_t context, uint16_t flwIndex, uint16_t fliValue) const;
        void tryPatchFlowNode(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                              uint16_t context,
                              uint8_t* mutFlowNode,
                              const uint8_t* patchTable,
                              EntityInfoIdx entityInfoIdx) const;

        /* 0x00 */ char magic[4]; // "BMG0"
        /* 0x04 */ uint16_t entityInfoTableOffset;
        /* 0x06 */ uint16_t tableSliceInfosOffset;
        /* 0x08 */ uint16_t wordCompValsOffset;
        /* 0x0A */ uint16_t shortCompValsOffset;
        /* 0x0C */ uint16_t nodeRemapTableOffset;
        /* 0x0E */ uint16_t branchPatchTableOffset;
        /* 0x10 */ uint16_t branchNextNodeBaseIdxTableOffset;
        /* 0x12 */ uint16_t branchNextNodeTableOffset;
        /* 0x14 */ uint16_t eventNextNodeTableOffset;
        /* 0x16 */ uint16_t eventPatchTableOffset;
        /* 0x18 */ uint16_t strOffsetTableOffset;
        /* 0x1A */ uint16_t strTableOffset;
        /* 0x1C */ uint16_t strTableEncodedStart;
        /* 0x1E */ uint16_t encodedStrTableNumBlocks;
        /* 0x20 */ uint32_t encryptionKey[4];
        /* 0x30 */ bool isDecrypted;
    } __attribute__((__packed__));
} // namespace mod::rando
#endif