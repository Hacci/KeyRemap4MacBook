#include "CommonData.hpp"
#include "GlobalLock.hpp"
#include "TimerWrapper.hpp"


#include "Config.hpp"
			// Haci 2011.03.07(月)



#define super    OSObject
OSDefineMetaClassAndStructors(org_pqrs_KeyRemap4MacBook_TimerWrapperObject, OSObject);

org_pqrs_KeyRemap4MacBook_TimerWrapperObject*
org_pqrs_KeyRemap4MacBook_TimerWrapperObject::timerEventSource(OSObject* owner, IOTimerEventSource::Action action)
{
  org_pqrs_KeyRemap4MacBook_TimerWrapperObject* p = new org_pqrs_KeyRemap4MacBook_TimerWrapperObject;

  if (p && ! p->init(owner, action)) {
    if (p) p->release();
    return NULL;
  }
  return p;
}

bool
org_pqrs_KeyRemap4MacBook_TimerWrapperObject::init(OSObject* owner, IOTimerEventSource::Action action)
{
  if (! super::init()) {
    return false;
  }

  owner_ = owner;
  action_ = action;
  active_ = false;

  return true;
}

void
org_pqrs_KeyRemap4MacBook_TimerWrapperObject::free(void)
{
  super::free();
  return;
}

#undef super


//Haci
// RESTORE専用クラス用(初期化用、VK_JIS_COMMAND_SPACEの起点用を兼ねる)
// 2011.01.24(月) ､04.26(火)
#define super_restore    OSObject
OSDefineMetaClassAndStructors(org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject, OSObject);

org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject*
org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject::timerEventSource(OSObject* owner, IOTimerEventSource::Action action)
{
  org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject* p = new org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject;

  if (p && ! p->init(owner, action)) {
    if (p) p->release();
    return NULL;
  }
  return p;
}

bool
org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject::init(OSObject* owner, IOTimerEventSource::Action action)
{
  if (! super_restore::init()) {
    return false;
  }

  owner_ = owner;
  action_ = action;
  active_ = false;

  return true;
}

void
org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject::free(void)
{
  super_restore::free();
  return;
}

#undef super_restore



// ================================================================================
namespace org_pqrs_KeyRemap4MacBook {
  void
  TimerWrapper::initialize(IOWorkLoop* wl, OSObject* owner, IOTimerEventSource::Action func)
  {
    if (timer_) terminate();

    if (! wl) return;

    workloop_ = wl;

    object_ = org_pqrs_KeyRemap4MacBook_TimerWrapperObject::timerEventSource(owner, func);
    if (! object_) return;

    timer_ = IOTimerEventSource::timerEventSource(object_, callback_);

    if (workloop_->addEventSource(timer_) != kIOReturnSuccess) {
      IOLOG_ERROR("TimerWrapper addEventSource failed\n");
      timer_->release();
      timer_ = NULL;
    }
  }

  void
  TimerWrapper::terminate(void)
  {
    if (timer_) {
      timer_->cancelTimeout();
      if (workloop_) {
        workloop_->removeEventSource(timer_);
      }
      timer_->release();
      timer_ = NULL;
    }
    workloop_ = NULL;

    if (object_) {
      object_->release();
      object_ = NULL;
    }
  }

  IOReturn
  TimerWrapper::setTimeoutMS(UInt32 ms, bool overwrite)
  {
    if (! object_) return kIOReturnError;

    if (! timer_) {
      return kIOReturnNoResources;
    }
    if (! overwrite && object_->isActive()) {
      return kIOReturnSuccess;
    }

    object_->setActive(true);
    IOReturn retval = timer_->setTimeoutMS(ms);
    if (retval != kIOReturnSuccess) {
      IOLOG_WARN("setTimeoutMS is failed\n");
      object_->setActive(false);
    }
    return retval;
  }

  void
  TimerWrapper::cancelTimeout(void)
  {
    if (! object_) return;

    if (timer_) {
      timer_->cancelTimeout();
    }
    object_->setActive(false);
  }

  void
  TimerWrapper::callback_(OSObject* owner, IOTimerEventSource* sender)
  {
    GlobalLock::ScopedLock lk;
    if (! lk) return;

    org_pqrs_KeyRemap4MacBook_TimerWrapperObject* object = OSDynamicCast(org_pqrs_KeyRemap4MacBook_TimerWrapperObject, owner);
    if (! object) return;

    object->setActive(false);
    (object->getaction())(object->getowner(), sender);
  }


//Haci
//--------------------------
// RESTORE専用クラス用(初期化用、VK_JIS_COMMAND_SPACEの起点用を兼ねる)
// 2011.01.29(土)､04.26(火)
  void
  TimerWrapperRestore::initialize(IOWorkLoop* wl, OSObject* owner, IOTimerEventSource::Action func)
  {
    if (timer_) terminate();

    if (! wl) return;

    workloop_ = wl;

    object_ = org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject::timerEventSource(owner, func);
    if (! object_) return;

    timer_ = IOTimerEventSource::timerEventSource(object_, callback_);

    if (workloop_->addEventSource(timer_) != kIOReturnSuccess) {
      IOLOG_ERROR("TimerWrapperRestore addEventSource failed\n");
      timer_->release();
      timer_ = NULL;
    }
  }

  void
  TimerWrapperRestore::terminate(void)
  {
    if (timer_) {
      timer_->cancelTimeout();
      if (workloop_) {
        workloop_->removeEventSource(timer_);
      }
      timer_->release();
      timer_ = NULL;
    }
    workloop_ = NULL;

    if (object_) {
      object_->release();
      object_ = NULL;
    }
  }

  IOReturn
  TimerWrapperRestore::setTimeoutMS(UInt32 ms, bool overwrite)
  {
    if (! object_) return kIOReturnError;

    if (! timer_) {
      return kIOReturnNoResources;
    }
    if (! overwrite && object_->isActive()) {
      return kIOReturnSuccess;
    }

    object_->setActive(true);
    IOReturn retval = timer_->setTimeoutMS(ms);
    if (retval != kIOReturnSuccess) {
      IOLOG_WARN("setTimeoutMS is failed\n");
      object_->setActive(false);
    }
    return retval;
  }

  void
  TimerWrapperRestore::cancelTimeout(void)
  {
    if (! object_) return;

    if (timer_) {
      timer_->cancelTimeout();
    }
    object_->setActive(false);
  }

  void
  TimerWrapperRestore::callback_(OSObject* owner, IOTimerEventSource* sender)
  {
    GlobalLock::ScopedLock lk;
    if (! lk) return;

    org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject* object = OSDynamicCast(org_pqrs_KeyRemap4MacBook_TimerWrapperRestoreObject, owner);
    if (! object) return;

    object->setActive(false);
    (object->getaction())(object->getowner(), sender);
  }



}
