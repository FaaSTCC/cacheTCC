//
// Created by Taras Lykhenko on 29/09/2020.
//
#include <causal/causal_state.hpp>
#include <causal/causal_object.hpp>


class ReadState : public State {
  map<Key,CausalObject> result_;
  set<Key> keys_;
  Address address_;
  std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse;
public:

  ReadState(set<Key> keys,
      Address address, std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse_) :keys_{keys}, address_{address}, pendingClientResponse{pendingClientResponse_}{

  }


  void handleCache(CausalObject tuple) override{
    keys_.erase(tuple.key);
    result_.insert(pair<Key,CausalObject>{tuple.key,CausalObject(tuple.key,tuple.payload, tuple.timestamp, tuple.promise)});


    if(keys_.size() == 0){
      pendingClientResponse.push(pair<Address, map<Key,CausalObject>>{address_,result_});
    }




  }
};








