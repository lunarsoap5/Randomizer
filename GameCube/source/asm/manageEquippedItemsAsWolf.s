.global asmManageEquippedItemsAsWolf
# asmManageEquippedItemsAsWolf needs to be used in at least one subrel, so it cannot be set to hidden

asmManageEquippedItemsAsWolf:
# Push stack
stwu %sp,-0x10(%sp)
mflr %r3
stw %r3,0x14(%sp)

# Backup important register values
stw %r4,0x8(%sp)

mr %r3,%r0
bl handleManageEquippedItemsAsWolf

# Store the returned value, which also restores the original instruction
stw %r3,0x128(%r31)

# Restore important register values
lwz %r4,0x8(%sp)

# Pop stack
lwz %r0,0x14(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr