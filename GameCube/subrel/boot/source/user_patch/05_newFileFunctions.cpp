#include "main.h"
#include "rando/data.h"
#include "rando/randomizer.h"
#include "user_patch/user_patch.h"
#include "asm_templates.h"
#include "memory.h"
#include "tp/d_event.h"
#include "functionHooks.h"
#include "patch.h"
#include "tp/d_msg_class.h"
#include "tp/d_msg_object.h"
#include "events.h"

namespace mod::user_patch
{
    void setInstantText(rando::Randomizer* randomizer)
    {
        using namespace libtp;

        (void)randomizer;

        // Hook the functions used for instant text
        gReturn_jmessage_tSequenceProcessor__do_begin =
            patch::hookFunction(tp::d_msg_class::jmessage_tSequenceProcessor__do_begin,
                                handle_jmessage_tSequenceProcessor__do_begin);

        gReturn_jmessage_tSequenceProcessor__do_tag = patch::hookFunction(tp::d_msg_class::jmessage_tSequenceProcessor__do_tag,
                                                                          handle_jmessage_tSequenceProcessor__do_tag);

        // Modify isSend button checks to allow for automashing through text
        const uint32_t isSendAddress = reinterpret_cast<uint32_t>(libtp::tp::d_msg_object::isSend);
        libtp::patch::writeBranchBL(isSendAddress + 0xE4, events::autoMashThroughText);
        libtp::patch::writeBranchBL(isSendAddress + 0x160, events::autoMashThroughText);
        libtp::patch::writeBranchBL(isSendAddress + 0x1B8, events::autoMashThroughText);
    }

    void skipMajorCutscenes(rando::Randomizer* randomizer)
    {
        (void)randomizer;

        uint32_t* skipperFunctionAddress =
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(libtp::tp::d_event::skipper) + 0x54);

        // Modifies the 'skipper' function to automatically attempt to skip all major cutscenes
        *skipperFunctionAddress = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE(30, 0);
        libtp::memory::clear_DC_IC_Cache(skipperFunctionAddress, sizeof(uint32_t));
    }
} // namespace mod::user_patch