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
    class FlwIdxRemap
    {
       public:
        FlwIdxRemap() {}
        ~FlwIdxRemap() {}

        uint16_t getFLIValue() const { return this->fliValue; }
        uint16_t getOldInitFLWIndex() const { return this->oldInitFlwIndex; }
        uint16_t getNewInitFLWIndex() const { return this->newInitFlwIndex; }

       private:
        uint16_t fliValue;
        uint16_t oldInitFlwIndex;
        uint16_t newInitFlwIndex;
        uint16_t padding;
    } __attribute__((__packed__));

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

    class BMG0Section
    {
       public:
        BMG0Section() {}
        ~BMG0Section() {}

        uint16_t getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t flwIndex) const;
        uint16_t getCustomINFIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow) const;

       private:
        /* 0x00 */ uint16_t signToInitFliOffset;
        /* 0x02 */ uint16_t numSignToInitFliOffsetEntries;
        /* 0x04 */ uint16_t flwIdxRemapOffset;
        /* 0x06 */ uint16_t numFlwIdxRemapEntries;
        /* 0x08 */ uint16_t infRemapOffset;
        /* 0x0A */ uint16_t numInfRemapEntries;
        /* 0x0C */ uint32_t padding;
    } __attribute__((__packed__));
} // namespace mod::rando
#endif