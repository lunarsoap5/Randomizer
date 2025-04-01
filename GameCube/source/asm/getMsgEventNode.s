.global asmGetMsgEventNode
# asmGetMsgEventNode needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _finished

asmGetMsgEventNode:
# Restore the original instruction
add %r29,%r4,%r0

# Push stack
stwu %sp,-0x20(%sp)
mflr %r0
stw %r0,0x24(%sp)
# Need to maintain r3 through r6
stw %r3,0x10(%sp)
stw %r4,0x14(%sp)
stw %r5,0x18(%sp)
stw %r6,0x1C(%sp)

# r3 already contains the dMsgFlow_c*
bl handleGetMsgEventNode

# If result is nullptr, then no custom result.
cmpwi %r3,0
beq+ _finished

# If not nullptr, replace eventNode ptr in r29 with new address.
mr %r29,%r3

_finished:
lwz %r3,0x10(%sp)
lwz %r4,0x14(%sp)
lwz %r5,0x18(%sp)
lwz %r6,0x1C(%sp)

lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20
blr