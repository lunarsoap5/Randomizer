/**	@file asm.h
 *  @brief ASM functions
 *
 *  @author AECX
 *  @author Zephiles
 *  @author Lunar Soap
 *  @author Isaac
 *  @bug No known bugs.
 */
#ifndef RANDO_ASM_H
#define RANDO_ASM_H

#include <cstdint>
#include "tp/dynamic_link.h"
#include "tp/f_op_actor.h"

// Original: li 0xE0
// Normally loads the poesoul item id into r4
#define e_hp_ExecDead_liOffset 0x247C
#define e_po_ExecDead_liOffset 0x3C9C

// Original:
// stb r0, 0x10c( r4 ) = > numPoeSouls
// Normally increments poe count
#define e_hp_ExecDead_incOffset 0x2354
#define e_po_ExecDead_incOffset 0x36A8

// d_a_obj_Lv5Key__Wait_offset:
// 0xBC is offset to the text section relative to the start of the decompressed
// REL. 0x4E4 is offset to Wait function relative to the start of the text
// section (as seen on line 14 of d_a_obj_Lv5Key.map).
#define d_a_obj_Lv5Key__Wait_offset 0xBC + 0x4E4

#ifdef TP_US
// This is where we do our custom hook to modify MemArchive contents in the
// thread in which they are being loaded right before they would normally be
// marked as fully loaded.
#define mDoDvdThd_mountArchive_c__execute_customHookAddr 0x800160E4
#elif defined TP_EU
#define mDoDvdThd_mountArchive_c__execute_customHookAddr 0x80016098
#elif defined TP_JP
#define mDoDvdThd_mountArchive_c__execute_customHookAddr 0x800160E4
#endif

namespace mod::assembly
{
    extern "C"
    {
        void asmDoLinkHook( void );
        void handleDoLinkHook( libtp::tp::dynamic_link::DynamicModuleControl* dmc );

        // d_e_hp.rel
        void asmAdjustPoeItem( void );
        int32_t handleAdjustPoeItem( void* );

        // d_e_po.rel
        void asmAdjustAGPoeItem( void );
        int32_t handleAdjustAGPoeItem( void* );

        // d_a_npc_kn.rel
        void asmAdjustHiddenSkillItem();
        void handleAdjustHiddenSkillItem( uint16_t eventIndex );

        // d_a_npc_ins.rel
        void asmAdjustBugReward();
        void handleAdjustBugReward( uint32_t msgEventAddress, uint8_t bugID );

        void asmAdjustSkyCharacter();
        uint8_t handleAdjustSkyCharacter();

        // d_a_obj_life_container.rel
        void asmAdjustFieldItemParams();
        void handleAdjustFieldItemParams( libtp::tp::f_op_actor::fopAc_ac_c* fopAC, void* daObjLife );

        // d_a_e_mk.rel
        void asmTransformOokWolf();
        void handleTransformFromWolf();

        // d_a_b_bq.rel
        void asmTransformDiababaWolf();

        // d_a_npc_hoz.rel
        void asmAdjustIzaWolf();
        void handleAdjustIzaWolf( int32_t flag );

        // d_kankyo_rain.o
        void asmShowReekfishPath();
        uint8_t handleShowReekfishPath( uint8_t scent );

        // d_a_demo_item.o
        void asmAdjustCreateItemParams();
        void handleAdjustCreateItemParams( void* daDitem );
    }
}     // namespace mod::assembly

#endif