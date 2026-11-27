"""Independent generated gRPC client, authenticated host and desktop controller."""
import hashlib
from pathlib import Path
import shutil
import ssl
import subprocess
import sys
import tempfile
import time

import grpc
import zarc_service_pb2 as pb
import zarc_service_pb2_grpc as rpc

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"host"))
from zarc_host.broker import Gateway
from zarc_host.security import Policy,Principal
from zarc_host.lab_backend import LabBackend
from zarc_host.grpc_service import start

openssl=shutil.which("openssl")
if not openssl:raise RuntimeError("OpenSSL fixture dependency required")
def run(*args):
    subprocess.run([openssl,*map(str,args)],check=True,timeout=20,
                   stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)

with tempfile.TemporaryDirectory() as directory:
    p=Path(directory);ca=p/"ca.pem";ca_key=p/"ca.key"
    run("req","-x509","-newkey","rsa:2048","-nodes","-keyout",ca_key,
        "-out",ca,"-subj","/CN=ZARC test CA","-days","1",
        "-addext","basicConstraints=critical,CA:TRUE",
        "-addext","keyUsage=critical,keyCertSign,cRLSign")
    def issue(name,server=False):
        cert,key,csr,ext=(p/(name+s) for s in (".pem",".key",".csr",".ext"))
        ext.write_text("basicConstraints=critical,CA:FALSE\nkeyUsage=critical,digitalSignature,keyEncipherment\n"+
            ("extendedKeyUsage=serverAuth\nsubjectAltName=IP:127.0.0.1\n" if server else "extendedKeyUsage=clientAuth\n"))
        run("req","-new","-newkey","rsa:2048","-nodes","-keyout",key,"-out",csr,"-subj","/CN="+name)
        run("x509","-req","-in",csr,"-CA",ca,"-CAkey",ca_key,"-CAcreateserial","-out",cert,"-days","1","-extfile",ext)
        return cert,key
    def digest(cert):
        return hashlib.sha256(ssl.PEM_cert_to_DER_cert(cert.read_text())).hexdigest()
    server_cert,server_key=issue("localhost",True)
    reader_cert,reader_key=issue("observer")
    writer_cert,writer_key=issue("operator")
    unknown_cert,unknown_key=issue("unknown")
    policy=Policy({digest(reader_cert):Principal("reader",frozenset({1}),frozenset(),50),
                   digest(writer_cert):Principal("writer",frozenset({1}),frozenset({1}),50)})
    controller=subprocess.Popen([sys.argv[1],"401"],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    backend=server=None;channels=[]
    try:
        backend=LabBackend(int(controller.stdout.readline()))
        server,port=start(Gateway(policy,backend),certificate=server_cert.read_bytes(),
                          key=server_key.read_bytes(),trust_roots=ca.read_bytes())
        def stub(cert=None,key=None):
            credentials=grpc.ssl_channel_credentials(root_certificates=ca.read_bytes(),
                         private_key=key.read_bytes() if key else None,certificate_chain=cert.read_bytes() if cert else None)
            channel=grpc.secure_channel(f"127.0.0.1:{port}",credentials)
            channels.append(channel)
            return rpc.IndicatorServiceStub(channel)
        reader=stub(reader_cert,reader_key);writer=stub(writer_cert,writer_key)
        snapshot=pb.SnapshotRequest(capability=1)
        intent=pb.Intent(capability=1,desired=pb.INDICATOR_ON,validity_ms=1000)
        first=reader.GetSnapshot(snapshot,timeout=5)
        assert first.HasField("status") and first.status==pb.COMPLETED and first.value==pb.INDICATOR_OFF
        assert reader.RequestState(intent,timeout=5).status==pb.DENIED
        accepted=writer.RequestState(intent,timeout=5)
        assert accepted.HasField("status") and accepted.status==pb.ACCEPTED and accepted.request_id
        end=time.monotonic()+3
        while True:
            observed=reader.GetSnapshot(snapshot,timeout=3)
            if observed.value==pb.INDICATOR_ON:
                assert observed.quality==pb.QUALITY_VALID
                break
            assert time.monotonic()<end
            time.sleep(.05)
        for unauthorized in (stub(unknown_cert,unknown_key),stub()):
            try:
                unauthorized.GetSnapshot(snapshot,timeout=3)
                raise AssertionError("unauthorized connection accepted")
            except grpc.RpcError as e:
                assert e.code() in (grpc.StatusCode.PERMISSION_DENIED,grpc.StatusCode.UNAUTHENTICATED,grpc.StatusCode.UNAVAILABLE)
        print("Generated gRPC service, certificate scope and independent feedback passed")
    finally:
        for channel in channels:channel.close()
        if server is not None:server.stop(0).wait(5)
        if backend is not None:backend.invalidate()
        controller.terminate()
        try:controller.communicate(timeout=3)
        except subprocess.TimeoutExpired:controller.kill();controller.communicate()
