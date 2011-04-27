#include "base.hpp"
#include "CommonData.hpp"
#include "Config.hpp"
#include "EventOutputQueue.hpp"
#include "FlagStatus.hpp"
#include "RemapClass.hpp"
#include "UserClient_kext.hpp"
#include "VirtualKey.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  void
  VirtualKey::initialize(IOWorkLoop& workloop)
  {
    Handle_VK_MOUSEKEY::initialize(workloop);
    Handle_VK_JIS_TEMPORARY::initialize(workloop);

    Handle_VK_CONFIG::initialize();
  }

  void
  VirtualKey::terminate(void)
  {
    Handle_VK_MOUSEKEY::terminate();
    Handle_VK_JIS_TEMPORARY::terminate();

    Handle_VK_CONFIG::terminate();
  }

  void
  VirtualKey::reset(void)
  {
    Handle_VK_MOUSEKEY::reset();
  }

  bool
  VirtualKey::isKeyLikeModifier(KeyCode keycode)
  {
    if (Handle_VK_CONFIG::is_VK_CONFIG_SYNC_KEYDOWNUP(keycode)) return true;
    if (Handle_VK_LAZY::getModifierFlag(keycode) != ModifierFlag::NONE) return true;
    if (Handle_VK_MOUSEKEY::is_VK_MOUSEKEY(keycode)) return true;
    return false;
  }


//Haci(KeyToKey)
    TimerWrapperRestore VirtualKey::timer_restore2_;		// 2011.01.31(月)
    IOWorkLoop*  VirtualKey::workLoop_restore2_ = NULL;	// 2011.02.01(火)
    int VirtualKey::callback2_ = CALLBACK_INIT;			// 2011.02.27(日)
    int VirtualKey::pass_initialize2_ = INIT_NOT;			// 2011.03.01(火)インストールした時にその時点でのモードを活かすために最初のキー入力時はパスするため。
	int VirtualKey::case1_pass_restore2_ = 0;				// 2011.03.02(水)

//Haci
//KeyRemap4MacBook_bridge::GetWorkspaceData::Reply VirtualKey::wsd_public_;
//KeyRemap4MacBook_bridge::GetWorkspaceData::Reply VirtualKey::wsd_save_[7];
  BridgeWorkSpaceData VirtualKey::wsd_public_; 		//2011.01.13(木) 作業用のworkspacedata
  BridgeWorkSpaceData VirtualKey::wsd_save_[7];		//2011.03.05(土) workspacedataの学習用
  int VirtualKey::pre_index2_ = -1;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
  int VirtualKey::cur_index2_ = -1;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
  int VirtualKey::others_index2_ = -1;				//2011.04.05(火) VK_JIS_COMMAND_SPACE用
  int VirtualKey::sign_plus_minus2_ = -99;			//2011.04.06(水) REPLACE_PLUS_MINUS用の符号
  int VirtualKey::counter_plus_minus2_ = 0;			//2011.04.06(水) REPLACE_PLUS_MINUS用の連続実行をカウントする。
  int VirtualKey::pre_counter_plus_minus2_ = 0;		//2011.04.06(水) REPLACE_PLUS_MINUS用のリマップ前の上記カウンター値
  bool VirtualKey::seesaw_init2_ = false;			//2011.04.09(土) = 1:タイムアウト後にseesawの起点にする。

//Haci(KeyToKey)
	//=======================================
	// この関数はCore.cppのstart()関数で使用するためのもの
	//2011.01.31(月)〜02.10(木)
    void
    VirtualKey::static_initialize(IOWorkLoop& workloop)
    {
      timer_restore2_.initialize(&workloop, NULL, VirtualKey::timeoutAfterKey2);
      workLoop_restore2_ = &workloop;
    }

//Haci(KeyToKey)
	//=======================================
	// この関数はCore.cppのstop()関数で使用するためのもの
	//2011.02.01(火)
    void
    VirtualKey::static_terminate(void)
    {
      timer_restore2_.terminate();
    }

//Haci(KeyToKey)
	//=======================================
	// 外部からこのタイマーを停止するには、VirtualKey::static_cancelTimeout()を実行する。
	//2011.02.02(水)〜02.28(月)
    void
    VirtualKey::static_cancelTimeout(void)
    {
      timer_restore2_.cancelTimeout();	// タイマーを停止
    }

//Haci(KeyToKey)
	//=======================================
	// Core.cppで初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を得る｡
	// VirtualKey::static_get_pass_initialize()を実行する。
	//2011.03.01(火)
    int
    VirtualKey::static_get_pass_initialize(void)
    {
      return pass_initialize2_;
    }


//Haci(KeyToKey)
	//=======================================
	// Core.cppで初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を変更
	// VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT or INIT_DO)を実行する。
	//2011.03.01(火)
    void
    VirtualKey::static_set_pass_initialize(int pass_initialize00)
    {
	  if(pass_initialize00 != INIT_NOT && pass_initialize00 != INIT_DO) return;
      pass_initialize2_ = pass_initialize00;
    }

//Haci(KeyToKey)
	//=======================================
	// タイムアウト後のコールバック関数内での実行の種類を指定して､タイマーを起動する。
	// 実行するには、VirtualKey::static_setTimeoutMS()を実行する。
	//2011.04.09,27 統合
    void
    VirtualKey::static_setTimeoutMS(int callback00)
    {
	  int vk_restore_timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_parameter_keyinterval_timeout);
	  			// キー入力からタイムアウトするまでの時間(msec)
		if(workLoop_restore2_){
			callback2_  = callback00;	// コールバック関数内での実行の種類を指定
			timer_restore2_.setTimeoutMS(vk_restore_timeout);
		}
    }

//Haci(KeyToKey)
	//=======================================
	// KeyToKey(Case1)でリストアを実行するか、しないかを決める。
	// 今のところKeyOverlaidModifierが実行された時に､最初にKeyToKey(Case1)に入るが､その時はリストアはしないように制御するために使用する。
	// VirtualKey::static_set_case1_pass_restore(1 or 0)を実行する。
	// 		case1_pass_restore00 = 1: KeyToKey(Case1)でリストアを実行しないようにする。
	//							 = 0: KeyToKey(Case1)でリストアを実行するように戻す。
	//2011.03.02(水)〜03(木)
    void
    VirtualKey::static_set_case1_pass_restore(int case1_pass_restore00)
    {
	  int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);
		// VK_JIS_TEMPORARY_RESTOREを無視するチェックボックスのオンオフ値
	  if(!ignore_vk_restore || case1_pass_restore00 != 0 && case1_pass_restore00 != 1) return;
      case1_pass_restore2_ = case1_pass_restore00;
    }


//Haci(KeyToKey)
	//-----------------------------------------------------------------------
	// キー入力する度に起動する専用タイマーに登録するcallbackルーチン
	// 別の2つのタイマー用も兼ねる｡タイマー自体は共通｡
	//	2010.2.24〜3.5, 2011.01.22(土)〜04.09(土)
      void
      VirtualKey::timeoutAfterKey2(OSObject* owner, IOTimerEventSource* sender)
      {
///	IOLockWrapper::ScopedLock lk(timer_restore2_.getlock());
/// 2010.3.5、2011.02.13 KeyOverlaidModifier.cppを真似て、remap関数の先頭でも対で実行してみたが、やはり、すぐにロック状態で、Macを再起動。やはり駄目。なくても特に問題は無い｡
		if(callback2_ == CALLBACK_INIT){
			// VK_JIS_TEMPORARY系を含まないリマップあるいはリマップなしの場合は､初期化モードにする。
			// このメカニズムによって､認識できないモード変更キーのようなケースでも､タイムアウト後なら保証できる。
			// つまり、キーアップから例えば1000ms後まで次のキー入力がなければ､このルーチンが自動的に実行される｡
			// タイムアウト前に次のキーが押されると(つまり、連打)､Core.cppでこのタイマーは停止するようにしてある。
			pass_initialize2_ = INIT_DO;	//次のキー入力時に､Core.cppで作業用のworkspaces値などを必ず初期化させるため。
		} else if(callback2_ == CALLBACK_RESTORE && 
				  Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_restore_after_timeout)){
			// VK_JIS_TEMPORARY_RESTOREを無視した時にこのタイマーを起動した場合は、ここでリストアする｡これは見た目の問題｡
			// ただし、タイムアウト後のリストア実行用のチェックボックスがオンの場合のみ。
			// 2011.03.08(火)
			EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, CommonData::getcurrent_keyboardType()); //モードを元に戻すだけ｡
			pass_initialize2_ = INIT_NOT;	// 元に戻るだけなので､次のキー入力時に､Core.cppで作業用のworkspaces値などを初期化させないため。
		} else if(callback2_ == CALLBACK_SEESAW_INIT){
			// トグル系の最大の欠点の防止策
			// VK_JIS_COMMAND_SPACE_SEESAW系のキーで、起点(固定)の入力モードに切り替えた時にこのタイマーが起動される｡
			// タイムアウト内に押せばトグルし(2度押しと似ている)､タイムアウト後なら起点のモードのままにする。
			// そのために、ここで、フラグを初期化する｡
			// 2011.04.09(土)
			VirtualKey::init_seesaw();
		}
		
      }

//Haci(KeyToKey)
	//------------------------------------------------------------------------------
	// VK_JIS_TEMPORARY_RESTOREのキーダウン/アップを汎用化
	// 外部から起動する場合は､keytokey_.vk_restore(remapParams); のように記述する。
	// 内部から起動する場合は､vk_restore(remapParams); のように記述すればいい。
	// 外部からは、今のところ、KeyOverlaidModifierで、fromキーを押し続けてそのまま離した時に､モードを戻すために使用している。
	//	execute_fire00 = 1: fireコマンドも実行する。
	//				   = 0: キューに入れるだけ。
	//	2011.03.04(金)
    bool
    VirtualKey::vk_restore(const Params_KeyboardEventCallBack& params, int execute_fire00)
    {
      if (&params) {
      	EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
	  	if(execute_fire00){
       		EventOutputQueue::FireKey::fire(params);
		}
		return true;
      }
      return false;
    }

//Haci(KeyToKey) ----------------ここまで全て


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
//	2011.04.27(水) Core.cppからここに移動。
      void
      VirtualKey::ControlWorkspaceData(Params_KeyboardEventCallBack& params, int stage00)
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
			VirtualKey::reverse_sign_REPLACE_PLUS_MINUS(0);	// VK_COMMAND_SPACEのリプレース用のカウンター値を記憶。
		} else if(stage00 == JUST_AFTER_REMAP){	// リマップ直後
			// VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系の仮想キーが連続していなければ、符号を逆転｡
			// 例えば､Shift+かなに割り当てている場合は､Shiftキーをキーアップした時も含む｡
			// カウンターが進んでいなければ、VK_COMMAND_SPACE_REPLACE_PLUS_MINUS系が実行されなかったと判断する｡
			VirtualKey::reverse_sign_REPLACE_PLUS_MINUS(1);
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
			static_setTimeoutMS(CALLBACK_INIT);	//初期化指定のタイマーを起動｡
		}

		return;	//キーアップ時は、これ以外は何もしない｡
	  }

// 以下はすべてキーダウン時 -----------------------------
	  //(C1) RESTORE用のタイマーをまずは停止する。
	  // 	 タイマーが起動していなければ、何も実行されないだけ｡
	  if(stage00 == POST_REMAP){
	  	// RESTOREタイマーを停止
		  VirtualKey::static_cancelTimeout();	//10.2.28
	  }

	  if(stage00 == POST_REMAP && learn_workspacedata){
		//(C0) 英字・ひらがな・カタカナ・アイヌのworkspacedataをそれぞれ学習(保存)する。
	  	// 2011.03.05(土)、07(月)、10(木)
		VirtualKey::learn_WSD();
	  }

	  pass_initialize00 = VirtualKey::static_get_pass_initialize();
	  		//2011.03.01(火) 初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を得る
			//2011.03.14(月) learn_WSDの中でこの値を変更するようにして、最初から正しくするようにしたので､ここに移動。

	  // 作業用workspacedataの更新条件
	  bool conC2_1 = isPPP  && stage00 == NON_REMAPPED;
	  		//(条件C2-1) モード変更キーあるいは機能キーだが､リマップなし。
	  bool conC2_2 = !isPPP && stage00 == POST_REMAP && pass_initialize00 == VirtualKey::INIT_DO;		//2011.03.01(火)
	  		//(条件C2-2) 通常のキーだが、リマップ無/有共通で､初期化指定があるとき。
	  		//		 キー入力からのタイムアウト後なら、動作が保証される。
	  bool conC3   = !isPPP && stage00 == NON_REMAPPED;
	  		//(条件C3)   通常キーで、リマップ無し

	  if(!learn_workspacedata){ // 学習のチェックボックスがオフの場合
	    if (conC2_1 || conC2_2){
			//(C2) 作業用のWSD(workspacedata)とSavedIMDの初期化
			//	2011.02.18(金)〜10(木)
			VirtualKey::update_WSD();	// 作業用のworkspacedataを更新
			VirtualKey::static_set_pass_initialize(conC2_1 ? VirtualKey::INIT_DO : VirtualKey::INIT_NOT);
						//条件1(conC2_1)なら次のキー入力時に､初期化する指定、条件2(conC2_2)なら初期化しない指定を設定する。
			Handle_VK_JIS_TEMPORARY::resetSavedIMD();	//2011.02.18(金) VK_JIS_TEMPORARY系の保存値を初期化。
	    }
	  } else { // 学習がオンの場合
	    if (conC2_1){ // リマップ無しの素のモード変更キー
			//(C2) 作業用のWSD(workspacedata)とSavedIMDの初期化
			//	2011.03.05(土)
			result00 = VirtualKey::replace_WSD(key00, flag00);		// 作業用のWSDをすり替える｡
			if(result00){
			  VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);	// 次のキー入力時には初期化しないようにする。
			  Handle_VK_JIS_TEMPORARY::resetSavedIMD();	//2011.02.18(金) VK_JIS_TEMPORARY系の保存値を初期化。
			} else {//workspacedataの学習をしない設定の場合あるいは未学習の場合は､作業用のworkspacedataを更新する必要がある｡
			  //2011.04.16(土)am11:13
			  VirtualKey::static_set_pass_initialize(VirtualKey::INIT_DO);	// 次のキー入力時には初期化する。
			}
	    }else if (conC2_2){
			//(C2) 作業用のWSD(workspacedata)の更新とSavedIMDの初期化
			//	2011.02.18(金)〜03.10(木)
			VirtualKey::update_WSD();	// WSDの更新
			VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);	// 次のキー入力時は初期化しないようにする。
			Handle_VK_JIS_TEMPORARY::resetSavedIMD();	// VK_JIS_TEMPORARY系の保存値を初期化。
	    }
	  }
	  if(conC3){
			//(C3-1) リマップなしの場合で、モード変更キーや機能キーでなければモードを戻す。
			// 戻すべき値があるかどうかは調べない｡連打の時はそういう条件が追いつかないので逆効果になるから。
			// 例えば､リマップしていないキーボードでCommand_Lをキーダウンしただけで､ここに入って、モードが戻る。
			// 2011.03.01(火)
			EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
			VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);
				//  元のモードに戻すだけなので､次のキー入力時に作業用workspacesなどをそのまま使用すればいいので、初期化をしない指定とする。
	  }
		return;
      }	//ControlWorkspaceDataの終わり


//Haci
  //--------------------------------------------------------
  // Core.cppでリマップ前後に実行して、
  // REPLACE_PLUS_MINUS_SKIP系が実行した時のカウンターがカウントアップしていれば何もせず､
  // カウントアップしていなければ､符号を逆転する。
  // when00 = 0: リマップ前
  //		= 1: リマップ直後(リマップしなかった場合も含む)
  //		= 9: REPLACE_PLUS_MINUS_SIP系のキーダウン時
  // 2011.04.06(水)
  void
  VirtualKey::reverse_sign_REPLACE_PLUS_MINUS(int when00)
  {
    if(when00 == 0){			// リマップ前
		pre_counter_plus_minus2_ = counter_plus_minus2_; //リマップ前の値を保存
    } else if(when00 == 1) {	// リマップ直後
      if (counter_plus_minus2_ == pre_counter_plus_minus2_ && counter_plus_minus2_ > 0){
		// counter_plus_minus2_が1以上の場合
		// リマップ後に､カウンターが進んでないので､REPLACE_PLUS_MINUSは実行されずに､他のキーが押されたことを意味するので、
		// カウンターを初期化し､符号を逆転する。
		pre_counter_plus_minus2_ = 0;
		counter_plus_minus2_ = 0;
		if(sign_plus_minus2_ == -99){
		  sign_plus_minus2_ = 1;		// 最初は正方向
		}
		sign_plus_minus2_ = - sign_plus_minus2_;
	  } else {
      }
    } else { // REPLACE_PLUS_MINUSのキーダウン時
      ++counter_plus_minus2_;
    }
  }

//Haci
//====================================================================================================
// 入力モード詳細値、あるいは、モード変更用のキーコードからそれに対応するwsd_save_配列のインデックス値(1〜)を得る。
//	実際には､以下の2つの関数から呼ばれるマスター関数｡
//		IMD2index(InputModeDetail imd00)
//		modeKey2index(KeyCode modekey00, Flags flag00)
//	2011.03.19(土)、26(土)、31(木)
  int
  VirtualKey::get_WSDindex(InputModeDetail imd00, KeyCode modekey00, Flags flag00){
	//(共通処理) 入力モード値を格納する多次元配列変数のインデックス値を得る 2011.03.15(火)
	int index00;
	bool CtlSft = (flag00 == (ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L) ||
				   flag00 == (ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_R) ||
				   flag00 == (ModifierFlag::CONTROL_R | ModifierFlag::SHIFT_L) ||
				   flag00 == (ModifierFlag::CONTROL_R | ModifierFlag::SHIFT_R) );

	if(imd00 == InputModeDetail::UNKNOWN){
	  //2011.03.26(土)
      if (modekey00 == KeyCode::JIS_EISUU && !(flag00 == ModifierFlag::SHIFT_L || flag00 == ModifierFlag::SHIFT_R) ||
      	  modekey00 == KeyCode::JIS_COLON && CtlSft){	//英字モード
      			 // 2011.04.12 [ctl]+[sft]+[:]
		imd00 = InputModeDetail::ROMAN;
      } else if (modekey00 == KeyCode::JIS_KANA  && !(flag00 == ModifierFlag::SHIFT_L || flag00 == ModifierFlag::SHIFT_R) ||
      			 modekey00 == KeyCode::J && CtlSft){	//ひらがなモード
      			 // 2011.04.12 [ctl]+[sft]+[J]
		imd00 = InputModeDetail::JAPANESE;
      } else if (modekey00 == KeyCode::JIS_KANA  && (flag00 == ModifierFlag::SHIFT_L || flag00 == ModifierFlag::SHIFT_R) ||
      			 modekey00 == KeyCode::K && CtlSft){	// カタカナモード
		imd00 = InputModeDetail::JAPANESE_KATAKANA;
      } else if (       modekey00 == KeyCode::JIS_EISUU && (flag00 == ModifierFlag::SHIFT_L || flag00 == ModifierFlag::SHIFT_R) ||
      			 modekey00 == KeyCode::SEMICOLON && CtlSft){	//半角カタカナモード
      			 // 2011.03.31(木) [ctl]+[sft]+[;]
		imd00 = InputModeDetail::JAPANESE_HALFWIDTH_KANA;
      } else if (modekey00 == KeyCode::JIS_KANA  && (flag00 == ModifierFlag::OPTION_L || flag00 == ModifierFlag::OPTION_R) ||
      			 modekey00 == KeyCode::JIS_BRACKET_RIGHT && CtlSft){	//AINUモード
      			 // 2011.04.12 AINU: [ctl]+[sft]+]
		imd00 = InputModeDetail::AINU;
      } else if (modekey00 == KeyCode::L && CtlSft){	//全角英数モード
      			 // 2011.04.01(金) [ctl]+[sft]+[L]
		imd00 = InputModeDetail::JAPANESE_FULLWIDTH_ROMAN;
	  } else {
	  	return -1;
	  }
	}

	if(        imd00 == InputModeDetail::ROMAN){
		index00 = wsdEISU;
	} else if (imd00 == InputModeDetail::JAPANESE_HIRAGANA || imd00 == InputModeDetail::JAPANESE){
		index00 = wsdHIRA;
	} else if (imd00 == InputModeDetail::JAPANESE_KATAKANA){
		index00 = wsdKATA;
	} else if (imd00 == InputModeDetail::JAPANESE_HALFWIDTH_KANA){	//2011.03.31(木)
		index00 = wsdHKAT;
	} else if (imd00 == InputModeDetail::AINU){
		index00 = wsdAINU;
	} else if (imd00 == InputModeDetail::JAPANESE_FULLWIDTH_ROMAN){	//2011.04.01(金)
		index00 = wsdFEIS;
	} else {
		return -1;
	}
	return index00;
  }

// Haci
//====================================================================
// 作業用のworkspacedataの更新、学習、すり替え、元に戻すためのマスター関数
// 実際には､以下の4つの関数から呼び出される｡
//		learn_WSD(void)
//		update_WSD(void)
//		replace_WSD(KeyCode modekey00, Flags flag00)
//		restore_WSD(InputModeDetail IMDsaved00)
// (目的1) VK_RESTOREを含むキーの連打・リピートでの遅延対策
// 		  最初のキーが押された時の状態を保存するということ。この値をフィルターでも用いる｡
// 		  だから、VK_RESTOREを一旦無視しても、次のキーは同じフィルターを確実に通ることが保証される。
// (目的2) モード変更キーを押した時､あるいは、toKeyが日本語入力モード変更キーの時､それと同時に次の文字キーを打つと､遅延のせいで誤入力が起きるので､それも防止する｡
//		  これにより、遅延バグによる誤入力は完全に排除できる。ほとんど同時に押しても大丈夫。
//		  ただし、オリジナルのCommand+Spaceのように動作が特定できないものは救えないが、VK_COMMAND_SPACE系ではこの仕組みを利用して完璧｡
//	mode00 = 0(目的1、2共通):作業用のworkspacedataの更新のみ(学習方式でない場合も)
//		   以下は学習方式の場合のみ。
//		   = 1(目的2):       最新の正しいworkspacedata値を日本語入力詳細モード毎に個別に学習(保存)する。
//		   = 2(目的2):       指定した入力モード変更キーに対応するWSD似すり替える。ただし、workspacedataが学習(保存)済なら｡
//		   = 3(目的1):       指定した入力モード値に対応するWSDにすり替える。
//	modekey00, flag00:      モード変更キーを与える。
//	IMD00            :      入力モード自体を指定する。主に､handle_RESTOREで使用｡
//	戻り値 = false   :       未学習のためすり替えなかった｡呼び出し側では、INIT_DOを実行することで､次の文字キー入力時の誤入力を防止すること。
// 2011.01.13〜18、03.05〜15、04.16,27
  bool
  VirtualKey::control_WSD(int mode00, KeyCode modekey00, Flags flag00, InputModeDetail IMD00)
  {
    int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
				// 入力モード変更遅延対策の中の「モード変更時の誤入力対策」を行うかどうかのチェックボックス。通常はオン。
	InputModeDetail IMDsv;
	int index00 = 1;	// 多次元配列のインデックス値
	BridgeWorkSpaceData curWSD00 = CommonData::getcurrent_workspacedata();	//2011.04.27

	if(mode00 == 0){	// 更新のみ
      wsd_public_ = curWSD00;

	  index00 = IMD2index(wsd_public_.inputmodedetail);	// 現在値 2011.03.29(火)
	  set_new_index(index00);	// シーソー用:2011.03.29(火)
	  return true;
	}

	if(!(learn_workspacedata  && (mode00 == 1 || mode00 == 2 || mode00 == 3))) return false;

	// 入力モード値を得る
	if(mode00 == 1){	// 学習
		index00 = IMD2index(curWSD00.inputmodedetail);	//現在値 2011.03.29(火)
	} else if(mode00 == 2){	// キーコード・フラグ値を使ってすり替え
	  // キーコード・フラグ値を入力モード値に変換 2011.03.15(火)
		index00 = modeKey2index(modekey00, flag00);		//2011.03.29(火)
	} else { // mode00 = 3
		index00 = IMD2index(IMD00);		//2011.03.29(火) これを入れ忘れて変換キーを押したら2度もフリーズ。
	}
	if(index00 < 0) return false;

	if(mode00 == 1){	// 最新の正しい値を学習(保存)する。
	  IMDsv = wsd_save_[wsdSET].inputmodedetail;	// 1つでも保存済かを示すフラグ
	  if(IMDsv == 0 ) {	//保存済でなければ
		//if(pre_index2_ == -1)	でもいい。2011.03.29(火)
	    // 最初のキー入力と同時に､Core.cppで初期化が実行できるようにするため。
	  	wsd_save_[wsdSET] = curWSD00;	// 現在値を保存(この値に意味は無い)。これによりこれ以降はここには入らない。
	  	VirtualKey::static_set_pass_initialize(VirtualKey::INIT_DO);	// すぐに初期化するため。
		set_indexes_directly(-1, index00, -1);	//2011.04.11(月)
	  }
	  wsd_save_[index00] = curWSD00;	//学習
	  return true;		//2011.04.16(土)

	} else if((mode00 == 2 || mode00 == 3 ) && learn_workspacedata){
	  // 入力モード値に従って､そのモードのworkspacedataが学習済であれば、それを利用して、すり替える｡
	  IMDsv = wsd_save_[index00].inputmodedetail;
	  if(IMDsv != 0){
	  	wsd_public_ = wsd_save_[index00];	 // 保存されていればそれを利用する。
		if(mode00 == 2){ //モード変更キーの場合のみ
			// 2011.03.29(火)
			set_new_index(index00);	// シーソー用:2011.03.29(火)
		}
	  } else {	//2011.04.16(土)
	  	return false;		//この戻り値は意味がある｡
	  }
	} else {
	  return false;		//2011.04.16(土)
	}
	return true;		//2011.04.16(土)
  }

// Haci 追加関数終わり========================================================================



  // ----------------------------------------------------------------------
  bool
  Handle_VK_LOCK::handle(const Params_KeyboardEventCallBack& params)
  {
    Flags flags(0);
    bool force_on = false;
    bool force_off = false;

    /**/ if (params.key == KeyCode::VK_LOCK_COMMAND_L)           { flags.add(ModifierFlag::COMMAND_L);                   }
    else if (params.key == KeyCode::VK_LOCK_COMMAND_R)           { flags.add(ModifierFlag::COMMAND_R);                   }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_L)           { flags.add(ModifierFlag::CONTROL_L);                   }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_R)           { flags.add(ModifierFlag::CONTROL_R);                   }
    else if (params.key == KeyCode::VK_LOCK_FN)                  { flags.add(ModifierFlag::FN);                          }
    else if (params.key == KeyCode::VK_LOCK_OPTION_L)            { flags.add(ModifierFlag::OPTION_L);                    }
    else if (params.key == KeyCode::VK_LOCK_OPTION_R)            { flags.add(ModifierFlag::OPTION_R);                    }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_L)             { flags.add(ModifierFlag::SHIFT_L);                     }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_R)             { flags.add(ModifierFlag::SHIFT_R);                     }
    else if (params.key == KeyCode::VK_LOCK_EXTRA1)              { flags.add(ModifierFlag::EXTRA1);                      }
    else if (params.key == KeyCode::VK_LOCK_EXTRA2)              { flags.add(ModifierFlag::EXTRA2);                      }
    else if (params.key == KeyCode::VK_LOCK_EXTRA3)              { flags.add(ModifierFlag::EXTRA3);                      }
    else if (params.key == KeyCode::VK_LOCK_EXTRA4)              { flags.add(ModifierFlag::EXTRA4);                      }
    else if (params.key == KeyCode::VK_LOCK_EXTRA5)              { flags.add(ModifierFlag::EXTRA5);                      }

    else if (params.key == KeyCode::VK_LOCK_COMMAND_L_FORCE_ON)  { flags.add(ModifierFlag::COMMAND_L); force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_COMMAND_R_FORCE_ON)  { flags.add(ModifierFlag::COMMAND_R); force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_L_FORCE_ON)  { flags.add(ModifierFlag::CONTROL_L); force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_R_FORCE_ON)  { flags.add(ModifierFlag::CONTROL_R); force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_FN_FORCE_ON)         { flags.add(ModifierFlag::FN);        force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_OPTION_L_FORCE_ON)   { flags.add(ModifierFlag::OPTION_L);  force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_OPTION_R_FORCE_ON)   { flags.add(ModifierFlag::OPTION_R);  force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_L_FORCE_ON)    { flags.add(ModifierFlag::SHIFT_L);   force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_R_FORCE_ON)    { flags.add(ModifierFlag::SHIFT_R);   force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_EXTRA1_FORCE_ON)     { flags.add(ModifierFlag::EXTRA1);    force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_EXTRA2_FORCE_ON)     { flags.add(ModifierFlag::EXTRA2);    force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_EXTRA3_FORCE_ON)     { flags.add(ModifierFlag::EXTRA3);    force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_EXTRA4_FORCE_ON)     { flags.add(ModifierFlag::EXTRA4);    force_on = true;  }
    else if (params.key == KeyCode::VK_LOCK_EXTRA5_FORCE_ON)     { flags.add(ModifierFlag::EXTRA5);    force_on = true;  }

    else if (params.key == KeyCode::VK_LOCK_COMMAND_L_FORCE_OFF) { flags.add(ModifierFlag::COMMAND_L); force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_COMMAND_R_FORCE_OFF) { flags.add(ModifierFlag::COMMAND_R); force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_L_FORCE_OFF) { flags.add(ModifierFlag::CONTROL_L); force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_CONTROL_R_FORCE_OFF) { flags.add(ModifierFlag::CONTROL_R); force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_FN_FORCE_OFF)        { flags.add(ModifierFlag::FN);        force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_OPTION_L_FORCE_OFF)  { flags.add(ModifierFlag::OPTION_L);  force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_OPTION_R_FORCE_OFF)  { flags.add(ModifierFlag::OPTION_R);  force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_L_FORCE_OFF)   { flags.add(ModifierFlag::SHIFT_L);   force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_SHIFT_R_FORCE_OFF)   { flags.add(ModifierFlag::SHIFT_R);   force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_EXTRA1_FORCE_OFF)    { flags.add(ModifierFlag::EXTRA1);    force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_EXTRA2_FORCE_OFF)    { flags.add(ModifierFlag::EXTRA2);    force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_EXTRA3_FORCE_OFF)    { flags.add(ModifierFlag::EXTRA3);    force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_EXTRA4_FORCE_OFF)    { flags.add(ModifierFlag::EXTRA4);    force_off = true; }
    else if (params.key == KeyCode::VK_LOCK_EXTRA5_FORCE_OFF)    { flags.add(ModifierFlag::EXTRA5);    force_off = true; }

    else if (params.key == KeyCode::VK_LOCK_ALL_FORCE_OFF) {
      FlagStatus::lock_clear();
      return true;
    } else {
      return false;
    }

    if (params.ex_iskeydown && params.repeat == false) {
      if (force_off) {
        FlagStatus::lock_decrease(flags);
      } else if (force_on) {
        FlagStatus::lock_increase(flags);
      } else {
        FlagStatus::lock_toggle(flags);
      }
      EventOutputQueue::FireModifiers::fire();
    }

    return true;
  }

  // ----------------------------------------------------------------------
  bool
  Handle_VK_STICKY::handle(const Params_KeyboardEventCallBack& params)
  {
    ModifierFlag flag = ModifierFlag::NONE;

    /**/ if (params.key == KeyCode::VK_STICKY_COMMAND_L) { flag = ModifierFlag::COMMAND_L; }
    else if (params.key == KeyCode::VK_STICKY_COMMAND_R) { flag = ModifierFlag::COMMAND_R; }
    else if (params.key == KeyCode::VK_STICKY_CONTROL_L) { flag = ModifierFlag::CONTROL_L; }
    else if (params.key == KeyCode::VK_STICKY_CONTROL_R) { flag = ModifierFlag::CONTROL_R; }
    else if (params.key == KeyCode::VK_STICKY_FN)        { flag = ModifierFlag::FN;        }
    else if (params.key == KeyCode::VK_STICKY_OPTION_L)  { flag = ModifierFlag::OPTION_L;  }
    else if (params.key == KeyCode::VK_STICKY_OPTION_R)  { flag = ModifierFlag::OPTION_R;  }
    else if (params.key == KeyCode::VK_STICKY_SHIFT_L)   { flag = ModifierFlag::SHIFT_L;   }
    else if (params.key == KeyCode::VK_STICKY_SHIFT_R)   { flag = ModifierFlag::SHIFT_R;   }
    else if (params.key == KeyCode::VK_STICKY_EXTRA1)    { flag = ModifierFlag::EXTRA1;    }
    else if (params.key == KeyCode::VK_STICKY_EXTRA2)    { flag = ModifierFlag::EXTRA2;    }
    else if (params.key == KeyCode::VK_STICKY_EXTRA3)    { flag = ModifierFlag::EXTRA3;    }
    else if (params.key == KeyCode::VK_STICKY_EXTRA4)    { flag = ModifierFlag::EXTRA4;    }
    else if (params.key == KeyCode::VK_STICKY_EXTRA5)    { flag = ModifierFlag::EXTRA5;    }
    else {
      return false;
    }

    if (params.ex_iskeydown && params.repeat == false) {
      FlagStatus::sticky_toggle(flag);
      EventOutputQueue::FireModifiers::fire();
    }

    return true;
  }

  // ----------------------------------------------------------------------
  bool
  Handle_VK_LAZY::handle(const Params_KeyboardEventCallBack& params)
  {
    ModifierFlag f = getModifierFlag(params.key);
    if (f == ModifierFlag::NONE) return false;

    // ----------------------------------------
    if (params.repeat) return true;

    if (params.eventType == EventType::DOWN) {
      FlagStatus::increase(f);
    } else if (params.eventType == EventType::UP) {
      FlagStatus::decrease(f);
    } else {
      IOLOG_ERROR("Handle_VK_LAZY invalid EventType.\n");
    }

    return true;
  }

  ModifierFlag
  Handle_VK_LAZY::getModifierFlag(KeyCode keycode)
  {
    /**/ if (keycode == KeyCode::VK_LAZY_COMMAND_L) { return ModifierFlag::COMMAND_L; }
    else if (keycode == KeyCode::VK_LAZY_COMMAND_R) { return ModifierFlag::COMMAND_R; }
    else if (keycode == KeyCode::VK_LAZY_CONTROL_L) { return ModifierFlag::CONTROL_L; }
    else if (keycode == KeyCode::VK_LAZY_CONTROL_R) { return ModifierFlag::CONTROL_R; }
    else if (keycode == KeyCode::VK_LAZY_FN)        { return ModifierFlag::FN;        }
    else if (keycode == KeyCode::VK_LAZY_OPTION_L)  { return ModifierFlag::OPTION_L;  }
    else if (keycode == KeyCode::VK_LAZY_OPTION_R)  { return ModifierFlag::OPTION_R;  }
    else if (keycode == KeyCode::VK_LAZY_SHIFT_L)   { return ModifierFlag::SHIFT_L;   }
    else if (keycode == KeyCode::VK_LAZY_SHIFT_R)   { return ModifierFlag::SHIFT_R;   }

    return ModifierFlag::NONE;
  }

  // ----------------------------------------------------------------------
  bool
  Handle_VK_CHANGE_INPUTMODE::handle(const Params_KeyboardEventCallBack& params)
  {
    if (params.key == KeyCode::VK_CHANGE_INPUTMODE_ENGLISH ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_FRENCH ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_GERMAN ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_JAPANESE ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_SWEDISH ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_CANADIAN ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_RUSSIAN ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_RUSSIAN_TYPOGRAPHIC ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_ENGLISH_TYPOGRAPHIC ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_TRADITIONAL_CHINESE_YAHOO_KEYKEY ||
        params.key == KeyCode::VK_CHANGE_INPUTMODE_ESTONIAN) {

      if (params.ex_iskeydown && params.repeat == false) {
        org_pqrs_driver_KeyRemap4MacBook_UserClient_kext::send_notification_to_userspace(BRIDGE_USERCLIENT_NOTIFICATION_TYPE_CHANGE_INPUT_SOURCE, params.key.get());
      }
      return true;

    } else {
      return false;
    }
  }

  // ----------------------------------------------------------------------
  Handle_VK_CONFIG::Vector_Item* Handle_VK_CONFIG::items_ = NULL;

  void
  Handle_VK_CONFIG::initialize(void)
  {
    items_ = new Vector_Item();
  }

  void
  Handle_VK_CONFIG::terminate(void)
  {
    if (items_) {
      delete items_;
    }
  }

  void
  Handle_VK_CONFIG::add_item(RemapClass* remapclass,
                             unsigned int keycode_toggle,
                             unsigned int keycode_force_on,
                             unsigned int keycode_force_off,
                             unsigned int keycode_sync_keydownup)
  {
    if (! items_) return;

    items_->push_back(Item(remapclass, keycode_toggle, keycode_force_on, keycode_force_off, keycode_sync_keydownup));
  }

  void
  Handle_VK_CONFIG::clear_items(void)
  {
    items_->clear();
  }

  bool
  Handle_VK_CONFIG::handle(const Params_KeyboardEventCallBack& params)
  {
    if (! items_) return false;

    for (size_t i = 0; i < items_->size(); ++i) {
      RemapClass* remapclass              = (*items_)[i].remapclass;
      unsigned int keycode_toggle         = (*items_)[i].keycode_toggle;
      unsigned int keycode_force_on       = (*items_)[i].keycode_force_on;
      unsigned int keycode_force_off      = (*items_)[i].keycode_force_off;
      unsigned int keycode_sync_keydownup = (*items_)[i].keycode_sync_keydownup;

      if (! remapclass) return false;

      if (params.ex_iskeydown && params.repeat == false) {
        /*  */ if (params.key == keycode_toggle) {
          remapclass->toggleEnabled();
          goto refresh;

        } else if (params.key == keycode_force_on) {
          remapclass->setEnabled(true);
          goto refresh;

        } else if (params.key == keycode_force_off) {
          remapclass->setEnabled(false);
          goto refresh;

        } else if (params.key == keycode_sync_keydownup) {
          remapclass->setEnabled(true);
          goto refresh;
        }

      } else if (params.eventType == EventType::UP) {
        if (params.key == keycode_toggle ||
            params.key == keycode_force_on ||
            params.key == keycode_force_off) {
          goto finish;
        }

        if (params.key == keycode_sync_keydownup) {
          remapclass->setEnabled(false);
          goto refresh;
        }
      }
    }

    return false;

  refresh:
    RemapClassManager::refresh();

  finish:
    EventOutputQueue::FireModifiers::fire();
    return true;
  }

  bool
  Handle_VK_CONFIG::is_VK_CONFIG_SYNC_KEYDOWNUP(KeyCode keycode)
  {
    if (! items_) return false;

    for (size_t i = 0; i < items_->size(); ++i) {
      unsigned int keycode_sync_keydownup = (*items_)[i].keycode_sync_keydownup;
      if (keycode == keycode_sync_keydownup) return true;
    }

    return false;
  }

  // ----------------------------------------------------------------------
  void
  Handle_VK_MOUSEKEY::initialize(IOWorkLoop& workloop)
  {
    dx_ = 0;
    dy_ = 0;
    scale_ = 1;
    scrollmode_ = false;
    highspeed_ = false;

    fire_timer_.initialize(&workloop, NULL, Handle_VK_MOUSEKEY::fire_timer_callback);
  }

  void
  Handle_VK_MOUSEKEY::terminate(void)
  {
    fire_timer_.terminate();
  }

  void
  Handle_VK_MOUSEKEY::reset(void)
  {
    dx_ = 0;
    dy_ = 0;
    scale_ = 1;
    scrollmode_ = false;
    highspeed_ = false;

    fire_timer_.cancelTimeout();
  }

  bool
  Handle_VK_MOUSEKEY::handle(const Params_KeyboardEventCallBack& params)
  {
    if (handle_button(params)) return true;
    if (handle_move(params)) return true;
    return false;
  }

  PointingButton
  Handle_VK_MOUSEKEY::getPointingButton(KeyCode keycode)
  {
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_LEFT)    { return PointingButton::LEFT;    }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_MIDDLE)  { return PointingButton::MIDDLE;  }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_RIGHT)   { return PointingButton::RIGHT;   }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_BUTTON4) { return PointingButton::BUTTON4; }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_BUTTON5) { return PointingButton::BUTTON5; }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_BUTTON6) { return PointingButton::BUTTON6; }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_BUTTON7) { return PointingButton::BUTTON7; }
    if (keycode == KeyCode::VK_MOUSEKEY_BUTTON_BUTTON8) { return PointingButton::BUTTON8; }
    return PointingButton::NONE;
  }

  bool
  Handle_VK_MOUSEKEY::is_VK_MOUSEKEY(KeyCode keycode)
  {
    if (getPointingButton(keycode) != PointingButton::NONE) return true;
    if (keycode == KeyCode::VK_MOUSEKEY_UP)           { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_DOWN)         { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_LEFT)         { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_RIGHT)        { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_SCROLL_UP)    { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_SCROLL_DOWN)  { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_SCROLL_LEFT)  { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_SCROLL_RIGHT) { return true; }
    if (keycode == KeyCode::VK_MOUSEKEY_HIGHSPEED)    { return true; }
    return false;
  }

  bool
  Handle_VK_MOUSEKEY::handle_button(const Params_KeyboardEventCallBack& params)
  {
    PointingButton button = getPointingButton(params.key);
    if (button == PointingButton::NONE) return false;

    if (params.repeat) return true;

    // ----------------------------------------
    if (params.ex_iskeydown) {
      ButtonStatus::increase(button);
      EventOutputQueue::FireRelativePointer::fire(ButtonStatus::makeButtons());

    } else {
      ButtonStatus::decrease(button);
      EventOutputQueue::FireRelativePointer::fire(ButtonStatus::makeButtons());
    }

    return true;
  }

  bool
  Handle_VK_MOUSEKEY::handle_move(const Params_KeyboardEventCallBack& params)
  {
    /*  */ if (params.key == KeyCode::VK_MOUSEKEY_UP) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { --dy_; scrollmode_ = false; } else { ++dy_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_DOWN) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { ++dy_; scrollmode_ = false; } else { --dy_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_LEFT) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { --dx_; scrollmode_ = false; } else { ++dx_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_RIGHT) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { ++dx_; scrollmode_ = false; } else { --dx_; }
      }

    } else if (params.key == KeyCode::VK_MOUSEKEY_SCROLL_UP) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { --dy_; scrollmode_ = true; } else { ++dy_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_SCROLL_DOWN) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { ++dy_; scrollmode_ = true; } else { --dy_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_SCROLL_LEFT) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { --dx_; scrollmode_ = true; } else { ++dx_; }
      }
    } else if (params.key == KeyCode::VK_MOUSEKEY_SCROLL_RIGHT) {
      if (params.repeat == false) {
        if (params.ex_iskeydown) { ++dx_; scrollmode_ = true; } else { --dx_; }
      }

    } else if (params.key == KeyCode::VK_MOUSEKEY_HIGHSPEED) {
      if (params.repeat == false) {
        highspeed_ = params.ex_iskeydown;
      }

    } else {
      return false;
    }

    if (dx_ != 0 || dy_ != 0) {
      fire_timer_.setTimeoutMS(TIMER_INTERVAL, false);
    } else {
      scale_ = 1;

      // keep scrollmode_ & highspeed_.
      //
      // When VK_MOUSEKEY_SCROLL_UP and VK_MOUSEKEY_SCROLL_DOWN are pressed at the same time,
      // this code will be executed.
      //
      // In the above case, we need to keep scrollmode_, highspeed_ value.

      fire_timer_.cancelTimeout();
    }

    return true;
  }

  void
  Handle_VK_MOUSEKEY::fire_timer_callback(OSObject* notuse_owner, IOTimerEventSource* notuse_sender)
  {
    if (! scrollmode_) {
      int s = scale_;
      if (highspeed_) s = HIGHSPEED_RELATIVE_SCALE;

      EventOutputQueue::FireRelativePointer::fire(ButtonStatus::makeButtons(), dx_ * s, dy_ * s);

    } else {
      int s = scale_;
      if (highspeed_) s = HIGHSPEED_SCROLL_SCALE;

      int delta1 = -dy_ * s * EventOutputQueue::FireScrollWheel::DELTA_SCALE;
      int delta2 = -dx_ * s * EventOutputQueue::FireScrollWheel::DELTA_SCALE;
      EventOutputQueue::FireScrollWheel::fire(delta1, delta2);
    }

    if (scale_ < SCALE_MAX) {
      ++scale_;
    }

    fire_timer_.setTimeoutMS(TIMER_INTERVAL);
  }

  int Handle_VK_MOUSEKEY::dx_;
  int Handle_VK_MOUSEKEY::dy_;
  int Handle_VK_MOUSEKEY::scale_;
  bool Handle_VK_MOUSEKEY::scrollmode_;
  bool Handle_VK_MOUSEKEY::highspeed_;
  TimerWrapper Handle_VK_MOUSEKEY::fire_timer_;


  //Haci
  // ----------------------------------------------------------------------
  // 作業用のworkspacedataを使用することは、このトグルのケースでもメリットがある。
  // 例えば､ひらがなモードでVK_JIS_TEMPORARY_ROMANにして半角の「7」を出すキーを打つや否や､このトグルキーを押すと､
  // 本来は最初のひらがなモードの逆の英字モードになるのが正しい。ところが、従来のようにVK_JIS_TEMPORARY_RESTOREを実行していてもそれが完了していないので､
  // このトグルキーによって､英字モードの状態からひらがなモードに戻ってしまう｡
  // VK_JIS_TEMPORARY_RESTOREを無視する場合は、
  // 作業用のworkspacedataが最初の状態を記憶して変化しないので､たとえモード変更を複数実行しているような場合であっても､正確に切り替わるという訳です。
  // さらに、学習したworkspacedataを使ってすり替えることで､同時に次の文字キーを打鍵しても､正しく入力できる｡
  //	2011.03.08(火)〜04.16(土)
  bool
  Handle_VK_JIS_TOGGLE_EISUU_KANA::handle(const Params_KeyboardEventCallBack& params)
  {
    if (params.key != KeyCode::VK_JIS_TOGGLE_EISUU_KANA) return false;


//Haci
    int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);
	  			//「連打遅延対策」を切り替えるチェックボックス
	int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
				//「モード変更時の誤入力対策」を行うかどうかのチェックボックス。

    if (params.ex_iskeydown) {
      if (!(ignore_vk_restore || learn_workspacedata) && InputMode::JAPANESE == CommonData::getcurrent_workspacedata().inputmode ||
      	   (ignore_vk_restore || learn_workspacedata) && InputMode::JAPANESE == VirtualKey::getwsd_public().inputmode) {
		// チェックボックスの設定のいずれかがオンなら作業用のworkspacedataを使用。



        newkeycode_ = KeyCode::JIS_EISUU;
      } else {
        newkeycode_ = KeyCode::JIS_KANA;
      }


//Haci
	  bool result00 = false;	//2011.04.16(土)
	  if(learn_workspacedata){	// 学習機能の設定がチェックされていれば､作業用のworkspacedataをすり替える｡
		result00 = VirtualKey::replace_WSD(newkeycode_, ModifierFlag::NONE);
	  }
	  if(result00){	//学習済ですり替えられたので､次のキー入力時にCore.cppでの更新をしないようにする。
		VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);
	  } else {	//workspacedataの学習をしない設定の場合を含めて、未学習の場合も､次のキーの時に、作業用のworkspacedataを更新する必要がある｡
 	  	VirtualKey::static_set_pass_initialize(VirtualKey::INIT_DO);
	  }



    }

    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType, params.flags, newkeycode_,
                                                                                   params.keyboardType, params.repeat));
    if (ptr) {
      EventOutputQueue::FireKey::fire(*ptr);
    }
    return true;
  }

  KeyCode Handle_VK_JIS_TOGGLE_EISUU_KANA::newkeycode_;


//Haci
  KeyCode Handle_VK_JIS_COMMAND_SPACE::newkeycode_;
  Flags   Handle_VK_JIS_COMMAND_SPACE::newflag_;
  // ----------------------------------------------------------------------
  // Command+Spaceのバグ(時々モード変更がおかしくなる)の代替策として作成を開始し､
  // Command+Space、Option+Command+Spaceの不統一さ、使い難さを改良しつつ､
  // Smart[英数/かな]キーへと到達した｡
  // 12種類の仮想キーの組み合わせで､従来のCommand+Spaceの再現・改良〜Smart[英数/カナ]キーまでほぼ全てに対応する｡
  // かわせみver1のバグ(ver2で改善予定とのメーカーの説明)、つまり、
  // 「カナ入力」かつ「controlキーで英数入力する(初期値)」に設定した場合に､[ctl] を含むショートカットが誤動作するので、その設定を外すこと｡
  // アイヌを「Option+かな」で与えると､通常は問題ないが､連続して切り替えると､Option抜きで実行されてしまうという「ことえり」のバグが判明した。「かわせみ」などでは問題ない｡
  // Contol+Shift系のショートカットを使用すると問題ないことも判明したので､ひらがなモードなども含めて､全て､Contol+Shift系のショートカットを与えるようにした｡
  // 2011.03.29(火)〜04.15(金)
  // 2011.04.15(金) SEESAW_EISUU_OTHERSなど追加整理して､12種類の仮想キーとなった｡
  bool
  Handle_VK_JIS_COMMAND_SPACE::handle(const Params_KeyboardEventCallBack& params)
  {
	int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
				// 「モード変更時の誤入力対策」を行うかどうかのチェックボックス。
	int use_ainu = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_use_ainu);
				// 「モード変更時の誤入力対策」を行うかどうかのチェックボックス。
	KeyCode key00  = params.key;
    int index00;
    int skip00[VirtualKey::wsdMAX +1] = {0};	//2011.04.05(火)
    int replace_num00;							//2011.04.05(火)
	int seesawType00 = -1;
	int skipType00   = -1;		//2011.04.15(金)

    if (key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_CUR_PRE ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_NONE  ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_NONE ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_PRE ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_PRE ||

        key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_KANA ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_OTHERS ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_EISUU ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_OTHERS ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA_EISUU ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_EISUU ||
        key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA){
    } else {
      return false;
	}

	if(params.ex_iskeydown){	//キーダウン時の処理(始まり)----------------------
		//2011.04.09(土)pm03:19 キーアップは最後の部分のみ｡
	  if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_CUR_PRE){ // シーソー切替
		seesawType00 = VirtualKey::CUR_PRE;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_KANA){
		seesawType00 = VirtualKey::EISUU_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_OTHERS){
		seesawType00 = VirtualKey::KANA_OTHERS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_EISUU){		//2011.04.15(金)
		seesawType00 = VirtualKey::KANA_EISUU;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_OTHERS){	//2011.04.15(金)
		seesawType00 = VirtualKey::EISUU_OTHERS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_NONE){ // リプレース
		skipType00 = VirtualKey::SKIP_NONE_PLUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_PRE){
		skipType00 = VirtualKey::SKIP_PRE_PLUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_NONE){
		skipType00 = VirtualKey::SKIP_NONE_MINUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_PRE){
		skipType00 = VirtualKey::SKIP_PRE_MINUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA_EISUU){
		skipType00 = VirtualKey::SKIP_EISUU_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA){	//2011.04.15(金)
		skipType00 = VirtualKey::SKIP_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_EISUU){	//2011.04.15(金)
		skipType00 = VirtualKey::SKIP_EISUU;
	  }

	  newflag_ = ModifierFlag::NONE;	//2011.04.01(金)
	  if(seesawType00 != -1){	// seesawの場合
		index00 = VirtualKey::get_index_for_seesaw_AtoB_WSD(seesawType00);	// 2011.04.10(日)統合

		if (seesawType00 == VirtualKey::EISUU_KANA   && index00 == VirtualKey::wsdEISU ||
			seesawType00 == VirtualKey::KANA_OTHERS  && index00 == VirtualKey::wsdHIRA ||
			seesawType00 == VirtualKey::KANA_EISUU   && index00 == VirtualKey::wsdHIRA ||
			seesawType00 == VirtualKey::EISUU_OTHERS && index00 == VirtualKey::wsdEISU){
			// 起点のモードに切り替えた時点からタイマーをかける｡
			// タイムアウト後は、フラグseesaw_init2_が立つので、
			// 次に同じseesawキーが押された時は､CommonData.hppの(後処理AA)が実行されて、モードは変化しない｡
			// これで、トグル系の最大の欠点がついに解消｡
			//2011.04.09(土)、15(金)
			VirtualKey::static_setTimeoutMS(VirtualKey::CALLBACK_SEESAW_INIT);
		}

	  } else {	// replaceの場合
		int sign00;	//符号= -1,1 
		  if(skipType00 == VirtualKey::SKIP_EISUU_KANA ||
		  	 skipType00 == VirtualKey::SKIP_KANA ||
		  	 skipType00 == VirtualKey::SKIP_EISUU){
		  	//2011.04.06(水)、15(金)、17(日)
			VirtualKey::reverse_sign_REPLACE_PLUS_MINUS(9);	// カウンターを進める。次の時に､方向を逆転させるため｡
		  }

		  if(skipType00 == VirtualKey::SKIP_NONE_MINUS ||
	  		 skipType00 == VirtualKey::SKIP_PRE_MINUS){
			sign00 = -1;
		  } else {	// REPLACE_MINUS,MINUS2の場合
		  	// 2011.04.06(水) REPLACE_PLUS_MINUS_SKIP系もとりあえず､正方向とするが、この値は用いない。
			sign00 =  1;
		  }


		if(!use_ainu){
		   skip00[VirtualKey::wsdAINU] = 1;	// AINUをスキップする。
											//2011.04.11(月) チェックボックスで設定するようにした｡
		}
		if(skipType00 == VirtualKey::SKIP_NONE_PLUS ||
	  		 skipType00 == VirtualKey::SKIP_NONE_MINUS){
			replace_num00 = 1;
		} else {	// 2011.04.05(火)
			if(skipType00 == VirtualKey::SKIP_EISUU_KANA){
				skip00[VirtualKey::wsdEISU] = 1;	// 英字モードをスキップ
				skip00[VirtualKey::wsdHIRA] = 1;	// ひらがなモード
				replace_num00 = 3;
			} else if(skipType00 == VirtualKey::SKIP_KANA){
				skip00[VirtualKey::wsdHIRA] = 1;	// ひらがなモード
				replace_num00 = 3;
			} else if(skipType00 == VirtualKey::SKIP_EISUU){
				skip00[VirtualKey::wsdEISU] = 1;	// 英字モード
				replace_num00 = 3;
			} else {	// SKIP_PRE_PLUS(MINUS)
				replace_num00 = 2;
				// スキップするのは前のモードなので､それは以下のget_index_for_replaceWSD関数の中で与えられる｡
			}
		}
		index00 = VirtualKey::get_index_for_replaceWSD(sign00, skip00, replace_num00);	//2011.04.01(金)、05(火)
	  }

    if (index00 == VirtualKey::wsdEISU) {					//英字
		// 2011.04.14(木) KeyCode::JIS_EISUU、ModifierFlag::NONE
    	newkeycode_ = KeyCode::JIS_COLON;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == VirtualKey::wsdHIRA) {			//ひらがな
		// 2011.04.14(木) KeyCode::JIS_KANA、ModifierFlag::NONE
    	newkeycode_ = KeyCode::J;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == VirtualKey::wsdKATA) {			//カタカナ
		// 2011.04.14(木) KeyCode::JIS_KANA、ModifierFlag::SHIFT_L
    	newkeycode_ = KeyCode::K;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == VirtualKey::wsdHKAT) {			//半角カタカナ
    	//2011.03.31(木)
    	newkeycode_ = KeyCode::SEMICOLON;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == VirtualKey::wsdFEIS) {			//全角英数
    	//2011.04.01(金)
    	newkeycode_ = KeyCode::L;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == VirtualKey::wsdAINU) {			// AINU
		// 2011.04.14(木) KeyCode::JIS_KANA、ModifierFlag::SHIFT_Lは、「ことえり」のバグのため突然おかしくなる｡
    	newkeycode_ = KeyCode::JIS_BRACKET_RIGHT;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
	} else {
		return true;
    }

	}	 //------------- (キーダウン時の処理終わり)

	if (params.ex_iskeydown) {	//2011.04.03(日)
       FlagStatus::temporary_decrease(params.flags);
       FlagStatus::temporary_increase(newflag_);
	} else {
      FlagStatus::temporary_decrease(newflag_);
      FlagStatus::temporary_increase(params.flags);
	}

	if(params.ex_iskeydown){
	  bool result00 = false;	//2011.04.16(土)
	  if(learn_workspacedata){
		result00 = VirtualKey::replace_WSD(newkeycode_, newflag_);	//作業用のworkspacedataをすり替える｡
	  }
	  if(result00){	//学習済ですり替えられたので､次のキー入力時にCore.cppでの更新をしないようにする。
		VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);
	  } else {		//workspacedataの学習をしない設定の場合を含めて、未学習の場合も､次のキーの時に、作業用のworkspacedataを更新する必要がある｡
 	  	VirtualKey::static_set_pass_initialize(VirtualKey::INIT_DO);
	  }
	}

    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType,
    																			   FlagStatus::makeFlags(),
    																			   newkeycode_,
    																			   params.keyboardType,
    																			   params.repeat));			//2011.04.03(日)
    if (ptr) {
      EventOutputQueue::FireKey::fire(*ptr);
    }
    return true;
  }



  // ----------------------------------------------------------------------
  bool
  handle_VK_JIS_BACKSLASH(const Params_KeyboardEventCallBack& params)
  {
    if (params.key != KeyCode::VK_JIS_BACKSLASH) return false;

    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType, params.flags, KeyCode::BACKSLASH,
                                                                                   KeyboardType::MACBOOK, params.repeat));
    if (ptr) {
      EventOutputQueue::FireKey::fire(*ptr);
    }
    return true;
  }

  bool
  handle_VK_JIS_YEN(const Params_KeyboardEventCallBack& params)
  {
    if (params.key != KeyCode::VK_JIS_YEN) return false;

    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType, params.flags, KeyCode::JIS_YEN,
                                                                                   KeyboardType::JIS_MACBOOK_2008, params.repeat));
    if (ptr) {
      EventOutputQueue::FireKey::fire(*ptr);
    }
    return true;
  }

  // ----------------------------------------------------------------------
  void
  Handle_VK_JIS_TEMPORARY::initialize(IOWorkLoop& workloop)
  {
    fire_timer_.initialize(&workloop, NULL, Handle_VK_JIS_TEMPORARY::fire_timer_callback);
    fireKeyInfo_.active = false;
  }

  void
  Handle_VK_JIS_TEMPORARY::terminate(void)
  {
    fire_timer_.terminate();
  }


//Haci
//--------------------------------------------------------------------------------------------------------------------
//	保存値savedinputmodedetail_, currentinputmodedetail_を初期化。
//	Core.cppで使用｡
//	2011.02.18(金)
  void
  Handle_VK_JIS_TEMPORARY::resetSavedIMD(void)
  {
        savedinputmodedetail_   = InputModeDetail::NONE;
        currentinputmodedetail_ = InputModeDetail::NONE;
  }



  bool
  Handle_VK_JIS_TEMPORARY::handle_core(const Params_KeyboardEventCallBack& params,
                                       KeyCode key,
                                       InputModeDetail inputmodedetail)
  {
    if (params.key != key) return false;

    if (params.ex_iskeydown) {
      if (savedinputmodedetail_ == InputModeDetail::NONE) {


//Haci
		// VK_RESTOREを含むキーの連打遅延対策あるいは入力モード変更キー押下時の遅延対策のため作業用のworkspacedataへすり替え
		// 連打遅延対策あるいは学習機能を切り替えるチェックボックスの設定がオンの場合のみ｡
		// 2011.02.10(木)、20(日)、03.08(火)
		!(Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore) ||
		  Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata)) ? 
			savedinputmodedetail_ = CommonData::getcurrent_workspacedata().inputmodedetail :
			savedinputmodedetail_ = VirtualKey::getwsd_public().inputmodedetail;
        currentinputmodedetail_ = savedinputmodedetail_;



      }
      firekeytoinputdetail(params, inputmodedetail);
    }

    return true;
  }

  bool
  Handle_VK_JIS_TEMPORARY::handle_RESTORE(const Params_KeyboardEventCallBack& params)
  {
    if (params.key != KeyCode::VK_JIS_TEMPORARY_RESTORE) return false;

    if (params.ex_iskeydown) {
      if (savedinputmodedetail_ != InputModeDetail::NONE) {


//Haci
		// 学習したworkspacedataにすり替えることで､モードが戻る瞬間に次のキーが来ても誤入力を完全に排除できる。
		// 「workspacedataの学習」を行うかどうかのチェックボックスがオンの場合のみ。
		// リストアするということは既に学習済なので､INT_DOが必要なケースは無い｡
		// 2011.03.07(月)、08(火)
		if(Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata)){
		  VirtualKey::restore_WSD(savedinputmodedetail_);
		  VirtualKey::static_set_pass_initialize(VirtualKey::INIT_NOT);	//次のキー入力時にCore.cppでの更新をしないため
		}



        firekeytoinputdetail(params, savedinputmodedetail_);
        savedinputmodedetail_ = InputModeDetail::NONE;
        currentinputmodedetail_ = InputModeDetail::NONE;
      }
    }

    return true;
  }

  bool
  Handle_VK_JIS_TEMPORARY::handle(const Params_KeyboardEventCallBack& params)
  {
    // ------------------------------------------------------------
    if (handle_core(params,
                    KeyCode::VK_JIS_TEMPORARY_ROMAN,
                    InputModeDetail::ROMAN)) return true;

    if (handle_core(params,
                    KeyCode::VK_JIS_TEMPORARY_HIRAGANA,
                    InputModeDetail::JAPANESE_HIRAGANA)) return true;

    if (handle_core(params,
                    KeyCode::VK_JIS_TEMPORARY_KATAKANA,
                    InputModeDetail::JAPANESE_KATAKANA)) return true;

    // OK, Ainu is not Japanese.
    // But the input source of Ainu is Kotoeri, we need to handle it here.
    if (handle_core(params,
                    KeyCode::VK_JIS_TEMPORARY_AINU,
                    InputModeDetail::AINU)) return true;

    // ------------------------------------------------------------
    if (handle_RESTORE(params)) return true;

    // ------------------------------------------------------------
    // flash keyevent
    if (fireKeyInfo_.active) {
      fire_timer_.cancelTimeout();
      fire();
    }

    return false;
  }

  void
  Handle_VK_JIS_TEMPORARY::firekeytoinputdetail(const Params_KeyboardEventCallBack& params,
                                                InputModeDetail inputmodedetail)
  {
    inputmodedetail = normalize(inputmodedetail);
    currentinputmodedetail_ = normalize(currentinputmodedetail_);

    // ------------------------------------------------------------
    if (inputmodedetail == currentinputmodedetail_) return;
    if (inputmodedetail == InputModeDetail::NONE) return;

    currentinputmodedetail_ = inputmodedetail;

    // ------------------------------------------------------------
    if (inputmodedetail == InputModeDetail::ROMAN) {
      fireKeyInfo_.flags = Flags(0);
      fireKeyInfo_.key = KeyCode::JIS_EISUU;

    } else if (inputmodedetail == InputModeDetail::JAPANESE_HIRAGANA) {
      fireKeyInfo_.flags = Flags(0);
      fireKeyInfo_.key = KeyCode::JIS_KANA;

    } else if (inputmodedetail == InputModeDetail::JAPANESE_KATAKANA) {
      fireKeyInfo_.flags = ModifierFlag::SHIFT_L;
      fireKeyInfo_.key = KeyCode::JIS_KANA;


//Haci
	// 半角カタカナ、全角英数にも戻れるようにする｡
	// 「Option+かな」(AINU)は「ことえり」で誤動作するケースがあることがVK_COMMAND_SPACEの処理で判明したので､
	//  ここでもControl+Shift系のショートカットを用いるようにした。
	//  英数モードなども同様にした方がいいかもしれない｡
	// 2011.4.12
    } else if (inputmodedetail == InputModeDetail::JAPANESE_HALFWIDTH_KANA) {	//半角カタカナ
      fireKeyInfo_.flags = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      fireKeyInfo_.key = KeyCode::SEMICOLON;
    } else if (inputmodedetail == InputModeDetail::JAPANESE_FULLWIDTH_ROMAN) {	//全角英数
      fireKeyInfo_.flags = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      fireKeyInfo_.key = KeyCode::L;



    } else if (inputmodedetail == InputModeDetail::AINU) {
      fireKeyInfo_.flags = ModifierFlag::OPTION_L;
      fireKeyInfo_.key = KeyCode::JIS_KANA;

    } else {
      return;
    }

    fireKeyInfo_.keyboardType = params.keyboardType;
    fireKeyInfo_.active = true;

    fire_timer_.setTimeoutMS(KEYEVENT_DELAY_MS);
  }

  InputModeDetail
  Handle_VK_JIS_TEMPORARY::normalize(InputModeDetail imd)
  {
    if (imd == InputModeDetail::JAPANESE) {
      return InputModeDetail::JAPANESE_HIRAGANA;
    }
    return imd;
  }

  void
  Handle_VK_JIS_TEMPORARY::fire_timer_callback(OSObject* notuse_owner, IOTimerEventSource* notuse_sender)
  {
    fire();
  }

  void
  Handle_VK_JIS_TEMPORARY::fire(void)
  {
    if (! fireKeyInfo_.active) return;
    fireKeyInfo_.active = false;
    EventOutputQueue::FireKey::fire_downup(fireKeyInfo_.flags, fireKeyInfo_.key, fireKeyInfo_.keyboardType);
  }

  InputModeDetail Handle_VK_JIS_TEMPORARY::savedinputmodedetail_(0);
  InputModeDetail Handle_VK_JIS_TEMPORARY::currentinputmodedetail_(0);
  Handle_VK_JIS_TEMPORARY::FireKeyInfo Handle_VK_JIS_TEMPORARY::fireKeyInfo_;
  TimerWrapper Handle_VK_JIS_TEMPORARY::fire_timer_;
}
