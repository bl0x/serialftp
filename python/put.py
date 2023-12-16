#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import sys
import lib.serialftp

args = sys.argv

dest = None
if len(args) > 2:
    file = args[1]
    dest = args[2]
else:
    print("Usage: put file dest")
    print("       file: Path on local file system")
    print("       dest: Path on flash file system")
    exit()

s = lib.serialftp.SerialFTP()
s.begin()
s.put(file, dest)
s.end()
