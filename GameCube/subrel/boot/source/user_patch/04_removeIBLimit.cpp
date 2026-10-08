/** @file 04_removeIBLimit.cpp
 * @author Lunar Soap
 * @author Zephiles
 * @brief Patches the heavy state function to remove Iron Boots speed limit
 *
 * @bug No known bugs
 */

#include "rando/randomizer.h"
#include "tp/d_a_alink.h"
#include "user_patch/user_patch.h"
#include "gc_wii/OSCache.h"

namespace mod::user_patch
{
    void removeIBLimit(rando::Randomizer* randomizer)
    {
        (void)randomizer;

        // Set the float that Link's actor references when heavy to be the default value.
        float* heavyStateSpeedPtr = &libtp::tp::d_a_alink::ironBootsVars.heavyStateSpeed;
        *heavyStateSpeedPtr = 1.f;

        // Clear the cache for the modified value
        libtp::gc_wii::os_cache::DCFlushRange(heavyStateSpeedPtr, sizeof(float));
    }
} // namespace mod::user_patch