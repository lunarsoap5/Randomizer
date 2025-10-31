.global asmGetFlowQueryFnPtr
# asmGetFlowQueryFnPtr needs to be used in at least one subrel, so it cannot be set to hidden

.hidden asmGetFlowQueryFnPtrDone

# Checks if the query index is larger than the vanilla range. If it is not, then we do nothing. If it
# is, then we translate it to point to our own custom query function (PTMF) based on its value.

asmGetFlowQueryFnPtr:
# Restore the original instruction immediately.
add %r12,%r0,%r8

# Push stack
stwu %sp,-0x20(%sp)
mflr %r0
stw %r0,0x24(%sp)

# Backup important register values (need r3 to r6)
stw %r3,0x10(%sp)
stw %r4,0x14(%sp)
stw %r5,0x18(%sp)
stw %r6,0x1C(%sp)

# Load the query index from the FLW node as a param to handler.
lhz %r3,0x2(%r31)

bl handleGetFlowQueryFnPtr

# If result is nullptr, then no custom result
cmplwi %r3,0
beq+ asmGetFlowQueryFnPtrDone

# Else if have result, overwrite r12 with our custom PTMF ptr.
mr %r12,%r3

asmGetFlowQueryFnPtrDone:
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