#include "CommonData.hpp"
#include "EventOutputQueue.hpp"
#include "KeyToKey.hpp"
#include "KeyboardRepeat.hpp"
#include "VirtualKey.hpp"


#include "Config.hpp"
			//2011.01.26(水) Haci



namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {

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
			  	VirtualKey::set_pass_initialize(VirtualKey::INIT_DO);
				// モード変更キーにリマップした場合(モード変更キーを指定するケースは大抵ここに来る)、
				// 次の文字キーが押された時にCore.cppでこの条件によって、wsd_publicとSavedIMDの初期化が実行されて全てがうまく行く｡
			  } else {	// 学習モードの時
			  	VirtualKey::replace_WSD(firstKey, firstKeyFlags);	//作業用のworkspacedataをすり替える。
			  	VirtualKey::set_pass_initialize(VirtualKey::INIT_NOT);		// 次のキー入力時に更新の必要がないので。
			  }
          	} else if(firstKey != KeyCode::VK_JIS_TEMPORARY_KATAKANA &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_HIRAGANA &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_ROMAN    &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_AINU     &&
					  firstKey != KeyCode::VK_JIS_TEMPORARY_RESTORE  &&
					  VirtualKey::get_case1_pass_restore() == 0 &&
					  ignore_vk_restore){
				// toKeyが1つだけのときに普通はVK_JIS_TEMPORARY系は指定しないので、大抵は､必ずここが実行される。
				// 例えば､半角スペースにリマップしたキーを押して変換してすぐに(RESTOREタイマーがタイムアウトする前に)、
				// KeyCode::RETURNをKeyCode::ENTERにリマップしているキーで確定すると､英字モードのままで戻らないので､ここでリストアされる。
				// また、Command+Spaceのようなシステムのショートカットでのモード変更の場合にも､Commandキーを押すとここで戻る。
				// case1_pass_restore2_は、今のところKeyOverlaidModifierのキーをダウンした場合にリストアしないようにするため。外部からオン/オフ制御を行う。
				// 2011.02.21(月)、03.02(水)、08(火)
///			  EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
			  VirtualKey::vk_restore(remapParams.params, 0);		//2011.03.04(金) 汎用化関数を使用して、VK_JIS_TEMPORARY_RESTOREを挿入
			}

 			if(ignore_vk_restore || learn_workspacedata){
				// VK_RESTORE無視のとき、あるいはworkspacedata学習方式の場合は､タイマーを停止する。
				VirtualKey::static_cancelTimeout();
				VirtualKey::set_pass_initialize(VirtualKey::INIT_NOT);		// 次のキー入力時に更新の必要がないので。
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
 				VirtualKey::vk_restore(remapParams.params, 0);		//2011.03.04(金) 汎用化関数を使用して、VK_JIS_TEMPORARY_RESTOREを挿入
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
	 				VirtualKey::set_pass_initialize(VirtualKey::INIT_DO);		//2011.03.01(火)
	 				//ここはtoKeyが2つ以上のケースなので、モード変更キーを指定することはまずないが、念の為。
	 				//ほとんどの場合は、toKeyが1つだけのCase1で処理される｡
				  } else { // 学習モードの時
				  	//2011.03.06(日)
			  		VirtualKey::replace_WSD((*toKeys_)[i].key, (*toKeys_)[i].flags);	// 作業用のworkspacedataをすり替える。
	 		  		VirtualKey::set_pass_initialize(VirtualKey::INIT_NOT);		// 次のキー入力時に更新の必要がないので。
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
			if(ignore_vk_restore && lastKey == KeyCode::VK_JIS_TEMPORARY_RESTORE){
			  //2011.02.06(日)〜03.02(水)、03.11(金)
			  // VK_JIS_TEMPORARY_RESTOREを無視した場合のみ、RESTORE用として専用タイマーを再起動する｡
			  // コールバック関数の中の実行を変えるため。
			  // RESTOREを実行するのは見た目(画面右上の表示)の問題だけで、この処理がなくても次のキー入力時に元に戻るようにしてある。
				///timer_restore2_.cancelTimeout();	//2011.03.11(金) 念の為、タイマーを停止｡停止しなくても特に問題ないので､とりあえず外す｡
				VirtualKey::static_setTimeoutMS(VirtualKey::CALLBACK_RESTORE);
			} else if(ignore_vk_restore || learn_workspacedata) {	// タイマーを停止する｡
				VirtualKey::static_cancelTimeout();
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
  }
}
