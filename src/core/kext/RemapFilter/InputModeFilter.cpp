#include "base.hpp"
#include "bridge.h"
#include "CommonData.hpp"
#include "InputModeFilter.hpp"


#include "Config.hpp"
			//2011.02.09(水) Haci



namespace org_pqrs_KeyRemap4MacBook {
  namespace RemapFilter {
    InputModeFilter::InputModeFilter(unsigned int t) : type_(t)
    {
      targets_ = new Vector_FilterValue();
    }

    InputModeFilter::~InputModeFilter(void)
    {
      if (targets_) {
        delete targets_;
      }
    }

    void
    InputModeFilter::add(unsigned int newval)
    {
      if (! targets_) return;

      targets_->push_back(newval);
    }

    bool
    InputModeFilter::isblocked(void)
    {
      if (! targets_) return false;

      unsigned int current = 0;
      switch (type_) {
        case BRIDGE_FILTERTYPE_INPUTMODE_NOT:
        case BRIDGE_FILTERTYPE_INPUTMODE_ONLY:


//Haci
		  !(Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore) ||
		    Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata)) ? 
          	  current = CommonData::getcurrent_workspacedata().inputmode :
          	  current = CommonData::getwsd_public().inputmode;			// 2011.03.03(木)
			// 連打遅延対策あるいはモード変更キー押下時の遅延対策を切り替えるチェックボックスの設定がオンなら作業用のworkspacedataを使用。
			// 2011.03.08(火) 入力モード変更時の誤入力対策の判定も追加。



          break;

        case BRIDGE_FILTERTYPE_INPUTMODEDETAIL_NOT:
        case BRIDGE_FILTERTYPE_INPUTMODEDETAIL_ONLY:


//Haci
		  !(Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_ignore_vk_jis_temporary_restore) ||
		    Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata)) ? 
          	  current = CommonData::getcurrent_workspacedata().inputmodedetail :
          	  current = CommonData::getwsd_public().inputmodedetail;	// 2011.01.13(木)〜02.09(水)
			// 連打遅延対策あるいはモード変更キー押下時の遅延対策を切り替えるチェックボックスの設定がオンなら作業用のworkspacedataを使用。
			// 2011.03.08(火) 入力モード変更時の誤入力対策の判定も追加。



          break;
      }

      switch (type_) {
        case BRIDGE_FILTERTYPE_INPUTMODE_NOT:
        case BRIDGE_FILTERTYPE_INPUTMODEDETAIL_NOT:
        {
          for (size_t i = 0; i < targets_->size(); ++i) {
            if ((*targets_)[i] == current) {
              return true;
            }
          }
          return false;
        }

        case BRIDGE_FILTERTYPE_INPUTMODE_ONLY:
        case BRIDGE_FILTERTYPE_INPUTMODEDETAIL_ONLY:
        {
          for (size_t i = 0; i < targets_->size(); ++i) {
            if ((*targets_)[i] == current) {
              return false;
            }
          }
          return true;
        }

        default:
          IOLOG_ERROR("InputModeFilter::isblocked unknown type_(%d)\n", type_);
          break;
      }

      return false;
    }
  }
}
