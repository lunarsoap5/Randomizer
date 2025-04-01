.global asmAdjustMsgBranchResultNode
# asmAdjustMsgBranchResultNode needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _setCustomFlwIndex
.hidden _finished

asmAdjustMsgBranchResultNode:
# Move the branch proc_status which we need to r0
mr %r0,%r5

# Restore the original instruction
mr %r5,%r30

# Push stack
stwu %sp,-0x20(%sp)
stw %r0,0x10(%sp) # store r0 on stack so we can use r0
mflr %r0
# We need to be sure that r3 through r5 are restored
stw %r0,0x24(%sp)
stw %r3,0x14(%sp)
stw %r4,0x18(%sp)
stw %r5,0x1C(%sp)

# r3 already contains the dMsgFlow_c* from r29
# Restore the branch proc_status from stack to r4 and call fn
lwz %r4,0x10(%sp)

bl handleAdjustMsgBranchResultNode

# If result is nullptr, then no custom result.
cmpwi %r3,0
bne- _setCustomFlwIndex

# If no custom index, restore r4 to value from stack
lwz %r4,0x18(%sp)
b _finished

_setCustomFlwIndex:
lhz %r4,0x0(%r3)

_finished:
# Pop stack except r4 which was handled above
lwz %r3,0x14(%sp)
lwz %r5,0x1C(%sp)
lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20
blr