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
#include "causal/causal_cache_utils.hpp"

void put_request_handler(const string &serialized, StoreType &unmerged_store,
                         KvsClientInterface *client, logger log,SocketCache &pushers) {
  CausalRequest request;
  request.ParseFromString(serialized);
  map<Key, string> key_map;
  Address responseAddress;
  string resp_string = "";

  if(request.consistency() == ConsistencyType::NORMAL){
    KeyRequest keyRequest;
    keyRequest.ParseFromString(serialized);
    responseAddress = keyRequest.response_address();
    client->put_async(keyRequest.tuples()[0].key(), keyRequest.tuples()[0].payload(),keyRequest.tuples()[0].lattice_type());

  }else {
    for (CausalTuple tuple : request.tuples()) {
      Key key = tuple.key();
      key_map.insert(pair<Key, string>{key, tuple.payload()});
      log->info("Putting key {} to Anna", key);
    }
    client->writeTx(key_map);
    responseAddress = request.response_address();
  }

  kZmqUtil->send_string(resp_string, &pushers[responseAddress]);

}
