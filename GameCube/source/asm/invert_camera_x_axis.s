.global asmInvertCameraXAxis
# asmInvertCameraXAxis needs to be used in at least one subrel, so it cannot be set to hidden
asmInvertCameraXAxis:
lfs %f1, 0x10 (%r3) # Load the C-stick's horizontal axis for controlling the camera (same as the line we're replacing)
fneg %f1, %f1 # Negate the horizontal axis
b updatePadEnd