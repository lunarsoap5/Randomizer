/**	@file dvdentrynum.cpp
 *  @brief Used to handle comparisons against DvdEntryNums. You might use this
 *  when a JKRArchive is loading to identify which file on disk it refers to.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */

#include <cstdint>
#include "rando/dvdentrynum.h"
#include "gc_wii/dvdfs.h"

namespace mod::dvdentrynum
{
    int32_t lookupTable[DvdEntryNumIdSize];
}     // namespace mod::dvdentrynum