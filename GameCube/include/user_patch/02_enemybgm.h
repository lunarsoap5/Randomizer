/**
 * @file 02_enemybgm.h
 * @author jdflyer
 * @brief Patches the game to allow for optional enemy BGM
 *
 * @bug No known bugs
 */

#ifndef RANDO_ENEMY_BGM_PATCH_H
#define RANDO_ENEMY_BGM_PATCH_H

#include "rando/randomizer.h"
#include "Z2AudioLib/Z2SeqMgr.h"

namespace mod::user_patch
{
    /**
     * @brief Patches the game to allow for optional enemy BGM
     */
    void disableBattleMusic(rando::Randomizer* randomizer);
} // namespace mod::user_patch

#endif