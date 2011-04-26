#include "CommonData.hpp"
#include "Config.hpp"
#include "UserClient_kext.hpp"


#include "KeyToKey.hpp"
				//Haci 2011.03.14(月)



namespace org_pqrs_KeyRemap4MacBook {
  AbsoluteTime CommonData::current_ts_;
  KeyboardType CommonData::current_keyboardType_;
  DeviceVendor CommonData::current_deviceVendor_;
  DeviceProduct CommonData::current_deviceProduct_;
  BridgeWorkSpaceData CommonData::current_workspacedata_;


//Haci
//KeyRemap4MacBook_bridge::GetWorkspaceData::Reply CommonData::wsd_public_;
//KeyRemap4MacBook_bridge::GetWorkspaceData::Reply CommonData::wsd_save_[7];
  BridgeWorkSpaceData CommonData::wsd_public_; 		//2011.01.13(木) 作業用のworkspacedata
  BridgeWorkSpaceData CommonData::wsd_save_[7];		//2011.03.05(土) workspacedataの学習用
  int CommonData::pre_index2_ = -1;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
  int CommonData::cur_index2_ = -1;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
  int CommonData::others_index2_ = -1;				//2011.04.05(火) VK_JIS_COMMAND_SPACE用
  int CommonData::sign_plus_minus2_ = -99;			//2011.04.06(水) REPLACE_PLUS_MINUS用の符号
  int CommonData::counter_plus_minus2_ = 0;			//2011.04.06(水) REPLACE_PLUS_MINUS用の連続実行をカウントする。
  int CommonData::pre_counter_plus_minus2_ = 0;		//2011.04.06(水) REPLACE_PLUS_MINUS用のリマップ前の上記カウンター値
  bool CommonData::seesaw_init2_ = false;			//2011.04.09(土) = 1:タイムアウト後にseesawの起点にする。



  char CommonData::statusmessage_[BRIDGE_USERCLIENT_STATUS_MESSAGE__END__][BRIDGE_USERCLIENT_STATUS_MESSAGE_MAXLEN];

  int CommonData::alloccount_;

  bool
  CommonData::initialize(void)
  {
    for (int i = 0; i < BRIDGE_USERCLIENT_STATUS_MESSAGE__END__; ++i) {
      statusmessage_[i][0] = '\0';
    }

    return true;
  }

  void
  CommonData::terminate(void)
  {}


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
  CommonData::reverse_sign_REPLACE_PLUS_MINUS(int when00)
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
  CommonData::get_WSDindex(InputModeDetail imd00, KeyCode modekey00, Flags flag00){
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
// 2011.01.13(木)〜18(火)、03.05(土)〜15(火)、04.16(土)
  bool
  CommonData::control_WSD(int mode00, KeyCode modekey00, Flags flag00, InputModeDetail IMD00)
  {
    int learn_workspacedata = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_learn_workspacedata);
				// 入力モード変更遅延対策の中の「モード変更時の誤入力対策」を行うかどうかのチェックボックス。通常はオン。
	InputModeDetail IMDsv;
	int index00 = 1;	// 多次元配列のインデックス値

	if(mode00 == 0){	// 更新のみ
      wsd_public_ = current_workspacedata_;

	  index00 = IMD2index(wsd_public_.inputmodedetail);	// 現在値 2011.03.29(火)
	  set_new_index(index00);	// シーソー用:2011.03.29(火)
	  return true;
	}

	if(!(learn_workspacedata  && (mode00 == 1 || mode00 == 2 || mode00 == 3))) return false;

	// 入力モード値を得る
	if(mode00 == 1){	// 学習
		index00 = IMD2index(current_workspacedata_.inputmodedetail);	//現在値 2011.03.29(火)
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
	  	wsd_save_[wsdSET] = current_workspacedata_;	// 現在値を保存(この値に意味は無い)。これによりこれ以降はここには入らない。
	  	RemapFunc::KeyToKey::static_set_pass_initialize(RemapFunc::KeyToKey::INIT_DO);	// すぐに初期化するため。
		set_indexes_directly(-1, index00, -1);	//2011.04.11(月)
	  }
	  wsd_save_[index00] = current_workspacedata_;	//学習
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



  void
  CommonData::clear_statusmessage(int index)
  {
    if (index <= BRIDGE_USERCLIENT_STATUS_MESSAGE_NONE) return;
    if (index >= BRIDGE_USERCLIENT_STATUS_MESSAGE__END__) return;

    statusmessage_[index][0] = '\0';
  }

  void
  CommonData::append_statusmessage(int index, const char* message)
  {
    if (index <= BRIDGE_USERCLIENT_STATUS_MESSAGE_NONE) return;
    if (index >= BRIDGE_USERCLIENT_STATUS_MESSAGE__END__) return;

    strlcat(statusmessage_[index], message, sizeof(statusmessage_[index]));
  }

  void
  CommonData::send_notification_statusmessage(int index)
  {
    if (index <= BRIDGE_USERCLIENT_STATUS_MESSAGE_NONE) return;
    if (index >= BRIDGE_USERCLIENT_STATUS_MESSAGE__END__) return;

    org_pqrs_driver_KeyRemap4MacBook_UserClient_kext::send_notification_to_userspace(BRIDGE_USERCLIENT_NOTIFICATION_TYPE_STATUS_MESSAGE_UPDATED, index);
  }

  const char*
  CommonData::get_statusmessage(int index)
  {
    if (index <= BRIDGE_USERCLIENT_STATUS_MESSAGE_NONE) return NULL;
    if (index >= BRIDGE_USERCLIENT_STATUS_MESSAGE__END__) return NULL;

    return statusmessage_[index];
  }

  void
  CommonData::increase_alloccount(void)
  {
    ++alloccount_;
    if (alloccount_ > 1024) {
      IOLOG_WARN("alloccount_ > 1024\n");
    }
    //IOLOG_DEVEL("CommonData::increase_alloccount alloccount_:%d\n", alloccount_);
  }

  void
  CommonData::decrease_alloccount(void)
  {
    --alloccount_;
    //IOLOG_DEVEL("CommonData::decrease_alloccount alloccount_:%d\n", alloccount_);
  }
}
