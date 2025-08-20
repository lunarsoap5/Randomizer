/** @file 00__wallet.cpp
 * @author AECX
 * @brief Patches Links wallet to be bigger.
 *
 * @bug No known bugs
 */

#include "user_patch/user_patch.h"
#include "user_patch/00_wallet.h"
#include "asm_templates.h"
#include "main.h"
#include "rando/randomizer.h"
#include "tp/d_menu_collect.h"
#include "tp/d_save.h"
#include "memory.h"

namespace mod::user_patch
{
    void patchWallet(uint8_t walletSize)
    {
        using namespace libtp;

        const uint32_t getRupeeMaxPtr = reinterpret_cast<uint32_t>(tp::d_save::getRupeeMax);
        const uint32_t setWalletMaxNumPtr = reinterpret_cast<uint32_t>(tp::d_menu_collect::setWalletMaxNum);

        // Small Wallet
        *reinterpret_cast<uint32_t*>(getRupeeMaxPtr + 0x30) =
            ASM_LOAD_IMMEDIATE(3, mod::user_patch::walletValues[walletSize][0]);

        // Big Wallet
        *reinterpret_cast<uint32_t*>(getRupeeMaxPtr + 0x38) =
            ASM_LOAD_IMMEDIATE(3, mod::user_patch::walletValues[walletSize][1]);

        // Giant wallet
        *reinterpret_cast<uint32_t*>(getRupeeMaxPtr + 0x40) =
            ASM_LOAD_IMMEDIATE(3, mod::user_patch::walletValues[walletSize][2]);

        // Small Wallet
        *reinterpret_cast<uint32_t*>(setWalletMaxNumPtr + 0x24) =
            ASM_COMPARE_WORD_IMMEDIATE(0, mod::user_patch::walletValues[walletSize][0]);

        // Big Wallet
        *reinterpret_cast<uint32_t*>(setWalletMaxNumPtr + 0x18) =
            ASM_COMPARE_WORD_IMMEDIATE(0, mod::user_patch::walletValues[walletSize][1]);

        // Giant Wallet
        *reinterpret_cast<uint32_t*>(setWalletMaxNumPtr + 0x30) =
            ASM_COMPARE_WORD_IMMEDIATE(0, mod::user_patch::walletValues[walletSize][2]);

        // Clear the cache for the modified values
        // Assembly instructions need to clear the instruction cache as well
        libtp::memory::clear_DC_IC_Cache(reinterpret_cast<void*>(getRupeeMaxPtr), 0x41);
        libtp::memory::clear_DC_IC_Cache(reinterpret_cast<void*>(setWalletMaxNumPtr), 0x31);
    }
} // namespace mod::user_patch