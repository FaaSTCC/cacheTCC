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
#include "cacheTransaction.cpp"
#include "cacheState.cpp"

#include "causal/causal_cache_utils.hpp"


template<typename Map> typename Map::const_iterator
greatest_less(Map const& m, typename Map::key_type const& k) {
  typename Map::const_iterator it = m.lower_bound(k);
  if(it != m.begin()) {
    return --it;
  }
  return m.end();
}

void get_request_handler(
    const string &serialized, set<Key> &key_set, StoreType &cache_store,
    SocketCache &pushers, KvsClientInterface *client, logger log, map<Key, std::multimap<unsigned long long, State*>> &pending_key_requests , std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse) {

  CausalRequest causalRequest;
  causalRequest.ParseFromString(serialized);
  if(causalRequest.consistency() == ConsistencyType::NORMAL){
    KeyRequest keyRequest;
    keyRequest.ParseFromString(serialized);
    set<Key> keys;
    key_set.insert(keyRequest.tuples()[0].key());
    keys.insert(keyRequest.tuples()[0].key());

    if (cache_store.find(keyRequest.tuples()[0].key()) != cache_store.end()) {
      CausalResponse clientResponse;
      auto cache_entry = cache_store.at(
          keyRequest.tuples()[0].key());
      CausalTuple *tp = clientResponse.add_tuples();
      tp->set_key(keyRequest.tuples()[0].key());
      tp->set_payload(serialize(DataType{TimestampValuePair<string>{cache_entry->reveal().timestamp, cache_entry->reveal().promise, cache_entry->reveal().value}}));
      tp->set_error(AnnaError::NO_ERROR);
      tp->set_lattice_type(LatticeType::LWW);
      log->info("Got Normal request for key {}", keyRequest.tuples()[0].key());

      string resp_string;
      clientResponse.SerializeToString(&resp_string);
      kZmqUtil->send_string(resp_string, &pushers[keyRequest.response_address()]);
      return;
    }

    State * state =new ReadState(keys, keyRequest.response_address(),pendingClientResponse);
    for(Key key : keys){
      if (pending_key_requests.find(key) == pending_key_requests.end()) {
        pending_key_requests.insert(pair<Key, std::multimap<unsigned long long, State*>>{key,std::multimap<unsigned long long, State*>{}});
      }
      pending_key_requests.at(key).insert(pair<unsigned long long, State*>{0,state});
    }
      log->info("Requesting normal key {} to Anna", keyRequest.tuples()[0].key());
      client->get_async(keyRequest.tuples()[0].key());

    return;
  }
  set<Key> keys;
  for(CausalTuple ct : causalRequest.tuples()){
    key_set.insert(ct.key());
    keys.insert(ct.key());
  }

  map<Key, CausalObject> response;
  map<Key, CausalObject> possible_response;
  set<Key> storageReadKeys;
  unsigned long long t_low = causalRequest.t_low();
  unsigned long long t_high = causalRequest.t_high();
  unsigned long long d_low = t_low;
  unsigned long long d_high = t_high;
  map<Key, unsigned long long> key_t_low;
  for (Key key : keys) {
      log->info("Got request to key {}", key);

      if (cache_store.find(key) != cache_store.end()) {
          log->info("Have key {} in cache", key);

          auto cache_entry = cache_store.at(key);
      if (cache_entry->reveal().timestamp <= t_high && t_low <= cache_entry->reveal().promise) {
          log->info("Key {} is valid in cache", key);

          response.insert(pair<Key, CausalObject>{key, CausalObject{key,cache_entry->reveal().value, cache_entry->reveal().timestamp, cache_entry->reveal().promise}});
        d_low = std::max({d_low, cache_store.at(key)->reveal().timestamp});
      } else {
          log->info("Must request Key {} to storage", key);

          storageReadKeys.insert(key);
        if (t_low >= cache_entry->reveal().promise) {
            log->info("Key {} has possible response with t_low {}", key, t_low);

            possible_response.insert(pair<Key, CausalObject>{key, CausalObject{key,cache_entry->reveal().value, cache_entry->reveal().timestamp, cache_entry->reveal().promise}});
        }
        key_t_low.insert(pair<Key, unsigned long long>{key, cache_entry->reveal().timestamp});

      }
    } else {
          log->info("Requesting key {} to storage", key);
          storageReadKeys.insert(key);
      key_t_low.insert(pair<Key, unsigned long long>{key, std::numeric_limits<unsigned long long>::min()});
    }
  }
  set<Key> erase;
  for (pair<Key, CausalObject> kv : response) {
    if (kv.second.promise >= d_low) {
      d_high = std::min({d_high, kv.second.promise});
    } else {
        log->info("Key {} must be requested to storage", kv.first);
        possible_response.insert(pair<Key, CausalObject>{kv.first, CausalObject{kv.first,kv.second.payload, kv.second.timestamp, kv.second.promise}});
        erase.insert(kv.first);
      storageReadKeys.insert(kv.first);
      key_t_low.insert(pair<Key, unsigned long long>{kv.first, kv.second.timestamp});
    }
  }
    for (Key key: erase) {
        response.erase(key);
    }

  set<Key> erase2;

    set<Key> missing_keys{storageReadKeys};
    /*
    for (Key key: storageReadKeys) {
      log->info("Must request key {} to Anna", key);

      if (pending_key_requests.find(key) != pending_key_requests.end()) {
          log->info("Request for key {} already exists", key);
      auto it = pending_key_requests.at(key).upper_bound(d_high);
      if (it != pending_key_requests.at(key).begin()) {
        it--;
        if (it->first >= d_low) {
            log->info("Updating d_high for key {}", key);

            d_high = std::min({d_high, it->first});
          erase2.insert(key);
        }
      }
    }
  }

  for (Key key: erase2) {
    storageReadKeys.erase(key);
    key_t_low.erase(key);
  }

    */
  if (missing_keys.size() != 0) {
    if(storageReadKeys.size() != 0){
      client->readSliceTx(storageReadKeys, key_t_low, d_high, d_high);
    }

    State * state =new ReadCacheState(missing_keys, storageReadKeys, key_t_low, d_high, d_high, response, possible_response, causalRequest.response_address(),pendingClientResponse, log);
    for(Key key :missing_keys){
      if (pending_key_requests.find(key) == pending_key_requests.end()) {
        pending_key_requests.insert(pair<Key, std::multimap<unsigned long long, State*>>{key,std::multimap<unsigned long long, State*>{}});
      }
        log->info("Missing key {} with promise {}", key, d_high);
        pending_key_requests.at(key).insert(pair<unsigned long long, State*>{d_high,state});
    }
  } else {
      log->info("I have everything to respond, returning");

      CausalResponse clientResponse;
    for (pair<Key, CausalObject> entry : response) {
      CausalTuple *tp = clientResponse.add_tuples();
      tp->set_key(entry.first);
      tp->set_payload(serialize(DataType{TimestampValuePair<string>{entry.second.timestamp, entry.second.promise, entry.second.payload}}));
      tp->set_lattice_type(LatticeType::LWW);

    }
    string resp_string;
    clientResponse.SerializeToString(&resp_string);
    kZmqUtil->send_string(resp_string, &pushers[causalRequest.response_address()]);
  }

}
