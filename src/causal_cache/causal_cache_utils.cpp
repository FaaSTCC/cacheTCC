//  Copyright 2019 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//  Modifications copyright (C) 2021 Taras Lykhenko, Rafael Soares
#include <causal/causal_object.hpp>
#include "causal/causal_cache_utils.hpp"
#include "causal/causal_state.hpp"



void process_response(
    const KeyTuple &tuple,
    StoreType &unmerged_store,
    SocketCache &pushers, KvsClientInterface *client, logger log, map<Key, std::multimap<unsigned long long, State*>> &pending_key_requests, std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse_ ) {
  std::shared_ptr<DataType> lattice;
  lattice =
      std::make_shared<DataType>(deserialize_lww(tuple.payload()));

    log->info("Got response for key {}", tuple.key());

  if(unmerged_store.find(tuple.key())==unmerged_store.end()){
      log->info("Key {} is not in unmerged store", tuple.key());

      unmerged_store.insert(pair<Key, std::shared_ptr<DataType>>{tuple.key(), lattice});
    }else{
      unmerged_store.at(tuple.key())->merge(lattice->reveal());
    }
    if(pending_key_requests.find(tuple.key()) != pending_key_requests.end()){
        log->info("Request is pending key {}", tuple.key());
        log->info("Tuple high {} Tuple low {}", tuple.t_high(), tuple.t_low());

        auto it = pending_key_requests.at(tuple.key()).upper_bound(tuple.t_high());
      while(it != pending_key_requests.at(tuple.key()).begin()) {
        --it;
          log->info("Pending key {} got key with commit time {} and got promise {}", tuple.key(), tuple.t_low(), it->first);

          if(tuple.t_low() <=  it->first){
            log->info("Pending key {} is handled in state", tuple.key());

            it->second->handleCache(CausalObject(tuple.key(),lattice->reveal().value,lattice->reveal().timestamp,lattice->reveal().promise));
              log->info("Handled cache for key {}", tuple.key());

              it = pending_key_requests.at(tuple.key()).erase(it);
        }else{
          break;
        }
      }
        if (pending_key_requests.at(tuple.key()).empty()){
            log->info("Pending request for key {} is empty", tuple.key());

            pending_key_requests.erase(tuple.key());
      }
    }
    log->info("Completed");

    while(!pendingClientResponse_.empty()){
        log->info("Starting response to client");

        auto entry = pendingClientResponse_.front();
      pendingClientResponse_.pop();
      auto response = entry.second;
      CausalResponse clientResponse;
      for (pair<Key, CausalObject> entry : response) {
          log->info("Responding to request to key {}", entry.first);
          CausalTuple *tp = clientResponse.add_tuples();
        tp->set_key(entry.first);
        tp->set_payload(serialize(DataType{TimestampValuePair<string>{entry.second.timestamp, entry.second.promise, entry.second.payload}}));
        tp->set_error(AnnaError::NO_ERROR);
        tp->set_lattice_type(LatticeType::LWW);
      }
      string resp_string;
      clientResponse.SerializeToString(&resp_string);
      kZmqUtil->send_string(resp_string, &pushers[entry.first]);
    }
}

void respond_to_client(
    SocketCache &pushers, KvsClientInterface *client, map<Key, std::shared_ptr<DataType>> result, Address responseAddress){

    CausalResponse clientResponse;
    for (pair<Key, std::shared_ptr<DataType>> entry : result) {
      CausalTuple *tp = clientResponse.add_tuples();
      tp->set_key(entry.first);
      tp->set_payload(serialize(entry.second->reveal()));
    }
    string resp_string;
    clientResponse.SerializeToString(&resp_string);
    kZmqUtil->send_string(resp_string, &pushers[responseAddress]);
}




