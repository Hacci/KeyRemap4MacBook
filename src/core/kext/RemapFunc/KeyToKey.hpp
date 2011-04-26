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


//Haci
      static void static_initialize(IOWorkLoop& workloop);				// 2011.01.31(月)
      static void static_terminate(void);								// 2011.02.01(火)
      static void static_cancelTimeout(void);							// 2011.02.02(水)
      static int  static_get_pass_initialize(void);						// 2011.03.01(火)
      static void static_set_pass_initialize(int pass_initialize00);	// 2011.03.01(火)
      static void static_setTimeoutMS_init(void);						// 2011.02.28(月)、03.11(金)
      static void static_setTimeoutMS_seesaw_init(void);				// 2011.04.09(土)
      static void static_set_case1_pass_restore(int case1_pass_restore00);	// 2011.03.02(水)
      enum {						// 2011.03.01(火) 
      	INIT_NOT = 1,				// Core.cpp内で作業用のworkspacesなどの初期化をパスする。
      	INIT_DO  = 0,				// 初期化をパスしない。
      	CALLBACK_INIT    = 0,		// 通常のリマップあるいはリマップ無しの場合のタイムアウト後のコールバック関数で次のキー入力で参照する初期化フラグを立てる。
      	CALLBACK_RESTORE = 1,		// VK_JIS_TEMPORARY_RESTOREを無視した場合のタイムアウト後のコールバック関数でRESTOREを実行する。
      	CALLBACK_SEESAW_INIT  = 2,	// 2011.04.09(土) seesawで基本モード(<<英字>>←→ひらがなの英字など)にした場合に、
      								// 				  タイムアウト後のコールバック関数でモードを変更しないためのフラグを立てる。
      };



      KeyToKey(void);
      ~KeyToKey(void);

      bool remap(RemapParams& remapParams);


//Haci
      bool vk_restore(const Params_KeyboardEventCallBack& params, int execute_fire00);		// 2011.03.04(金)



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


//Haci
      static void timeoutAfterKey2(OSObject* owner, IOTimerEventSource* sender);	//2011.01.31(月)
      static TimerWrapperRestore timer_restore2_;	//2011.01.31(月)
      static IOWorkLoop*  workLoop_restore2_;		//2011.02.01(火)
	  static int callback2_;						//2011.02.28(月) CALLBACK_INITなどを指定
	  static int pass_initialize2_;					//2011.03.01(火) INIT_DOかINIT_NOTを指定
	  static int case1_pass_restore2_;				//2011.03.02(水)



    };
  }
}

#endif
