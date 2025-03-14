/**	@file bmg0.cpp
 *  @brief BMG0 section in seed data
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#include "rando/bmg0.h"

namespace mod::rando
{
    uint16_t BMG0Section::getCustomInitNodeIndex(libtp::tp::d_msg_flow::dMsgFlow* msgFlow, uint16_t flwIndex) const
    {
        if (msgFlow == nullptr)
            return -1;

        // Only allow remapping 0xFFFF if the msgFlow itself is being
        // initialized. Otherwise you can get stuck in a loop of messages when
        // it tries to exit normally using 0xFFFF.
        if (flwIndex == 0xFFFF && msgFlow->mMsg != 0xFFFFFFFF)
            return -1;

        const uint16_t targetFLIValue = msgFlow->mFlow;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->signToInitFliOffset);
        const FlwIdxRemap* entries = reinterpret_cast<const FlwIdxRemap*>(headerPtr + this->flwIdxRemapOffset);
        const uint16_t num_entries = this->numFlwIdxRemapEntries;

        for (uint32_t i = 0; i < num_entries; i++)
        {
            if (entries[i].getFLIValue() == targetFLIValue && entries[i].getOldInitFLWIndex() == flwIndex)
            {
                return entries[i].getNewInitFLWIndex();
            }
        }
        return -1;
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
} // namespace mod::rando
