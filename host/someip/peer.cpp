#include "contract.hpp"
#include <vsomeip/vsomeip.hpp>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

namespace binding=zarc::someip;
int main(){
 auto runtime=vsomeip::runtime::get();
 auto app=runtime->create_application("zarc_someip_peer");
 if(!app->init())return 1;
 std::promise<bool> done;auto result=done.get_future();
 std::atomic<bool> sent{false},finished{false};
 app->register_message_handler(binding::service,binding::instance,binding::snapshot_method,
  [&](const std::shared_ptr<vsomeip::message> &response){
   auto p=response->get_payload();
   bool ok=response->get_message_type()==vsomeip::message_type_e::MT_RESPONSE&&
     response->get_interface_version()==binding::major&&p&&p->get_length()==19;
   if(ok){
    auto d=p->get_data();
    std::uint64_t boot=0,generation=0;
    for(std::size_t i=3;i<11;++i)boot=(boot<<8)|d[i];
    for(std::size_t i=11;i<19;++i)generation=(generation<<8)|d[i];
    ok=d[0]==1&&d[1]==1&&d[2]==1&&boot==501&&generation==1;
   }
   if(!finished.exchange(true))done.set_value(ok);
  });
 app->register_availability_handler(binding::service,binding::instance,
  [&](vsomeip::service_t,vsomeip::instance_t,bool available){
   if(!available||sent.exchange(true))return;
   auto request=runtime->create_request(true);
   request->set_service(binding::service);request->set_instance(binding::instance);
   request->set_method(binding::snapshot_method);request->set_interface_version(binding::major);
   auto payload=runtime->create_payload();payload->set_data(std::vector<vsomeip::byte_t>{0,0,0,1});
   request->set_payload(payload);app->send(request);
  },binding::major,binding::minor);
 app->register_state_handler([&](vsomeip::state_type_e state){
  if(state==vsomeip::state_type_e::ST_REGISTERED)
   app->request_service(binding::service,binding::instance,binding::major,binding::minor);
 });
 std::thread worker([&]{app->start();});
 bool ok=result.wait_for(std::chrono::seconds(10))==std::future_status::ready&&result.get();
 app->release_service(binding::service,binding::instance);
 app->stop();worker.join();
 if(ok)std::cout<<"SOME/IP peer snapshot contract passed\n";
 return ok?0:1;
}
