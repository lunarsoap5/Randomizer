/** @file game_patch.h
 * @author AECX
 * @brief Game patches are functions that change game functions.
 *
 * @bug No known bugs
 */
#ifndef RANDO_GAME_PATCH_H
#define RANDO_GAME_PATCH_H

#include <cstdint>

// Instruction templates
#define ASM_LOAD_IMMEDIATE( register, value ) ( 0x38000000 + ( register * 0x200000 ) ) | ( value & 0xFFFF )
#define ASM_COMPARE_WORD_IMMEDIATE( register, value ) ( 0x2C000000 + ( register * 0x10000 ) ) | ( value & 0xFFFF )

namespace mod::game_patch
{
    /**
     *  @brief Defines a game_patch function
     *
     *  @param set If true we apply the patch, otherwise restore original behavior
     */
    typedef void ( *GamePatch )( bool set );

    // Available Game patches accessible by index
    extern GamePatch patches[1];

}     // namespace mod::game_patch
#endif