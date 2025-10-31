.global asmAdjustSelectMsg
# asmAdjustSelectMsg needs to be used in at least one subrel, so it cannot be set to hidden

.hidden asmAdjustSelectMsgSetCustom
.hidden asmAdjustSelectMsgDone

# Checks if selectMsg (the menu options) should be a custom string. If no custom string, we leave r0 as it was and let
# the vanilla calculations run. If custom string, we set r0 to point to our custom string and skip ahead to where the
# string ptr gets stored. We never need to restore any registers other than r0.

asmAdjustSelectMsg:
# Restore the original instruction immediately.
mulli %r0,%r3,0x14

# Push stack
stwu %sp,-0x10(%sp)
stw %r0,0x8(%sp) # store r0 before we overwrite it
mflr %r0
stw %r0,0x14(%sp)

# The INF index param we need is already in r3 at this point.
# Move ptr to the current INF1 chunk we are operating in to r4.
mr %r4,%r28

bl handleAdjustSelectMsg

# If result is nullptr, then no custom result.
cmplwi %r3,0
bne- asmAdjustSelectMsgSetCustom

# For no custom string, restore r0 to old value and begin normal stack pop.
lwz %r0,0x8(%sp)
lwz %r3,0x14(%sp)
b asmAdjustSelectMsgDone

# If custom result, put char* into r0 and skip calculations for finding the vanilla string.
asmAdjustSelectMsgSetCustom:
mr %r0,%r3
lwz %r3,0x14(%sp)
addi %r3,%r3,0x10 # Adjust LR address to skip instructions when have custom result.

asmAdjustSelectMsgDone:
# Finish popping stack. LR address is already in r3, and r0 value is set to appropriate value.
mtlr %r3
addi %sp,%sp,0x10
blr