//
// Created by Taras Lykhenko on 29/09/2020.
//
#include <causal/causal_state.hpp>
#include <causal/causal_object.hpp>


class ReadCacheState : public State {
  map<Key,CausalObject> result_;
  map<Key,CausalObject> result_possible_;
  set<Key> keys_;
  Address address_;
  std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse;
  logger log_;
public:

  ReadCacheState(set<Key> missing_keys, set<Key> keys, map<Key, unsigned long long> key_t_low, unsigned long long t_high, unsigned long long lastSeen,
      map<Key,CausalObject> result, map<Key,CausalObject> result_possible,
      Address address, std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse_, logger &log) :keys_{missing_keys}, result_{result}, result_possible_{result_possible}, address_{address}, pendingClientResponse{pendingClientResponse_}, log_{log}{

  }


  void handleCache(CausalObject tuple) override{
      log_->info("Handling key {}", tuple.key);
    keys_.erase(tuple.key);
    result_.insert(pair<Key,CausalObject>{tuple.key,CausalObject(tuple.key,tuple.payload, tuple.timestamp, tuple.promise)});


    if(keys_.size() == 0){
        log_->info("Finishing request, can be sent to client");
        pendingClientResponse.push(pair<Address, map<Key,CausalObject>>{address_,result_});
      delete this;
    }




  }
};








