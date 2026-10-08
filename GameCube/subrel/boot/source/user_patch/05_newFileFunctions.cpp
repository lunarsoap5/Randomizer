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
#include "tp/d_camera.h"
#include "asm.h"

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

    void invertCameraAxis(rando::Randomizer* randomizer)
    {
        (void)randomizer;
        uint32_t* updatePadAddress =
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(libtp::tp::d_camera::updatePad) + 0xF8);
        *updatePadAddress = 0xfc200850; // Previous ec01002b.
        libtp::memory::clear_DC_IC_Cache(updatePadAddress, sizeof(uint32_t));
    }

    void makeLightSwordGlow(rando::Randomizer* randomizer)
    {
        (void)randomizer;
        const uint32_t setLightningSwordEffect_address =
            reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::setLightningSwordEffect);
        libtp::patch::writeBranchBL(setLightningSwordEffect_address + 0x8C, assembly::asmAdjustLightSwordGlow);

        uint32_t* setLightningSwordEffectAddress =
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::setLightningSwordEffect) + 0x90);
        *setLightningSwordEffectAddress = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE(0, 0x49);

        setLightningSwordEffectAddress =
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(libtp::tp::d_a_alink::setLightningSwordEffect) + 0x94);
        *setLightningSwordEffectAddress = 0x40820080;
    }
} // namespace mod::user_patch