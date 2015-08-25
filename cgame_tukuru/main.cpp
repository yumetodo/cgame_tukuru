#include <windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);// ウィンドウプロシージャ


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
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // アイコン
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);         // マウスカーソル
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);          // クライアント領域背景色
	wc.lpszMenuName = NULL;                                // ウィンドウメニュー
	wc.lpszClassName = TEXT("Windows");                     // ウィンドウクラス名
	if (!RegisterClass(&wc)) return 0;

	// ウィンドウの作成
	HWND hWnd = CreateWindow(
		wc.lpszClassName,                   // ウィンドウクラス名
		TEXT("タイトル"),                   // ウィンドウタイトル
		WS_OVERLAPPEDWINDOW,                // ウィンドウスタイル
		CW_USEDEFAULT,                      // 表示X座標
		CW_USEDEFAULT,                      // 表示Y座標
		CW_USEDEFAULT,                      // 幅
		CW_USEDEFAULT,                      // 高さ
		NULL,                               // 親ウィンドウ
		NULL,                               // ウィンドウメニュー
		hInstance,                          // インスタンスハンドル
		NULL);                              // WM_CREATE情報

											// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);             // 表示状態の設定
	UpdateWindow(hWnd);                     // クライアント領域の更新

											// メッセージループ
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);             // 仮想キーメッセージの変換
		DispatchMessage(&msg);              // ウィンドウプロシージャへ転送
	}

	return msg.wParam;
}

//==============================================================================================
// ウィンドウプロシージャ
//==============================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_DESTROY:                                    // ウィンドウが破棄されたときの処理
														// 終了メッセージ
		PostQuitMessage(0);
		return 0;

	default:                                            // デフォルト処理
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
