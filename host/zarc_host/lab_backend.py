"""Actual private ZARC desktop loopback binding, not an authenticated MCU link."""
import socket
import struct

HEADER = struct.Struct("<2sBBBBHQQQ")
HELLO, SNAPSHOT, DOMAIN, RESULT = 1, 3, 4, 10
STATUS = ("ACCEPTED", "COMPLETED", "DENIED", "BUSY", "CONFLICT",
          "INVALID", "EXPIRED", "INCOMPATIBLE", "UNAVAILABLE", "UNKNOWN_OUTCOME")

class LabBackend:
    def __init__(self, port):
        if type(port) is not int or not 1 <= port <= 65535:
            raise ValueError("invalid port")
        self.port = port
        self.sock = None
        self.boot = self.session = self.request_id = 0

    def invalidate(self):
        if self.sock is not None:
            self.sock.close()
        self.sock = None
        self.boot = self.session = self.request_id = 0

    def _read(self, n):
        result = bytearray()
        while len(result) < n:
            part = self.sock.recv(n-len(result))
            if not part:
                raise OSError("peer disconnected")
            result.extend(part)
        return bytes(result)

    def _exchange(self, op, payload):
        self.request_id += 1
        self.sock.sendall(HEADER.pack(b"ZR", 1, 0, op, 0, len(payload),
                                      self.boot, self.session, self.request_id)+payload)
        magic, major, minor, operation, reserved, length, boot, session, rid = HEADER.unpack(self._read(HEADER.size))
        if (magic, major, minor, operation, reserved) != (b"ZR", 1, 0, RESULT, 0) or not 1 <= length <= 256 or rid != self.request_id:
            raise ValueError("invalid response")
        data = self._read(length)
        if data[0] >= len(STATUS):
            raise ValueError("invalid status")
        if op == HELLO:
            if STATUS[data[0]] != "COMPLETED" or not boot or not session or length != 1:
                raise OSError("session unavailable")
            self.boot, self.session = boot, session
        elif (boot, session) != (self.boot, self.session):
            raise OSError("epoch changed")
        return data

    def _connect(self):
        if self.sock is None:
            self.sock = socket.create_connection(("127.0.0.1", self.port), timeout=1)
            self._exchange(HELLO, b"")

    def snapshot(self, capability):
        self._connect()
        data = self._exchange(SNAPSHOT, struct.pack("<I", capability))
        result = {"status": STATUS[data[0]]}
        if data[0] == 1:
            if len(data) != 11 or data[1] > 3 or data[2] > 3:
                raise ValueError("invalid observation")
            result.update(value=data[1], quality=data[2],
                          generation=int.from_bytes(data[3:11], "little"), boot=self.boot)
        elif len(data) != 1:
            raise ValueError("invalid error response")
        return result

    def request(self, capability, desired, validity):
        self._connect()
        data = self._exchange(DOMAIN, struct.pack("<IBI", capability, desired, validity))
        if len(data) != 1:
            raise ValueError("invalid admission")
        return {"status": STATUS[data[0]], "request_id": self.request_id}
