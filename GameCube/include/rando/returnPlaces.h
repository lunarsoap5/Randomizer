/**	@file returnPlaces.h
 *  @brief Contains classes for managing return places for S+Q and Midna.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_RETURNPLACES_H
#define RANDO_RETURNPLACES_H

#include "tp/d_stage.h"

#include <cstdint>

namespace mod::rando
{
    class ReturnPlace
    {
       public:
        ReturnPlace() {}
        ~ReturnPlace() {}

        uint8_t getStageIDX() const { return this->stageIDX; }
        uint8_t getRoomNo() const { return this->roomNo; }
        uint8_t getPoint() const { return this->point; }
        int8_t getLayer() const { return this->layer; }

       private:
        uint8_t stageIDX;
        uint8_t roomNo;
        uint8_t point;
        int8_t layer;
    } __attribute__((__packed__));

    class ReturnPlaceSection
    {
       public:
        ReturnPlaceSection() {}
        ~ReturnPlaceSection() {}

        const ReturnPlace* getReturnPlace(uint8_t stageIDX, int8_t roomNo, int8_t point, int8_t layer) const;

       private:
        class Comparison
        {
           public:
            Comparison() {}
            ~Comparison() {}

            uint8_t stageIDX;
            int8_t roomNo;
            int8_t point;
            int8_t layer;
        } __attribute__((__packed__));

        /* 0x00 */ uint16_t numComparisons;
        /* 0x02 */ uint16_t matchIndexOffset;
        /* 0x04 */ uint16_t comparisonsOffset;
        /* 0x06 */ uint16_t numReturnPlaces;
        /* 0x08 */ uint16_t returnPlacesOffset;
    } __attribute__((__packed__));

} // namespace mod::rando
#endif