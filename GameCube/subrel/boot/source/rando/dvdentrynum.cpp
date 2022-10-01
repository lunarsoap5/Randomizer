/**	@file dvdentrynum.cpp
 *  @brief Used to handle comparisons against DvdEntryNums. You might use this
 *  when a JKRArchive is loading to identify which file on disk it refers to.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */

#include "rando/dvdentrynum.h"

#include "gc_wii/dvdfs.h"

namespace mod::dvdentrynum
{
    // This function gets called once on startup. After that, you will want to
    // call `getDvdEntryNum` located in the header file.
    void initLookupTable()
    {
        using libtp::gc_wii::dvdfs::DVDConvertPathToEntrynum;

        lookupTable[ResObjectKmdl] = DVDConvertPathToEntrynum( "/res/Object/Kmdl.arc" );
        // lookupTable[ResObjectZmdl] = DVDConvertPathToEntrynum( "/res/Object/Zmdl.arc" );
        // lookupTable[ResObjectWmdl] = DVDConvertPathToEntrynum( "/res/Object/Wmdl.arc" );
        // lookupTable[ResObjectCWShd] = DVDConvertPathToEntrynum( "/res/Object/CWShd.arc" );
        // lookupTable[ResObjectSWShd] = DVDConvertPathToEntrynum( "/res/Object/SWShd.arc" );
        // lookupTable[ResObjectHyShd] = DVDConvertPathToEntrynum( "/res/Object/HyShd.arc" );
    }
}     // namespace mod::dvdentrynum