/**
 * @file 05_newFileFunctions.h
 * @author Lunar Soap
 * @brief Contains function definitions that patch the game based on user settings.
 *
 * @bug No known bugs
 */

#ifndef RANDO_NEW_FILE_FUNCTIONS_PATCH_H
#define RANDO_NEW_FILE_FUNCTIONS_PATCH_H

#include "rando/randomizer.h"

namespace mod::user_patch
{
    /**
     * @brief A list of potential functions to run upon seed initialization.
     */
    void clearFaronTwilight(rando::Randomizer* randomizer);
    void clearEldinTwilight(rando::Randomizer* randomizer);
    void clearLanayruTwilight(rando::Randomizer* randomizer);
    void setMinorCutsceneValues(rando::Randomizer* randomizer);
    void clearMDH(rando::Randomizer* randomizer);
    void setInstantText(rando::Randomizer* randomizer);
    void setMapRegionBits(rando::Randomizer* randomizer);
    void skipMajorCutscenes(rando::Randomizer* randomizer);
    void invertCameraAxis(rando::Randomizer* randomizer);
} // namespace mod::user_patch

#endif