# serialftp - A tiny library to transmit files over serial / UART

`serialftp` can be used as a way to carry out simple file transfer operations
over a serial connection to a microcontroller's flash memory.


## Examples

### Arduino

```c
#include <Arduino.h>
#include <serialftp.h>

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

## Supported

### Operations

- ls
- get
- put
- rm

### File systems

- LittleFS

### Hardware

`serialftp` is tested on

- Raspberry Pi Pico / Pico W (earlephilhower/arduino-pico)

## Dependencies

- Arduino framework
- LittleFS


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


## Authors:

* **Bastian Löher** - *Initial work*
