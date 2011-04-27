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
        VirtualKey::static_initialize(*workLoop);	//2011.02.01(火)〜10(木)



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
        VirtualKey::static_terminate();	// 2011.02.01(火)〜09(水) これがないとmake reloadで時々解放エラーになる。



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
	  //	10.2.15〜16 関数化, 2011.01.13(木)〜02.05(土)、04.27(水)
	  //**************************************************************
	  VirtualKey::ControlWorkspaceData(params, VirtualKey::POST_REMAP);



      // ------------------------------------------------------------
      RemapParams remapParams(params);

      // ------------------------------------------------------------
      FlagStatus::set(params.key, params.flags);

      RemapClassManager::remap_key(remapParams);


//Haci
	  //--------------------------------------------------------------
	  // リマップ直後
	  //	2011.04.06(水)、04.27(水)
	  //**************************************************************
	  VirtualKey::ControlWorkspaceData(params, VirtualKey::JUST_AFTER_REMAP);



      // ------------------------------------------------------------
      if (! remapParams.isremapped) {
        Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType, FlagStatus::makeFlags(), params.key,
                                                                                       params.charCode, params.charSet, params.origCharCode, params.origCharSet,
                                                                                       params.keyboardType, false));


//Haci
		// 全くリマップ指定のないキーはここに来る。
	    VirtualKey::ControlWorkspaceData(params, VirtualKey::NON_REMAPPED);
			// 全くリマップ指定のないキーの場合は、
			//  ・キーダウン時にモードを戻し(1つ前のキーがVK系のときのため)、
			//  ・モード変更キーが押されたのであれば､キーアップ時に作業用のworkspacedataの更新を行う｡
			// 2010.2.16〜27,2011.01.20(木)、04.27(水)



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
