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
      	   (ignore_vk_restore || learn_workspacedata) && InputMode::JAPANESE == CommonData::getwsd_public().inputmode) {
		// チェックボックスの設定のいずれかがオンなら作業用のworkspacedataを使用。



        newkeycode_ = KeyCode::JIS_EISUU;
      } else {
        newkeycode_ = KeyCode::JIS_KANA;
      }


//Haci
	  bool result00 = false;	//2011.04.16(土)
	  if(learn_workspacedata){	// 学習機能の設定がチェックされていれば､作業用のworkspacedataをすり替える｡
		result00 = CommonData::replace_WSD(newkeycode_, ModifierFlag::NONE);
	  }
	  if(result00){	//学習済ですり替えられたので､次のキー入力時にCore.cppでの更新をしないようにする。
		RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);
	  } else {	//workspacedataの学習をしない設定の場合を含めて、未学習の場合も､次のキーの時に、作業用のworkspacedataを更新する必要がある｡
 	  	RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_DO);
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
    int skip00[CommonData::wsdMAX +1] = {0};	//2011.04.05(火)
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
		seesawType00 = CommonData::CUR_PRE;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_KANA){
		seesawType00 = CommonData::EISUU_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_OTHERS){
		seesawType00 = CommonData::KANA_OTHERS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_KANA_EISUU){		//2011.04.15(金)
		seesawType00 = CommonData::KANA_EISUU;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_SEESAW_EISUU_OTHERS){	//2011.04.15(金)
		seesawType00 = CommonData::EISUU_OTHERS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_NONE){ // リプレース
		skipType00 = CommonData::SKIP_NONE_PLUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_SKIP_PRE){
		skipType00 = CommonData::SKIP_PRE_PLUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_NONE){
		skipType00 = CommonData::SKIP_NONE_MINUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_MINUS_SKIP_PRE){
		skipType00 = CommonData::SKIP_PRE_MINUS;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA_EISUU){
		skipType00 = CommonData::SKIP_EISUU_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_KANA){	//2011.04.15(金)
		skipType00 = CommonData::SKIP_KANA;
	  } else if(key00 == KeyCode::VK_JIS_COMMAND_SPACE_REPLACE_PLUS_MINUS_SKIP_EISUU){	//2011.04.15(金)
		skipType00 = CommonData::SKIP_EISUU;
	  }

	  newflag_ = ModifierFlag::NONE;	//2011.04.01(金)
	  if(seesawType00 != -1){	// seesawの場合
		index00 = CommonData::get_index_for_seesaw_AtoB_WSD(seesawType00);	// 2011.04.10(日)統合

		if (seesawType00 == CommonData::EISUU_KANA   && index00 == CommonData::wsdEISU ||
			seesawType00 == CommonData::KANA_OTHERS  && index00 == CommonData::wsdHIRA ||
			seesawType00 == CommonData::KANA_EISUU   && index00 == CommonData::wsdHIRA ||
			seesawType00 == CommonData::EISUU_OTHERS && index00 == CommonData::wsdEISU){
			// 起点のモードに切り替えた時点からタイマーをかける｡
			// タイムアウト後は、フラグseesaw_init2_が立つので、
			// 次に同じseesawキーが押された時は､CommonData.hppの(後処理AA)が実行されて、モードは変化しない｡
			// これで、トグル系の最大の欠点がついに解消｡
			//2011.04.09(土)、15(金)
			RemapFunc::KeyToKey::static_setTimeoutMS_seesaw_init();
		}

	  } else {	// replaceの場合
		int sign00;	//符号= -1,1 
		  if(skipType00 == CommonData::SKIP_EISUU_KANA ||
		  	 skipType00 == CommonData::SKIP_KANA ||
		  	 skipType00 == CommonData::SKIP_EISUU){
		  	//2011.04.06(水)、15(金)、17(日)
			CommonData::reverse_sign_REPLACE_PLUS_MINUS(9);	// カウンターを進める。次の時に､方向を逆転させるため｡
		  }

		  if(skipType00 == CommonData::SKIP_NONE_MINUS ||
	  		 skipType00 == CommonData::SKIP_PRE_MINUS){
			sign00 = -1;
		  } else {	// REPLACE_MINUS,MINUS2の場合
		  	// 2011.04.06(水) REPLACE_PLUS_MINUS_SKIP系もとりあえず､正方向とするが、この値は用いない。
			sign00 =  1;
		  }


		if(!use_ainu){
		   skip00[CommonData::wsdAINU] = 1;	// AINUをスキップする。
											//2011.04.11(月) チェックボックスで設定するようにした｡
		}
		if(skipType00 == CommonData::SKIP_NONE_PLUS ||
	  		 skipType00 == CommonData::SKIP_NONE_MINUS){
			replace_num00 = 1;
		} else {	// 2011.04.05(火)
			if(skipType00 == CommonData::SKIP_EISUU_KANA){
				skip00[CommonData::wsdEISU] = 1;	// 英字モードをスキップ
				skip00[CommonData::wsdHIRA] = 1;	// ひらがなモード
				replace_num00 = 3;
			} else if(skipType00 == CommonData::SKIP_KANA){
				skip00[CommonData::wsdHIRA] = 1;	// ひらがなモード
				replace_num00 = 3;
			} else if(skipType00 == CommonData::SKIP_EISUU){
				skip00[CommonData::wsdEISU] = 1;	// 英字モード
				replace_num00 = 3;
			} else {	// SKIP_PRE_PLUS(MINUS)
				replace_num00 = 2;
				// スキップするのは前のモードなので､それは以下のget_index_for_replaceWSD関数の中で与えられる｡
			}
		}
		index00 = CommonData::get_index_for_replaceWSD(sign00, skip00, replace_num00);	//2011.04.01(金)、05(火)
	  }

    if (index00 == CommonData::wsdEISU) {					//英字
		// 2011.04.14(木) KeyCode::JIS_EISUU、ModifierFlag::NONE
    	newkeycode_ = KeyCode::JIS_COLON;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == CommonData::wsdHIRA) {			//ひらがな
		// 2011.04.14(木) KeyCode::JIS_KANA、ModifierFlag::NONE
    	newkeycode_ = KeyCode::J;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == CommonData::wsdKATA) {			//カタカナ
		// 2011.04.14(木) KeyCode::JIS_KANA、ModifierFlag::SHIFT_L
    	newkeycode_ = KeyCode::K;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == CommonData::wsdHKAT) {			//半角カタカナ
    	//2011.03.31(木)
    	newkeycode_ = KeyCode::SEMICOLON;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == CommonData::wsdFEIS) {			//全角英数
    	//2011.04.01(金)
    	newkeycode_ = KeyCode::L;
    	newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
    } else if (index00 == CommonData::wsdAINU) {			// AINU
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
		result00 = CommonData::replace_WSD(newkeycode_, newflag_);	//作業用のworkspacedataをすり替える｡
	  }
	  if(result00){	//学習済ですり替えられたので､次のキー入力時にCore.cppでの更新をしないようにする。
		RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);
	  } else {		//workspacedataの学習をしない設定の場合を含めて、未学習の場合も､次のキーの時に、作業用のworkspacedataを更新する必要がある｡
 	  	RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_DO);
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
			savedinputmodedetail_ = CommonData::getwsd_public().inputmodedetail;
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
		  CommonData::restore_WSD(savedinputmodedetail_);
		  RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_NOT);	//次のキー入力時にCore.cppでの更新をしないため
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
