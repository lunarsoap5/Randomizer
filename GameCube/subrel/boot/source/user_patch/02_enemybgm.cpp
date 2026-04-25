/**
 * @file 02_enemybgm.h
 * @author jdflyer
 * @brief Patches the game to allow for optional enemy BGM
 *
 * @bug No known bugs
 */
#include "user_patch/02_enemybgm.h"
#include "events.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "asm_templates.h"

namespace mod::user_patch
{
    void disableBattleMusic(rando::Randomizer* randomizer)
    {
        (void)randomizer;

        // Prevent startBattleBgm from running
        events::performStaticASMReplacement(reinterpret_cast<uint32_t>(libtp::z2audiolib::z2seqmgr::startBattleBgm),
                                            ASM_BRANCH_LINK_REGISTER);

        // getConsole() << "[2] EnemyBgmDisabled [" << ( set ? "x" : " " ) << "]\n";
    }
} // namespace mod::user_patch