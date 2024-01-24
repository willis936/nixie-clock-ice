import sys

print(', '.join(map(hex, open("{0}/hardware.bin".format(sys.argv[1]), 'rb').read())))