#ifndef KEYTOKEY_HPP
#define KEYTOKEY_HPP

#include "bridge.h"
#include "RemapFuncBase.hpp"
#include "FromKeyChecker.hpp"


#include "TimerWrapper.hpp"
			//Haci 2011.01.31(月) KeyOverlaidModifier.hppを真似る。



namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    class KeyToKey {
    public:

      KeyToKey(void);
      ~KeyToKey(void);

      bool remap(RemapParams& remapParams);

      // ----------------------------------------
      // [0] => fromKey_
      // [1] => toKeys_[0]
      // [2] => toKeys_[1]
      // [3] => ...
      void add(unsigned int datatype, unsigned int newval);

      // ----------------------------------------
      // utility functions
      void add(KeyCode newval) { add(BRIDGE_DATATYPE_KEYCODE, newval.get()); }
      void add(Flags newval)   { add(BRIDGE_DATATYPE_FLAGS,   newval.get()); }
      void add(Option newval)  { add(BRIDGE_DATATYPE_OPTION,  newval.get()); }

      bool call_remap_with_VK_PSEUDO_KEY(EventType eventType);

    private:
      size_t index_;
      FromKeyChecker fromkeychecker_;
      PairKeyFlags fromKey_;
      Vector_PairKeyFlags* toKeys_;
      int keyboardRepeatID_;
      bool isRepeatEnabled_;

    };
  }
}

#endif
