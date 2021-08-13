#!/usr/bin/env python
"""Like 'cat', using mmap()
"""

from __future__ import print_function

import sys
import os
from mmap import *

def readfile(name):
    with open(name, 'r') as F:
        return F.read()

uio = sys.argv[1] # eg. "uio0"
region = int(sys.argv[2])

print('Device:', readfile('/sys/class/uio/%s/name'%uio).strip(), file=sys.stderr)
print('Region:', readfile('/sys/class/uio/%s/maps/map%d/name'%(uio, region)).strip(), file=sys.stderr)

size = int(readfile('/sys/class/uio/%s/maps/map%d/size'%(uio, region)).strip(), 0)

with open('/dev/%s'%uio, 'rb', buffering=0) as F:
    with mmap(F.fileno(), size, flags=MAP_SHARED, prot=PROT_READ, offset=region*PAGESIZE) as M:
        sys.stdout.buffer.write(M.read())
