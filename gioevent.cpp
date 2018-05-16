// gioevnet.cpp
// gpio入力検出プログラム
// リリース時は３箇所を変更
// テスト用　　キーボード入力では、GIOConditionでG_IO_INを使用
// リリース用　GPIOピン入力では、G_IO_PRIを使用 G_IO_INを使うとg_main_loop_unrefでアサートエラー
// チャタリング防止にソフトディレイ 50msを入れている
// ハードディレイ　10kΩ x 0.1~0.84μF　今回は0.47μFを使用

#include <glib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include "beaglelib.h"
using namespace std;

//グローバル変数
struct itimerspec itval;	//
timer_t timer_id;			//
int timerfl = 1;			//タイマフラグ

struct usrtype{      		//g_io_add_watch(,,,gpoint user_data)で渡すための構造体
	GMainLoop *loop;		//セルフポインタ
	char *func;			//ドリブンイベント
};

//---------------------------------タイマ割り込みジャンプルーチン
void sighandler( int signo ){
	timerfl = 1;					//タイマフラグリセット
}

//---------------------------------GPIOコールバックルーチン
static gboolean read_callback(GIOChannel *channel, GIOCondition cond, gpointer user_data) {
	static bool first = TRUE;	//初回のみ初期化
	usrtype *udata = (usrtype*)user_data;
	gboolean continue_to_watch = FALSE;

//①リリース時変更
	if (cond & G_IO_IN){	//標準入力用
//	if (cond & G_IO_PRI){	//GPIO用
		GError *error = NULL;
		gchar *text;
		gsize len;
		gsize bytes_read = 0;

		g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
		GIOStatus rc =g_io_channel_read_line(channel, &text, &len, &bytes_read, &error);
		switch ( rc ) {
		case G_IO_STATUS_NORMAL:
//			if(first){	//初回の割り込みを読み飛ばす
//				first = FALSE;
//				continue_to_watch = TRUE;
//				return continue_to_watch;
//			}
			if(timerfl){	//ディレイタイマ後
				timerfl = 0;
				timer_settime( timer_id, 0, &itval, NULL);		//タイマーセット
				g_message("%s: read line: %s", __func__, text);
				g_free(text);
//				system(udata->func);		//割り込み時の発生イベント
			}
//			else{	//ディレイタイマ中、ここを有効にすると割り込みタイマ中にチャタリング回数分Waitingを表示する
//				cout << "Waiting " << endl;
//			}
			
			continue_to_watch = TRUE;
			break;
		case G_IO_STATUS_AGAIN:
			g_message("%s: AGAIN", __func__);
			continue_to_watch = TRUE;
			break;
		case G_IO_STATUS_ERROR:
			g_message("%s: error: %s", __func__, error->message);
			g_error_free(error);
			break;
		case G_IO_STATUS_EOF:
			g_message("%s: EOF", __func__);
			break;
		default:
			break;
		}
	}
	if (! continue_to_watch) {
		g_main_loop_quit(udata->loop);
		g_main_loop_unref(udata->loop);
	}
	return continue_to_watch;
}

int main(int argc, char *argv[]) {
//引数未使用、紛らわしいので引数なしにしては（未検証）
//或いは、argv[1]で実行ファンクションを渡すか
	//Root Check
	if( getuid()){
		cout << "Not Root" << endl;
		return 1;
	}

	gpio_export( 60 );			// No 60Port
	gpio_init_in( 60, 2);			// Init Input falling Edge

	char func[] = "./test.sh";	//スイッチON時のイベントを登録

	//------------------------------タイマ割り込みを設定
	//チャタリング防止のためのソフトタイマー
	struct sigaction sigact;
//	struct itimerspec itval;		//グローバルで宣言 GPIOコールバックルーチンでも割り込みを設定するため
//	timer_t timer_id;				//グローバルで宣言
	
	sigact.sa_handler = sighandler;					//シグナルハンドラを設定　割り込みルーチン
	sigact.sa_flags = SA_SIGINFO | SA_RESTART;		//ハンドラの動作フラグ
	sigemptyset( &sigact.sa_mask );					//対象のシグナルをクリアする
	if( sigaction( SIGALRM, &sigact, NULL ) < 0){
		perror("sigaction error");
	}
	if(timer_create( CLOCK_REALTIME, NULL, &timer_id ) < 0){//タイマを作成
		perror("timer_create error");
		exit(1);
	}
	//割り込みまでに時間を設定
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 50 * 1000000;
	//割り込み後の繰り返し時間を設定
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = 0;

	//--------------------------GMainLoopの設定
	GMainLoop *loop = g_main_loop_new( NULL, FALSE);
	usrtype usr_data;
	usrtype *p_usr = &usr_data;
	p_usr->loop = loop;
	p_usr->func = func;

// ②リリース時変更
	GIOChannel *channel = g_io_channel_unix_new( 0 );	//テスト用　標準入力
//	int fd = open("/sys/class/gpio/gpio60/value", O_RDONLY | O_NONBLOCK );	//GPIO入力用
//	if(fd < 0 ){
//		cout << "File Open Error!!" << endl;
//		return 1;
//	}
//	GIOChannel *channel = g_io_channel_unix_new(fd);

//③リリース時変更
	guint tag = g_io_add_watch(channel, G_IO_IN, read_callback, p_usr);	//コールバック関数を追加
//	guint tag = g_io_add_watch(channel, G_IO_PRI, read_callback, p_usr);

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);	//(channel,flags,gerror)
		//NONBLOCKに設定ブロックに設定しないとG_IO_STATUS_AGAINが返るかわりにブロックされる
	g_io_channel_set_close_on_unref(channel, TRUE);//(channel,gbool)
		//問題発生時にチャンネルを閉じる
	g_io_channel_set_encoding(channel, NULL, NULL);//(channel,encoding,gerror)default UTF-8
	g_main_loop_run(loop);//ループを呼び出しg_main_loop_quitが呼ばれるまで実行

	g_io_channel_unref(channel);	//チャンネルの参照カウントを減算
	g_main_loop_unref(loop);//メインループオブジェクトの参照カウントを１つ減算０ならメモリ解放

	gpio_unexport(60);
	
	return 0;
}
