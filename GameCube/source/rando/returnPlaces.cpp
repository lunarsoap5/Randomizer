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
    const uint8_t* ReturnPlaceSection::getMatchIndex(uint8_t stageIDX, libtp::tp::d_stage::dStage_startStage* startStgPtr) const
    {
        using namespace libtp::tp;
        using namespace libtp::tp::d_com_inf_game;

        // Check if there is any special handling for our S+Q location based on where we are loading into. Returns a
        // pointer to an index. If we return nullptr, then there is no special handling. Otherwise, the returned pointer
        // should be passed in to the `getReturnPlace` function below.

        const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->numComparisons);
        const Comparison* comparisonsTable = reinterpret_cast<const Comparison*>(headerPtr + this->comparisonsOffset);

        uint32_t totalEntries = this->numComparisons;
        for (uint32_t i = 0; i < totalEntries; i++)
        {
            const Comparison* comparison = &comparisonsTable[i];

            if (comparison->stageIDX == stageIDX)
            {
                int8_t roomNo = comparison->roomNo;
                int8_t point = comparison->point;
                int8_t layer = comparison->layer;

                if ((roomNo == -1 || roomNo == startStgPtr->mRoomNo) && (point == -1 || point == startStgPtr->mPoint) &&
                    (layer == -1 || layer == startStgPtr->mLayer))
                {
                    const uint8_t* matchIndexTable = headerPtr + this->matchIndexOffset;
                    return &matchIndexTable[i];
                }
            }
        }
        return nullptr;
    }

    const ReturnPlace* ReturnPlaceSection::getReturnPlace(const uint8_t* matchIndex) const
    {
        // Returns nullptr if the place we are loading into is not valid to store. Else returns the new place to store.
        // This function should only be called using a non-null return value from the above `getMatchIndex` function
        if (matchIndex != nullptr)
        {
            uint8_t index = *matchIndex;

            if (index < this->numReturnPlaces)
            {
                const uint8_t* headerPtr = reinterpret_cast<const uint8_t*>(&this->numComparisons);
                const ReturnPlace* returnPlacesTable =
                    reinterpret_cast<const ReturnPlace*>(headerPtr + this->returnPlacesOffset);

                return &returnPlacesTable[index];
            }
        }
        return nullptr;
    }

} // namespace mod::rando