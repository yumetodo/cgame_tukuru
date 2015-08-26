#include <windows.h>
#include <tchar.h>
#include <mmsystem.h> // timeGetTime()のため
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);       // ウィンドウプロシージャ

void GameMain(void); // ゲームメイン処理
void Wait(DWORD); // ウェイト
void FPSCount(DWORD*); // FPS計測
HDC CreateEmptyBMP(HDC hdc, int width, int height);
HDC LoadBMPfromFile(HDC hdc, LPCTSTR f_name_of_bmp);
HDC LoadBMPfromResourceSTR(HDC hdc, HINSTANCE hinst, LPCTSTR ID);
HDC LoadBMPfromResourceINT(HDC hdc, HINSTANCE hinst, int ID);
HDC CreateMask(HDC hdc, COLORREF transparent_color);
void SpriteBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, HDC hdcMask);

constexpr DWORD FPS = 60; // FPS設定
constexpr int pWid = 640, pHei = 480; //幅・高さ
BOOL EndFlag = FALSE; // 終了フラグ
DWORD fps; // FPS計測値
HINSTANCE hInst; // インスタンス
HWND hWnd; // ウィンドウハンドル
HDC hDC_Wnd; // ウィンドウのデバイスコンテキスト

//==============================================================================================
// ビットマップ読み込み（ファイルから）
//==============================================================================================
HDC LoadBMPfromFile(HDC hdc, LPCTSTR f_name_of_bmp) {
	HBITMAP hbmp = (HBITMAP)LoadImage(0, f_name_of_bmp, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc_work = CreateCompatibleDC(hdc);
	SelectObject(hdc_work, hbmp);
	DeleteObject(hbmp);
	return hdc_work;
}

//==============================================================================================
// ビットマップ読み込み（リソースから、ID文字列）
//==============================================================================================
HDC LoadBMPfromResourceSTR(HDC hdc, HINSTANCE hinst, LPCTSTR ID) {
	HBITMAP hbmp = (HBITMAP)LoadImage(hinst, ID, IMAGE_BITMAP, 0, 0, 0);
	HDC hdc_work = CreateCompatibleDC(hdc);
	SelectObject(hdc_work, hbmp);
	DeleteObject(hbmp);
	return hdc_work;
}

//==============================================================================================
// ビットマップ読み込み（リソースから、ID整数）
//==============================================================================================
HDC LoadBMPfromResourceINT(HDC hdc, HINSTANCE hinst, int ID) {
	HBITMAP hbmp = (HBITMAP)LoadImage(hinst, MAKEINTRESOURCE(ID), IMAGE_BITMAP, 0, 0, 0);
	HDC hdc_work = CreateCompatibleDC(hdc);
	SelectObject(hdc_work, hbmp);
	DeleteObject(hbmp);
	return hdc_work;
}

//==============================================================================================
// 空のビットマップ作成
//==============================================================================================
HDC CreateEmptyBMP(HDC hdc, int width, int height) {
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, width, height);
	HDC hdc_work = CreateCompatibleDC(hdc);
	SelectObject(hdc_work, hbmp);
	PatBlt(hdc_work, 0, 0, width, height, WHITENESS); // 白で塗りつぶす
	DeleteObject(hbmp);
	return hdc_work;
}
//==============================================================================================
// マスク作成（マスク：透過する部分 = 白、キャラクタ部分 = 黒）（元絵の加工含む）
//==============================================================================================
HDC CreateMask(HDC hdc, COLORREF transparent_color) {
	const int width = GetDeviceCaps(hdc, HORZRES);
	const int height = GetDeviceCaps(hdc, VERTRES);

	COLORREF default_bkcolor = SetBkColor(hdc, transparent_color); // 透過色の設定

																   // モノクロビットマップでマスク作成
	HBITMAP hbmp_mono = CreateBitmap(width, height, 1, 1, 0);
	HDC hdc_mono = CreateCompatibleDC(hdc);
	SelectObject(hdc_mono, hbmp_mono);
	BitBlt(hdc_mono, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	DeleteObject(hbmp_mono);

	SetBkColor(hdc, default_bkcolor); // 背景色の設定を戻す

									  // カラービットマップに変換
	HDC hdc_color_white = CreateEmptyBMP(hdc, width, height);
	BitBlt(hdc_color_white, 0, 0, width, height, hdc_mono, 0, 0, SRCCOPY);
	DeleteDC(hdc_mono);

	// 元絵の透過色部分を黒にする
	HDC hdc_color_black = CreateEmptyBMP(hdc, width, height);
	BitBlt(hdc_color_black, 0, 0, width, height, hdc_color_white, 0, 0, NOTSRCCOPY);
	BitBlt(hdc, 0, 0, width, height, hdc_color_black, 0, 0, SRCAND);
	DeleteDC(hdc_color_black);

	return hdc_color_white;
}
//==============================================================================================
// スプライト
//==============================================================================================
void SpriteBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, HDC hdcMask) {
	BitBlt(hdc, x, y, cx, cy, hdcMask, x1, y1, SRCAND);
	BitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, SRCPAINT);
}
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
	hWnd = CreateWindow(
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

	hInst = hInstance;
	hDC_Wnd = GetDC(hWnd);
								// ゲームメイン処理へ～
	GameMain();
	ReleaseDC(hWnd, hDC_Wnd);

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
LPCTSTR LoadSound(LPCTSTR fname) {
	HANDLE hfile = CreateFile(fname, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD file_size = GetFileSize(hfile, NULL);
	LPCTSTR re = (LPCTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, file_size);
	DWORD read_byte;
	ReadFile(hfile, (LPVOID)re, file_size, &read_byte, NULL);
	CloseHandle(hfile);
	return re;
}
//==============================================================================================
// ゲームメイン処理
//==============================================================================================
void GameMain(void) {

	TIMECAPS Caps;

	timeGetDevCaps(&Caps, sizeof(TIMECAPS)); // 性能取得
	timeBeginPeriod(Caps.wPeriodMin); // 設定
	HDC hDC_Back = CreateEmptyBMP(hDC_Wnd, pWid, pHei);
	HDC hDC_Wheel = LoadBMPfromFile(hDC_Wnd, TEXT("wheel.bmp"));
	HDC hDC_Block = LoadBMPfromResourceSTR(hDC_Wnd, hInst, TEXT("IDB_BLOCKIDB_BLOCK"));
	HDC hDC_org = LoadBMPfromResourceSTR(hDC_Wnd, hInst, TEXT("IDB_ORG"));
	HDC hDC_Mask = CreateMask(hDC_org, RGB(0, 0, 0));
	HANDLE hfile;
	DWORD file_size, read_byte;
	mciSendString(_T("close BGM"), NULL, 0, NULL); // （前回ちゃんと閉じられなかったかも…）
	mciSendString(_T("open SHOOT!.mid type sequencer alias BGM"), NULL, 0, NULL); // 開く

	TCHAR BGMStatus[256] = { 0 };
	mciSendString(_T("status BGM mode"), BGMStatus, 256, NULL); // 状態の取得
	if (lstrcmp(BGMStatus, TEXT("stopped")) == 0) {
		mciSendString(TEXT("play BGM from 0"), NULL, 0, NULL); // 停止中なら演奏
	}


	Sleep(15000);
	LPCTSTR Sound = LoadSound(_T("notify.wav"));
	PlaySound(Sound, NULL, SND_ASYNC | SND_MEMORY);
	
	//メインループ
	while (!EndFlag) {
		const DWORD StartTime = timeGetTime();
		//～ ゲーム処理いろいろ ～
		//PatBlt(hDC_Back, 0, 0, pWid, pHei, WHITENESS);
		BitBlt(hDC_Back, 0, 0, 32, 32, hDC_Wheel, 0, 0, SRCCOPY);
		BitBlt(hDC_Back, 64, 0, 32, 32, hDC_Block, 0, 0, SRCCOPY);
		//SpriteBlt(hDC_Back, 32, 0, 32, 32, hDC_org, 0, 0, hDC_Mask);
		BitBlt(hDC_Wnd, 0, 0, pWid, pHei, hDC_Back, 0, 0, SRCCOPY);
		const DWORD PassTime = timeGetTime() - StartTime; // 経過時間の計算
		(1000 / FPS > PassTime) ? Wait(1000 / FPS - PassTime) : Wait(0); // 待つ。
		mciSendString(TEXT("status BGM mode"), BGMStatus, 256, NULL); // 状態の取得
		if (lstrcmp(BGMStatus, TEXT("stopped")) == 0) {
			mciSendString(TEXT("play BGM from 0"), NULL, 0, NULL); // 停止中なら演奏
		}

		FPSCount(&fps); // FPS の計測
	}
	timeEndPeriod(Caps.wPeriodMin); // 後処理
	HeapFree(GetProcessHeap(), 0, (LPVOID)Sound); // 解放
	DeleteDC(hDC_Wheel);
	DeleteDC(hDC_Block);
}
