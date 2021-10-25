//
// Created by Taras Lykhenko on 22/07/2020.
//

#ifndef CAUSALOBJECT_HPP_
#define CAUSALOBJECT_HPP_


#include <types.hpp>

class CausalObject {
public:
    Key key;
    string payload;
    unsigned long long timestamp;
    unsigned long long promise;





  CausalObject(Key key_, string payload_, unsigned long long t_, unsigned long long promise_):key{key_}, payload{payload_}, timestamp{t_}, promise{promise_}{
    }

  bool operator< (const CausalObject &right) const
  {
    return key < right.key;
  }

};
#endif //CAUSALOBJECT_HPP_
