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

#include "causal/causal_cache_utils.hpp"

void key_version_request_handler(const string &serialized,
                                 VersionStoreType &version_store,
                                 SocketCache &pushers, logger log,
                                 ZmqUtilInterface *kZmqUtil) {
  KeyVersionRequest request;
  request.ParseFromString(serialized);

  KeyVersionResponse response;
  response.set_id(request.id());
  if (version_store.find(request.id()) != version_store.end()) {
    for (const auto &key : request.keys()) {
      if (version_store[request.id()].find(key) ==
          version_store[request.id()].end()) {
        log->error(
            "Requested key {} for client ID {} not available in versioned "
            "store.",
            key, request.id());
      } else {
        CausalTuple *tp = response.add_tuples();
        tp->set_key(key);
        tp->set_payload(serialize(*(version_store[request.id()][key])));
      }
    }
  } else {
    log->error("Client ID {} not available in versioned store.", request.id());
  }
  // send response
  string resp_string;
  response.SerializeToString(&resp_string);
  kZmqUtil->send_string(resp_string, &pushers[request.response_address()]);
}

void key_version_response_handler(
    const string &serialized, StoreType &causal_cut_store,
    VersionStoreType &version_store,
    map<Address, PendingClientMetadata> &pending_multi_key_metadata,
    map<string, set<Address>> &client_id_to_address_map, SocketCache &pushers,
    ZmqUtilInterface *kZmqUtil, logger log) {



}
