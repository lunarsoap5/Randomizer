/**	@file clr0.h
 *  @brief Handles reading recoloring data from the seed data.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_CLR0_H
#define RANDO_CLR0_H

#include <cstdint>

namespace mod::rando
{
    // Each RecolorId represents a thing or group of things which are considered
    // a single unit and which should all be recolored using a specific color
    // when the user says "I want this thing to be this color". Basically, a
    // single RecolorId does NOT equal a single texture.
    // - Intentionally uint16_t (going over 0xFF is not impossible, and the CLR0
    //   chunk was designed to support up to 0xFFFF).
    enum RecolorId : uint16_t
    {
        HerosClothes = 0x00,     // Cap and body
        ABtn = 0x01,
        BBtn = 0x02,
        XBtn = 0x03,
        YBtn = 0x04,
        ZBtn = 0x05,
        LanternGlow = 0x06,
        Hearts = 0x07,
        // ZoraArmorPrimary = 0x01,
        // ZoraArmorSecondary = 0x02,
        // ZoraArmorHelmet = 0x03,
    };

    struct CLR0RgbArray
    {
        uint8_t* rgbArrPtr;
        uint8_t arrLength;
    };

    class CLR0
    {
       private:
        // Should always be "CLR0".
        /* 0x00 */ char magic[4];
        // Total byte size of the entire CLR0 chunk including the header and any
        // padding.
        /* 0x04 */ uint32_t totalByteLength;
        // Unused; always 0 for now.
        /* 0x08 */ uint8_t reserved;
        // Offset to bitTable section relative to start of header. This is 0 if
        // there are no colors stored in the CLR0, else it is 0x16 which is also
        // the header size.
        /* 0x09 */ uint8_t bitTableOffset;
        /* 0x0A */ uint16_t minRecolorId;
        // Any RecolorId larger than this is guaranteed to not have any RGB
        // values in the data. Generated seed should set this value as low as
        // possible based on selected user settings.
        /* 0x0C */ uint16_t maxRecolorId;
        // Offset to cummulativeSums section relative to start of header, or 0
        // if section is empty.
        /* 0x0E */ uint16_t cummSumsOffset;
        // Offset to complexData section relative to start of header, or 0 if
        // section is empty.
        /* 0x10 */ uint32_t complexDataOffset;
        // Offset to basicData section relative to start of header, or 0 if
        // section is empty.
        /* 0x14 */ uint16_t basicDataOffset;

       public:
        /**
         * @brief If the RecolorId has data in the CLR0 chunk, returns a pointer
         * to a 3 byte array (R,G,B). Else returns nullptr (meaning recolor
         * should not occur).
         *
         * @return uint8_t* Pointer to 3 bytes (R,G,B), or nullptr.
         */
        uint8_t* getRecolorRgb( RecolorId );
        bool getRecolorRgbArray( RecolorId recolorId, CLR0RgbArray* out );

       private:
        enum RecolorType : uint8_t
        {
            Rgb = 0,
            RgbArray = 1,
            Invalid = 0xFF,
        };

        uint32_t* getBasicDataEntryPtr( RecolorId, RecolorType );
        uint8_t* getComplexDataEntryPtr( RecolorId, RecolorType );

    } __attribute__( ( __packed__ ) );

}     // namespace mod::rando
#endif