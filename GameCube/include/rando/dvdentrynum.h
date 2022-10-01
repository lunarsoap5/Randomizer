/**	@file dvdentrynum.h
 *  @brief Used to handle comparisons against DvdEntryNums. You might use this
 *  when a JKRArchive is loading to identify which file on disk it refers to.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef DVDENTRYNUM_DVDENTRYNUM_H
#define DVDENTRYNUM_DVDENTRYNUM_H

#include <cstdint>

#include "gc_wii/dvdfs.h"

namespace mod::dvdentrynum
{
    enum DvdEntryNumId : uint8_t
    {
        // DO NOT set any of these enums to a specific value. The exact values
        // and the order are irrelevant (other than `DvdEntryNumIdSize` which
        // must go last).
        ResObjectKmdl,     // Link wearing Hero's Clothes
                           // ResObjectZmdl,      // Link wearing Zora Armor
                           // ResObjectWmdl,      // Wolf Link and Midna on back
                           // ResObjectCWShd,     // Ordon Shield
                           // ResObjectSWShd,     // Wooden Shield
                           // ResObjectHyShd,     // Hylian Shield

        DvdEntryNumIdSize,
        // DvdEntryNumIdSize MUST GO LAST. When adding a new enum, put it above
        // this one and don't forget to actually add the lookup in the
        // `dvdentrynum.cpp` file!
    };

    extern int32_t lookupTable[DvdEntryNumIdSize];

    /**
     * @brief Essentially call dvd::DVDConvertPathToEntrynum in very few
     * assembly instructions using results cached during startup.
     *
     * @param dvdEntryNumId Enum of the dvd filepath for which you want the
     * entryNum.
     * @return int32_t dvdEntryNum Result from dvd::DVDConvertPathToEntrynum
     */
    inline int32_t getDvdEntryNum( DvdEntryNumId dvdEntryNumId )
    {
        if ( dvdEntryNumId >= DvdEntryNumId::DvdEntryNumIdSize )
        {
            return -1;
        }
        return lookupTable[dvdEntryNumId];
    }

    void initLookupTable();
}     // namespace mod::dvdentrynum
#endif