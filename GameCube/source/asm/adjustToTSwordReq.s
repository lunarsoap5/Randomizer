.global asmAdjustToTSwordReq
# asmAdjustToTSwordReq needs to be used in at least one subrel, so it cannot be set to hidden

asmAdjustToTSwordReq:
# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

bl handleAdjustToTSwordReq

# Pop stack
lwz %r0,0x14(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr