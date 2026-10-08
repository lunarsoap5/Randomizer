.global asmAdjustLightSwordColor
# This function needs to be used in at least one subrel, so it cannot be set to hidden

asmAdjustLightSwordColor:
# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

cmplwi %r3, 0
beq 0x8
bl handleAdjustLightSwordColor

# Run the replaced instruction set
addi %r27, %r27, 0x1

# Pop stack
lwz %r0,0x14(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr