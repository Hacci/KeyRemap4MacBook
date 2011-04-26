#include "base.hpp"
#include "ButtonStatus.hpp"
#include "CommonData.hpp"
#include "Config.hpp"
#include "Core.hpp"
#include "RemapClass.hpp"
#include "remap.hpp"
#include "util/CallBackWrapper.hpp"
#include "util/EventInputQueue.hpp"
#include "util/EventOutputQueue.hpp"
#include "util/EventWatcher.hpp"
#include "util/GlobalLock.hpp"
#include "util/KeyboardRepeat.hpp"
#include "util/ListHookedConsumer.hpp"
#include "util/ListHookedKeyboard.hpp"
#include "util/ListHookedPointing.hpp"
#include "util/NumHeldDownKeys.hpp"
#include "util/PressDownKeys.hpp"
#include "util/TimerWrapper.hpp"
#include "RemapFunc/HoldingKeyToKey.hpp"
#include "RemapFunc/KeyOverlaidModifier.hpp"
#include "RemapFunc/PointingRelativeToScroll.hpp"
#include "VirtualKey.hpp"

#include <sys/errno.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOTimerEventSource.h>

namespace org_pqrs_KeyRemap4MacBook {
  namespace Core {
    namespace {
      IOWorkLoop* workLoop = NULL;


//Haci
//------------------------------------------------------------
// (全体の概略)
// 入力モード切替え遅延(MacOSのバグ)は、連打やリピートの遅延が起きることが先ずは問題ですが､
// それ以上に､誤入力が起きることが致命的です。
//
// 入力モード切替え遅延(MacOSのバグ)対策は、大きく2つのファイルに分かれる。
//	  (A) リマップしたものはKeyToKey.cppで処理(VK_JIS_TEMPORARY系を含むケースと含まないケース)
//	  (B) リマップしないものはCore.cppで処理
//	 Core.cppでは､作業用のworkspacedataを更新するかしないかの制御やworkspacedata値の学習などを行う。
//	 InputModeFilter.cpp(フィルター)とVirtualKey.cppで作業用のworkspacedataを使用する。
//	 キーアップからのタイムアウトを見るための専用タイマーおよびコールバック関数は､3つの用途、
//		・作業用のworkspacedataの初期化(次のキー入力時に) ...これが基本で、モード変更キーが押された場合に(押されたかどうかはチェックするまでもなく)、
//														切替遅延があっても､タイムアウト後には正しくなっているはずという前提｡
//	 	・入力モードのリストア							  ...VK_JIS_TEMPORARY_RESTOREを無視した場合
//		・VK_COMMAND_SPACE_SEESAW系で起点(固定)となっているモードに切り替えた後の切替ロック
//	 に共用できる仕組みにしてある。3つはお互いに独立しているので､かぶることはない。

      enum {  // どの段階かを示す値(ControlWorkspaceData関数の引数stage00に与える)
		POST_REMAP       = 0,	//	リマップ前
		JUST_AFTER_REMAP = 1,	//	リマップ直後。但し､リマップされない場合もある｡そのときは、次の､NO_REMAPPEDの段階に進む｡
								//	2011.04.06(水) 追加。
		NON_REMAPPED     = 2,	//	リマップされなかった。
      };						//  2011.04.16(土)

//Haci
//======================================
// 主に､連打遅延対策の一部を制御する。本関数の処理を簡単にまとめると､
//		(C0) workspacedataの学習
//		(C1) タイマーをまずは停止する。連打したときのため。
//		(C2) 作業用のworkspacedataなどを更新・初期化
//			(条件C2-1) モード変更キーあるいは機能キーだが､リマップなし。
//			(条件C2-2) 通常のキーだが、リマップ無/有共通で､初期化指定があったとき。
//					  キー入力からのタイムアウト後なら動作が保証される。
//		(C3) 通常キーでリマップなしの場合は、
//			(C3-1) リストアする(キーダウン時)。
//			(C3-2) タイマーを起動(キーアップ時)。

//		stage00 = POST_REMAP:       リマップ前。KeyToKeyでリマップするキー、全くリマップ指定のないキーの共通処理をする。
//									  ・タイマーを停止。
//				= JUST_AFTER_REMAP: リマップ直後
//									  ・VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系の符号を逆転。
//				= NON_REMAPPED:     全くリマップ指定のないキーの処理をする。
//									  ・タイマーを起動。
//	2010.2.14〜3.5
//	2011.01.16(日)〜04.16(土) ver7.0.37〜7.2.30で、新方式をインプリメント｡
      void
      ControlWorkspaceData(Params_KeyboardEventCallBack& params, int stage00)
      {
	  bool isKeyDown = params.ex_iskeydown;	//10.2.15、2011.01.16(日)
	  bool keyisCCOS;		//10.2.18 =1: キーとしてCCOSが押されている。
	  bool isonCCO;			//10.2.15 =1: モディファイアーとしてCCO(Command,Control,Option)のどれかが押されている。
	  bool isonS;			//10.2.16 =1: モディファイアーとしてS(Shift)が押されている。
	  bool is_eisuu_kana_kanashift;	//10.2.15 モード変更のための英数キー、かなキーが押されている。
	  bool isonCCOSonly;	//10.2.16 =1: モディファイアーとしてCCOS(Command,Control,Option,Shift)のみが押されていて、文字キーは押されていない。
	  int pass_initialize00;	//2011.03.01(火)
	  int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);	
	  			//2011.02.05(土) checkbox.xmlの「General --- Ignore VK_JIS_TEMPORARY_RESTORE」の設定値
	  int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
				// 2011.03.07(月) 「モード変更キー押下時の遅延対策」を行うかどうかのチェックボックス。効果を確かめるために分けてあるが､通常はオン。
///	  int restore_after_timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_restore_after_timeout);
				// 2011.03.08(火) 「タイムアウト後のリストア」を行うかどうかのチェックボックス。
	  bool result00;	//2011.04.16(土)

	  if(!ignore_vk_restore && !learn_workspacedata) return;	//2011.02.05(土)、03.08(火)
			// restore_after_timeoutだけがオンの場合は無意味｡

	  keyisCCOS = params.key == KeyCode::COMMAND_L || params.key == KeyCode::COMMAND_R ||
	  			  params.key == KeyCode::CONTROL_L || params.key == KeyCode::CONTROL_R ||
	  			  params.key == KeyCode::OPTION_L  || params.key == KeyCode::OPTION_R  ||
	  			  params.key == KeyCode::SHIFT_L   || params.key == KeyCode::SHIFT_R;		//10.2.18

	  isonCCO = FlagStatus::makeFlags().isOn(ModifierFlag::COMMAND_L) || params.flags == ModifierFlag::COMMAND_L ||
	  		    FlagStatus::makeFlags().isOn(ModifierFlag::COMMAND_R) || params.flags == ModifierFlag::COMMAND_R ||
	  		    FlagStatus::makeFlags().isOn(ModifierFlag::CONTROL_L) || params.flags == ModifierFlag::CONTROL_L ||
	  		    FlagStatus::makeFlags().isOn(ModifierFlag::CONTROL_R) || params.flags == ModifierFlag::CONTROL_R ||
  	  		    FlagStatus::makeFlags().isOn(ModifierFlag::OPTION_L)  || params.flags == ModifierFlag::OPTION_L  ||
	  		    FlagStatus::makeFlags().isOn(ModifierFlag::OPTION_R)  || params.flags == ModifierFlag::OPTION_R;

	  isonS = FlagStatus::makeFlags().isOn(ModifierFlag::SHIFT_L) || params.flags == ModifierFlag::SHIFT_L ||
        	  FlagStatus::makeFlags().isOn(ModifierFlag::SHIFT_R) || params.flags == ModifierFlag::SHIFT_R;
        						  //10.2.16
	  is_eisuu_kana_kanashift =  params.key == KeyCode::JIS_EISUU && !isonCCO && !isonS ||
	  							 params.key == KeyCode::JIS_EISUU && !isonS ||
	  							 params.key == KeyCode::JIS_KANA  && !isonCCO && !isonS ||
            					 params.key == KeyCode::JIS_KANA  && isonS;
        						//10.2.15 英数キー

	  isonCCOSonly = (isonS || isonCCO) && !keyisCCOS;		// 10.2.16 モードキーがCCOSのとき
	  														// 10.2.18 モード変更直後にシフトケースのキーを押すと誤入力にならないようにするため、
	  														//		   文字キーがCCOSでないときという条件を追加。
	  														// VK_JIS_TEMPORARY系でモード変更して戻していなければNONE以外の値になっている。

	  int isPPP = (is_eisuu_kana_kanashift || isonCCOSonly);	//2011.02.13(日) モード変更キーあるいは機能キー
	  KeyCode key00  = params.key;
	  Flags   flag00 = params.flags;

	  // VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系の符号を逆転する。
	  //	2011.04.06(水)
	  if(isKeyDown){	//キーダウン時
		if(stage00 == POST_REMAP){	//リマップ前
			CommonData::reverse_sign_REPLACE_PLUS_MINUS(0);	// VK_COMMAND_SPACEのリプレース用のカウンター値を記憶。
		} else if(stage00 == JUST_AFTER_REMAP){	// リマップ直後
			// VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系の仮想キーが連続していなければ、符号を逆転｡
			// 例えば､Shift+かなに割り当てている場合は､Shiftキーをキーアップした時も含む｡
			// カウンターが進んでいなければ、VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系が実行されなかったと判断する｡
			CommonData::reverse_sign_REPLACE_PLUS_MINUS(1);
			return;
		}
	  }

// キーアップ時 -----------------
	  if (!isKeyDown) {
		if(!isPPP && !isonCCO && stage00 == POST_REMAP){
			//(C3-2) キーアップしたとき、初期化指定としてタイマーを起動
			//2011.02.28(月)
			//2011.03.11(金) 常にタイマーを起動｡VK_RESTOREを含んだ場合は､KeyToKey側で起動し直される。
			//2011.04.15(金) Command+Spaceのように、最後にCommandキーをアップしたような場合も除くため、!isonCCOを追加｡
			RemapFunc::KeyToKey::static_setTimeoutMS_init();	//初期化指定のタイマーを起動｡
		}

		return;	//キーアップ時は、これ以外は何もしない｡
	  }

// 以下はすべてキーダウン時 -----------------------------
	  //(C1) RESTORE用のタイマーをまずは停止する。
	  // 	 タイマーが起動していなければ、何も実行されないだけ｡
	  if(stage00 == POST_REMAP){
	  	// RESTOREタイマーを停止
		  RemapFunc::KeyToKey::static_cancelTimeout();	//10.2.28
	  }

	  if(stage00 == POST_REMAP && learn_workspacedata){
		//(C0) 英字・ひらがな・カタカナ・アイヌのworkspacedataをそれぞれ学習(保存)する。
	  	// 2011.03.05(土)、07(月)、10(木)
		CommonData::learn_WSD();
	  }

	  pass_initialize00 = RemapFunc::KeyToKey::static_get_pass_initialize();
	  		//2011.03.01(火) 初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を得る
			//2011.03.14(月) learn_WSDの中でこの値を変更するようにして、最初から正しくするようにしたので､ここに移動。

	  // 作業用workspacedataの更新条件
	  bool conC2_1 = isPPP  && stage00 == NON_REMAPPED;
	  		//(条件C2-1) モード変更キーあるいは機能キーだが､リマップなし。
	  bool conC2_2 = !isPPP && stage00 == POST_REMAP && pass_initialize00 == RemapFunc::KeyToKey::INIT_DO;		//2011.03.01(火)
	  		//(条件C2-2) 通常のキーだが、リマップ無/有共通で､初期化指定があるとき。
	  		//		 キー入力からのタイムアウト後なら、動作が保証される。
	  bool conC3   = !isPPP && stage00 == NON_REMAPPED;
	  		//(条件C3)   通常キーで、リマップ無し

	  if(!learn_workspacedata){ // 学習のチェックボックスがオフの場合
	    if (conC2_1 || conC2_2){
			//(C2) 作業用のWSD(workspacedata)とSavedIMDの初期化
			//	2011.02.18(金)〜10(木)
			CommonData::update_WSD();	// 作業用のworkspacedataを更新
			RemapFunc::KeyToKey::static_set_pass_initialize(conC2_1 ? RemapFunc::KeyToKey::INIT_DO : RemapFunc::KeyToKey::INIT_NOT);
						//条件1(conC2_1)なら次のキー入力時に､初期化する指定、条件2(conC2_2)なら初期化しない指定を設定する。
			Handle_VK_JIS_TEMPORARY::resetSavedIMD();	//2011.02.18(金) VK_JIS_TEMPORARY系の保存値を初期化。
	    }
	  } else { // 学習がオンの場合
	    if (conC2_1){ // リマップ無しの素のモード変更キー
			//(C2) 作業用のWSD(workspacedata)とSavedIMDの初期化
			//	2011.03.05(土)
			result00 = CommonData::replace_WSD(key00, flag00);		// 作業用のWSDをすり替える｡
			if(result00){
			  RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);	// 次のキー入力時には初期化しないようにする。
			  Handle_VK_JIS_TEMPORARY::resetSavedIMD();	//2011.02.18(金) VK_JIS_TEMPORARY系の保存値を初期化。
			} else {//workspacedataの学習をしない設定の場合あるいは未学習の場合は､作業用のworkspacedataを更新する必要がある｡
			  //2011.04.16(土)am11:13
			  RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_DO);	// 次のキー入力時には初期化する。
			}
	    }else if (conC2_2){
			//(C2) 作業用のWSD(workspacedata)の更新とSavedIMDの初期化
			//	2011.02.18(金)〜03.10(木)
			CommonData::update_WSD();	// WSDの更新
			RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);	// 次のキー入力時は初期化しないようにする。
			Handle_VK_JIS_TEMPORARY::resetSavedIMD();	// VK_JIS_TEMPORARY系の保存値を初期化。
	    }
	  }
	  if(conC3){
			//(C3-1) リマップなしの場合で、モード変更キーや機能キーでなければモードを戻す。
			// 戻すべき値があるかどうかは調べない｡連打の時はそういう条件が追いつかないので逆効果になるから。
			// 例えば､リマップしていないキーボードでCommand_Lをキーダウンしただけで､ここに入って、モードが戻る。
			// 2011.03.01(火)
			EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
			RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);
				//  元のモードに戻すだけなので､次のキー入力時に作業用workspacesなどをそのまま使用すればいいので、初期化をしない指定とする。
	  }
		return;
      }	//ControlWorkspaceDataの終わり
// Haci 追加関数終わり========================================================================



    }

    void
    start(void)
    {
      GlobalLock::initialize();
      CommonData::initialize();
      EventWatcher::initialize();
      PressDownKeys::initialize();
      FlagStatus::initialize();
      ButtonStatus::initialize();

      ListHookedKeyboard::instance().initialize();
      ListHookedConsumer::instance().initialize();
      ListHookedPointing::instance().initialize();

      workLoop = IOWorkLoop::workLoop();
      if (! workLoop) {
        IOLOG_ERROR("IOWorkLoop::workLoop failed\n");
      } else {
        KeyboardRepeat::initialize(*workLoop);
        EventInputQueue::initialize(*workLoop);
        VirtualKey::initialize(*workLoop);
        EventOutputQueue::initialize(*workLoop);
        RemapFunc::HoldingKeyToKey::static_initialize(*workLoop);
        RemapFunc::KeyOverlaidModifier::static_initialize(*workLoop);


//Haci
        RemapFunc::KeyToKey::static_initialize(*workLoop);	//2011.02.01(火)〜10(木)



        RemapFunc::PointingRelativeToScroll::static_initialize(*workLoop);
        ListHookedKeyboard::static_initialize(*workLoop);
        RemapClassManager::initialize(*workLoop);
      }

      Config::sysctl_register();
    }

    void
    stop(void)
    {
      // Destroy global lock.
      // Then, all callbacks and hooked functions become inactive.
      GlobalLock::terminate();

      // ------------------------------------------------------------
      ListHookedKeyboard::instance().terminate();
      ListHookedConsumer::instance().terminate();
      ListHookedPointing::instance().terminate();

      // ------------------------------------------------------------
      // call terminate
      Config::sysctl_unregister();

      RemapClassManager::terminate();
      KeyboardRepeat::terminate();
      EventInputQueue::terminate();
      VirtualKey::terminate();
      EventOutputQueue::terminate();
      RemapFunc::HoldingKeyToKey::static_terminate();
      RemapFunc::KeyOverlaidModifier::static_terminate();


//Haci
        RemapFunc::KeyToKey::static_terminate();	// 2011.02.01(火)〜09(水) これがないとmake reloadで時々解放エラーになる。



      RemapFunc::PointingRelativeToScroll::static_terminate();
      ListHookedKeyboard::static_terminate();

      if (workLoop) {
        workLoop->release();
        workLoop = NULL;
      }

      EventWatcher::terminate();
      PressDownKeys::terminate();

      CommonData::terminate();
    }

    // ======================================================================
    bool
    IOHIKeyboard_gIOMatchedNotification_callback(void* target, void* refCon, IOService* newService, IONotifier* notifier)
    {
      GlobalLock::ScopedLock lk;
      if (! lk) return false;

      IOLOG_DEBUG("%s newService:%p\n", __FUNCTION__, newService);

      IOHIDevice* device = OSDynamicCast(IOHIKeyboard, newService);
      if (! device) return false;

      ListHookedKeyboard::instance().push_back(new ListHookedKeyboard::Item(device));
      ListHookedConsumer::instance().push_back(new ListHookedConsumer::Item(device));
      return true;
    }

    bool
    IOHIKeyboard_gIOTerminatedNotification_callback(void* target, void* refCon, IOService* newService, IONotifier* notifier)
    {
      GlobalLock::ScopedLock lk;
      if (! lk) return false;

      IOLOG_DEBUG("%s newService:%p\n", __FUNCTION__, newService);

      IOHIDevice* device = OSDynamicCast(IOHIKeyboard, newService);
      if (! device) return false;

      ListHookedKeyboard::instance().erase(device);
      ListHookedConsumer::instance().erase(device);
      return true;
    }

    bool
    IOHIPointing_gIOMatchedNotification_callback(void* target, void* refCon, IOService* newService, IONotifier* notifier)
    {
      GlobalLock::ScopedLock lk;
      if (! lk) return false;

      IOLOG_DEBUG("%s newService:%p\n", __FUNCTION__, newService);

      IOHIDevice* device = OSDynamicCast(IOHIPointing, newService);
      if (! device) return false;

      ListHookedPointing::instance().push_back(new ListHookedPointing::Item(device));
      return true;
    }

    bool
    IOHIPointing_gIOTerminatedNotification_callback(void* target, void* refCon, IOService* newService, IONotifier* notifier)
    {
      GlobalLock::ScopedLock lk;
      if (! lk) return false;

      IOLOG_DEBUG("%s newService:%p\n", __FUNCTION__, newService);

      IOHIDevice* device = OSDynamicCast(IOHIPointing, newService);
      if (! device) return false;

      ListHookedPointing::instance().erase(device);
      return true;
    }

    // ======================================================================
    void
    remap_KeyboardEventCallback(Params_KeyboardEventCallBack& params)
    {
      params.log();


//Haci
	  //--------------------------------------------------------------
	  // 連打遅延対策の処理(リマップするキー､しないキー共通)
	  //	10.2.15〜16 関数化, 2011.01.13(木)〜02.05(土)
	  //**************************************************************
	  ControlWorkspaceData(params, POST_REMAP);



      // ------------------------------------------------------------
      RemapParams remapParams(params);

      // ------------------------------------------------------------
      FlagStatus::set(params.key, params.flags);

      RemapClassManager::remap_key(remapParams);


//Haci
	  //--------------------------------------------------------------
	  // リマップ直後
	  //	2011.04.06(水)
	  //**************************************************************
	  ControlWorkspaceData(params, JUST_AFTER_REMAP);



      // ------------------------------------------------------------
      if (! remapParams.isremapped) {
        Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType, FlagStatus::makeFlags(), params.key,
                                                                                       params.charCode, params.charSet, params.origCharCode, params.origCharSet,
                                                                                       params.keyboardType, false));


//Haci
		// 全くリマップ指定のないキーはここに来る。
	    ControlWorkspaceData(params, NON_REMAPPED);
			// 全くリマップ指定のないキーの場合は、
			//  ・キーダウン時にモードを戻し(1つ前のキーがVK系のときのため)、
			//  ・モード変更キーが押されたのであれば､キーアップ時に作業用のworkspacedataの更新を行う｡
			// 2010.2.16〜27,2011.01.20(木)



        if (ptr) {
          KeyboardRepeat::set(*ptr);
          EventOutputQueue::FireKey::fire(*ptr);
        }
      }

      if (NumHeldDownKeys::iszero()) {
        NumHeldDownKeys::reset();
        KeyboardRepeat::cancel();
        EventWatcher::reset();
        FlagStatus::reset();
        ButtonStatus::reset();
        VirtualKey::reset();
        EventOutputQueue::FireModifiers::fire(FlagStatus::makeFlags());
        EventOutputQueue::FireRelativePointer::fire();
        PressDownKeys::clear();
      }

      RemapFunc::PointingRelativeToScroll::cancelScroll();
    }

    void
    remap_KeyboardSpecialEventCallback(Params_KeyboardSpecialEventCallback& params)
    {
      params.log();

      RemapConsumerParams remapParams(params);

      // ------------------------------------------------------------
      RemapClassManager::remap_consumer(remapParams);

      // ----------------------------------------
      if (! remapParams.isremapped) {
        Params_KeyboardSpecialEventCallback::auto_ptr ptr(Params_KeyboardSpecialEventCallback::alloc(params.eventType, FlagStatus::makeFlags(), params.key,
                                                                                                     params.flavor, params.guid, false));
        if (ptr) {
          KeyboardRepeat::set(*ptr);
          EventOutputQueue::FireConsumer::fire(*ptr);
        }
      }

      RemapFunc::PointingRelativeToScroll::cancelScroll();
    }

    void
    remap_RelativePointerEventCallback(Params_RelativePointerEventCallback& params)
    {
      params.log();

      RemapPointingParams_relative remapParams(params);

      ButtonStatus::set(params.ex_button, params.ex_isbuttondown);

      RemapClassManager::remap_pointing(remapParams);

      // ------------------------------------------------------------
      if (! remapParams.isremapped) {
        EventOutputQueue::FireRelativePointer::fire(ButtonStatus::makeButtons(), params.dx, params.dy);
      }
    }

    void
    remap_ScrollWheelEventCallback(Params_ScrollWheelEventCallback& params)
    {
      params.log();
      EventOutputQueue::FireScrollWheel::fire(params);

      RemapFunc::PointingRelativeToScroll::cancelScroll();
    }
  }
}
