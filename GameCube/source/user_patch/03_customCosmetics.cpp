/** @file 03_customCosmetics.cpp
 * @author Lunar Soap
 * @brief Contains function definitions for patching cosmetics.
 *
 * @bug No known bugs
 */
#include "user_patch/03_customCosmetics.h"

#include "events.h"
#include "main.h"
#include "data/items.h"
#include "tp/d_a_alink.h"
#include "tp/d_meter2_info.h"
#include "user_patch/user_patch.h"
#include "rando/clr0.h"

namespace mod::user_patch
{
    void getLanternGlowColor( uint8_t* outLanternColors )
    {
        uint8_t* lanternGlowRgb = randomizer->getRecolorRgb( rando::RecolorId::LanternGlow );
        if ( lanternGlowRgb != nullptr )
        {
            outLanternColors[0] = lanternGlowRgb[0];
            outLanternColors[1] = lanternGlowRgb[1];
            outLanternColors[2] = lanternGlowRgb[2];
            outLanternColors[3] = lanternGlowRgb[0];
            outLanternColors[4] = lanternGlowRgb[1];
            outLanternColors[5] = lanternGlowRgb[2];
        }
        else
        {
            // Default colors
            outLanternColors[0] = 0x50;
            outLanternColors[1] = 0x28;
            outLanternColors[2] = 0x14;
            outLanternColors[3] = 0x28;
            outLanternColors[4] = 0x1E;
            outLanternColors[5] = 0x0A;
        }
    }

    // Returns a u32 where the first 3 bytes are R, G, and B, and the 4th byte
    // is always 0xFF.
    uint32_t getButtonColor( rando::RecolorId recolorId )
    {
        uint8_t* aBtnRgb = randomizer->getRecolorRgb( recolorId );
        if ( aBtnRgb != nullptr )
        {
            return *reinterpret_cast<uint32_t*>( aBtnRgb ) | 0xFF;
        }
        return 0xFFFFFFFF;
    }

    void setHUDCosmetics( rando::Randomizer* randomizer )
    {
        // Make sure the randomizer is loaded/enabled and a seed is loaded
        if ( !getCurrentSeed( randomizer ) )
        {
            return;
        }

        using namespace libtp::tp::d_meter2_info;
        using namespace libtp::data::items;

        libtp::tp::d_meter2_draw::dMeter2Draw_c* mpMeterDraw = g_meter2_info.mMeterClass->mpMeterDraw;

        uint32_t mWindowARaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpButtonA->mWindow );
        uint32_t mWindowBRaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpButtonB->mWindow );
        uint32_t mWindowXRaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpButtonXY[0]->mWindow );
        uint32_t mWindowYRaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpButtonXY[1]->mWindow );
        uint32_t mWindowZRaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpButtonXY[2]->mWindow );

        const uint32_t aButtonColor = getButtonColor( rando::RecolorId::ABtn );
        const uint32_t bButtonColor = getButtonColor( rando::RecolorId::BBtn );
        const uint32_t xButtonColor = getButtonColor( rando::RecolorId::XBtn );
        const uint32_t yButtonColor = getButtonColor( rando::RecolorId::YBtn );
        const uint32_t zButtonColor = getButtonColor( rando::RecolorId::ZBtn );

        for ( uint32_t i = 0x248; i <= 0x254; i += 0x4 )
        {
            // Patch the A Button Color
            if ( mWindowARaw )
            {
                *reinterpret_cast<uint32_t*>( mWindowARaw + i ) = aButtonColor;
            }

            // Patch the B Button Color
            if ( mWindowBRaw )
            {
                *reinterpret_cast<uint32_t*>( mWindowBRaw + i ) = bButtonColor;
            }

            // Patch the X Button Color
            if ( mWindowXRaw )
            {
                *reinterpret_cast<uint32_t*>( mWindowXRaw + i ) = xButtonColor;
            }

            // Patch the Y Button Color
            if ( mWindowYRaw )
            {
                *reinterpret_cast<uint32_t*>( mWindowYRaw + i ) = yButtonColor;
            }

            // Patch the Z Button Color
            if ( mWindowZRaw )
            {
                *reinterpret_cast<uint32_t*>( mWindowZRaw + i ) = zButtonColor;
            }
        }

        // Patch Heart Color
        uint32_t mWindowRaw;

        uint32_t* tempHeartColorRGBA;
        uint32_t heartListSize = 1;

        rando::CLR0RgbArray heartRgbArr;
        if ( randomizer->getRecolorRgbArray( rando::RecolorId::Hearts, &heartRgbArr ) )
        {
            heartListSize = heartRgbArr.arrLength;

            tempHeartColorRGBA = new uint32_t[heartListSize];

            for ( uint8_t rgbIdx = 0; rgbIdx < heartListSize; rgbIdx++ )
            {
                uint8_t* rgbPtr = heartRgbArr.rgbArrPtr + 3 * rgbIdx;
                uint32_t rgba = *reinterpret_cast<uint32_t*>( rgbPtr ) | 0xFF;
                tempHeartColorRGBA[rgbIdx] = rgba;
            }
        }
        else
        {
            tempHeartColorRGBA = new uint32_t[1];

            uint8_t* heartRgb = randomizer->getRecolorRgb( rando::RecolorId::Hearts );
            if ( heartRgb != nullptr )
            {
                tempHeartColorRGBA[0] = *reinterpret_cast<uint32_t*>( heartRgb ) | 0xFF;
            }
            else
            {
                tempHeartColorRGBA[0] = 0xFFFFFFFF;
            }
        }

        /*
        const uint32_t maxHeart =
            libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.maxHealth / 5;
        */

        for ( uint32_t i = 0; i < 20; i++ )
        {
            libtp::tp::d_pane_class::CPaneMgr* currentLifeTexture = mpMeterDraw->mpLifeTexture[i][1];
            if ( !currentLifeTexture )
            {
                continue;
            }

            mWindowRaw = reinterpret_cast<uint32_t>( currentLifeTexture->mWindow );
            if ( !mWindowRaw )
            {
                continue;
            }

            const uint32_t currentHeartIndexColor = tempHeartColorRGBA[i % heartListSize];
            for ( uint32_t j = 0x138; j <= 0x144; j += 0x4 )
            {
                *reinterpret_cast<uint32_t*>( mWindowRaw + j ) = currentHeartIndexColor;
            }
        }

        // Patch the Big Heart and Rupee colors
        uint32_t currentWallet;
        uint32_t rupeeColor;

        if ( events::haveItem( Item::Giant_Wallet ) )
        {
            currentWallet = Wallets::GIANT_WALLET;
            rupeeColor = 0xaf00ffff;
        }
        else if ( events::haveItem( Item::Big_Wallet ) )
        {
            currentWallet = Wallets::BIG_WALLET;
            rupeeColor = 0xff0000ff;
        }
        else
        {
            currentWallet = Wallets::WALLET;
            rupeeColor = 0;
        }

        uint32_t tempBigHeartColor;
        if ( heartListSize > 1 )
        {
            tempBigHeartColor = tempHeartColorRGBA[ulRand( &randNext, heartListSize )];
        }
        else
        {
            tempBigHeartColor = tempHeartColorRGBA[0];
        }

        mWindowRaw = reinterpret_cast<uint32_t>( mpMeterDraw->mpBigHeart->mWindow );
        if ( mWindowRaw )
        {
            for ( uint32_t i = 0x3F8, rupee = 0x1038; i <= 0x404; i += 0x4, rupee += 0x4 )
            {
                // Patch the Big Heart color
                *reinterpret_cast<uint32_t*>( mWindowRaw + i ) = tempBigHeartColor;
                *reinterpret_cast<uint32_t*>( mWindowRaw + ( i + ( 0x1B0 ) ) ) = tempBigHeartColor;
                *reinterpret_cast<uint32_t*>( mWindowRaw + ( i + ( 0x1B0 * 2 ) ) ) = tempBigHeartColor;
                *reinterpret_cast<uint32_t*>( mWindowRaw + ( i + ( 0x1B0 * 3 ) ) ) = tempBigHeartColor;

                // Patch the Rupee color
                if ( currentWallet != Wallets::WALLET )
                {
                    *reinterpret_cast<uint32_t*>( mWindowRaw + rupee ) = rupeeColor;
                }
            }
        }

        delete[] tempHeartColorRGBA;
    }

    void setLanternColor( rando::Randomizer* randomizer )
    {
        // Make sure the randomizer is loaded/enabled and a seed is loaded
        if ( !getCurrentSeed( randomizer ) )
        {
            return;
        }

        using namespace libtp::tp::d_a_alink;

        // Set lantern variables
        daAlinkHIO_kandelaar_c0* tempLanternVars = &lanternVars;
        daAlinkHIO_huLight_c0* tempHuLightVars = &huLightVars;

        uint8_t lanternGlowColors[6];
        getLanternGlowColor( lanternGlowColors );

        tempLanternVars->innerSphereR = lanternGlowColors[0];
        tempLanternVars->innerSphereG = lanternGlowColors[1];
        tempLanternVars->innerSphereB = lanternGlowColors[2];
        tempLanternVars->outerSphereR = lanternGlowColors[3];
        tempLanternVars->outerSphereG = lanternGlowColors[4];
        tempLanternVars->outerSphereB = lanternGlowColors[5];
        tempHuLightVars->lanternAmbienceR = lanternGlowColors[0];
        tempHuLightVars->lanternAmbienceG = lanternGlowColors[1];
        tempHuLightVars->lanternAmbienceB = lanternGlowColors[2];
    }
}     // namespace mod::user_patch