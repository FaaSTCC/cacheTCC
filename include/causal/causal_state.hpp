//
// Created by Taras Lykhenko on 01/10/2020.
//

#ifndef ANNA_CAUSAL_STATE_HPP
#define ANNA_CAUSAL_STATE_HPP

class CausalObject;

class State {
  /**
   * @var Context
   */
public:
  virtual ~State() {
  }


  virtual void handleCache(CausalObject keyResponse) = 0;

};
#endif //ANNA_CAUSAL_STATE_HPP
