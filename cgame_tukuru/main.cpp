#include <windows.h>
#include <mmsystem.h> // timeGetTime()のため
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);       // ウィンドウプロシージャ

void GameMain(void); // ゲームメイン処理
void Wait(DWORD); // ウェイト
void FPSCount(DWORD*); // FPS計測


constexpr DWORD FPS = 60; // FPS設定
constexpr int pWid = 640, pHei = 480; //幅・高さ
BOOL EndFlag = FALSE; // 終了フラグ
DWORD fps; // FPS計測値

//==============================================================================================
// ウェイト
//==============================================================================================
void Wait(DWORD wait_time) {
	MSG msg;
	DWORD start_time = timeGetTime();

	do {
		// メッセージ処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (wait_time > 0) Sleep(1); // ちょっと休憩（CPUの占有率を下げるため）
	} while (timeGetTime() < wait_time + start_time); // wait_time だけ回る
}
//==============================================================================================
// FPS の計測
//==============================================================================================
void FPSCount(DWORD *fps) {
	static DWORD before_time = timeGetTime(); // 以前の時間
	DWORD        now_time = timeGetTime(); // 現在の時間
	static DWORD fps_ctr = 0;

	if (now_time - before_time >= 1000) {
		// 初期化
		before_time = now_time;
		*fps = fps_ctr;
		fps_ctr = 0;
	}

	fps_ctr++;
}
//==============================================================================================
// Windows メイン処理
//==============================================================================================
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	// ウィンドウクラスの登録
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;             // ウィンドウスタイル
	wc.lpfnWndProc = (WNDPROC)WndProc;                    // ウィンドウプロシージャ
	wc.cbClsExtra = 0;                                   // 補助領域サイズ
	wc.cbWndExtra = 0;                                   // 補助領域サイズ
	wc.hInstance = hInstance;                           // インスタンスハンドル
	wc.hIcon = LoadIcon(hInstance, TEXT("IDI_ICON"));     // アイコン
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);         // マウスカーソル
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);          // クライアント領域背景色
	wc.lpszMenuName = NULL;                                // ウィンドウメニュー
	wc.lpszClassName = TEXT("Windows");                     // ウィンドウクラス名
	if (!RegisterClass(&wc)) return 0;

	// ウィンドウの作成
	HWND hWnd = CreateWindow(
		wc.lpszClassName,       // ウィンドウクラス名
		TEXT("タイトル"),       // ウィンドウタイトル
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,        // ウィンドウスタイル
		CW_USEDEFAULT,          // 表示X座標
		CW_USEDEFAULT,          // 表示Y座標
		pWid + GetSystemMetrics(SM_CXDLGFRAME) * 2,         // 幅
		pHei + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),    // 高さ
		NULL,                   // 親ウィンドウ
		NULL,                   // ウィンドウメニュー
		hInstance,              // インスタンスハンドル
		NULL);                  // WM_CREATE情報

								// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow); // 表示状態の設定
	UpdateWindow(hWnd);         // クライアント領域の更新

								// ゲームメイン処理へ～
	GameMain();

	return 0; // とりあえず0を返す
}

//==============================================================================================
// ウィンドウプロシージャ
//==============================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_DESTROY:                                    // ウィンドウが破棄されたときの処理
		PostQuitMessage(0);// 終了メッセージ
		EndFlag = TRUE; // 終了フラグをあげる。
		return 0;

	default:                                            // デフォルト処理
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

//==============================================================================================
// ゲームメイン処理
//==============================================================================================
void GameMain(void) {

	TIMECAPS Caps;

	timeGetDevCaps(&Caps, sizeof(TIMECAPS)); // 性能取得
	timeBeginPeriod(Caps.wPeriodMin); // 設定

	//メインループ
	while (!EndFlag) {
		const DWORD StartTime = timeGetTime();

		//～ ゲーム処理いろいろ ～

		const DWORD PassTime = timeGetTime() - StartTime; // 経過時間の計算
		(1000 / FPS > PassTime) ? Wait(1000 / FPS - PassTime) : Wait(0); // 待つ。
		FPSCount(&fps); // FPS の計測
	}
	timeEndPeriod(Caps.wPeriodMin); // 後処理
}