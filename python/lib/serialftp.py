import serial
import time

class SerialFTP:
    def __init__(self, dev="/dev/ttyACM0", baudrate=115200):
        self.dev = dev
        self.baudrate = baudrate
        self.s = serial.Serial(dev, baudrate, timeout=1000)

    def get(self, src, dest):
        print(f"$ get {src}")
        self.s.write(b"get " + bytes(src,"ascii") + b"\n")

        time.sleep(0.01)

        f = None

        data = self.s.read(5)
        # print("data = ", data)
        if data != b"BEGIN":
            print("did not see BEGIN!")
            print(f"data = '{data}'")
            return

        # print("begin of file")

        data = self.s.read(4)
        # print("data = ", data)
        size = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24)
        # print(f"size = {hex(size)}")

        fbytes = self.s.read(size)
        if len(fbytes) != size:
            print("read too few bytes")
            return
        # print("fbytes = ", fbytes)

        data = self.s.read(3)
        # print("data = ", data)
        if data != b"XOR":
            print("did not see XOR!")
            return

        data = self.s.read(1)
        # print("data = ", data)
        csum = data[0]

        data = self.s.read(3)
        # print("data = ", data)
        if data != b"EOF":
            print("did not see EOF!")
            return

        asum = 0
        for byte in fbytes:
            asum = asum ^ byte

        if asum != csum:
            print(f"  ERROR: checksum mismatch: got {csum}, expected {asum}")
            return
        else:
            print(f"  checksum OK ({csum})!")

        n_bytes = len(fbytes)
        print(f"  Read file with {n_bytes} bytes")

        f = open(dest, "wb")
        f.write(fbytes)
        f.close()

        print(f"  Wrote file to {dest}")


    def begin(self):
        print("Switch to FS mode")
        self.s.write(b"XFS\n")
        time.sleep(0.01)
        while self.s.in_waiting > 0:
            data = self.s.read_until()
            if data == b"Switch to FS mode.\r\n":
                break
        time.sleep(0.1)

    def end(self):
        print("Leave FS mode")
        self.s.write(b"XSE\n")
        data = self.s.readline()

    def ls(self):
        files = []
        print("$ ls")
        self.s.write(b"ls\n");
        time.sleep(0.01)
        while self.s.in_waiting > 0:
            data = self.s.read_until()
            if data == b"Listing '/':\n":
                one = 1
            else:
                fields = data.decode('ascii').strip().split()
                if len(fields) == 2:
                    name, size = fields
                    # print(f"{name}\t{size}")
                    files.append({"name": name, "size": size})
                # else:
                    # print(fields)
        time.sleep(0.1)
        return files

    def rm(self, dest):
        self.s.write(b"rm " + bytes(dest, "ascii") + b"\n")

        while True:
            data = self.s.read_until().strip()
            print(data)
            if data == b"OK":
                break

    def put(self, src, dest):
        content = None
        with open(src, "r") as f:
            content = f.read()
            if len(content) > 4096:
                print("File is too large!")
                return

        if content is None:
            print("Nothing read.")
            return

        print(f"$ put {src}")
        self.s.write(b"put " + bytes(dest, "ascii") + b"\n")

        time.sleep(0.01)

        while True:
            data = self.s.read_until().strip()
            print(data)
            if data == b"OKGO":
                break

        self.s.write(bytes(content, "ascii"))
        self.s.write(b"\x04")

        time.sleep(0.1)
        data = self.s.read_until()
        print(data)


    def list_files(self):
        listing = self.ls()
        for f in listing:
            print(f"  {f['name']}\t{f['size']}")

