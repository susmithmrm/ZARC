"""AAOS android16-release power property mapping (not electrical control).

The adapter is called by the trusted VHAL backend; Android reports AP state,
while the vehicle power coordinator emits requests. Neither is rail feedback.
"""
from dataclasses import dataclass
from enum import IntEnum

AP_POWER_STATE_REQ = 0x11410A00
AP_POWER_STATE_REPORT = 0x11410A01

class Request(IntEnum):
    ON = 0
    SHUTDOWN_PREPARE = 1
    CANCEL_SHUTDOWN = 2
    FINISHED = 3

class Report(IntEnum):
    WAIT_FOR_VHAL = 1
    DEEP_SLEEP_ENTRY = 2
    DEEP_SLEEP_EXIT = 3
    SHUTDOWN_POSTPONE = 4
    SHUTDOWN_START = 5
    ON = 6
    SHUTDOWN_PREPARE = 7
    SHUTDOWN_CANCELLED = 8
    HIBERNATION_ENTRY = 9
    HIBERNATION_EXIT = 10

@dataclass(frozen=True)
class PowerReport:
    state: Report
    parameter: int
    ready: bool

def decode_report(property_id, values):
    if (type(property_id) is not int or property_id != AP_POWER_STATE_REPORT or
            not isinstance(values, (tuple, list)) or len(values) != 2):
        raise ValueError("report direction or width")
    state, parameter = values
    if type(state) is not int or type(parameter) is not int or not 0 <= parameter <= 0x7FFFFFFF:
        raise ValueError("invalid report")
    state = Report(state)
    terminal = (Report.DEEP_SLEEP_ENTRY, Report.SHUTDOWN_START, Report.HIBERNATION_ENTRY)
    if state == Report.SHUTDOWN_POSTPONE:
        if parameter > 5000:
            raise ValueError("postpone exceeds Android limit")
    elif state not in terminal and parameter != 0:
        raise ValueError("unexpected report parameter")
    return PowerReport(state, parameter, state in terminal)

def encode_request(request, parameter, previous_report, *, allowed_shutdown_params):
    if (isinstance(request, bool) or not isinstance(request, (int, Request)) or
            isinstance(previous_report, bool) or not isinstance(previous_report, (int, Report))):
        raise ValueError("request or previous report type")
    request = Request(request)
    previous_report = Report(previous_report)
    allowed = {
        Request.ON: {Report.WAIT_FOR_VHAL, Report.DEEP_SLEEP_EXIT, Report.HIBERNATION_EXIT, Report.SHUTDOWN_CANCELLED},
        Request.SHUTDOWN_PREPARE: {Report.WAIT_FOR_VHAL, Report.ON, Report.DEEP_SLEEP_EXIT,
            Report.HIBERNATION_EXIT, Report.SHUTDOWN_CANCELLED, Report.SHUTDOWN_POSTPONE, Report.SHUTDOWN_PREPARE},
        Request.CANCEL_SHUTDOWN: {Report.SHUTDOWN_POSTPONE, Report.SHUTDOWN_PREPARE},
        Request.FINISHED: {Report.DEEP_SLEEP_ENTRY, Report.HIBERNATION_ENTRY, Report.SHUTDOWN_START},
    }
    if previous_report not in allowed[request]:
        raise ValueError("invalid request for last AP report")
    if type(parameter) is not int:
        raise ValueError("parameter")
    if request == Request.SHUTDOWN_PREPARE:
        try:
            approved = tuple(allowed_shutdown_params)
        except TypeError as exc:
            raise ValueError("invalid shutdown mode policy") from exc
        if (any(type(mode) is not int for mode in approved) or
                parameter not in approved or not 1 <= parameter <= 7):
            raise ValueError("unapproved shutdown mode")
    elif parameter != 0:
        raise ValueError("unexpected request parameter")
    return AP_POWER_STATE_REQ, (int(request), parameter)
