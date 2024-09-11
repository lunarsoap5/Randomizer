.global asmManageEquippedItemsAsWolf

asmManageEquippedItemsAsWolf:
# Push stack
stwu %sp,-0x20(%sp)
mr %r5, %r0
mflr %r0
stw %r0,0x24(%sp)

# Backup important register values
stw %r3,0xC(%sp)
stw %r4,0x8(%sp)

mr %r3, %r5
bl handleManageEquippedItemsAsWolf
mr %r0, %r3

# Restore important register values
lwz %r3,0xC(%sp)
lwz %r4,0x8(%sp)

# Restore the original instruction
stw %r0, 0x128(%r31)

# Pop stack
lwz %r0,0x24(%sp)
mtlr %r0
addi %sp,%sp,0x20

blr