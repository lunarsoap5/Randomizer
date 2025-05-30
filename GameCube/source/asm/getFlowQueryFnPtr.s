.global asmGetFlowQueryFnPtr
# asmGetFlowQueryFnPtr needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _finished

asmGetFlowQueryFnPtr:
# Restore the original instruction immediately. If we end up doing nothing
# special here, then we want to use the value that was already calculated
# into r12 like normal.
add %r12,%r0,%r8

# Push stack
stwu %sp,-0x20(%sp)
mflr %r0
stw %r0,0x24(%sp)
# Need to maintain r3 through r6
stw %r3,0x10(%sp)
stw %r4,0x14(%sp)
stw %r5,0x18(%sp)
stw %r6,0x1C(%sp)

# Load the query index from the mesg_flow_node_branch as param to handler.
lhz %r3,0x2(%r31)

bl handleGetFlowQueryFnPtr
cmplwi %r3,0
beq+ _finished
mr %r12,%r3

_finished:
lwz %r3,0x10(%sp)
lwz %r4,0x14(%sp)
lwz %r5,0x18(%sp)
lwz %r6,0x1C(%sp)

lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20
blr