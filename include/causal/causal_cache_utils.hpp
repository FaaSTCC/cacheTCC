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

#ifndef INCLUDE_CAUSAL_CAUSAL_CACHE_UTILS_HPP_
#define INCLUDE_CAUSAL_CAUSAL_CACHE_UTILS_HPP_

#include "causal.pb.h"
#include "client/kvs_client.hpp"
#include "cloudburst.pb.h"
#include "causal/causal_object.hpp"


// period to report to the KVS about its key set
const unsigned kCausalCacheReportThreshold = 5;

// period to migrate keys from unmerged store to causal cut store
const unsigned kMigrateThreshold = 10;

// macros used for vector clock comparison
const unsigned kCausalGreaterOrEqual = 0;
const unsigned kCausalLess = 1;
const unsigned kCausalConcurrent = 2;

using DataType = LWWPairLattice<string>;
using StoreType =
    map<Key, std::shared_ptr<DataType>>;
using InPreparationType = map< Key,
    pair<set<Address>,
         map<Key, std::shared_ptr<DataType>>>>;
using VersionStoreType =
map<string,
    map<Key, std::shared_ptr<MultiKeyCausalLattice<SetLattice<string>>>>>;

struct PendingClientMetadata {
  PendingClientMetadata() = default;

  PendingClientMetadata(string client_id, set<Key> read_set,
                        set<Key> to_cover_set)
      : client_id_(std::move(client_id)), read_set_(std::move(read_set)),
        to_cover_set_(std::move(to_cover_set)) {}

  PendingClientMetadata(string client_id, set<Key> read_set,
                        set<Key> to_cover_set,
                        map<Address, map<Key, VectorClock>> prior_causal_chains,
                        set<Key> future_read_set, set<Key> remote_read_set,
                        set<Key> dne_set,
                        map<Key, string> serialized_local_payload,
                        map<Key, string> serialized_remote_payload)
      : client_id_(std::move(client_id)), read_set_(std::move(read_set)),
        to_cover_set_(std::move(to_cover_set)),
        prior_causal_chains_(std::move(prior_causal_chains)),
        future_read_set_(std::move(future_read_set)),
        remote_read_set_(std::move(remote_read_set)),
        dne_set_(std::move(dne_set)),
        serialized_local_payload_(std::move(serialized_local_payload)),
        serialized_remote_payload_(std::move(serialized_remote_payload)) {}

  string client_id_;
  set<Key> read_set_;
  set<Key> to_cover_set_;
  map<Address, map<Key, VectorClock>> prior_causal_chains_;
  set<Key> future_read_set_;
  set<Key> remote_read_set_;
  set<Key> dne_set_;
  map<Key, string> serialized_local_payload_;
  map<Key, string> serialized_remote_payload_;

  bool operator==(const PendingClientMetadata &input) const {
    if (client_id_ == input.client_id_ && read_set_ == input.read_set_ &&
        to_cover_set_ == input.to_cover_set_ &&
        prior_causal_chains_ == input.prior_causal_chains_ &&
        future_read_set_ == input.future_read_set_ &&
        remote_read_set_ == input.remote_read_set_ &&
        dne_set_ == input.dne_set_ &&
        serialized_local_payload_ == input.serialized_local_payload_ &&
        serialized_remote_payload_ == input.serialized_remote_payload_) {
      return true;
    } else {
      return false;
    }
  }
};

struct VectorClockHash {
  std::size_t operator()(const VectorClock &vc) const {
    std::size_t result = std::hash<int>()(-1);
    for (const auto &pair : vc.reveal()) {
      result = result ^ std::hash<string>()(pair.first) ^
               std::hash<unsigned>()(pair.second.reveal());
    }
    return result;
  }
};



void process_response(
    const KeyTuple &tuple,
    StoreType &unmerged_store,
    SocketCache &pushers, KvsClientInterface *client, logger log, map<Key, std::multimap<unsigned long long, State*>> &pending_key_requests, std::queue<pair<Address, map<Key,CausalObject>>> &pendingClientResponse_);

void respond_to_client(
    const KeyTuple &tuple,
    StoreType &unmerged_store,
    SocketCache &pushers, KvsClientInterface *client, logger log, map<Key, std::multimap<unsigned long long, State*>> pending_key_requests);

#endif // FUNCTIONS_CACHE_INCLUDE_CAUSAL_CACHE_UTILS_HPP_
