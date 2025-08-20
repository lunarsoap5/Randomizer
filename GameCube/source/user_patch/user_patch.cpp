/** @file game_patch.cpp
 * @author AECX
 * @brief Game patches are functions that change game functions.
 *
 * @bug No known bugs
 */

#include "user_patch/user_patch.h"

#include "user_patch/03_customCosmetics.h"
#include "user_patch/05_newFileFunctions.h"

namespace mod::user_patch
{

    KEEP_VAR uint16_t walletValues[4][3] = {{99, 500, 1000}, {300, 500, 1000}, {500, 1000, 5000}, {1000, 5000, 9999}};
    GamePatch volatilePatches[6] =
        {clearFaronTwilight, clearEldinTwilight, clearLanayruTwilight, setMinorCutsceneValues, clearMDH, setMapRegionBits};
} // namespace mod::user_patch