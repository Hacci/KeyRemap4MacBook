#ifndef COMMONDATA_HPP
#define COMMONDATA_HPP

#include "base.hpp"
#include "KeyCode.hpp"
#include "bridge.h"
#include <string.h>

namespace org_pqrs_KeyRemap4MacBook {
  class CommonData {
  public:
    static bool initialize(void);
    static void terminate(void);

    static void setcurrent_ts(const AbsoluteTime& ts) { current_ts_ = ts; }
    static const AbsoluteTime& getcurrent_ts(void) { return current_ts_; }

    static void setcurrent_keyboardType(KeyboardType keyboardType) { current_keyboardType_ = keyboardType; }
    static KeyboardType getcurrent_keyboardType(void) { return current_keyboardType_; }
    static void setcurrent_vendorProduct(DeviceVendor vendor, DeviceProduct product) {
      current_deviceVendor_ = vendor;
      current_deviceProduct_ = product;
    }
    static bool isEqualVendorProduct(DeviceVendor vendor, DeviceProduct product) {
      if (current_deviceVendor_  != vendor) return false;
      if (current_deviceProduct_ != product) return false;
      return true;
    }

    static void setcurrent_workspacedata(const BridgeWorkSpaceData& newval) { current_workspacedata_ = newval; }
    static const BridgeWorkSpaceData& getcurrent_workspacedata(void) { return current_workspacedata_; }

    static void clear_statusmessage(int index);
    static void append_statusmessage(int index, const char* message);
    static void send_notification_statusmessage(int index);
    static const char* get_statusmessage(int index);

    // for debug
    static void increase_alloccount(void);
    static void decrease_alloccount(void);

  private:
    static AbsoluteTime current_ts_;
    static KeyboardType current_keyboardType_;
    static DeviceVendor current_deviceVendor_;
    static DeviceProduct current_deviceProduct_;
    static BridgeWorkSpaceData current_workspacedata_;
    static char statusmessage_[BRIDGE_USERCLIENT_STATUS_MESSAGE__END__][BRIDGE_USERCLIENT_STATUS_MESSAGE_MAXLEN];

    static int alloccount_;
  };
}

#endif
