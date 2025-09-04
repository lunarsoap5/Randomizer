.global asmFmapPreventPortalsRegion
.hidden asmFmapPreventPortalsRegionDone

.global asmFmapPreventPortalsSpot
.hidden asmFmapPreventPortalsSpotDone

# asmFmapPreventPortalsRegion and asmFmapPreventPortalsSpot need to be used in at least one subrel, so they cannot be set to hidden

# We check if currently viewed region is unlocked. If unlocked, function proceeds normally. If not unlocked, we jump
# further down the function where it plays a `Z2SE_SYS_ERROR` sound effect and exits the function.
# There are 2 very similar versions in this file: one for the region view and one for the spot view.


asmFmapPreventPortalsRegion:
# Restore the original instruction immediately
lwz %r3,0x18(%r30)

# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Backup important register values
stw %r3,0x8(%sp)

mr %r3,%r30 # dMenu_Fmap*
bl handleFmapPreventPortals
cmplwi %r3,1

# Begin pop stack
lwz %r3,0x14(%sp)

# Adjust LR address to return to if region not unlocked
beq+ asmFmapPreventPortalsRegionDone
addi %r3,%r3,0x94
asmFmapPreventPortalsRegionDone:
mr %r0,%r3

# Restore important register values and end pop stack
lwz %r3,0x8(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr


### Spot version
# spot_map_proc is missing the handling for post-AG but pre-mirrorRaising that is present in the
# region_map_proc function. This was corrected in the JP version, hence the different versions below.

#ifdef TP_JP

asmFmapPreventPortalsSpot:
# Restore the original instruction immediately
lwz %r3,0x18(%r30)

# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Backup important register values
stw %r3,0x8(%sp)

mr %r3,%r30 # dMenu_Fmap*
bl handleFmapPreventPortals
cmplwi %r3,1

# Begin pop stack
lwz %r3,0x14(%sp)

# Adjust LR address to return to if region not unlocked
beq+ asmFmapPreventPortalsSpotDone
addi %r3,%r3,0x254
asmFmapPreventPortalsSpotDone:
mr %r0,%r3

# Restore important register values and end pop stack
lwz %r3,0x8(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr

#else

asmFmapPreventPortalsSpot:
# Restore the original instruction immediately
lwz %r3,0x18(%r31)

# Push stack
stwu %sp,-0x10(%sp)
mflr %r0
stw %r0,0x14(%sp)

# Backup important register values
stw %r3,0x8(%sp)

mr %r3,%r31 # dMenu_Fmap*
bl handleFmapPreventPortals
cmplwi %r3,1

# Begin pop stack
lwz %r3,0x14(%sp)

# Adjust LR address to return to if region not unlocked
beq+ asmFmapPreventPortalsSpotDone
addi %r3,%r3,0x1A0
asmFmapPreventPortalsSpotDone:
mr %r0,%r3

# Restore important register values and end pop stack
lwz %r3,0x8(%sp)
mtlr %r0
addi %sp,%sp,0x10
blr

#endif