#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import lib.serialftp

s = lib.serialftp.SerialFTP()

s.begin()

s.list_files()
s.get("hello", "/tmp/hello.read")

s.put("data/hello.orig", "hello3")
s.get("hello", "/tmp/hello.read")
s.get("hello2", "/tmp/hello2.read")
s.get("hello3", "/tmp/hello3.read")
s.list_files()
s.rm("hello3")
s.list_files()

s.end()

