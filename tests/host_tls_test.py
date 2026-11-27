"""Real TLS handshake and desktop-controller integration; temporary lab keys only."""
import hashlib
import json
from pathlib import Path
import shutil
import socket
import ssl
import subprocess
import sys
import tempfile
import threading
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "host"))
from zarc_host.broker import Gateway
from zarc_host.lab_backend import LabBackend
from zarc_host.security import Principal, Policy, server_context, client_context

OPENSSL = shutil.which("openssl")
if OPENSSL is None:
    print("OpenSSL CLI unavailable; TLS integration not executed")
    raise SystemExit(77)

def run(*args):
    subprocess.run([OPENSSL, *map(str,args)], check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, timeout=20)

with tempfile.TemporaryDirectory() as directory:
    p=Path(directory)
    ca, ca_key = p/"ca.pem", p/"ca.key"
    run("req","-x509","-newkey","rsa:2048","-nodes","-keyout",ca_key,
        "-out",ca,"-subj","/CN=ZARC ephemeral fixture CA","-days","1",
        "-addext","basicConstraints=critical,CA:TRUE",
        "-addext","keyUsage=critical,keyCertSign,cRLSign")
    def issue(name, server=False):
        cert,key,csr,ext=(p/(name+s) for s in (".pem",".key",".csr",".ext"))
        ext.write_text("basicConstraints=critical,CA:FALSE\n"
                       "keyUsage=critical,digitalSignature,keyEncipherment\n"
                       +("extendedKeyUsage=serverAuth\nsubjectAltName=DNS:localhost\n"
                         if server else "extendedKeyUsage=clientAuth\n"))
        run("req","-new","-newkey","rsa:2048","-nodes","-keyout",key,
            "-out",csr,"-subj","/CN="+name)
        run("x509","-req","-in",csr,"-CA",ca,"-CAkey",ca_key,
            "-CAcreateserial","-out",cert,"-days","1","-extfile",ext)
        return cert,key
    server_cert,server_key=issue("localhost",True)
    observer_cert,observer_key=issue("observer")
    operator_cert,operator_key=issue("operator")
    unknown_cert,unknown_key=issue("unknown")
    def fingerprint(cert):
        return hashlib.sha256(ssl.PEM_cert_to_DER_cert(cert.read_text())).hexdigest()
    observer=Principal("observer",frozenset({1}),frozenset(),50)
    operator=Principal("operator",frozenset({1}),frozenset({1}),50)
    policy=Policy({fingerprint(observer_cert):observer,fingerprint(operator_cert):operator})
    server=server_context(server_cert,server_key,ca)
    controller=subprocess.Popen([sys.argv[1],"201"],stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,text=True)
    listener=socket.socket()
    stop=threading.Event()
    worker=None
    backend=None
    try:
        port=int(controller.stdout.readline().strip())
        backend=LabBackend(port)
        gateway=Gateway(policy,backend)
        listener.bind(("127.0.0.1",0));listener.listen(4)
        worker=threading.Thread(target=gateway.serve,args=(listener,server,stop.is_set))
        worker.start()
        def exchange(ctx,obj):
            with socket.create_connection(listener.getsockname(),timeout=3) as raw:
                with ctx.wrap_socket(raw,server_hostname="localhost") as tls:
                    tls.sendall(json.dumps(obj).encode()+b"\n")
                    data=bytearray()
                    while len(data)<512:
                        chunk=tls.recv(1)
                        if not chunk:return None
                        if chunk==b"\n":return json.loads(data)
                        data.extend(chunk)
            raise AssertionError("missing bounded response")
        read={"operation":"snapshot","capability":1}
        write={"operation":"request","capability":1,"desired":2,"validity":1000}
        reader=client_context(observer_cert,observer_key,ca)
        writer=client_context(operator_cert,operator_key,ca)
        assert exchange(reader,read)["value"]==1
        assert exchange(reader,write)["status"]=="DENIED"
        assert exchange(writer,write)["status"]=="ACCEPTED"
        deadline=time.monotonic()+2
        while True:
            result=exchange(reader,read)
            if result["value"]==2:
                assert result["quality"]==1
                break
            assert time.monotonic()<deadline
            time.sleep(.05)
        for ctx in (client_context(unknown_cert,unknown_key,ca),
                    ssl.create_default_context(ssl.Purpose.SERVER_AUTH,cafile=ca)):
            try:
                assert exchange(ctx,read) is None
            except (ssl.SSLError,ConnectionError):
                pass
        assert exchange(reader,read)["value"]==2
        print("TLS guest identity, denial and real desktop backend verified")
    finally:
        stop.set()
        if worker is not None:
            worker.join(4)
            assert not worker.is_alive(), "gateway failed to stop"
        listener.close()
        if backend is not None:backend.invalidate()
        controller.terminate()
        try:controller.communicate(timeout=3)
        except subprocess.TimeoutExpired:
            controller.kill();controller.communicate()
