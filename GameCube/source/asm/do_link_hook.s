# asmDoLinkHookStart and asmDoLinkHookEnd need to be used in at least one subrel, so they cannot be set to hidden

.global asmDoLinkHookStart
.global asmDoLinkHookEnd

asmDoLinkHookStart:
mr %r3,%r31 # dmc
bl handleDoLinkHook

# Restore the original instruction
lwz %r3,0x10(%r31)

asmDoLinkHookEnd:
b 0