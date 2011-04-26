#include "CommonData.hpp"
#include "EventOutputQueue.hpp"
#include "KeyToKey.hpp"
#include "KeyboardRepeat.hpp"
#include "VirtualKey.hpp"


#include "Config.hpp"
			//2011.01.26(水) Haci



namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {


//Haci
    TimerWrapperRestore KeyToKey::timer_restore2_;		// 2011.01.31(月)
    IOWorkLoop*  KeyToKey::workLoop_restore2_ = NULL;	// 2011.02.01(火)
    int KeyToKey::callback2_ = CALLBACK_INIT;			// 2011.02.27(日)
    int KeyToKey::pass_initialize2_ = INIT_NOT;			// 2011.03.01(火)インストールした時にその時点でのモードを活かすために最初のキー入力時はパスするため。
	int KeyToKey::case1_pass_restore2_ = 0;				// 2011.03.02(水)

//Haci
	//=======================================
	// この関数はCore.cppのstart()関数で使用するためのもの
	//2011.01.31(月)〜02.10(木)
    void
    KeyToKey::static_initialize(IOWorkLoop& workloop)
    {
      timer_restore2_.initialize(&workloop, NULL, KeyToKey::timeoutAfterKey2);
      workLoop_restore2_ = &workloop;
    }

//Haci
	//=======================================
	// この関数はCore.cppのstop()関数で使用するためのもの
	//2011.02.01(火)
    void
    KeyToKey::static_terminate(void)
    {
      timer_restore2_.terminate();
    }

//Haci
	//=======================================
	// 外部からこのタイマーを停止するには、RemapFunc::KeyToKey::static_cancelTimeout()を実行する。
	//2011.02.02(水)〜02.28(月)
    void
    KeyToKey::static_cancelTimeout(void)
    {
      timer_restore2_.cancelTimeout();	// タイマーを停止
    }

//Haci
	//=======================================
	// Core.cppで初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を得る｡
	// RemapFunc::KeyToKey::static_get_pass_initialize()を実行する。
	//2011.03.01(火)
    int
    KeyToKey::static_get_pass_initialize(void)
    {
      return pass_initialize2_;
    }


//Haci
	//=======================================
	// Core.cppで初期化をパスするかどうかを判断するためのフラグ変数pass_initialize2_の値を変更
	// RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT or INIT_DO)を実行する。
	//2011.03.01(火)
    void
    KeyToKey::static_set_pass_initialize(int pass_initialize00)
    {
	  if(pass_initialize00 != INIT_NOT && pass_initialize00 != INIT_DO) return;
      pass_initialize2_ = pass_initialize00;
    }

//Haci
	//=======================================
	// 作業用のworkspacedataの初期化条件で専用タイマーを起動: これがこのタイマーの基本動作
	// タイムアウト後に実行するコールバック関数は共通(実行する処理はフラグcallback2_で指定しておく)となる仕組みにしてある｡
	// Core.cppでキーアップ時に必ず実行する。
	// Core.cppから実行するには、RemapFunc::KeyToKey::static_setTimeoutMS_init()を実行する。
	// KeyToKeyでVK_RESTOREを見つけたら、RESTORE用のタイマーとして再起動する｡タイマーが動いていても特に問題ない｡
	// また、シーソー系のモード切替で、起点(固定)のモードになった時点で、それ用のタイマーとして再起動される｡
	//2011.02.28(月)〜03.01(火)、11(金)、13(日)
    void
    KeyToKey::static_setTimeoutMS_init(void)
    {
	  int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);
		// 2011.03.13(日)VK_JIS_TEMPORARY_RESTOREを無視するチェックボックスのオンオフ値
	  int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
		// 2011.03.13(日) 入力モード変更遅延対策の中の「入力モード変更時の誤入力対策」を行うかどうかのチェックボックス。通常はオン。
	  int vk_restore_timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_parameter_keyinterval_timeout);
	  			// キー入力からタイムアウトするまでの時間(msec)
	  if((ignore_vk_restore || learn_workspacedata)&& workLoop_restore2_){
		callback2_  = CALLBACK_INIT;	// コールバック関数での条件(初期化)を指定
		timer_restore2_.setTimeoutMS(vk_restore_timeout);
		}

    }

//Haci
	//=======================================
	// seesaw系のモード切替で起点(固定)のモード(<<英字>>←→ひらがなの場合の英字等)になった時点で実行する｡
	// タイムアウト後のコールバック関数でモードを変更しないためのフラグを立てる。
	// 実行するには、RemapFunc::KeyToKey::static_setTimeoutMS_seesaw_init()を実行する。
	//2011.04.09(土)
    void
    KeyToKey::static_setTimeoutMS_seesaw_init(void)
    {
	  int vk_restore_timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_parameter_keyinterval_timeout);
	  			// キー入力からタイムアウトするまでの時間(msec)
		callback2_  = CALLBACK_SEESAW_INIT;	// コールバック関数での条件(seesaw初期化)を指定
		timer_restore2_.setTimeoutMS(vk_restore_timeout);
    }

//Haci
	//=======================================
	// KeyToKey(Case1)でリストアを実行するか、しないかを決める。
	// 今のところKeyOverlaidModifierが実行された時に､最初にKeyToKey(Case1)に入るが､その時はリストアはしないように制御するために使用する。
	// RemapFunc::KeyToKey::static_set_case1_pass_restore(1 or 0)を実行する。
	// 		case1_pass_restore00 = 1: KeyToKey(Case1)でリストアを実行しないようにする。
	//							 = 0: KeyToKey(Case1)でリストアを実行するように戻す。
	//2011.03.02(水)〜03(木)
    void
    KeyToKey::static_set_case1_pass_restore(int case1_pass_restore00)
    {
	  int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);
		// VK_JIS_TEMPORARY_RESTOREを無視するチェックボックスのオンオフ値
	  if(!ignore_vk_restore || case1_pass_restore00 != 0 && case1_pass_restore00 != 1) return;
      case1_pass_restore2_ = case1_pass_restore00;
    }

//Haci ----------------ここまで全て



    KeyToKey::KeyToKey(void) : index_(0), keyboardRepeatID_(-1), isRepeatEnabled_(true)
    {
      toKeys_ = new Vector_PairKeyFlags();
    }

    KeyToKey::~KeyToKey(void)
    {
      if (toKeys_) {
        delete toKeys_;
      }
    }

    void
    KeyToKey::add(unsigned int datatype, unsigned int newval)
    {
      if (! toKeys_) return;

      switch (datatype) {
        case BRIDGE_DATATYPE_KEYCODE:
        {
          switch (index_) {
            case 0:
              fromKey_.key = newval;
              break;
            default:
              toKeys_->push_back(PairKeyFlags(newval));
              break;
          }
          ++index_;

          break;
        }

        case BRIDGE_DATATYPE_FLAGS:
        {
          switch (index_) {
            case 0:
              IOLOG_ERROR("Invalid KeyToKey::add\n");
              break;
            case 1:
              fromKey_.flags = newval;
              break;
            default:
              if (! toKeys_->empty()) {
                (toKeys_->back()).flags = newval;
              }
              break;
          }
          break;
        }

        case BRIDGE_DATATYPE_OPTION:
        {
          if (Option::NOREPEAT == newval) {
            isRepeatEnabled_ = false;
          } else {
            IOLOG_ERROR("KeyToKey::add unknown option:%d\n", newval);
          }
          break;
        }

        default:
          IOLOG_ERROR("KeyToKey::add invalid datatype:%d\n", datatype);
          break;
      }
    }


//Haci
//-------------------------------------------------------------
// 連打遅延対策のメインの処理を組み込む
//	2010.2.16〜3.2
//	2011.01.17(月)〜03.14(月)
    bool
    KeyToKey::remap(RemapParams& remapParams)
    {
      if (! toKeys_) return false;


//Haci
	  int ignore_vk_restore = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore);
		// VK_JIS_TEMPORARY_RESTOREを無視するチェックボックスのオンオフ値
		//2011.01.26(水)〜02.04(金)
	  int vk_restore_timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_parameter_keyinterval_timeout);
	  	// VK_JIS_TEMPORARY_RESTOREを無視してからタイムアウトするまでの時間(msec)
		//2011.02.04(金) name.xmlは一時的なインストールでは駄目なので､正式にインストールが必要。
	  int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
		// 2011.03.07(月) 入力モード変更遅延対策の中の「入力モード変更時の誤入力対策」を行うかどうかのチェックボックス。通常はオン。
	  KeyCode firstKey      = (*toKeys_)[0].key;	// 最初のtoKey
      Flags   firstKeyFlags = (*toKeys_)[0].flags;	// 最初のtoKeyFlags 2011.03.06(日)



      if (remapParams.isremapped) return false;
      if (! fromkeychecker_.isFromKey(remapParams.params.ex_iskeydown, remapParams.params.key, FlagStatus::makeFlags(), fromKey_.key, fromKey_.flags)) return false;
      remapParams.isremapped = true;


//Haci
///NG	  IOLockWrapper::ScopedLock lk(timer_restore2_.getlock());	//2011.02.09(水) 対で宣言する必要があるのかなということで。やはりすぐにキーボードがロックされてしまう。
	  if(toKeys_->size() > 0){
	  	// 2011.03.23(水) サイズが0でなければ。例えば､KeyOverlaidModifierで間違った指定をして､空の時に､代入してしまうとフリーズするので、ちゃんと判定。
	  	firstKey      = (*toKeys_)[0].key;		// 最初のtoKey
	  	firstKeyFlags = (*toKeys_)[0].flags;	// 最初のtoKeyFlags 2011.03.06(日)
	  } else {
	  	firstKey      = KeyCode::VK_NONE;
	  }



      // ------------------------------------------------------------
      // handle EventType & Modifiers

      // Let's consider the following setting.
      //   --KeyToKey-- KeyCode::SHIFT_R, ModifierFlag::SHIFT_R | ModifierFlag::NONE, KeyCode::A, ModifierFlag::SHIFT_R
      // In this setting, we need decrease SHIFT_R only once.
      // So, we transform values of fromKey_.
      //
      // [before]
      //   fromKey_.key   : KeyCode::SHIFT_R
      //   fromKey_.flags : ModifierFlag::SHIFT_R | ModifierFlag::NONE
      //
      // [after]
      //   fromKey_.key   : KeyCode::SHIFT_R
      //   fromKey_.flags : ModifierFlag::NONE
      //
      // Note: we need to apply this transformation after calling fromkeychecker_.isFromKey.

      Flags fromFlags = fromKey_.flags;
      fromFlags.remove(fromKey_.key.getModifierFlag());

      if (remapParams.params.ex_iskeydown) {
        FlagStatus::decrease(fromKey_.key.getModifierFlag());
      } else {
        FlagStatus::increase(fromKey_.key.getModifierFlag());
      }

      switch (toKeys_->size()) {
        case 0:
          break;

        case 1:		//Haci ここはtoKeyが1つだけのケース
        {
          EventType newEventType = remapParams.params.ex_iskeydown ? EventType::DOWN : EventType::UP;
          KeyCode toKey = (*toKeys_)[0].key;
          ModifierFlag toModifierFlag = toKey.getModifierFlag();

          if (toModifierFlag == ModifierFlag::NONE && ! VirtualKey::isKeyLikeModifier(toKey)) {
            // toKey
            FlagStatus::temporary_decrease(fromFlags);
            FlagStatus::temporary_increase((*toKeys_)[0].flags);

          } else {
            // toModifier or VirtualKey::isKeyLikeModifier
            if (toModifierFlag != ModifierFlag::NONE) {
              newEventType = EventType::MODIFY;
            }

            if (remapParams.params.ex_iskeydown) {
              FlagStatus::increase((*toKeys_)[0].flags | toModifierFlag);
              FlagStatus::decrease(fromFlags);
            } else {
              FlagStatus::decrease((*toKeys_)[0].flags | toModifierFlag);
              FlagStatus::increase(fromFlags);
            }
          }

          // ----------------------------------------
          Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(newEventType,
                                                                                         FlagStatus::makeFlags(),
                                                                                         toKey,
                                                                                         remapParams.params.keyboardType,
                                                                                         remapParams.params.repeat));
          if (! ptr) return false;
          Params_KeyboardEventCallBack& params = *ptr;


//Haci
		  if(remapParams.params.ex_iskeydown){	// キーダウン時 :2011.02.17(木)〜03.06(日)
			// リマップしていないキーはcore.cppでキーダウン時にモードを戻しているので､それに合わせる。
			if( firstKey == KeyCode::JIS_KANA || firstKey == KeyCode::JIS_EISUU){
			  // モード変更キー(JIS_KANA,JIS_EISUU)にリマップした場合(モード変更キーを指定するケースは大抵ここに来る)
			  // トグルキーはHandle_VK_JIS_TOGGLE_EISUU_KANA::handle内で処理
			  // VK_JIS_COMMAND_SPACEも同様｡
			  if(!learn_workspacedata){
			  	pass_initialize2_ = INIT_DO;
				// モード変更キーにリマップした場合(モード変更キーを指定するケースは大抵ここに来る)、
				// 次の文字キーが押された時にCore.cppでこの条件によって、wsd_publicとSavedIMDの初期化が実行されて全てがうまく行く｡
			  } else {	// 学習モードの時
			  	CommonData::replace_WSD(firstKey, firstKeyFlags);	//作業用のworkspacedataをすり替える。
	 		  	pass_initialize2_ = INIT_NOT;		// 次のキー入力時に更新の必要がないので。
			  }
          	} else if(firstKey != KeyCode::VK_JIS_TEMPORARY_KATAKANA &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_HIRAGANA &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_ROMAN    &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_AINU     &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_RESTORE  &&
					  case1_pass_restore2_ == 0 &&
					  ignore_vk_restore){
				// toKeyが1つだけのときに普通はVK_JIS_TEMPORARY系は指定しないので、大抵は､必ずここが実行される。
				// 例えば､半角スペースにリマップしたキーを押して変換してすぐに(RESTOREタイマーがタイムアウトする前に)、
				// KeyCode::RETURNをKeyCode::ENTERにリマップしているキーで確定すると､英字モードのままで戻らないので､ここでリストアされる。
				// また、Command+Spaceのようなシステムのショートカットでのモード変更の場合にも､Commandキーを押すとここで戻る。
				// case1_pass_restore2_は、今のところKeyOverlaidModifierのキーをダウンした場合にリストアしないようにするため。外部からオン/オフ制御を行う。
				// 2011.02.21(月)、03.02(水)、08(火)
///			  EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
			  vk_restore(remapParams.params, 0);		//2011.03.04(金) 汎用化関数を使用して、VK_JIS_TEMPORARY_RESTOREを挿入
			}

 			if(ignore_vk_restore || learn_workspacedata){
				// VK_RESTORE無視のとき、あるいはworkspacedata学習方式の場合は､タイマーを停止する。
				timer_restore2_.cancelTimeout();
				pass_initialize2_ = INIT_NOT;		// 次のキー入力時に更新の必要がないので。
			}
		  }



          if (remapParams.params.ex_iskeydown && ! isRepeatEnabled_) {
            KeyboardRepeat::cancel();
          } else {
            KeyboardRepeat::set(params);
          }
          EventOutputQueue::FireKey::fire(params);

          break;
        }

        default:
          KeyCode lastKey                  = (*toKeys_)[toKeys_->size() - 1].key;
          Flags lastKeyFlags               = (*toKeys_)[toKeys_->size() - 1].flags;
          ModifierFlag lastKeyModifierFlag = lastKey.getModifierFlag();
          bool isLastKeyModifier           = (lastKeyModifierFlag != ModifierFlag::NONE);
          bool isLastKeyLikeModifier       = VirtualKey::isKeyLikeModifier(lastKey);

          if (remapParams.params.ex_iskeydown) {	// キーダウン時 Haci


//Haci
			// -------------------
			if(firstKey != KeyCode::VK_JIS_TEMPORARY_KATAKANA &&
			   firstKey != KeyCode::VK_JIS_TEMPORARY_HIRAGANA &&
			   firstKey != KeyCode::VK_JIS_TEMPORARY_ROMAN    &&
			   firstKey != KeyCode::VK_JIS_TEMPORARY_AINU     &&
			   firstKey != KeyCode::VK_JIS_TEMPORARY_RESTORE  &&
	 		   ignore_vk_restore){
				// VK_JIS_TEMPORARY系キーが1番目に指定されていなければ、VK_JIS_TEMPORARY_RESTOREをキューの先頭に挿入し、モードを戻す｡
				// 遅延対策のチェックボックスが有効の場合のみ。戻すべきかどうかを下手に判定をするよりは、必ずリストアすればいい。
				// 10.2.16〜3.2,2011.01.17(月)〜3.11(金)
///           	EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, keyboardType);
///           	KeyboardRepeat::primitive_add_downup(  Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, keyboardType);	//これは必要?
 				vk_restore(remapParams.params, 0);		//2011.03.04(金) 汎用化関数を使用して、VK_JIS_TEMPORARY_RESTOREを挿入
			}



            KeyboardRepeat::cancel();

            FlagStatus::temporary_decrease(fromFlags);

            size_t size = toKeys_->size();
            // If the last key is modifier, we give it special treatment.
            // - Don't fire key repeat.
            // - Synchronous the key press status and the last modifier status.
            if (isLastKeyModifier || isLastKeyLikeModifier) {
              --size;
            }

            for (size_t i = 0; i < size; ++i) {
              FlagStatus::temporary_increase((*toKeys_)[i].flags);

              Flags f = FlagStatus::makeFlags();
              KeyboardType keyboardType = remapParams.params.keyboardType;


//Haci
			  	if(i == size -1 && (*toKeys_)[i].key == KeyCode::VK_JIS_TEMPORARY_RESTORE && ignore_vk_restore){
			  		// 最後のキーがVK_JIS_TEMPORARY_RESTOREであれば、いったん無視する。
			  		// 最後にVK_NONEを入れたりすることもあるかもしれないが､とりあえず普通のケースのみ。
			  		//2011.01.23(日)〜02.22(火)
			  		break;
			   	} else if( (*toKeys_)[i].key == KeyCode::JIS_KANA || (*toKeys_)[i].key == KeyCode::JIS_EISUU){
			      // VK_RESTORE無視、モードキー押下時遅延対策のチェックボックスのどちらでも。
			      // トグルキーはHandle_VK_JIS_TOGGLE_EISUU_KANA::handle側で処理
			   	  //2011.02.17(木)、03.08(火) 
				  if(!learn_workspacedata){
	 				pass_initialize2_ = INIT_DO;		//2011.03.01(火)
	 				//ここはtoKeyが2つ以上のケースなので、モード変更キーを指定することはまずないが、念の為。
	 				//ほとんどの場合は、toKeyが1つだけのCase1で処理される｡
				  } else { // 学習モードの時
				  	//2011.03.06(日)
			  		CommonData::replace_WSD((*toKeys_)[i].key, (*toKeys_)[i].flags);	// 作業用のworkspacedataをすり替える。
	 		  		pass_initialize2_ = INIT_NOT;		// 次のキー入力時に更新の必要がないので。
	 				//ここはtoKeyが2つ以上のケースなので、モード変更キーを指定することはまずないが、念の為。
	 				//ほとんどの場合は、toKeyが1つだけのCase1で処理される｡
	 			  }
			  	}



              EventOutputQueue::FireKey::fire_downup(f, (*toKeys_)[i].key, keyboardType);
              KeyboardRepeat::primitive_add_downup(f, (*toKeys_)[i].key, keyboardType);

              FlagStatus::temporary_decrease((*toKeys_)[i].flags);
            }

            if (isLastKeyModifier || isLastKeyLikeModifier) {
              // restore temporary flag.
              FlagStatus::temporary_increase(fromFlags);

              FlagStatus::increase(lastKeyFlags | lastKeyModifierFlag);
              FlagStatus::decrease(fromFlags);

              if (isLastKeyLikeModifier) {
                // Don't call EventOutputQueue::FireModifiers::fire here.
                //
                // Intentionally VK_LAZY_* stop sending MODIFY events.
                // EventOutputQueue::FireModifiers::fire destroys this behavior.
                Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(EventType::DOWN, FlagStatus::makeFlags(), lastKey, remapParams.params.keyboardType, false));
                if (ptr) {
                  EventOutputQueue::FireKey::fire(*ptr);
                }
              } else {
                EventOutputQueue::FireModifiers::fire();
              }
            }

            if (isLastKeyModifier || isLastKeyLikeModifier) {
              KeyboardRepeat::cancel();
            } else {
              if (isRepeatEnabled_) {
                keyboardRepeatID_ = KeyboardRepeat::primitive_start();
              } else {
                keyboardRepeatID_ = -1;
              }
            }

          } else {	// キーアップ時


//Haci
			if(ignore_vk_restore && lastKey == KeyCode::VK_JIS_TEMPORARY_RESTORE && workLoop_restore2_){
			  //2011.02.06(日)〜03.02(水)、03.11(金)
			  // VK_JIS_TEMPORARY_RESTOREを無視した場合のみ、RESTORE用として専用タイマーを再起動する｡
			  // コールバック関数の中の実行を変えるため。
			  // RESTOREを実行するのは見た目(画面右上の表示)の問題だけで、この処理がなくても次のキー入力時に元に戻るようにしてある。
				///timer_restore2_.cancelTimeout();	//2011.03.11(金) 念の為、タイマーを停止｡停止しなくても特に問題ないので､とりあえず外す｡
				callback2_ = CALLBACK_RESTORE;	// コールバック関数での条件(RESTORE実行)を指定
 				timer_restore2_.setTimeoutMS(vk_restore_timeout);	// 専用タイマーを起動
			} else if(ignore_vk_restore || learn_workspacedata) {	// タイマーを停止する｡
				timer_restore2_.cancelTimeout();
			}



            if (isLastKeyModifier || isLastKeyLikeModifier) {
              // For Lazy-Modifiers (KeyCode::VK_LAZY_*),
              // we need to handle these keys before restoring fromFlags, lastKeyFlags and lastKeyModifierFlag.
              // The unnecessary modifier events occur unless we do it.
              if (isLastKeyLikeModifier) {
                Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(EventType::UP, FlagStatus::makeFlags(), lastKey, remapParams.params.keyboardType, false));
                if (ptr) {
                  EventOutputQueue::FireKey::fire(*ptr);
                }
              }

              FlagStatus::decrease(lastKeyFlags | lastKeyModifierFlag);
              FlagStatus::increase(fromFlags);
              EventOutputQueue::FireModifiers::fire();

            } else {
              if (KeyboardRepeat::getID() == keyboardRepeatID_) {
                KeyboardRepeat::cancel();
              }
            }
          }
          break;
      }

      return true;
    }

    bool
    KeyToKey::call_remap_with_VK_PSEUDO_KEY(EventType eventType)
    {
      Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(eventType,
                                                                                     FlagStatus::makeFlags(),
                                                                                     KeyCode::VK_PSEUDO_KEY,
                                                                                     CommonData::getcurrent_keyboardType(),
                                                                                     false));
      if (! ptr) return false;
      Params_KeyboardEventCallBack& params = *ptr;

      RemapParams rp(params);
      return remap(rp);
    }


//Haci
	//-----------------------------------------------------------------------
	// キー入力する度に起動する専用タイマーに登録するcallbackルーチン
	// 別の2つのタイマー用も兼ねる｡タイマー自体は共通｡
	//	2010.2.24〜3.5, 2011.01.22(土)〜04.09(土)
      void
      KeyToKey::timeoutAfterKey2(OSObject* owner, IOTimerEventSource* sender)
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
			CommonData::init_seesaw();
		}
		
      }

//Haci
	//------------------------------------------------------------------------------
	// VK_JIS_TEMPORARY_RESTOREのキーダウン/アップを汎用化
	// 外部から起動する場合は､keytokey_.vk_restore(remapParams); のように記述する。
	// 内部から起動する場合は､vk_restore(remapParams); のように記述すればいい。
	// 外部からは、今のところ、KeyOverlaidModifierで、fromキーを押し続けてそのまま離した時に､モードを戻すために使用している。
	//	execute_fire00 = 1: fireコマンドも実行する。
	//				   = 0: キューに入れるだけ。
	//	2011.03.04(金)
    bool
    KeyToKey::vk_restore(const Params_KeyboardEventCallBack& params, int execute_fire00)
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



  }
}
