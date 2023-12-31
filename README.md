# serialftp - A tiny library to transmit files over serial / UART

`serialftp` can be used as a way to carry out simple file transfer operations
over a serial connection to a microcontroller's flash memory.

The aim of this project is not to have full-fledged access to any file system via a microcontroller, but to provide the bare minimum needed to get a file in or out.

For more advanced use cases other projects exist, such as:
- [serialtransfer](https://www.arduino.cc/reference/en/libraries/serialtransfer/)

A companion library written in Python is supplied in [python/lib](python/lib).

## Examples

### Arduino

```c
#include <Arduino.h>
#include <serialftp.h>

SerialFTP sftp;

void setup() {
    Serial.begin();
    sftp.begin();
}

void loop() {
    sftp.run();
}
```

### Python

```python
import serialftp

s = serialftp.SerialFTP()

with open("/tmp/hello", "w") as f:
    f.write("Hello world!\n")

s.begin()
s.put("/tmp/hello", "hello")
s.get("hello", "/tmp/hello.read")
s.list_files()
s.rm("hello")
s.end()
```

### Bash

```bash
./ls.py              # List root directory
./ls.py dir          # List 'dir'
./get.py file dest   # Copy remote 'file' to local 'dest'
./put.py file dest   # Copy local 'file' to remote 'dest'
./rm.py file         # Remove remote 'file'
```

## Supported

### Operations

- ls
- cd
- get
- put
- rm

### File systems

- LittleFS

### Hardware

`serialftp` is tested on

- Raspberry Pi Pico / Pico W (earlephilhower/arduino-pico)

and only with small files (<1 kByte).

## Limitations

- Fixed `begin` / `end` special character sequences
- Maximum file size is 4096 bytes (value of `SERIALFTP_MAX_FILE_SIZE`)
- File names are limited to these characters: `[a-z][A-Z][0-9]._-/`
- No empty directories (because of LittleFS)
- `run` function is not very flexible
- used serial port is `Serial`
- `cd` does not support `..` for entering the parent directory


## Dependencies

- Arduino framework
- LittleFS


## How it works

A special character sequence switches the microcontroller to "file transfer" mode and back.

While in file transfer mode, the operations listed above can be made.
Each operation must be terminated with a trailing newline `'\n'`.

Operations, such as `ls`, `get` and `rm` produce human readable output and can be used directly from a terminal program.

For byte-correct I/O, the `get` and `put` operations should not be written by hand, but best used via the supplied python library or a similar tool.


## Protocols

### get

```
input : "get\n"
output: "BEGIN" SIZE DATA "XOR" CHECKSUM "EOF"
```

Quoted strings are transmitted verbatim.

`SIZE` is a 4-byte little-endian unsigned integer that contains the size of the following `DATA` array.

`DATA` is a sequence of single bytes. The sequence is `SIZE` bytes long.

`CHECKSUM` is an 8-bit XOR-checksum over the bytes in `DATA`.

### put

```
input : "put" FILENAME "\n"
output: "OKGO"
input : DATA "\x04"
output: "Wrote " SIZE " bytes to file '" PATH "'"
```

`FILENAME` is the path and name of the file in the flash memory.
In case `FILENAME` fails validation, instead of `"OKGO"`, an error message is returned and the `put` operation is cancelled.

`DATA` is the content of the file to be written. `DATA` is followed by the EOT (end-of-transmission) character (`0x04` or Ctrl-D).

`SIZE` is the size of the file content on flash memory.

`PATH` is the path, where the file is stored and can be accessed from.

`PATH` should be equal to `FILENAME`.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


## Authors:

* **Bastian Löher** - *Initial work*
