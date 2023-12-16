#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import sys
import lib.serialftp

args = sys.argv

dest = None
if len(args) > 1:
    file = args[1]
else:
    print("Usage: rm file")
    print("       file: Path on flash file system")
    exit()

s = lib.serialftp.SerialFTP()
s.begin()
s.rm(file)
s.end()
