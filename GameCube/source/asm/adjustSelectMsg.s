.global asmAdjustSelectMsg
# asmAdjustSelectMsg needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _normalStackPop
.hidden _finished

asmAdjustSelectMsg:
# Restore the original instruction immediately. If we end up doing nothing
# special here, then we want to use the existing code.
mulli %r0,%r3,0x14

# Push stack
stwu %sp,-0x10(%sp)
stw %r0,0x8(%sp) # store r0 before we overwrite it
mflr %r0
stw %r0,0x14(%sp)
stw %r3,0xC(%sp)

# The INF index param we need is already in r3 at this point.

# There is a pointer to the current INF1 chunk we are operating in in r28, so
# move it to r4. We do not need to worry about restoring r4 since the current
# value is not used.
mr %r4,%r28

bl handleAdjustSelectMsg
cmplwi %r3,0

# Pop stack
beq+ _normalStackPop

# If we did have a string remap
# Put char* into r0
mr %r0,%r3
lwz %r3,0x14(%sp)
# Update LR addr to skip some code we don't want to run
addi %r3,%r3,0x10
b _finished

# If there was no string remap
_normalStackPop:
lwz %r3,0x14(%sp)
lwz %r0,0x8(%sp)

_finished:
mtlr %r3
lwz %r3,0xC(%sp)
addi %sp,%sp,0x10
blr