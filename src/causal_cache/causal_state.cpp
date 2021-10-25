//
// Created by Taras Lykhenko on 01/10/2020.
//

class ReadCacheState : public State {
  map<Key,KeyValueEntry> result_;
  map<Key,KeyValueEntry> result_possible_;
  set<Key> keys_;
  Address address_;
  SocketCache pushers_;
public:

  ReadCacheState(set<Key> missing_keys, set<Key> keys, map<Key, unsigned long long> key_t_low, unsigned long long t_high, unsigned long long lastSeen,
                 map<Key,KeyValueEntry> result, map<Key,KeyValueEntry> result_possible,
                 Address address, SocketCache &pushers) :keys_{missing_keys}, result_{result}, result_possible_{result_possible}, address_{address}, pushers_{pushers}{
    context_->getClient()->readSliceTx(keys, key_t_low, t_high, lastSeen);
  }

  void start(){
    KvsClientInterface * client = context_->getClient();


  }

  vector<KeyResponse> handle(KeyResponse response) override{
    return vector<KeyResponse>();



  }

  void handleCache(KeyTuple tuple) override{
      if (keys_.find(tuple.key()) != keys_.end()){
          keys_.erase(tuple.key());
          if(tuple.payload() == ""){
              result_.insert(pair<Key,KeyValueEntry>{tuple.key(),result_possible_.at(tuple.key())});
          }else{
              result_.insert(pair<Key,KeyValueEntry>{tuple.key(),KeyValueEntry(tuple.key(),tuple.payload(), tuple.t_low(), tuple.t_high())});
          }
          if(keys_.size() == 0){
              CausalResponse clientResponse;
              for (pair<Key, KeyValueEntry> entry : result_) {
                  CausalTuple *tp = clientResponse.add_tuples();
                  tp->set_key(entry.first);
                  tp->set_payload(serialize(DataType{entry.second.tvp}));
              }
              string resp_string;
              clientResponse.SerializeToString(&resp_string);
              kZmqUtil->send_string(resp_string, &pushers_[address_]);
              delete this->context_;
          }
      }
  }
};

