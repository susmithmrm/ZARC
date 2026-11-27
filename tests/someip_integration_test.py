"""Actual COVESA local routing, generated gRPC, broker and controller.
No Ethernet wire conformance, remote client authentication or SD multicast claim.
"""
import hashlib
import os
from pathlib import Path
import ssl
import subprocess
import sys
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"host"))
from zarc_host.broker import Gateway
from zarc_host.lab_backend import LabBackend
from zarc_host.security import Policy,Principal
from zarc_host.grpc_service import start

def run(*args):
    subprocess.run(["openssl",*map(str,args)],check=True,timeout=20,
                   stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)

with tempfile.TemporaryDirectory() as directory:
    p=Path(directory);ca=p/"ca.pem";ca_key=p/"ca.key"
    run("req","-x509","-newkey","rsa:2048","-nodes","-keyout",ca_key,
        "-out",ca,"-subj","/CN=ZARC SOMEIP fixture CA","-days","1",
        "-addext","basicConstraints=critical,CA:TRUE",
        "-addext","keyUsage=critical,keyCertSign,cRLSign")
    def issue(name,server=False):
        cert,key,csr,ext=(p/(name+s) for s in (".pem",".key",".csr",".ext"))
        ext.write_text("basicConstraints=critical,CA:FALSE\nkeyUsage=critical,digitalSignature,keyEncipherment\n"+
            ("extendedKeyUsage=serverAuth\nsubjectAltName=IP:127.0.0.1\n" if server else "extendedKeyUsage=clientAuth\n"))
        run("req","-new","-newkey","rsa:2048","-nodes","-keyout",key,"-out",csr,"-subj","/CN="+name)
        run("x509","-req","-in",csr,"-CA",ca,"-CAkey",ca_key,"-CAcreateserial","-out",cert,"-days","1","-extfile",ext)
        return cert,key
    cert,key=issue("localhost",True);client_cert,client_key=issue("someip-readonly")
    digest=hashlib.sha256(ssl.PEM_cert_to_DER_cert(client_cert.read_text())).hexdigest()
    policy=Policy({digest:Principal("someip-fixture",frozenset({1}),frozenset(),8)})
    controller=subprocess.Popen([sys.argv[1],"501"],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    backend=server=provider=None
    env={**os.environ,"VSOMEIP_CONFIGURATION":str(ROOT/"host/someip/local-fixture.json")}
    try:
        backend=LabBackend(int(controller.stdout.readline()))
        server,port=start(Gateway(policy,backend),certificate=cert.read_bytes(),
                          key=key.read_bytes(),trust_roots=ca.read_bytes())
        with (p/"provider.log").open("w+") as log:
            provider=subprocess.Popen([sys.argv[2],f"127.0.0.1:{port}",str(ca),str(client_cert),str(client_key)],
                                      env=env,stdout=log,stderr=log)
            try:
                time.sleep(1)
                result=subprocess.run([sys.argv[3]],env=env,timeout=18,capture_output=True,text=True)
                print(result.stdout,result.stderr)
                if result.returncode!=0:
                    log.seek(0);print(log.read())
                assert result.returncode==0
            finally:
                provider.terminate()
                try:provider.wait(timeout=5)
                except subprocess.TimeoutExpired:provider.kill();provider.wait()
                provider=None
    finally:
        if provider is not None:provider.kill();provider.wait()
        if server is not None:server.stop(0).wait(5)
        if backend is not None:backend.invalidate()
        controller.terminate()
        try:controller.communicate(timeout=3)
        except subprocess.TimeoutExpired:controller.kill();controller.communicate()
