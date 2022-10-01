/**	@file clr0.cpp
 *  @brief Handles reading color data from the CLR0 chunk of the seed.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */

#include "rando/clr0.h"

namespace mod::rando
{
    uint32_t CLR0_AS_U32 = 0x434C5230;     // "CLR0" as a u32

    // Maps nibble value to how many bits it has set.
    uint8_t NIBBLE_LOOKUP[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    // Maps a bit index to a bit mask which can be used to clear all bits but
    // the ones to the right of the given bit index.
    uint8_t BITS_TO_RIGHT_MASK[8] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };

    // bitIndex is one of 0,1,2,3,4,5,6,7. 0 means the least significant bit.
    // Given a bit index, returns the number of bits which have a lower index and are set.
    inline uint8_t countSetBitsToRight( uint8_t byte, uint8_t bitIndex )
    {
        if ( bitIndex == 0 )
        {
            return 0;
        }

        uint8_t maskedByte = byte & BITS_TO_RIGHT_MASK[bitIndex];

        uint8_t result = NIBBLE_LOOKUP[maskedByte & 0x0f];

        if ( bitIndex >= 5 )
        {
            result += NIBBLE_LOOKUP[maskedByte >> 4];
        }

        return result;
    }

    uint32_t* CLR0::getBasicDataEntryPtr( RecolorId recolorId, RecolorType desiredRecolorType )
    {
        if ( *reinterpret_cast<uint32_t*>( magic ) != CLR0_AS_U32 || bitTableOffset == 0 || recolorId < minRecolorId ||
             recolorId > maxRecolorId )
        {
            // If magic doesn't match or bitTableOffset is 0 (meaning CLR0
            // stores no colors) or recolorEnum is not in range, then return
            // null result.
            return nullptr;
        }

        uint8_t* thisAddr = reinterpret_cast<uint8_t*>( this );
        uint8_t* bitTable = thisAddr + this->bitTableOffset;

        uint16_t diff = recolorId - minRecolorId;

        uint8_t bitTableIndex = diff >> 3;
        uint8_t bitIndex = diff & 0x7;
        uint8_t bitTableEntry = bitTable[bitTableIndex];

        if ( ( bitTableEntry & ( 1 << bitIndex ) ) == 0 )
        {
            // This RecolorId is not enabled, so return nullptr.
            return nullptr;
        }

        uint16_t cummSum = 0;
        if ( bitTableIndex > 0 )
        {
            uint16_t* cummulativeSums = reinterpret_cast<uint16_t*>( thisAddr + cummSumsOffset );
            cummSum = cummulativeSums[bitTableIndex - 1];
        }

        uint16_t basicDataIndex = cummSum + countSetBitsToRight( bitTableEntry, bitIndex );

        uint32_t* basicDataTable = reinterpret_cast<uint32_t*>( thisAddr + basicDataOffset );

        uint32_t* basicDataEntryPtr = &basicDataTable[basicDataIndex];

        uint8_t actualRecolorType = ( *basicDataEntryPtr >> 24 ) & 0xff;
        if ( actualRecolorType != desiredRecolorType )
        {
            return nullptr;
        }

        return basicDataEntryPtr;
    }

    uint8_t* CLR0::getComplexDataEntryPtr( RecolorId recolorId, RecolorType desiredRecolorType )
    {
        // Only return complexData if recolorType is one which has complexData.
        // Right now the only one which does is RgbArray.
        if ( desiredRecolorType != RecolorType::RgbArray )
        {
            return nullptr;
        }

        uint32_t* basicDataEntryPtr = getBasicDataEntryPtr( recolorId, desiredRecolorType );

        if ( basicDataEntryPtr == nullptr )
        {
            return nullptr;
        }

        uint32_t complexDataEntryOffset = *basicDataEntryPtr & 0x00FFFFFF;

        uint8_t* thisAddr = reinterpret_cast<uint8_t*>( this );
        uint8_t* complexData = thisAddr + this->complexDataOffset;

        return complexData + complexDataEntryOffset;
    }

    uint8_t* CLR0::getRecolorRgb( RecolorId recolorId )
    {
        uint32_t* basicDataEntryPtr = getBasicDataEntryPtr( recolorId, RecolorType::Rgb );

        if ( basicDataEntryPtr == nullptr )
        {
            return nullptr;
        }

        return reinterpret_cast<uint8_t*>( basicDataEntryPtr ) + 1;
    }

    bool CLR0::getRecolorRgbArray( RecolorId recolorId, CLR0RgbArray* outStruct )
    {
        uint8_t* complexDataEntryPtr = getComplexDataEntryPtr( recolorId, RecolorType::RgbArray );

        if ( complexDataEntryPtr == nullptr )
        {
            return false;
        }

        // Fill in struct.
        outStruct->arrLength = complexDataEntryPtr[0];
        outStruct->rgbArrPtr = complexDataEntryPtr + 1;

        return true;
    }
}     // namespace mod::rando