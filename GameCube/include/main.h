
/**	@file main.h
 *  @brief Main structure of the Randomizer
 *
 *	@author AECX
 *	@bug No known bugs.
 */

#ifndef MAIN_H
#define MAIN_H

#include "rando/randomizer.h"
#include "display/console.h"
#include "tp/d_a_alink.h"

#include <cstdint>

// Number of lines that should not be cleared when flushing the console screen
#define CONSOLE_PROTECTED_LINES 4

#ifndef DVD
// Subrel module ids
#define SUBREL_BOOT_ID 0x1001

// Seed id to be used in place of module id
#define SEED_GCI_ID 0x53454544 // SEED
#endif

// Number of bytes reserved for giving items to the player at arbitrary times via initGiveItemToPlayer
#define GIVE_PLAYER_ITEM_RESERVED_BYTES 8

// Number of bytes reserved for giving items to the player at arbitrary times via initGiveItemToPlayer
#define GIVE_PLAYER_ITEM_RESERVED_BYTES 8

// Converting item ids to msg ids and vice versa
#define ITEM_TO_ID(item) (item + 0x64)
#define ID_TO_ITEM(msgId) (msgId - 0x64)

// May be moved somewhere else later
// Required for keeping certain unused functions/variables from being removed
#define KEEP_FUNC __attribute__((used, visibility("default")))
#define KEEP_VAR __attribute__((visibility("default")))

namespace mod
{
    enum GameState : uint8_t
    {
        GAME_BOOT = 0, // Default; At startup (should only ever be active once)
        GAME_TITLE,    // When user is on title screen
        GAME_ACTIVE,   // When user is ingame and rando should be active
    };

    // Variables
    extern libtp::display::Console* gConsole;
    extern bool gConsoleState;

    void setConsoleScreen(bool state); // Sets visibility of console
    bool checkButtonsPressedThisFrame(uint32_t buttons);
    bool checkButtonCombo(uint32_t combo, bool checkAnalog);
    float intToFloat(int32_t value);
    void initGiveItemToPlayer(libtp::tp::d_a_alink::daAlink* linkMapPtr, rando::Randomizer* randoPtr);
    void handleFoolishItem(rando::Randomizer* randoPtr);
    void handleBonkDamage();

    // Inline getConsole, as it's just a shortcut to get a reference to the console variable
    inline libtp::display::Console& getConsole()
    {
        return *mod::gConsole;
    }

    int32_t initCreatePlayerItem(uint32_t itemID,
                                 uint32_t flag,
                                 const float pos[3],
                                 int32_t roomNo,
                                 const int16_t rot[3],
                                 const float scale[3]);

    // Subrel functions
    void hookFunctions(rando::Seed* seedPtr);
    void initArcLookupTable(rando::Randomizer* randoPtr);
    void writeCodehandlerToMemory();
} // namespace mod
#endif