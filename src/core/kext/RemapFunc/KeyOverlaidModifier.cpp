#include "Config.hpp"
#include "EventWatcher.hpp"
#include "KeyOverlaidModifier.hpp"
#include "VirtualKey.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFunc {
    TimerWrapper KeyOverlaidModifier::firerepeat_timer_;
    KeyOverlaidModifier* KeyOverlaidModifier::target_ = NULL;

    void
    KeyOverlaidModifier::static_initialize(IOWorkLoop& workloop)
    {
      firerepeat_timer_.initialize(&workloop, NULL, KeyOverlaidModifier::firerepeat_timer_callback);
    }

    void
    KeyOverlaidModifier::static_terminate(void)
    {
      firerepeat_timer_.terminate();
    }

    KeyOverlaidModifier::KeyOverlaidModifier(void) :
      index_(0), isAnyEventHappen_(false), savedflags_(0), isRepeatEnabled_(false)
    {
      ic_.begin();
    }

    KeyOverlaidModifier::~KeyOverlaidModifier(void)
    {
      if (target_ == this) {
        firerepeat_timer_.cancelTimeout();
        target_ = NULL;
      }
    }

    void
    KeyOverlaidModifier::add(unsigned int datatype, unsigned int newval)
    {
      switch (datatype) {
        case BRIDGE_DATATYPE_KEYCODE:
        {
          switch (index_) {
            case 0:
              keytokey_.add(KeyCode(newval));
              keytokey_fire_.add(KeyCode::VK_PSEUDO_KEY);
              break;

            case 1:
              toKey_.key = newval;
              keytokey_.add(KeyCode(newval));
              break;

            default:
              keytokey_fire_.add(KeyCode(newval));
              break;
          }
          ++index_;

          break;
        }

        case BRIDGE_DATATYPE_FLAGS:
        {
          switch (index_) {
            case 0:
              IOLOG_ERROR("Invalid KeyOverlaidModifier::add\n");
              break;

            case 1:
              keytokey_.add(Flags(newval));
              break;

            case 2:
              toKey_.flags = newval;
              keytokey_.add(Flags(newval));
              break;

            default:
              keytokey_fire_.add(Flags(newval));
              break;
          }
          break;
        }

        case BRIDGE_DATATYPE_OPTION:
        {
          if (Option::KEYOVERLAIDMODIFIER_REPEAT == newval) {
            isRepeatEnabled_ = true;
          } else {
            IOLOG_ERROR("KeyOverlaidModifier::add unknown option:%d\n", newval);
          }
          break;
        }

        default:
          IOLOG_ERROR("KeyOverlaidModifier::add invalid datatype:%d\n", datatype);
          break;
      }
    }

    bool
    KeyOverlaidModifier::remap(RemapParams& remapParams)
    {
      bool savedIsAnyEventHappen = isAnyEventHappen_;


//Haci
	  VirtualKey::static_set_case1_pass_restore(1);	//2011.03.02(水)
		// KeyToKey(Case1)でリストア(モードを戻す)しないようにする。
		// KeyOverlaidModifierはKeyToKeyを最大2回実行するという特殊な使い方をするが、
		// 最初に以下のkeytokey_.remap(remapParams)で、KeyToKeyのCase1に先ずは行くので、そこでモードを戻さないようにフラグを立てる。
		// そして、checkbox.xmlに他の設定があると､ここを何度も通ってしまうので､その時はresultが偽になるので、やはり、フラグを一旦落とす必要がある。
		// 本関数のタイムアウト前にキーアップすると、本関数の後半部分の処理で､toKeys_fire群(VK系が含まれることも)がKeyToKeyのdefaultで実行される。



      bool result = keytokey_.remap(remapParams);


//Haci
	  VirtualKey::static_set_case1_pass_restore(0);	//2011.03.03(木)
	    	// KeyToKey(Case1)でリストアを実行できる状態に戻す。



      if (! result) return false;

      // ------------------------------------------------------------
      if (remapParams.params.ex_iskeydown) {
        EventWatcher::set(isAnyEventHappen_);
        ic_.begin();

        // ----------------------------------------
        // We store the flags when KeyDown.
        // Because it lets you make a natural input when the following sequence.
        //
        // ex. "Space to Shift (when type only, send Space)"
        // (1) KeyDown Command_L
        // (2) KeyDown Space        save flags (Command_L)
        // (3) KeyUp   Command_L
        // (4) KeyUp   Space        fire Command_L+Space

        FlagStatus::temporary_decrease(toKey_.flags | toKey_.key.getModifierFlag() | Handle_VK_LAZY::getModifierFlag(toKey_.key));
        savedflags_ = FlagStatus::makeFlags();
        FlagStatus::temporary_increase(toKey_.flags | toKey_.key.getModifierFlag() | Handle_VK_LAZY::getModifierFlag(toKey_.key));

        // ----------------------------------------
        if (isRepeatEnabled_) {
          target_ = this;
          isfirenormal_ = false;
          isfirerepeat_ = false;
          firerepeat_timer_.setTimeoutMS(Config::get_keyoverlaidmodifier_initial_wait());
        }

      } else {
        firerepeat_timer_.cancelTimeout();

        if (isfirerepeat_) {
          FlagStatus::ScopedTemporaryFlagsChanger stfc(savedflags_);
          keytokey_fire_.call_remap_with_VK_PSEUDO_KEY(EventType::UP);

        } else {
          isfirenormal_ = true;

          if (savedIsAnyEventHappen == false) {
            int timeout = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_parameter_keyoverlaidmodifier_timeout);
            if (timeout <= 0 || ic_.checkThreshold(timeout) == false) {
              FlagStatus::ScopedTemporaryFlagsChanger stfc(savedflags_);

              keytokey_fire_.call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);
              keytokey_fire_.call_remap_with_VK_PSEUDO_KEY(EventType::UP);


//Haci
            } else if(!Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_remap_jis_ignore_improvement_IM_changing)) {
            	//タイムアウト後にfromKeyを押し続けて何もしないままキーアップしてそのキーが捨てられるので、ここでは元々は何もしないが、
				// IM改良無効のチェックボックスがオフの場合は､リストアを実行する。
				// これは、最初のkeytokey_.remapのところでリストアしないので､機能キーを捨てる時は、リストアしなければならない。
				// これが必要な例は､「3」キーを半角にリマップするキーを押して、モードが元に戻る前に､
				// KeyOverlaidModifierのキーを押すと、押している間はモードが戻らず､キーアップしてもここでリストアしないと､そのままモードは戻らない。
            	// 2011.03.03(木)〜04(金)
///    			EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, remapParams.params.keyboardType);
          		VirtualKey::vk_restore(remapParams.params, 1);	//2011.03.04(金) 関数化: 引数は0でも1でもなぜか同じ。



            }
          }
          EventWatcher::unset(isAnyEventHappen_);
        }
      }

      return true;
    }

    void
    KeyOverlaidModifier::firerepeat_timer_callback(OSObject* owner, IOTimerEventSource* sender)
    {
      if (! target_) return;

      if (target_->isAnyEventHappen_) return;

      if (! target_->isfirenormal_) {
        target_->isfirerepeat_ = true;

        FlagStatus::ScopedTemporaryFlagsChanger stfc(target_->savedflags_);
        (target_->keytokey_fire_).call_remap_with_VK_PSEUDO_KEY(EventType::DOWN);
      }
    }
  }
}
