
/* =============================================================
	GAME TEMPLATE - SE102 (DirectX 10 + Win32)

	Cách dùng:
		1/ Đổi các #define trong phần CONFIG
		2/ Khai báo / thêm object trong LoadResources()
		3/ Viết KeyEventHandler riêng (kế thừa CKeyEventHandler)
		4/ Các hàm còn lại (Update, Render, Window, Run, WinMain) giữ nguyên
================================================================ */

#include <windows.h>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"

#include "Animation.h"
#include "Animations.h"
#include "Sprite.h"
#include "Sprites.h"
#include "Tank.h"

// ---- Include các object & key handler của dự án ----
// #include "Mario.h"
// #include "Brick.h"
// #include "MyKeyEventHandler.h"

/* =============================================================
	CONFIG
================================================================ */
#define WINDOW_CLASS_NAME  L"GameWindow"
#define MAIN_WINDOW_TITLE  L"Game Title"
#define WINDOW_ICON_PATH   L"icon.ico"

#define BACKGROUND_COLOR   D3DXCOLOR(200.0f/255, 200.0f/255, 255.0f/255, 0.0f)

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ---- ID texture ----
#define ID_TEX_TANK 0
#define ID_TEX_MISC 10
// #define ID_TEX_MISC 20

// ---- Đường dẫn texture ----
#define TEXTURES_DIR L"textures"
#define TEXTURE_PATH_TANK TEXTURES_DIR "\\Tank.png"
#define TEXTURE_PATH_MISC TEXTURES_DIR "\\misc.png"

#define TANK_START_X 200.0f
#define TANK_START_Y 10.0f
/* =============================================================
	GLOBALS
================================================================ */
// CMario* player = NULL;
// CMyKeyHandler* keyHandler;
Tank* tank = NULL;
vector<LPGAMEOBJECT> objects;

/* =============================================================
	WINDOW PROC
================================================================ */
LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/* =============================================================
	LOAD RESOURCES: texture -> sprite -> animation -> object
================================================================ */
void LoadResources()
{
	CTextures* textures = CTextures::GetInstance();
	CSprites* sprites = CSprites::GetInstance();
	CAnimations* animations = CAnimations::GetInstance();

	// 1/ Texture
	textures->Add(ID_TEX_TANK, TEXTURE_PATH_TANK);
	LPTEXTURE texTank = textures->Get(ID_TEX_TANK);

	// 2/ Sprite: Add(id, left, top, right, bottom, texture)
	sprites->Add(10001, 140, 51, 429, 291, texTank);

	

	// 3/ Animation: CAnimation(thời gian mỗi frame - ms)
	LPANIMATION ani;

	ani = new CAnimation(100);
	ani->Add(10001);
	animations->Add(36, ani);

	tank = new Tank(TANK_START_X, TANK_START_Y);
	// 4/ Object
	// player = new CMario(START_X, START_Y);
	// objects.push_back(player);

	// for (int i = 0; i < NUM; i++)
	// {
	//     objects.push_back(new CBrick(x + i * BRICK_WIDTH, y));
	// }
}

/* =============================================================
	UPDATE / RENDER
	dt: thời gian (ms) giữa frame trước và frame này
================================================================ */
void Update(DWORD dt)
{
	tank->Update(dt);
	//for (int i = 0; i < (int)objects.size(); i++)
	//	objects[i]->Update(dt);
}

void Render()
{
	CGame* g = CGame::GetInstance();

	ID3D10Device* pD3DDevice = g->GetDirect3DDevice();
	IDXGISwapChain* pSwapChain = g->GetSwapChain();
	ID3D10RenderTargetView* pRenderTargetView = g->GetRenderTargetView();
	ID3DX10Sprite* spriteHandler = g->GetSpriteHandler();

	pD3DDevice->ClearRenderTargetView(pRenderTargetView, BACKGROUND_COLOR);

	spriteHandler->Begin(D3DX10_SPRITE_SORT_TEXTURE);

	FLOAT NewBlendFactor[4] = { 0,0,0,0 };
	pD3DDevice->OMSetBlendState(g->GetAlphaBlending(), NewBlendFactor, 0xffffffff);

	//for (int i = 0; i < (int)objects.size(); i++)
	//	objects[i]->Render();
	tank->Render();

	spriteHandler->End();
	pSwapChain->Present(0, 0);
}

/* =============================================================
	CREATE WINDOW
================================================================ */
HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(hInstance, WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(
		WINDOW_CLASS_NAME,
		MAIN_WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		ScreenWidth,
		ScreenHeight,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
	{
		OutputDebugString(L"[ERROR] CreateWindow failed");
		return NULL;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

/* =============================================================
	GAME LOOP
================================================================ */
int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG frameStart = GetTickCount64();
	ULONGLONG tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();
		ULONGLONG dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;
			Update((DWORD)dt);
			CGame::GetInstance()->ProcessKeyboard();
			Render();
		}
		else
			Sleep((DWORD)(tickPerFrame - dt));
	}

	return 1;
}

/* =============================================================
	ENTRY POINT
================================================================ */
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
) {
	HWND hWnd = CreateGameWindow(hInstance, nCmdShow, SCREEN_WIDTH, SCREEN_HEIGHT);

	SetDebugWindow(hWnd);

	CGame* game = CGame::GetInstance();
	game->Init(hWnd, hInstance);

	// keyHandler = new CMyKeyHandler();
	// game->InitKeyboard(keyHandler);

	// Phóng to cửa sổ x2 (tuỳ chọn)
	SetWindowPos(hWnd, 0, 0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,
		SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	LoadResources();

	Run();

	return 0;
}
