//  Copyright 2021 Taras Lykhenko, Rafael Soares
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
