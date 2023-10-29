# SPDX-License-Identifier: MIT

import lib.serialftp
s = lib.serialftp.SerialFTP()
s.begin()
with open("/tmp/hello", "w") as f:
    f.write("Hello world!\n")
s.put("/tmp/hello", "hello")
s.get("hello", "/tmp/hello.read")
s.list_files()
s.rm("hello")
s.end()
