.global asmGetFlowEventNode
# asmGetFlowEventNode needs to be used in at least one subrel, so it cannot be set to hidden

# This updates r29 with a ptr to our mutable copy of the node. This node copy is what gets processed by the vanilla
# proc code. Inside `handleGetFlowEventNode`, the node copy is patched if it was supposed to be.

asmGetFlowEventNode:
# We can skip the original instruction since we will update r29 below.
# Original instruction was: add %r29,%r4,%r0

# Push stack
stwu %sp,-0x20(%sp)
mflr %r0
stw %r0,0x24(%sp)

# Backup important register values (need r3 to r6)
stw %r3,0x10(%sp)
stw %r4,0x14(%sp)
stw %r5,0x18(%sp)
stw %r6,0x1C(%sp)

# r3 already contains the dMsgFlow_c*
bl handleGetFlowEventNode

# Replace node ptr in r29 with ptr to our mutable node copy.
mr %r29,%r3

# Restore important register values
lwz %r3,0x10(%sp)
lwz %r4,0x14(%sp)
lwz %r5,0x18(%sp)
lwz %r6,0x1C(%sp)

# Pop stack
lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20
blr