"""Generated protobuf/gRPC binding of the candidate semantic service.

The same sole-owner Gateway is used by this transport. Run this endpoint instead
of, not concurrently with, the TLS JSON listener for the same backend.
The downstream lab backend is not authenticated MCU production transport.
"""
from concurrent.futures import ThreadPoolExecutor
import json
import ssl
import grpc
import zarc_service_pb2 as pb
import zarc_service_pb2_grpc as rpc

def _principal(gateway, context):
    auth=context.auth_context()
    certs=auth.get("x509_pem_cert", ())
    if b"ssl" not in auth.get("transport_security_type",()) or len(certs)!=1:
        context.abort(grpc.StatusCode.UNAUTHENTICATED,"verified client certificate required")
    try:
        der=ssl.PEM_cert_to_DER_cert(certs[0].decode("ascii"))
        return gateway.policy.identify_der(der)
    except (ValueError,UnicodeError,PermissionError):
        context.abort(grpc.StatusCode.PERMISSION_DENIED,"certificate not authorized")

def _status(result):
    try:return pb.Status.Value(result.get("status","UNAVAILABLE"))
    except (ValueError,TypeError):return pb.UNAVAILABLE

class Service(rpc.IndicatorServiceServicer):
    def __init__(self,gateway):
        self.gateway=gateway

    def GetSnapshot(self,request,context):
        principal=_principal(self.gateway,context)
        if not context.is_active():
            context.abort(grpc.StatusCode.CANCELLED,"request no longer active")
        result=self.gateway.dispatch(principal,json.dumps(dict(operation="snapshot",
                        capability=request.capability)).encode())
        status=_status(result)
        if status!=pb.COMPLETED:return pb.Observation(status=status)
        try:
            value,quality,boot,generation=(result[k] for k in ("value","quality","boot","generation"))
            if value not in (0,1,2,3) or quality not in (0,1,2,3) or not boot:
                raise ValueError("invalid backend observation")
            return pb.Observation(status=status,value=value,quality=quality,
                                  boot=boot,generation=generation)
        except (KeyError,TypeError,ValueError):
            return pb.Observation(status=pb.UNAVAILABLE)

    def GetHealth(self,request,context):
        principal=_principal(self.gateway,context)
        if not context.is_active():
            context.abort(grpc.StatusCode.CANCELLED,"request no longer active")
        result=self.gateway.dispatch(principal,json.dumps(dict(operation="health",
                            capability=request.capability)).encode())
        status=_status(result)
        if status!=pb.COMPLETED:return pb.Health(status=status)
        try:
            return pb.Health(status=status,**{k:result[k] for k in (
                "capability","failures","ready","pending","effect_owned","sample_tick","source_sequence","boot","session")})
        except (KeyError,TypeError,ValueError):
            return pb.Health(status=pb.UNAVAILABLE)

    def GetRequestStatus(self,request,context):
        principal=_principal(self.gateway,context)
        if not context.is_active():
            context.abort(grpc.StatusCode.CANCELLED,"request no longer active")
        result=self.gateway.dispatch(principal,json.dumps(dict(operation="request_status",
              capability=request.capability,request_id=request.request_id,boot=request.boot,session=request.session)).encode())
        return pb.RequestOutcome(status=_status(result))

    def RequestState(self,request,context):
        principal=_principal(self.gateway,context)
        if not context.is_active():
            context.abort(grpc.StatusCode.CANCELLED,"request no longer active")
        result=self.gateway.dispatch(principal,json.dumps(dict(operation="request",
              capability=request.capability,desired=request.desired,validity=request.validity_ms)).encode())
        status=_status(result)
        rid=result.get("request_id",0)
        if status==pb.ACCEPTED and (type(rid) is not int or not 1<=rid<=0xffffffffffffffff):
            return pb.Admission(status=pb.UNKNOWN_OUTCOME)
        if status!=pb.ACCEPTED:return pb.Admission(status=status)
        boot,session=result.get("boot"),result.get("session")
        if any(type(x) is not int or not 1<=x<=0xffffffffffffffff for x in (boot,session)):
            return pb.Admission(status=pb.UNKNOWN_OUTCOME)
        return pb.Admission(status=status,request_id=rid,boot=boot,session=session)

def start(gateway, *, certificate, key, trust_roots, port=0):
    if type(port) is not int or not 0<=port<=65535:
        raise ValueError("invalid port")
    if not certificate or not key or not trust_roots:
        raise ValueError("mutual TLS credentials required")
    credentials=grpc.ssl_server_credentials(((key,certificate),),root_certificates=trust_roots,
                                            require_client_auth=True)
    # Serialize all calls; reject work beyond one concurrent RPC. No direct
    # device calls or automatic retries. Library TLS configuration is not a
    # claim of an approved automotive cipher/provisioning profile.
    server=grpc.server(ThreadPoolExecutor(max_workers=1),maximum_concurrent_rpcs=1,
                       options=(("grpc.max_receive_message_length",512),
                                ("grpc.max_send_message_length",512)))
    rpc.add_IndicatorServiceServicer_to_server(Service(gateway),server)
    bound=server.add_secure_port(f"127.0.0.1:{port}",credentials)
    if not bound:
        raise OSError("secure bind failed")
    server.start()
    return server,bound
