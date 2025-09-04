.global asmAdjustFMapShowRegionPortals
# asmAdjustFMapShowRegionPortals needs to be used in at least one subrel, so it cannot be set to hidden

.hidden _finished

# At instruction 801c86e0, do vanilla instruction which loads into r3 the
# `mpDraw2DTop`. We check on this the field at offset 0x1227 which is a byte
# indicating the current region. We check if this is unlocked. If it is, then we
# return to the normal flow. If it is not unlocked, then we branch to the "play
# sound effect" setting up which is at 801c8778. The only register we need to
# protect is r3.

asmAdjustFMapShowRegionPortals:
# Restore the original instruction immediately
lwz %r3,0x18(%r30)

# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Backup important register values
stw %r3,0x8(%sp)

mr %r3,%r30 # Move dMenu_Fmap*
bl handleAdjustFMapShowRegionPortals
cmplwi %r3,1

# If result is 0 (not unlocked), then we need to adjust the link register so we return to 


# Pop stack
lwz %r3,0x14(%sp)
beq+ _finished
# Increase to soundEffect code if not unlocked
addi %r3,%r3,0x94

_finished:
mr %r0,%r3
# Restore important register values
lwz %r3,0x8(%sp)

mtlr %r0
addi %sp,%sp,0x10
blr