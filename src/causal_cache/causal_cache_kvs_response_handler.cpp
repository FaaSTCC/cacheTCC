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

void kvs_response_handler(
    const KeyResponse &response, StoreType &unmerged_store,
    InPreparationType &in_preparation, StoreType &causal_cut_store,
    VersionStoreType &version_store,
    map<Key, set<Address>> &single_key_callback_map,
    map<Address, PendingClientMetadata> &pending_single_key_metadata,
    map<Address, PendingClientMetadata> &pending_multi_key_metadata,
    map<Key, set<Key>> &to_fetch_map,
    map<Key, std::unordered_map<VectorClock, set<Key>, VectorClockHash>>
        &cover_map,
    SocketCache &pushers, KvsClientInterface *client, logger log,
    map<string, set<Address>> &client_id_to_address_map,
    map<string, Address> &request_id_to_address_map) {


}
