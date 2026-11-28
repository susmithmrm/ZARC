"""Typed SDV/SOME-IP service mapping for a trusted host adapter.

This maps a selected service contract to the gateway; it is not a SOME/IP
transport or service-discovery stack. Identifiers are deployment allocations.
"""
from dataclasses import dataclass
import json
import struct

STATUSES = ("ACCEPTED", "COMPLETED", "DENIED", "BUSY", "CONFLICT", "INVALID",
            "EXPIRED", "INCOMPATIBLE", "UNAVAILABLE", "UNKNOWN_OUTCOME")

@dataclass(frozen=True)
class Binding:
    service: int
    instance: int
    snapshot_method: int
    request_method: int
    observation_event: int
    event_group: int
    major: int = 1
    minor: int = 0

    def __post_init__(self):
        if any(type(x) is not int or not 1 <= x < 0xFFFF
               for x in (self.service,self.instance,self.event_group)):
            raise ValueError("unallocated identifier")
        if any(type(x) is not int or not 1 <= x < 0x8000
               for x in (self.snapshot_method,self.request_method)):
            raise ValueError("invalid method ID")
        if self.snapshot_method == self.request_method or not 0x8000 <= self.observation_event < 0xFFFF:
            raise ValueError("invalid method/event allocation")
        if (self.major,self.minor) != (1,0):
            raise ValueError("unsupported version")

class Service:
    def __init__(self, gateway, binding):
        self.gateway = gateway
        self.binding = binding

    def dispatch(self, principal, *, service, instance, method, major, payload):
        b = self.binding
        if (type(service) is not int or type(instance) is not int or
                type(method) is not int or type(major) is not int or
                type(payload) is not bytes):
            return bytes([STATUSES.index("INVALID")])
        if (service,instance,major) != (b.service,b.instance,b.major):
            return bytes([STATUSES.index("INCOMPATIBLE")])
        if method == b.snapshot_method and len(payload) == 4:
            cap, = struct.unpack(">I",payload)
            obj = {"operation":"snapshot","capability":cap}
        elif method == b.request_method and len(payload) == 9:
            cap, desired, validity = struct.unpack(">IBI",payload)
            obj = {"operation":"request","capability":cap,"desired":desired,"validity":validity}
        else:
            return bytes([STATUSES.index("INVALID")])
        response = self.gateway.dispatch(principal,json.dumps(obj).encode())
        status = response.get("status","UNAVAILABLE")
        if status not in STATUSES:
            status = "UNAVAILABLE"
        if method == b.snapshot_method and status == "COMPLETED":
            try:
                return struct.pack(">BBBQQ",STATUSES.index(status),response["value"],
                    response["quality"],response["boot"],response["generation"])
            except (KeyError,struct.error,TypeError):
                return bytes([STATUSES.index("UNAVAILABLE")])
        return bytes([STATUSES.index(status)])

def sdv_availability(service_ready, observation):
    """Service process availability never manufactures physical readiness."""
    if type(service_ready) is not bool or not service_ready or not isinstance(observation, dict):
        return {"service_available": service_ready is True, "physical_available": False}
    return {"service_available": True,
            "physical_available": observation.get("status") == "COMPLETED"
                                  and type(observation.get("quality")) is int
                                  and observation.get("quality") == 1,
            "observation": observation}
