.global asmAdjustFlowBranchNextNode
.hidden asmAdjustFlowBranchNextNodeSetCustomIndex
.hidden asmAdjustFlowBranchNextNodeDone

# asmAdjustFlowBranchNextNode needs to be used in at least one subrel, so it cannot be set to hidden

# Checks if the branch's next node should be custom or not. If custom, updates r4 to be the custom next node index.
# Otherwise keeps r4 at its current value (the vanilla next node index).

asmAdjustFlowBranchNextNode:
# Push stack
stwu %sp,-0x20(%sp)
mflr %r0
stw %r0,0x24(%sp)

# Backup important register values (need r3 to r5)
stw %r3,0x10(%sp)
stw %r4,0x14(%sp)
stw %r30,0x18(%sp) # Store r30 since we overwrote the instruction moving r30 to r5

# r3 already contains the dMsgFlow_c* from r29.
# Move the query result to r4
mr %r4,%r5

bl handleAdjustFlowBranchNextNode

# If result is nullptr, then no custom result
cmpwi %r3,0
bne- asmAdjustFlowBranchNextNodeSetCustomIndex

# If no custom index, restore r4 to value from stack
lwz %r4,0x14(%sp)
b asmAdjustFlowBranchNextNodeDone

asmAdjustFlowBranchNextNodeSetCustomIndex:
lhz %r4,0x0(%r3)

asmAdjustFlowBranchNextNodeDone:
# Restore important register values except r4 which was handled above
lwz %r3,0x10(%sp)
lwz %r5,0x18(%sp)

# Pop stack
lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20
blr