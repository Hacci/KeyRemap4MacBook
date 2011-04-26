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


//Haci
    static void reverse_sign_REPLACE_PLUS_MINUS(int when00);	//2011.04.06(水)
    static const BridgeWorkSpaceData& getwsd_public(void) { return wsd_public_; }  //2011.01.13(木)
    static bool control_WSD(int mode00, KeyCode modekey00, Flags flag00, InputModeDetail IMDsaved00);	//2011.03.10(木)
    static bool update_WSD(void){	// WSD(作業用のworkspacedata)の更新のみ
			return control_WSD(0, KeyCode::VK_NONE, ModifierFlag::NONE, InputModeDetail::UNKNOWN);
    };
    static bool learn_WSD(void){	// WSDの学習
			return control_WSD(1, KeyCode::VK_NONE, ModifierFlag::NONE, InputModeDetail::UNKNOWN);
    };
    static bool replace_WSD(KeyCode modekey00, Flags flag00){	// JIS_EISUUなどを指定し、該当のWSDを学習してあればそれを利用してすり替える。
			return control_WSD(2, modekey00, flag00, InputModeDetail::UNKNOWN);
    };
    static bool restore_WSD(InputModeDetail IMDsaved00){ // 元の入力モードに戻すためWSDを元に戻す｡
			return control_WSD(3, KeyCode::VK_NONE, ModifierFlag::NONE, IMDsaved00);
    };
    enum {
		//学習用のwsd_save_[]のインデックス値
    	// 2011.03.14(月)、31(木)
     	// 2011.04.08(金) 並び順を変更。先頭の英字モードから見て逆順、つまり、最後尾に全角英数を配置する。今後モードが増えた場合も同様。
     	//				  ひらがなモードから見て､順方向にカタカナ､半角カタカナの順に配置する。
      	wsdSET   = 0,	// 最初にセットしたものを入れる。このIMDが0であれば、1つもセットされていないことを意味するので､初期化させる。
      	wsdEISU  = 1,	//英字モード
      	wsdHIRA  = 2,	//ひらがなモード
      	wsdKATA  = 3,	//カタカナモード
      	wsdHKAT  = 4,	//半角カタカナモード
      	wsdAINU  = 5,	//アイヌモード
      	wsdFEIS  = 6,	//全角英数モード: 先頭の英字モードから見て逆順､つまり最後尾に必ず配置。
      	wsdMAX   = 6,	// 総数(wsdSETを除く)

      	//seesawの種類
      	CUR_PRE      = 0,
      	EISUU_KANA   = 1,
      	KANA_OTHERS  = 2,
      	KANA_EISUU   = 3,	//2011.04.15(金)
      	EISUU_OTHERS = 4,	//2011.04.15(金)
      	//skipの種類	
      	// 2011.04.15(金)
      	SKIP_NONE_PLUS    = 0,
      	SKIP_NONE_MINUS   = 1,
      	SKIP_PRE_PLUS     = 2,
      	SKIP_PRE_MINUS    = 3,
      	SKIP_EISUU_KANA   = 4,
      	SKIP_KANA         = 5,
      	SKIP_EISUU        = 6,
    };

//Haci
	//---------------------
	// インデックス値pre_index2_、cur_index2_、others_index2_をセット
	// 引数 = -1: 設定しない場合
	//	2011.04.11(月) 関数化
    static void set_indexes_directly(int new_pre, int new_cur, int new_others){
		if(new_pre > 0){	// -1を与えれば設定されない｡値0は無いので｡
		  pre_index2_  = new_pre;		// 新たな値を設定
		}
		if(new_cur > 0){
		  cur_index2_ = new_cur;
		}
		if(new_others > 0){
		  others_index2_ = new_others;
		}
    }

//Haci
	//--------------------------------------------------------------------
	// 新しいインデックス値をcur_index2_に設定し､それまでの値をpre_index2_に保存｡
	// 外部でも使用するかと思ったが､今のところ内部のみ
	// 2011.03.29(火)、04.11(月)
    static void set_new_index(int index00){
		if(cur_index2_ != -1 && pre_index2_ != -1){	//両方とも設定済の場合
		  if(cur_index2_ != index00){ //現在値と異なれば、今までの現在値がpre値で、今回の値が現在値
			set_indexes_directly(cur_index2_, index00, -1);	//2011.04.11(月)
		  }
		} else if(cur_index2_ == -1){ // 現在値が未保存の場合
		  if(pre_index2_ != index00){
			set_indexes_directly(-1, index00, -1);	//pre値と異なればcur値として保存。
		  }
		} else {	// 両方とも未設定の場合、pre値に設定する｡
			set_indexes_directly(index00, -1, -1);
		}
    };

//Haci
	//-------------------------------
	// タイムアウト時のコールバック関数から呼ぶ。
	// このフラグが立つと､seesaw系のキーを次に押した場合､seesawの起点に戻る。既に起点になっていれば変化しない｡
	// KeyToKeyで使用する｡
	// 2011.04.09(土)
    static void init_seesaw(void){
		seesaw_init2_ = true;
    };

//Haci
	//-------------------------------------------------
	// type00 = CUR_PRE (値0)    : 現在値(可変)←→1つ前の値(可変)間のシーソー切替
	// type00 = EISUU_KANA (値1) : 英字(固定かつ起点)←→ひらがな(固定)間のシーソー切替
	// type00 = KANA_OTHERS(値2) : ひらがな(固定かつ起点)←→「その他のモード(可変)」間のシーソー切替
	// type00 = KANA_EISUU (値3) : ひらがな(固定かつ起点)←→英字(固定)間のシーソー切替
	// type00 = EISUU_OTHERS(値4): 英字(固定かつ起点)←→「その他のモード(可変)」間のシーソー切替
	// 「その他のモード」の初期値はカタカナとする。
	//  キーダウン時にのみ実行すること｡
	//	2011.04.10(日) 〜15(金)
    static int get_index_for_seesaw_AtoB_WSD(int type00){
		int tmp_index;
		int Aindex00,Bindex00;

		if(type00 == CUR_PRE){
			Aindex00 = cur_index2_;		//起点のモード
			Bindex00 = pre_index2_;		//もう一方のモード
		} else if(type00 == EISUU_KANA){
			Aindex00 = wsdEISU;		//起点のモード
			Bindex00 = wsdHIRA;		//もう一方のモード
		} else if(type00 == KANA_EISUU){	//2011.04.15(金)
			Aindex00 = wsdHIRA;		//起点のモード
			Bindex00 = wsdEISU;		//もう一方のモード
		} else if(type00 == KANA_OTHERS){	//2011.04.15(金)
			if(others_index2_ == -1){	//保存済でなければ､カタカナ(初期値)にする。
			  set_indexes_directly(-1, -1, wsdKATA);
			}
			Aindex00 = wsdHIRA;
			Bindex00 = others_index2_;
		} else {	//EISUU_OTHERS
					//2011.04.15(金)
			if(others_index2_ == -1){	//保存済でなければ､カタカナ(初期値)にする。
			  set_indexes_directly(-1, -1, wsdKATA);	//2011.04.11(月)
			}
			Aindex00 = wsdEISU;
			Bindex00 = others_index2_;
		}

		if(cur_index2_ != -1 && pre_index2_ != -1){ //pre値とcur値が保存済なら
		  if(type00 == CUR_PRE){
			// pre値とcur値が保存済なら入れ替える。
			set_indexes_directly(Aindex00, Bindex00, -1);
			return cur_index2_;
		  } else {
			tmp_index = cur_index2_;	//現在のモードが前のモードになる。ひらがなモード以外の場合もあるので。
		  }

		} else {	// cur_index2_が未保存の場合は、
		  if(type00 == CUR_PRE){
		  	// 強制的に次を決める。
			if(pre_index2_ +1 > wsdMAX){
				set_indexes_directly(-1, 1, -1);
			} else {
				set_indexes_directly(-1, pre_index2_ +1, -1);
			}
			return cur_index2_;
		  } else {
			tmp_index = Bindex00;	// 前のモードを強制的に起点と逆のモードとする。
		  }
		}
		if(cur_index2_ != Aindex00){	//現在が､起点のモードでなければ、次の現在値を起点のモードにする。
			set_indexes_directly(tmp_index, Aindex00, -1);
		} else {	//起点のモードなら､それが前のモードになって､次の現在値はその他にする。
			set_indexes_directly(Aindex00, Bindex00, -1);
		}
		if(seesaw_init2_) {
			// (後処理AA)タイムアウト後なので､必ず､起点のモードにする｡既に､起点のモードになっていればそのまま｡
			if(cur_index2_ != Aindex00){
			set_indexes_directly(cur_index2_, Aindex00, -1);
			}
			seesaw_init2_ = false;	//フラグを初期化
		}
		return cur_index2_;
    };

//Haci
	//----------------------------------------------------------
	// リプレースを実行した時に、インデックス値cur_index2_とpre_index2_,others_index2_を更新した上で､
	// 入力モードに応じたWSDのインデックス値の現在値cur_index2_を返す。
	// (前提) 「ことえり」や「かわせみ」の入力ソースで、英数、ひらがな、カタカナ、半角カタカナ、全角英数を必ず有効にしておく｡
	//		  但し、アイヌモードは、「KeyRemapのGeneral--Japanese-Inputmode switching Delay: MacOS Bug」にオン/オフがあるので、入力ソースの設定に合わせる｡
	//		  入力ソースをKeyRemap4側から読み出せればいいが…。
	// sign00 : 符号(+1,-1を与える)
	// skip[ii] != 0: ii番目のモードをスキップさせる時に指定する。
	//			 = 0: スキップしない。 
	// replace_num00 = SKIP_NONE_PLUS/MINUS(タイプ1):	 ... スキップしない。
	//				 = SKIP_PRE_PLUS/MINUS (タイプ2):	 ... 前のモードをスキップ
	//				 = SKIP_EISUU_KANA/EISUU/KANA(タイプ3)... 特定のモードをスキップ
	//	タイプ3の場合､符号sign_plus_minus2_は、同じキーを連続して押している間は､同じ値である。
	// 				基本的には、別のキーを押した後には､逆転させる。
	//				但し、例えば英数モードの後は必ず全角英数にするために、符号をスマートに制御することで実現する｡そのため、wsd_save_配列の順番には意味がある｡
	// 				まずは、Handle_VK_JIS_COMMAND_SPACEのキーダウンしたときに､カウントアップして、
	// 				Core.cppでは、リマップ前とリマップ後にカウンターが増えたかどうかを調べる。
	// 				カウンターが増えなかったら､符号を逆転させる。そうなるのは、別のキーが押された場合か､
	// 				あるいは、例えば､「Shift+かな」にSKIP_EISUU_KANAを設定した場合で言えば､Shiftキーを一旦アップしてもう一度Shiftキーをダウンした時も含まれる。
    // 2011.03.29(火)〜04.12(火)
    static int get_index_for_replaceWSD(int sign00, int skip[], int replace_num00){
		int ii;
		int ret;
		int cur_index_tmp, pre_index_tmp,others_index_tmp;	//11.4.12


		cur_index_tmp    = cur_index2_;		//11.4.12
		pre_index_tmp    = pre_index2_;
		others_index_tmp = others_index2_;

		// 符号変更の条件
		// 通常は､キーを打ち直すと､自動的に方向転換するが､以下の条件の場合は､スマートに方向を調整する｡
		// 簡単に言えば、英数モードと全角英数、カタカナと半角カタカナができるだけ続けて出るようにする｡
		// 2011.04.10(日)
		bool cond00 = (cur_index2_ == wsdEISU);
		bool cond01 = (pre_index2_ == wsdHKAT && cur_index2_ == wsdKATA);
		bool cond02 = (pre_index2_ != wsdKATA && cur_index2_ == wsdHKAT);
		bool cond10 = (pre_index2_ == wsdEISU && cur_index2_ == wsdHIRA);
		bool cond11 = (pre_index2_ == wsdKATA && cur_index2_ == wsdHIRA && others_index2_ == wsdKATA);
		bool cond12 = (pre_index2_ == wsdHKAT && cur_index2_ == wsdHIRA && others_index2_ == wsdHKAT);
		bool cond13 = (pre_index2_ != wsdHKAT && cur_index2_ == wsdKATA);
		bool cond14 = (pre_index2_ == wsdKATA && cur_index2_ == wsdHKAT);
			// Smartひらキーを打った後に、[sft]+Smasrtひらキーを続けて打つ場合､
			// および、Smart英数キーの後で[sft]+Smasrtひらキーを打った場合、それぞれ
			// (逆方向に方向転換する条件)
		  	// 条件00: 現在が英字モードの場合は､強制的に逆順にして､最後尾の全角英数モードを選ぶようにする。2011.04.08(金)
			// 条件01: 現在がカタカナモードのとき、1つ前が半角カタカナなら、逆方向に進めることで、それ以外のモードにする｡
			// 条件02: 現在が半角カタカナモードのとき、1つ前がカタカナでなければ、逆方向に進めることで、カタカナモードになる｡
			// 		 (英数←→ひらがな間でひらがなとして使用している時に､その他のモードがカタカナのケースで､カタカナをスキップして半角カタカナになった場合も含む｡)
			// (順方向に方向転換する条件)
			// 条件10: 1つ前が英字モードで現在がひらがなモードの場合は､強制的に順方向にして､次のカタカナモードあるいは半角カタカナを選ぶようにする。2011.04.08(金)
			// 条件11: 現在がひらがなモードのとき、1つ前がカタカナモードなら、
			//  その他のモードがカタカナであれば､つまりは､シーソーで切り替えた状態で､そのときは､順方向に進めることで､カタカナはスキップして半角カタカナモードになる｡
			// 条件12: 現在がひらがなモードのとき、1つ前が半角カタカナなら、
			//  その他のモードが半角カタカナであれば、つまり､シーソーで切り替えた状態で､順方向に進めることで､カタカナモードになる｡
			// 条件13: 現在がカタカナモードのとき、
			// 1つ前が半角カタカナモードでなければ､順方向に進めることで、半角カタカナモードになる｡		
			//  (英数←→ひらがな間でひらがなとして使用している時に､その他のモードがカタカナ以外のケースで､カタカナになった場合も含む)
			// 条件14: 現在が半角カタカナモードのとき、1つ前がカタカナなら、順方向に進めることで､その他のモードになる｡
		if(replace_num00 == 2){	// SKIP_PRE_PLUS/MINUS (タイプ2)
			// 2011.04.15(金)
			skip[pre_index2_] = 1;	//前のモードをスキップする｡

		} else if(replace_num00 == 3){	// SKIP_EISUU_KANA/EISUU/KANA(タイプ3)
		  // REPLACE_PLUS_MINUS_SKIP_KANA/EISUU系の場合に符号は専用のsign_plus_minus2_を使用する。
		  //2011.04.06(水)
		  if(sign_plus_minus2_ == -99){
			sign_plus_minus2_ = 1;		// 最初は正方向。但し､英字モードの時は､以下のように逆順になる。
		  }
		  // 以下の条件では､スマートに方向を切り替える｡
		  if (sign_plus_minus2_ == 1 && 
		  	  (cond00 || cond01 ||cond02 )){
		  	sign_plus_minus2_ = -1;		//逆順に切り替える｡
		  	if(cond00){
			  // 全角英数モードの後で、英数キーを打って、英字モードに戻り、再度、本関数の機能によって、全角英数モードにしようとすると、
			  // others_index2_に全角英数モードが入っているため、それをスキップして、逆順の1つ先のAINUあるいは半角カタカナモードになってしまうので、
			  // others_index_tmpをそれ以外の値、一番確実な現在値の英字モードに成り済ます｡これで、再度、全角英数モードに切り替われる｡
		  	  //2011.04.13(水)
		  	  others_index_tmp = wsdEISU;
		  	}
		  } else if(sign_plus_minus2_ == -1	&& (
		  			cond10 || cond11 || cond12 || cond13 || cond14 )){
		  	sign_plus_minus2_ = 1;		// 順方向に切り替える｡
		  } else {
		  }
		  sign00 = sign_plus_minus2_;
		}

		// 現在値と前値と異なればそれが次の値
		// 現在の入力モード(cur_index2_)を起点にして見つける。
		int continue_end00 = 0;	//2011.04.05(火) 無限ループ対策
		for(ii = (cur_index_tmp == -1? 1:cur_index_tmp);;){
		  if(ii > wsdMAX && sign00 == 1 || ii < 1 && sign00 == -1){
		  	// 順方向で最後まで見つからなかった場合は、一度だけ、1番目に戻る。
		  	// あるいは、逆方向で先頭までの間に見つからなかった場合は､一度だけ最後に戻る。
			if(continue_end00 == 1){
			  ret = -1;	// 一回りしても見つからなかった。
			  break; // 一度一回りしても無ければ、無限ループになってしまうので、終了。
			}
			if(sign00 == 1){
			  //順方向で最後までの間に見つからなかったので、一度だけ先頭に戻る。
			  ii = 1;
			} else {	//逆方向で先頭までの間に見つからなかったので、一度だけ最後に戻る。
			  ii = wsdMAX;
			}
			continue_end00 = 1;	// この後一回りしても見つからなかった時に抜けるようにするため。
			continue;
		  }

		  // 現在値(cur_index2_)が未保存なら1番目から始め､保存済なら現在値から検索する。
		  if(cur_index_tmp != -1 && cur_index_tmp != ii &&
		     others_index_tmp != ii){
			// 現在値が保存済で､かつ、現在値と一致せず､かつその他のモードとも一致しなければ、それが次のインデックス。
		  	if(skip[ii] != 1) { // ただし、スキップ指定でなければ。
		  		// 2011.04.05(火)、11(月)
		  		ret = ii;
		  		break;
		  	}
		  }
		  ii = ii +sign00;
		}
		
		if(ret > 0){
			if(replace_num00 == 2){
				set_indexes_directly(-1, ret, -1);	//2011.04.16(土) Pre値はそのままで、Cur値のみセット｡
			} else {
				set_indexes_directly(cur_index_tmp, ret, -1);	// Pre値とCur値をセット｡
				//2011.04.08(金) リプレースだけでモード変更をしている場合は､基本的にothers_index2_で制御しているので､pre値は特に必要ないが､
				// 英字←→ひらがな間から直接､このリプレース(タイプ3)を呼んだ時に､pre値が英字なのかどうかが意味を持つので、保存しておく必要がある。
			}
			if(replace_num00 == 2 || replace_num00 == 3){
				//2011.04.05(火)、06(水)
				set_indexes_directly(-1, -1, ret);	//2011.04.11(月)

			}
		} else {
			ret = cur_index_tmp;	//2011.4.12 普通はここには来ないが、念の為、今までの現在値を与える｡
		}
		
		return ret;
    };



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


//Haci
    static BridgeWorkSpaceData wsd_public_;		// 2011.01.13(木)
    static BridgeWorkSpaceData wsd_save_[wsdMAX+1];	// 2011.03.05(土) 学習用
    static int pre_index2_;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
    static int cur_index2_;					//2011.03.29(火) VK_JIS_COMMAND_SPACE用
    static int others_index2_;				//2011.04.05(火) VK_JIS_COMMAND_SPACE用
    static int sign_plus_minus2_;			//2011.04.06(水) REPLACE_PLUS_MINUS用の符号
    static int counter_plus_minus2_;		//2011.04.06(水) REPLACE_PLUS_MINUS用の連続実行をカウントする。
    static int pre_counter_plus_minus2_;	//2011.04.06(水) REPLACE_PLUS_MINUS用のリマップ前の上記カウンター値
    static int get_WSDindex(InputModeDetail imd00, KeyCode modekey00, Flags flag00);	//2011.03.19(土)、26(土)
    static int IMD2index(InputModeDetail imd00){	// 2011.03.26(土)
    	//IMD値からインデックス値を得る。
    	return get_WSDindex(imd00, KeyCode::VK_NONE, ModifierFlag::NONE);
    }
    static int modeKey2index(KeyCode modekey00, Flags flag00){	//2011.03.26(土)
    	//モードキーからインデックス値を得る。
    	return get_WSDindex(InputModeDetail::UNKNOWN, modekey00, flag00);
    }
    static bool seesaw_init2_;				//2011.04.09(土) = true:タイムアウト後にseesawの起点にする。



    static char statusmessage_[BRIDGE_USERCLIENT_STATUS_MESSAGE__END__][BRIDGE_USERCLIENT_STATUS_MESSAGE_MAXLEN];

    static int alloccount_;
  };
}

#endif
