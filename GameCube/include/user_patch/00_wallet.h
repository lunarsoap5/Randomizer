/** @file 00_wallet.h
 * @author AECX
 * @brief Patches Links wallet to be bigger.
 *
 * @bug No known bugs
 */
#ifndef RANDO_WALLET_PATCH_H
#define RANDO_WALLET_PATCH_H

#include "rando/randomizer.h"

namespace mod::user_patch
{
    extern uint16_t walletValues[4][3];
    void patchWallet(uint8_t walletSize);
} // namespace mod::user_patch
#endif