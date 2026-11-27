#include "contract.hpp"
#include "zarc_service.grpc.pb.h"
#include <vsomeip/vsomeip.hpp>
#include <grpcpp/grpcpp.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace binding=zarc::someip;
namespace schema=zarc::reference::v1;
using Bytes=std::vector<vsomeip::byte_t>;

static std::string read_file(const char *path){
 std::ifstream in(path,std::ios::binary);
 if(!in)throw std::runtime_error("credential file unavailable");
 std::string data((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
 if(data.empty()||data.size()>65536)throw std::runtime_error("invalid credential size");
 return data;
}
static void append64(Bytes &bytes,std::uint64_t value){
 for(int i=7;i>=0;--i)bytes.push_back(static_cast<vsomeip::byte_t>(value>>(8*i)));
}

int main(int argc,char **argv){
 if(argc!=5){std::cerr<<"usage: service loopback_grpc_endpoint CA cert key\n";return 2;}
 try{
  std::string endpoint=argv[1];
  if(endpoint.rfind("127.0.0.1:",0)!=0)throw std::runtime_error("lab endpoint must be loopback");
  grpc::SslCredentialsOptions credentials;
  credentials.pem_root_certs=read_file(argv[2]);
  credentials.pem_cert_chain=read_file(argv[3]);
  credentials.pem_private_key=read_file(argv[4]);
  auto stub=schema::IndicatorService::NewStub(grpc::CreateChannel(endpoint,grpc::SslCredentials(credentials)));
  auto runtime=vsomeip::runtime::get();
  auto app=runtime->create_application("zarc_someip_service");
  if(!app->init())return 1;
  std::mutex owner;
  app->register_message_handler(binding::service,binding::instance,binding::snapshot_method,
   [&](const std::shared_ptr<vsomeip::message> &request){
    Bytes result{static_cast<vsomeip::byte_t>(schema::INVALID)};
    if(request->get_interface_version()!=binding::major)
     result[0]=static_cast<vsomeip::byte_t>(schema::INCOMPATIBLE);
    else {
     auto payload=request->get_payload();
     if(payload&&payload->get_length()==4){
      std::uint32_t id=0;
      for(std::size_t i=0;i<4;++i)id=(id<<8)|payload->get_data()[i];
      // No unauthenticated write or client-selected delegated identity.
      // This read-only lab service has its own restricted broker certificate.
      std::unique_lock<std::mutex> lock(owner,std::try_to_lock);
      if(!lock.owns_lock())result[0]=static_cast<vsomeip::byte_t>(schema::BUSY);
      else {
       grpc::ClientContext context;
       context.set_deadline(std::chrono::system_clock::now()+std::chrono::milliseconds(900));
       schema::SnapshotRequest query;query.set_capability(id);
       schema::Observation observation;
       auto status=stub->GetSnapshot(&context,query,&observation);
       if(!status.ok()||!observation.has_status())
        result[0]=static_cast<vsomeip::byte_t>(schema::UNAVAILABLE);
       else if(!schema::Status_IsValid(observation.status()))
        result[0]=static_cast<vsomeip::byte_t>(schema::UNAVAILABLE);
       else {
        result[0]=static_cast<vsomeip::byte_t>(observation.status());
        if(observation.status()==schema::COMPLETED){
         if(!schema::Indicator_IsValid(observation.value())||!schema::Quality_IsValid(observation.quality())||!observation.boot())
          result[0]=static_cast<vsomeip::byte_t>(schema::UNAVAILABLE);
         else {
          result.push_back(static_cast<vsomeip::byte_t>(observation.value()));
          result.push_back(static_cast<vsomeip::byte_t>(observation.quality()));
          append64(result,observation.boot());append64(result,observation.generation());
         }
        }
       }
      }
     }
    }
    auto response=runtime->create_response(request);
    auto body=runtime->create_payload();body->set_data(result);
    response->set_payload(body);app->send(response);
   });
  app->register_state_handler([&](vsomeip::state_type_e state){
   if(state==vsomeip::state_type_e::ST_REGISTERED)
    app->offer_service(binding::service,binding::instance,binding::major,binding::minor);
  });
  app->start();
  return 0;
 }catch(const std::exception &e){std::cerr<<e.what()<<"\n";return 1;}
}
