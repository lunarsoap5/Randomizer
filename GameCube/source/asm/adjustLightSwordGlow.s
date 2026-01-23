.global asmAdjustLightSwordGlow
# This function needs to be used in at least one subrel, so it cannot be set to hidden

asmAdjustLightSwordGlow:
# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Return the currently equipped sword
bl handleAdjustLightSwordGlow

# Pop stack
lwz %r0,0x14(%sp)
mtlr %r0
addi %sp,%sp,0x10
mr %r0, %r3
blr