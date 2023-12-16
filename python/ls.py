#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import sys
import lib.serialftp

args = sys.argv

dest = None
if len(args) > 1:
    dest = args[1]

s = lib.serialftp.SerialFTP()
s.begin()
if dest is not None:
    s.cd(dest)
else:
    s.cd("/")
s.list_files()
s.end()
