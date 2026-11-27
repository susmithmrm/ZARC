"""Constrained-deadline, fixed-priority uniprocessor response-time analysis.

All inputs are allocated upper bounds, not measured maxima. Reject unsupported
assumptions. ISR costs are modeled as higher-priority periodic/sporadic work.
"""
def analyze(tasks, *, uniprocessor, fully_preemptive, no_self_suspension):
    if not (uniprocessor and fully_preemptive and no_self_suspension):
        raise ValueError("unsupported timing assumptions")
    priorities=set();names=set()
    for t in tasks:
        if t["priority"] in priorities or t["name"] in names:
            raise ValueError("unique fixed priorities and task names required")
        priorities.add(t["priority"]);names.add(t["name"])
        for k in ("period_us","deadline_us","execution_us","blocking_us","jitter_us","priority"):
            if type(t[k]) is not int or t[k]<(0 if k in ("blocking_us","jitter_us","priority") else 1):
                raise ValueError("invalid timing allocation")
        if t["deadline_us"]>t["period_us"]:
            raise ValueError("arbitrary deadlines not supported")
    result={}
    ordered=sorted(tasks,key=lambda t:t["priority"])
    for i,t in enumerate(ordered):
        base=t["execution_us"]+t["blocking_us"]
        response=base
        limit=t["deadline_us"]-t["jitter_us"]
        converged=False
        for _ in range(10000):
            if response>limit:break
            nxt=base+sum(((response+h["jitter_us"]+h["period_us"]-1)//h["period_us"])*h["execution_us"] for h in ordered[:i])
            if nxt==response:
                converged=True;break
            response=nxt
        result[t["name"]]={"response_us":response+t["jitter_us"],
                           "deadline_us":t["deadline_us"],"admitted":converged and response<=limit}
    return result
