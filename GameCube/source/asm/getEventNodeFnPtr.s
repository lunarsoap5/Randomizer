.global asmGetEventNodeFnPtr
# asmGetEventNodeFnPtr needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _finished

asmGetEventNodeFnPtr:
# Restore the original instruction immediately. If we end up doing nothing
# special here, then we want to use the value that was already calculated
# into r12 like normal.
add %r12,%r0,%r7

# Move r0 to r3 since it contains the value we need
#mr %r3,%r0

# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Backup important register values
stw %r3,0xC(%sp)

# Load the event index from the mesg_flow_node_event as param to handler.
lbz %r3,0x1(%r29)

bl handleGetEventNodeFnPtr
#cmplwi %r3,1
cmplwi %r3,0
#beq cr6,+8
#beq +8
beq+ _finished
mr %r12,%r3

_finished:
# Restore important register values
lwz %r0,0xC(%sp)

# Pop stack
lwz %r3,0x14(%sp)
mtlr %r3
lwz %r3,0xC(%sp)
addi %sp,%sp,0x10
blr