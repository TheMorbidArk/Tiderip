from ctypes import *

# load the shared object file
adder = CDLL('build/libLibTiderip.so')

adder.runText()
