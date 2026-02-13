/**	@file returnPlaces.h
 *  @brief Contains classes for managing return places for S+Q and Midna.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#include "rando/returnPlaces.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_stage.h"
#include "rando/randomizer.h"

namespace mod::rando
{
    const ReturnPlace* ReturnPlaceSection::getReturnPlace(uint8_t stageIDX, int8_t roomNo, int8_t point, int8_t layer) const
    {
        using namespace libtp::tp;
        using namespace libtp::tp::d_com_inf_game;

        const uint8_t* matchIndexPtr = nullptr;

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->numComparisons);
        const Comparison* comparisonsTable = reinterpret_cast<const Comparison*>(headerPtr + this->comparisonsOffset);

        uint32_t totalEntries = this->numComparisons;
        for (uint32_t i = 0; i < totalEntries; i++)
        {
            const Comparison* comparison = &comparisonsTable[i];
            if (comparison->stageIDX == stageIDX)
            {
                int8_t compRoomNo = comparison->roomNo;
                int8_t compPoint = comparison->point;
                int8_t compLayer = comparison->layer;

                if ((compRoomNo == -1 || compRoomNo == roomNo) && (compPoint == -1 || compPoint == point) &&
                    (compLayer == -1 || compLayer == layer))
                {
                    const uint8_t* matchIndexTable = headerPtr + this->matchIndexOffset;
                    matchIndexPtr = &matchIndexTable[i];
                    break;
                }
            }
        }

        // Returns nullptr if no mapping was found, else returns a pointer to the new place to store. A stageIdx of 0xFF
        // in the returned pointer represents that there is no valid returnPlace.
        if (matchIndexPtr != nullptr)
        {
            uint8_t index = *matchIndexPtr;
            if (index < this->numReturnPlaces)
            {
                const ReturnPlace* returnPlacesTable =
                    reinterpret_cast<const ReturnPlace*>(headerPtr + this->returnPlacesOffset);
                return &returnPlacesTable[index];
            }
        }
        return nullptr;
    }

} // namespace mod::rando