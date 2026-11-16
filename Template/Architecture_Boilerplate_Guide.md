# Architecture & Boilerplate Guide — Game 2D C++ / DirectX 10 (Template "05-SceneManager")

> Nguồn: repo `dungdna2000-gamedev-intro-tutorials` (môn SE102 – Intro to Game Programming).
> Template chuẩn = **`05-SceneManager`** (bản hoàn chỉnh nhất). `01 → 04` chỉ là các bước tiến hóa (Skeleton → Sprite → Keyboard → Collision → SceneManager), không dùng làm khuôn.
> Khi tạo dự án/tính năng mới: **copy cấu trúc 05, đổi tên, làm theo checklist ở Mục 6.**

---

## 0. Tóm tắt 10 dòng (đọc cái này là đủ nhớ)

1. Ngôn ngữ **C++ thuần**, Win32 API, **Direct3D 10** + **DirectInput 8**, Visual Studio (`.sln`/`.vcxproj`, toolset v143, Unicode).
2. Thư viện ngoài duy nhất: NuGet `Microsoft.DXSDK.D3DX` 9.29.952.8. Link: `d3d10.lib; dxguid.lib; dinput8.lib`.
3. **Mọi thứ data-driven**: texture / scene / sprite / animation / object đều nằm trong file `.txt`, chia section `[TÊN]`, cột cách nhau bằng **TAB**.
4. Code chỉ hardcode **đúng 1 tên file**: `game->Load(L"mario-sample.txt")` trong `WinMain`.
5. Class prefix `C`, con trỏ `LP...`, hằng số `#define UPPER_SNAKE`.
6. Các "database" (`CTextures`, `CSprites`, `CAnimations`, `CGame`, `CCollision`) là **Singleton** qua `GetInstance()`.
7. Mọi object kế thừa `CGameObject` (abstract): `GetBoundingBox` + `Render` bắt buộc; `Update`, `OnCollisionWith`, `OnNoCollision`… tùy chọn.
8. Va chạm: **Swept AABB**, gọi qua `CCollision::GetInstance()->Process(this, dt, coObjects)` trong `Update` của object.
9. Scene: `CScene` (abstract) → `CPlayScene`. Chuyển scene **không làm ngay** mà `InitiateSwitchScene(id)` → thực thi ở cuối frame bằng `SwitchScene()`.
10. Thời gian dùng đơn vị **ms** (`DWORD dt`), tốc độ **px/ms** (ví dụ `0.1f`), gia tốc px/ms².

---

## 1. Cây thư mục chuẩn

Template gốc để **phẳng** (flat): toàn bộ `.h/.cpp/.txt` nằm chung 1 thư mục project. Hãy giữ y như vậy.

```
<TenRepo>/
├── samples.sln                      # Solution (có thể chứa nhiều project con)
└── <NN>-<TenProject>/               # VD: 05-SceneManager
    ├── <NN>-<TenProject>.vcxproj
    ├── <NN>-<TenProject>.vcxproj.filters
    ├── packages.config              # NuGet: Microsoft.DXSDK.D3DX
    │
    │── ===== ENGINE CORE (dùng lại nguyên xi, hầu như không sửa) =====
    ├── main.cpp                     # WinMain, WinProc, game loop, Update(), Render()
    ├── Game.h / Game.cpp            # CGame: D3D init, Draw, Keyboard, Camera, Scene switching
    ├── debug.h / debug.cpp          # DebugOut, DebugOutTitle, SetDebugWindow
    ├── Utils.h / Utils.cpp          # split(), ToWSTR(), ToLPCWSTR()
    ├── Texture.h                    # CTexture (wrapper ID3D10Texture2D) – header-only
    ├── Textures.h / Textures.cpp    # CTextures  (Singleton DB: id -> LPTEXTURE)
    ├── Sprite.h / Sprite.cpp        # CSprite    (1 hình chữ nhật cắt từ texture)
    ├── Sprites.h / Sprites.cpp      # CSprites   (Singleton DB: id -> LPSPRITE)
    ├── AnimationFrame.h             # CAnimationFrame (sprite + time) – header-only
    ├── Animation.h / Animation.cpp  # CAnimation (list frame, Render(x,y))
    ├── Animations.h / Animations.cpp# CAnimations (Singleton DB: id -> LPANIMATION)
    ├── Collision.h / Collision.cpp  # CCollision + CCollisionEvent (Swept AABB)
    ├── KeyEventHandler.h            # CKeyEventHandler (interface bàn phím)
    ├── Scene.h / Scence.cpp         # CScene (abstract) + CSceneKeyHandler   [*]
    ├── PlayScene.h / PlayScene.cpp  # CPlayScene: parse file scene, quản lý objects
    ├── AssetIDs.h                   # TẤT CẢ ID: texture, object type, sprite id
    │
    │── ===== GAMEPLAY (phần bạn thay bằng game của mình) =====
    ├── GameObject.h / GameObject.cpp# CGameObject – lớp cha của mọi object
    ├── Mario.h / Mario.cpp          # Player (state machine + animation chọn theo state)
    ├── Goomba.h / Goomba.cpp        # Enemy mẫu (AI đơn giản, die timeout)
    ├── Brick.h / Brick.cpp          # Object tĩnh, blocking
    ├── Coin.h / Coin.cpp            # Object tĩnh, KHÔNG blocking (nhặt được)
    ├── Platform.h / Platform.cpp    # Nền ghép từ N ô (begin/middle/end), chỉ chặn từ trên
    ├── Portal.h / Portal.cpp        # Vùng chuyển scene
    ├── SampleKeyEventHandler.h/.cpp # CSampleKeyHandler : CSceneKeyHandler
    │
    │── ===== DATA (text, phân cách bằng TAB) =====
    ├── mario-sample.txt             # FILE GỐC: [SETTINGS] [SCENES] [TEXTURES]
    ├── scene01.txt, scene02.txt ... # Mỗi scene: [ASSETS] + [OBJECTS]
    ├── mario.txt  goomba.txt  brick.txt  coin.txt  cloud.txt   # Asset: [SPRITES] + [ANIMATIONS]
    └── textures/                    # (ngoài repo nhưng được tham chiếu) *.png, bbox.png, mario.ico
```

> **[\*] Lưu ý lỗi chính tả của template:** file là `Scence.cpp` (sai chữ "Scene"), và nó `#include "Scence.h"` trong khi header thật là `Scene.h`. Constructor `CScene` đã được định nghĩa inline trong `Scene.h`, nên `Scence.cpp` thực chất thừa/xung đột. **Dự án mới: xóa `Scence.cpp`, chỉ dùng `Scene.h`.**

### Quy tắc đặt chỗ cho file mới

| Loại file | Đặt ở đâu | Ví dụ |
|---|---|---|
| Object gameplay mới | Cùng thư mục project, cặp `Tên.h` + `Tên.cpp` | `Koopas.h/.cpp` |
| Manager/DB (singleton) | Cùng thư mục, tên **số nhiều** của thứ nó quản lý | `Sprites`, `Animations` |
| Data của 1 loại object | `tênthường.txt` (chữ thường) | `koopas.txt` |
| Data của 1 màn | `sceneNN.txt` | `scene03.txt` |
| Mọi ID & object type | Thêm vào **`AssetIDs.h`** | `OBJECT_TYPE_KOOPAS` |
| Tham số riêng của 1 object | `#define` ở **đầu file `.h` của object đó** | `GOOMBA_GRAVITY` |

---

## 2. Quy tắc đặt tên

### 2.1. Code

| Đối tượng | Quy tắc | Ví dụ từ template |
|---|---|---|
| Class | Tiền tố **`C`** + PascalCase | `CGame`, `CGameObject`, `CPlayScene`, `CMario` |
| Con trỏ class | `typedef CTên* LP<TÊN_HOA>;` ngay dưới class | `LPGAME`, `LPSPRITE`, `LPANIMATION`, `LPGAMEOBJECT`, `LPTEXTURE`, `LPSCENE` |
| Con trỏ có tên 2 từ | `LP<TÊN>_<TỪ>` | `LPANIMATION_FRAME`, `LPCOLLISIONEVENT` |
| File | Tên class bỏ chữ `C`, PascalCase; số nhiều cho manager | `Mario.h`, `Sprites.cpp` |
| Ngoại lệ file | Tiện ích viết thường | `debug.h`, `debug.cpp` |
| Hàm / method | **PascalCase** | `GetBoundingBox`, `OnCollisionWith`, `SetState`, `InitiateSwitchScene` |
| Hàm parse nội bộ | `_ParseSection_<TÊN_HOA>` | `_ParseSection_SPRITES`, `_ParseSection_OBJECTS` |
| Biến thành viên | **camelCase / snake_case**, **KHÔNG** tiền tố `m_` | `x, y, vx, vy, nx, state, isDeleted, isOnPlatform, die_start, untouchable_start, current_scene` |
| Biến bool | Tiền tố `is` | `isDeleted`, `isSitting`, `isOnPlatform` |
| Tham số/biến cục bộ | camelCase hoặc ngắn gọn (`l,t,r,b`, `dt`, `e`) | |
| Singleton instance | `static C<Tên>* __instance;` (2 gạch dưới) | |
| Hằng số | `#define` **UPPER_SNAKE_CASE** (không dùng `const`/`constexpr`) | |
| Số thực | Hậu tố `f` | `0.002f` |

### 2.2. Hằng số `#define` — tiền tố theo nhóm

| Nhóm | Mẫu | Ví dụ |
|---|---|---|
| Thông số object | `<OBJ>_<THUỘC_TÍNH>` | `MARIO_WALKING_SPEED`, `GOOMBA_GRAVITY`, `BRICK_BBOX_WIDTH` |
| Kích thước bbox | `<OBJ>_BBOX_WIDTH/HEIGHT` (+ `_DIE`, `_SITTING`) | `GOOMBA_BBOX_HEIGHT_DIE` |
| State | `<OBJ>_STATE_<TÊN>` | `GOOMBA_STATE_WALKING`, `MARIO_STATE_JUMP` |
| Animation ID (trong object) | `ID_ANI_<OBJ>_<HÀNH_ĐỘNG>[_LEFT/_RIGHT]` | `ID_ANI_GOOMBA_WALKING`, `ID_ANI_MARIO_IDLE_LEFT` |
| Sprite ID (trong AssetIDs.h) | `ID_SPRITE_<OBJ>_<...>` | `ID_SPRITE_BRICK`, `ID_SPRITE_CLOUD_BEGIN` |
| Texture ID | `ID_TEX_<TÊN>` | `ID_TEX_MARIO`, `ID_TEX_BBOX` |
| Loại object trong scene | `OBJECT_TYPE_<TÊN>` | `OBJECT_TYPE_GOOMBA` |
| Level | `<OBJ>_LEVEL_<TÊN>` | `MARIO_LEVEL_SMALL` |
| Thời gian | `<OBJ>_<...>_TIME` / `_TIMEOUT` | `MARIO_UNTOUCHABLE_TIME`, `GOOMBA_DIE_TIMEOUT` |
| Nhóm macro | `#pragma region NAME … #pragma endregion` | `ANIMATION_ID`, `MARIO` |

### 2.3. Quy ước ID (số)

| Loại ID | Dải / quy ước |
|---|---|
| Texture ID | `-100` = bbox (đặc biệt), `0` = mario, `10` = enemies, `20` = misc |
| Object type | `0` Mario, `1` Brick, `2` Goomba, `3` Koopas, `4` Coin, `5` Platform, `50` Portal |
| Sprite ID | Theo nhóm 5 chữ số: Mario `1xxxx`, Brick `2xxxx`, Goomba `3xxxx`, Coin `4xxxx`, Cloud `5xxxx` |
| Animation ID | Số nhỏ theo đối tượng: Goomba `5000+`, Mario `400–1600`, Brick `10000`, Coin `11000` |
| State | Số bước 100: `100, 200, 300…`; hành động "nhả" = +1 (`300` JUMP → `301` RELEASE_JUMP); DIE đặc biệt `-10` (Mario) hoặc `200` (Goomba) |

> Sprite ID cho phép **tính bằng macro** (`ID_SPRITE_MARIO_BIG + 100 …`) để khỏi nhầm.

### 2.4. Phong cách viết code

- **Thụt đầu dòng bằng TAB**, ngoặc nhọn `{` xuống dòng riêng (Allman) cho hàm; class/ngắn có khi cùng dòng.
- Header luôn mở bằng `#pragma once` (không dùng `#ifndef` guard).
- Comment: `//` cho dòng; `/* ... */` mô tả class/hàm; **tiếng Anh**; ghi `TODO:` / `NOTE:` khi cần.
- Log: `DebugOut(L"[INFO] ...\n")`, `DebugOut(L"[ERROR] ...\n")`, `DebugOut(L"[WARNING] ...\n")` — chuỗi **wide `L"..."`**.
- Kiểu Windows: `DWORD dt`, `ULONGLONG` cho tick, `BOOLEAN`, `LPCWSTR` cho đường dẫn, `GetTickCount64()` cho thời gian.
- Dùng `dynamic_cast<CXxx*>(e->obj)` để nhận diện loại object khi va chạm (không dùng enum type).
- Cấp phát bằng `new`, giải phóng bằng `delete` thủ công; `NULL` (hoặc `nullptr`) cho con trỏ rỗng.
- `using namespace std;` đặt trong header.
- Hệ tọa độ: **(x, y) = TÂM của object**, y hướng xuống, `x - cam_x` khi vẽ.
- Chuỗi: file text đọc bằng `ifstream` + `getline`, tách bằng `split(line)` (mặc định delimiter = `"\t"`), đổi số bằng `atoi/atof`, đổi sang wide bằng `ToWSTR()`.

---

## 3. Luồng xử lý dữ liệu & chương trình

### 3.1. Khởi động & nạp dữ liệu

```
WinMain
 ├─ CreateGameWindow()
 ├─ SetDebugWindow(hWnd)
 ├─ CGame::GetInstance()->Init(hWnd, hInstance)     // D3D10 device, swapchain, sprite handler, blend, sampler
 ├─ game->InitKeyboard()                            // DirectInput
 ├─ game->Load(L"mario-sample.txt")                 // <<< file hardcode DUY NHẤT
 │     ├─ [SETTINGS] start=<scene_id>, width, height
 │     ├─ [SCENES]   <id> <file>  -> new CPlayScene(id, file) -> scenes[id]
 │     ├─ [TEXTURES] <id> <png>   -> CTextures::Add(id, path)
 │     └─ InitiateSwitchScene(start) -> SwitchScene()
 │           └─ scene->Load()                       // CPlayScene::Load
 │                 ├─ [ASSETS]  mario.txt, brick.txt ... -> LoadAssets(file)
 │                 │      ├─ [SPRITES]    -> CSprites::Add(id,l,t,r,b,tex)
 │                 │      └─ [ANIMATIONS] -> new CAnimation; ani->Add(sprite,time)...; CAnimations::Add
 │                 └─ [OBJECTS] <type> <x> <y> [extra...] -> new CXxx -> objects.push_back
 ├─ SetWindowPos(... SCREEN_WIDTH*2, SCREEN_HEIGHT*2 ...)   // phóng 2x cửa sổ
 └─ Run()
```

### 3.2. Game loop (cố định `MAX_FRAME_RATE = 100`)

```
Run():
  loop:
    PeekMessage / Dispatch
    dt = now - frameStart
    if dt >= 1000/MAX_FRAME_RATE:
        frameStart = now
        CGame::ProcessKeyboard()      // KeyState (giữ phím) + OnKeyDown/OnKeyUp (sự kiện)
        Update(dt)                    // -> CurrentScene->Update(dt)
        Render()                      // Clear -> sprite Begin -> CurrentScene->Render() -> End -> Present
        CGame::SwitchScene()          // chỉ đổi scene Ở ĐÂY (cuối frame)
    else Sleep(tickPerFrame - dt)
```

### 3.3. Update 1 frame trong `CPlayScene`

```
CPlayScene::Update(dt):
  coObjects = objects[1..n]                  // bỏ objects[0] (player). Player PHẢI là object đầu tiên trong [OBJECTS]
  foreach obj in objects: obj->Update(dt, &coObjects)
      └─ (object có chuyển động) vy += ay*dt; vx += ax*dt;
         CCollision::Process(this, dt, coObjects)
              ├─ Scan  : SweptAABB với từng obj trong coObjects -> coEvents
              ├─ không có va chạm -> obj->OnNoCollision(dt)   // x += vx*dt; y += vy*dt
              └─ có va chạm  -> Filter (chọn va chạm blocking sớm nhất theo trục X, Y)
                               -> đẩy vị trí, gọi obj->OnCollisionWith(e)
                               -> với va chạm KHÔNG blocking, gọi OnCollisionWith(e) sau cùng
  cập nhật camera theo player:  SetCamPos(max(0, px - W/2), 0)
  PurgeDeletedObjects()                      // xóa object có isDeleted == true
```

### 3.4. Chuyển scene

```
Mario chạm Portal -> OnCollisionWithPortal -> CGame::InitiateSwitchScene(portal->GetSceneId())  // chỉ đặt next_scene
... hết frame ...
CGame::SwitchScene(): current->Unload(); (xóa Sprites/Animations cũ); current_scene = next;
                      SetKeyHandler(scene->GetKeyEventHandler()); scene->Load();
```

> Vì sau `Unload()` `player == NULL`, `CPlayScene::Update` có kiểm tra `if (player == NULL) return;`. **Luôn giữ kiểm tra này.**

### 3.5. Mô hình trách nhiệm (ai gọi ai)

| Tầng | Lớp | Nhiệm vụ |
|---|---|---|
| App | `main.cpp` | Cửa sổ, vòng lặp, ủy quyền cho Scene hiện tại |
| Framework | `CGame` | D3D, vẽ, input, camera, quản lý scene |
| Asset DB | `CTextures` → `CSprites` → `CAnimations` | Chuỗi phụ thuộc: Animation tham chiếu Sprite, Sprite tham chiếu Texture |
| Scene | `CScene` → `CPlayScene` | Nạp file scene, giữ `vector<LPGAMEOBJECT> objects`, vòng Update/Render |
| Object | `CGameObject` → `CMario/...` | Trạng thái, bbox, animation, phản ứng va chạm |
| Service | `CCollision` | Swept AABB, không giữ state |

### 3.6. Quy tắc chọn animation (state → animation)

Object **không** lưu animation; mỗi frame `Render()` tính `aniId` từ `state`/`level`/hướng `nx`, rồi:
`CAnimations::GetInstance()->Get(aniId)->Render(x, y);`

---

## 4. Định dạng file dữ liệu (TAB-separated)

Quy ước chung: dòng bắt đầu `#` là comment; `[SECTION]` đổi section; section lạ → bỏ qua; dòng thiếu cột → bỏ qua.

### 4.1. `mario-sample.txt` (file gốc của game)

```
[SETTINGS]
start	1
width	320
height	240

#id	file
[SCENES]
1	scene01.txt
2	scene02.txt

# id	file
[TEXTURES]
-100	textures\bbox.png
0	textures\mario.png
10	textures\enemies.png
20	textures\misc.png
```

### 4.2. `sceneNN.txt`

```
[ASSETS]
mario.txt
brick.txt
coin.txt
goomba.txt
cloud.txt
#
[OBJECTS]
# type	x	y	extra_settings per object type
# type: 0 = Mario, 1 = Brick, 2 = Goomba, 3 = Koopas, 4 = Coin, 5 = Platform, 50 = Portal
# See OBJECT_TYPE constants in AssetIDs.h

# MARIO   (PHẢI là object đầu tiên)
0	20	10

# BRICK / GOOMBA / COIN: chỉ  type x y
1	0	148
2	300	10
4	100	120

# PLATFORM: type x y cell_w cell_h length sprite_begin sprite_middle sprite_end
5	90	136	16	15	16	51000	52000	53000

# PORTAL: type l t r b scene_id
50	300	110	316	140	2
```

### 4.3. File asset (`mario.txt`, `goomba.txt`, …)

```
# id	left	top	right	bottom	texture_id
[SPRITES]
31001	4	13	22	30	10
31002	24	13	42	30	10

# ani_id	sprite1_id	time1	sprite2_id	time2	...
[ANIMATIONS]
5000	31001	100	31002	100
```

> Cặp `sprite_id | frame_time(ms)` lặp lại → parser lặp `i += 2`.

### 4.4. Bảng "extra settings" theo OBJECT_TYPE (mở rộng khi thêm loại mới)

| type | Cột sau `type x y` |
|---|---|
| 0, 1, 2, 4 | *(không có)* |
| 5 Platform | `cell_w cell_h length spr_begin spr_mid spr_end` |
| 50 Portal | `r b scene_id` (x,y = góc trái-trên) |

---

## 5. Boilerplate cốt lõi (copy-paste)

> Thay `Xxx` bằng tên object. Giữ nguyên TAB, `#pragma once`, `LP...` typedef.

### 5.1. `AssetIDs.h` — nơi tập trung ID

```cpp
#pragma once

//
// This file contains all IDs of game assets 
//

#define ID_TEX_MARIO 0
#define ID_TEX_ENEMY 10
#define ID_TEX_MISC 20

#define OBJECT_TYPE_MARIO	0
#define OBJECT_TYPE_BRICK	1
#define OBJECT_TYPE_GOOMBA	2
#define OBJECT_TYPE_KOOPAS	3
#define OBJECT_TYPE_COIN 4
#define OBJECT_TYPE_PLATFORM 5

#define OBJECT_TYPE_PORTAL	50

#define ID_SPRITE_BRICK 20000
#define ID_SPRITE_GOOMBA 30000
#define ID_SPRITE_GOOMBA_WALK (ID_SPRITE_GOOMBA + 1000)
```

### 5.2. `GameObject.h` — lớp cha (giữ nguyên)

```cpp
#pragma once

#include <Windows.h>
#include <d3dx10.h>
#include <vector>

#include "Animation.h"
#include "Animations.h"
#include "Sprites.h"
#include "Collision.h"

using namespace std;

#define ID_TEX_BBOX -100		// special texture to draw object bounding box
#define BBOX_ALPHA 0.25f		// Bounding box transparency

class CGameObject
{
protected:
	float x; 
	float y;
	float vx;
	float vy;
	int nx;	 
	int state;
	bool isDeleted; 

public: 
	void SetPosition(float x, float y) { this->x = x, this->y = y; }
	void SetSpeed(float vx, float vy) { this->vx = vx, this->vy = vy; }
	void GetPosition(float &x, float &y) { x = this->x; y = this->y; }
	void GetSpeed(float &vx, float &vy) { vx = this->vx; vy = this->vy; }

	int GetState() { return this->state; }
	virtual void Delete() { isDeleted = true;  }
	bool IsDeleted() { return isDeleted; }

	void RenderBoundingBox();

	CGameObject();
	CGameObject(float x, float y) :CGameObject() { this->x = x; this->y = y; }

	virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom) = 0;
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects = NULL) {};
	virtual void Render() = 0;
	virtual void SetState(int state) { this->state = state; }

	// Collision ON or OFF ? This can change depending on object's state. For example: die
	virtual int IsCollidable() { return 0; };
	// When no collision has been detected (triggered by CCollision::Process)
	virtual void OnNoCollision(DWORD dt) {};
	// When collision with an object has been detected (triggered by CCollision::Process)
	virtual void OnCollisionWith(LPCOLLISIONEVENT e) {};
	// Is this object blocking other object? If YES, collision framework will automatically push the other object
	virtual int IsBlocking() { return 1; }
	// Does this object collide with other object at certain direction ( like ColorBox )
	virtual int IsDirectionColliable(float nx, float ny) { return 1; }

	~CGameObject();

	static bool IsDeleted(const LPGAMEOBJECT &o) { return o->isDeleted; }
};
```

### 5.3. Object **tĩnh** (mẫu `CBrick`; Coin chỉ khác `IsBlocking() { return 0; }`)

`Xxx.h`
```cpp
#pragma once

#include "GameObject.h"
#include "Animation.h"
#include "Animations.h"

#define ID_ANI_XXX 10000
#define XXX_BBOX_WIDTH 16
#define XXX_BBOX_HEIGHT 16

class CXxx : public CGameObject {
public:
	CXxx(float x, float y) : CGameObject(x, y) {}
	void Render();
	void Update(DWORD dt) {}
	void GetBoundingBox(float& l, float& t, float& r, float& b);
	// int IsBlocking() { return 0; }   // bật dòng này nếu là vật nhặt được (coin, item)
};
```

`Xxx.cpp`
```cpp
#include "Xxx.h"

void CXxx::Render()
{
	CAnimations* animations = CAnimations::GetInstance();
	animations->Get(ID_ANI_XXX)->Render(x, y);
	//RenderBoundingBox();
}

void CXxx::GetBoundingBox(float &l, float &t, float &r, float &b)
{
	l = x - XXX_BBOX_WIDTH/2;
	t = y - XXX_BBOX_HEIGHT/2;
	r = l + XXX_BBOX_WIDTH;
	b = t + XXX_BBOX_HEIGHT;
}
```

### 5.4. Object **động** có va chạm (mẫu `CGoomba` — enemy)

`Xxx.h`
```cpp
#pragma once
#include "GameObject.h"

#define XXX_GRAVITY 0.002f
#define XXX_WALKING_SPEED 0.05f

#define XXX_BBOX_WIDTH 16
#define XXX_BBOX_HEIGHT 14
#define XXX_BBOX_HEIGHT_DIE 7

#define XXX_DIE_TIMEOUT 500

#define XXX_STATE_WALKING 100
#define XXX_STATE_DIE 200

#define ID_ANI_XXX_WALKING 5000
#define ID_ANI_XXX_DIE 5001

class CXxx : public CGameObject
{
protected:
	float ax;				
	float ay; 

	ULONGLONG die_start;

	virtual void GetBoundingBox(float &left, float &top, float &right, float &bottom);
	virtual void Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects);
	virtual void Render();

	virtual int IsCollidable() { return 1; };
	virtual int IsBlocking() { return 0; }
	virtual void OnNoCollision(DWORD dt);
	virtual void OnCollisionWith(LPCOLLISIONEVENT e);

public: 	
	CXxx(float x, float y);
	virtual void SetState(int state);
};
```

`Xxx.cpp`
```cpp
#include "Xxx.h"

CXxx::CXxx(float x, float y):CGameObject(x, y)
{
	this->ax = 0;
	this->ay = XXX_GRAVITY;
	die_start = -1;
	SetState(XXX_STATE_WALKING);
}

void CXxx::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
	if (state == XXX_STATE_DIE)
	{
		left = x - XXX_BBOX_WIDTH/2;
		top = y - XXX_BBOX_HEIGHT_DIE/2;
		right = left + XXX_BBOX_WIDTH;
		bottom = top + XXX_BBOX_HEIGHT_DIE;
	}
	else
	{ 
		left = x - XXX_BBOX_WIDTH/2;
		top = y - XXX_BBOX_HEIGHT/2;
		right = left + XXX_BBOX_WIDTH;
		bottom = top + XXX_BBOX_HEIGHT;
	}
}

void CXxx::OnNoCollision(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;
};

void CXxx::OnCollisionWith(LPCOLLISIONEVENT e)
{
	if (!e->obj->IsBlocking()) return; 
	if (dynamic_cast<CXxx*>(e->obj)) return; 

	if (e->ny != 0 )      vy = 0;           // chạm sàn/trần
	else if (e->nx != 0)  vx = -vx;         // chạm tường -> quay đầu
}

void CXxx::Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects)
{
	vy += ay * dt;
	vx += ax * dt;

	if ( (state==XXX_STATE_DIE) && (GetTickCount64() - die_start > XXX_DIE_TIMEOUT) )
	{
		isDeleted = true;
		return;
	}

	CGameObject::Update(dt, coObjects);
	CCollision::GetInstance()->Process(this, dt, coObjects);
}

void CXxx::Render()
{
	int aniId = ID_ANI_XXX_WALKING;
	if (state == XXX_STATE_DIE) aniId = ID_ANI_XXX_DIE;

	CAnimations::GetInstance()->Get(aniId)->Render(x,y);
	RenderBoundingBox();
}

void CXxx::SetState(int state)
{
	CGameObject::SetState(state);
	switch (state)
	{
		case XXX_STATE_DIE:
			die_start = GetTickCount64();
			y += (XXX_BBOX_HEIGHT - XXX_BBOX_HEIGHT_DIE)/2;
			vx = 0; vy = 0; ay = 0; 
			break;
		case XXX_STATE_WALKING: 
			vx = -XXX_WALKING_SPEED;
			break;
	}
}
```

### 5.5. Player — khung `OnCollisionWith` (mẫu `CMario`)

```cpp
void CMario::Update(DWORD dt, vector<LPGAMEOBJECT> *coObjects)
{
	vy += ay * dt;
	vx += ax * dt;
	if (abs(vx) > abs(maxVx)) vx = maxVx;

	// reset untouchable timer if untouchable time has passed
	if ( GetTickCount64() - untouchable_start > MARIO_UNTOUCHABLE_TIME) 
	{
		untouchable_start = 0;
		untouchable = 0;
	}
	CCollision::GetInstance()->Process(this, dt, coObjects);
}

void CMario::OnNoCollision(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;
	isOnPlatform = false;
}

void CMario::OnCollisionWith(LPCOLLISIONEVENT e)
{
	if (e->ny != 0 && e->obj->IsBlocking())
	{
		vy = 0;
		if (e->ny < 0) isOnPlatform = true;
	}
	else if (e->nx != 0 && e->obj->IsBlocking())
	{
		vx = 0;
	}

	if (dynamic_cast<CGoomba*>(e->obj))       OnCollisionWithGoomba(e);
	else if (dynamic_cast<CCoin*>(e->obj))    OnCollisionWithCoin(e);
	else if (dynamic_cast<CPortal*>(e->obj))  OnCollisionWithPortal(e);
}

void CMario::OnCollisionWithCoin(LPCOLLISIONEVENT e)   { e->obj->Delete(); coin++; }

void CMario::OnCollisionWithPortal(LPCOLLISIONEVENT e)
{
	CPortal* p = (CPortal*)e->obj;
	CGame::GetInstance()->InitiateSwitchScene(p->GetSceneId());
}
```

Quy ước `e->nx / e->ny`: `ny < 0` = chạm từ **trên** (đứng lên vật), `ny > 0` = đụng trần, `nx != 0` = đụng cạnh bên.

### 5.6. Singleton DB (mẫu `CSprites`; `CTextures`, `CAnimations` y hệt)

`Xxxs.h`
```cpp
#pragma once
#include <unordered_map>
using namespace std;

class CXxxs
{
	static CXxxs* __instance;
	unordered_map<int, LPXXX> xxxs;
public:
	void Add(int id, /* params */ ...);
	LPXXX Get(int id);
	void Clear();

	static CXxxs* GetInstance();
};
```

`Xxxs.cpp`
```cpp
#include "Xxxs.h"

CXxxs* CXxxs::__instance = NULL;

CXxxs* CXxxs::GetInstance()
{
	if (__instance == NULL) __instance = new CXxxs();
	return __instance;
}

void CXxxs::Clear()
{
	for (auto x : xxxs)
	{
		LPXXX o = x.second;
		delete o;
	}
	xxxs.clear();
}
```

### 5.7. `Scene.h` — interface màn chơi + key handler

```cpp
#pragma once

#include "KeyEventHandler.h"

class CScene
{
protected:
	LPKEYEVENTHANDLER key_handler;
	int id;
	LPCWSTR sceneFilePath;

public: 
	CScene(int id, LPCWSTR filePath)
	{
		this->id = id;
		this->sceneFilePath = filePath;
		this->key_handler = NULL;
	}

	LPKEYEVENTHANDLER GetKeyEventHandler() { return key_handler; }
	virtual void Load() = 0;
	virtual void Unload() = 0;
	virtual void Update(DWORD dt) = 0;
	virtual void Render() = 0; 
};
typedef CScene * LPSCENE;

class CSceneKeyHandler : public CKeyEventHandler
{
protected: 
	CScene * scence; 

public:
	virtual void KeyState(BYTE *states) = 0;
	virtual void OnKeyDown(int KeyCode) = 0;
	virtual void OnKeyUp(int KeyCode) = 0;
	CSceneKeyHandler(LPSCENE s) :CKeyEventHandler() { scence = s; }
};
typedef CSceneKeyHandler* LPSCENEKEYHANDLER;
```

### 5.8. `KeyEventHandler.h` + handler mẫu

```cpp
// KeyEventHandler.h
#pragma once
#include <Windows.h>

class CKeyEventHandler
{
public:
	virtual void KeyState(BYTE* state) = 0;
	virtual void OnKeyDown(int KeyCode) = 0;
	virtual void OnKeyUp(int KeyCode) = 0;
};
typedef CKeyEventHandler* LPKEYEVENTHANDLER;
```

```cpp
// SampleKeyEventHandler.cpp
void CSampleKeyHandler::OnKeyDown(int KeyCode)       // 1 lần khi nhấn
{
	CMario* mario = (CMario *)((LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene())->GetPlayer(); 
	switch (KeyCode)
	{
	case DIK_S: mario->SetState(MARIO_STATE_JUMP); break;
	case DIK_DOWN: mario->SetState(MARIO_STATE_SIT); break;
	}
}

void CSampleKeyHandler::OnKeyUp(int KeyCode)         // 1 lần khi nhả
{
	CMario* mario = (CMario*)((LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene())->GetPlayer();
	switch (KeyCode)
	{
	case DIK_S: mario->SetState(MARIO_STATE_RELEASE_JUMP); break;
	case DIK_DOWN: mario->SetState(MARIO_STATE_SIT_RELEASE); break;
	}
}

void CSampleKeyHandler::KeyState(BYTE *states)       // mỗi frame khi đang giữ
{
	LPGAME game = CGame::GetInstance();
	CMario* mario = (CMario*)((LPPLAYSCENE)CGame::GetInstance()->GetCurrentScene())->GetPlayer();

	if (game->IsKeyDown(DIK_RIGHT))
		mario->SetState(game->IsKeyDown(DIK_A) ? MARIO_STATE_RUNNING_RIGHT : MARIO_STATE_WALKING_RIGHT);
	else if (game->IsKeyDown(DIK_LEFT))
		mario->SetState(game->IsKeyDown(DIK_A) ? MARIO_STATE_RUNNING_LEFT : MARIO_STATE_WALKING_LEFT);
	else
		mario->SetState(MARIO_STATE_IDLE);
}
```

Quy tắc: **`OnKeyDown/OnKeyUp` = hành động rời rạc** (nhảy, ngồi, đổi level); **`KeyState` = hành động liên tục** (đi, chạy). Mã phím dùng `DIK_*`.

### 5.9. `PlayScene.cpp` — bộ parse (khung bắt buộc giữ)

```cpp
#define SCENE_SECTION_UNKNOWN -1
#define SCENE_SECTION_ASSETS	1
#define SCENE_SECTION_OBJECTS	2

#define ASSETS_SECTION_UNKNOWN -1
#define ASSETS_SECTION_SPRITES 1
#define ASSETS_SECTION_ANIMATIONS 2

#define MAX_SCENE_LINE 1024

CPlayScene::CPlayScene(int id, LPCWSTR filePath):
	CScene(id, filePath)
{
	player = NULL;
	key_handler = new CSampleKeyHandler(this);
}

void CPlayScene::_ParseSection_SPRITES(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 6) return; // skip invalid lines

	int ID = atoi(tokens[0].c_str());
	int l = atoi(tokens[1].c_str());
	int t = atoi(tokens[2].c_str());
	int r = atoi(tokens[3].c_str());
	int b = atoi(tokens[4].c_str());
	int texID = atoi(tokens[5].c_str());

	LPTEXTURE tex = CTextures::GetInstance()->Get(texID);
	if (tex == NULL)
	{
		DebugOut(L"[ERROR] Texture ID %d not found!\n", texID);
		return; 
	}
	CSprites::GetInstance()->Add(ID, l, t, r, b, tex);
}

void CPlayScene::_ParseSection_ANIMATIONS(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 3) return; // an animation must have at least 1 frame

	LPANIMATION ani = new CAnimation();
	int ani_id = atoi(tokens[0].c_str());
	for (int i = 1; i < tokens.size(); i += 2)	// sprite_id | frame_time  
	{
		int sprite_id = atoi(tokens[i].c_str());
		int frame_time = atoi(tokens[i+1].c_str());
		ani->Add(sprite_id, frame_time);
	}
	CAnimations::GetInstance()->Add(ani_id, ani);
}

void CPlayScene::_ParseSection_ASSETS(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 1) return;
	wstring path = ToWSTR(tokens[0]);
	LoadAssets(path.c_str());
}

void CPlayScene::_ParseSection_OBJECTS(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 2) return;

	int object_type = atoi(tokens[0].c_str());
	float x = (float)atof(tokens[1].c_str());
	float y = (float)atof(tokens[2].c_str());

	CGameObject *obj = NULL;

	switch (object_type)
	{
	case OBJECT_TYPE_MARIO:
		if (player!=NULL) 
		{
			DebugOut(L"[ERROR] MARIO object was created before!\n");
			return;
		}
		obj = new CMario(x,y); 
		player = (CMario*)obj;  
		DebugOut(L"[INFO] Player object has been created!\n");
		break;
	case OBJECT_TYPE_GOOMBA: obj = new CGoomba(x,y); break;
	case OBJECT_TYPE_BRICK:  obj = new CBrick(x,y);  break;
	case OBJECT_TYPE_COIN:   obj = new CCoin(x, y);  break;

	// <<< THÊM LOẠI MỚI TẠI ĐÂY:  case OBJECT_TYPE_XXX: obj = new CXxx(x, y); break;

	case OBJECT_TYPE_PLATFORM:
	{
		float cell_width = (float)atof(tokens[3].c_str());
		float cell_height = (float)atof(tokens[4].c_str());
		int length = atoi(tokens[5].c_str());
		int sprite_begin = atoi(tokens[6].c_str());
		int sprite_middle = atoi(tokens[7].c_str());
		int sprite_end = atoi(tokens[8].c_str());
		obj = new CPlatform(x, y, cell_width, cell_height, length, sprite_begin, sprite_middle, sprite_end);
		break;
	}
	case OBJECT_TYPE_PORTAL:
	{
		float r = (float)atof(tokens[3].c_str());
		float b = (float)atof(tokens[4].c_str());
		int scene_id = atoi(tokens[5].c_str());
		obj = new CPortal(x, y, r, b, scene_id);
	}
	break;

	default:
		DebugOut(L"[ERROR] Invalid object type: %d\n", object_type);
		return;
	}

	// General object setup
	obj->SetPosition(x, y);
	objects.push_back(obj);
}

void CPlayScene::Load()
{
	DebugOut(L"[INFO] Start loading scene from : %s \n", sceneFilePath);

	ifstream f;
	f.open(sceneFilePath);
	int section = SCENE_SECTION_UNKNOWN;					

	char str[MAX_SCENE_LINE];
	while (f.getline(str, MAX_SCENE_LINE))
	{
		string line(str);

		if (line[0] == '#') continue;	// skip comment lines	
		if (line == "[ASSETS]") { section = SCENE_SECTION_ASSETS; continue; };
		if (line == "[OBJECTS]") { section = SCENE_SECTION_OBJECTS; continue; };
		if (line[0] == '[') { section = SCENE_SECTION_UNKNOWN; continue; }	

		switch (section)
		{ 
			case SCENE_SECTION_ASSETS: _ParseSection_ASSETS(line); break;
			case SCENE_SECTION_OBJECTS: _ParseSection_OBJECTS(line); break;
		}
	}
	f.close();
	DebugOut(L"[INFO] Done loading scene  %s\n", sceneFilePath);
}

void CPlayScene::Update(DWORD dt)
{
	// Player is the first object in the list, so it's not added into the collidable list
	vector<LPGAMEOBJECT> coObjects;
	for (size_t i = 1; i < objects.size(); i++) coObjects.push_back(objects[i]);

	for (size_t i = 0; i < objects.size(); i++) objects[i]->Update(dt, &coObjects);

	// skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return; 

	// Update camera to follow player
	float cx, cy;
	player->GetPosition(cx, cy);

	CGame *game = CGame::GetInstance();
	cx -= game->GetBackBufferWidth() / 2;
	cy -= game->GetBackBufferHeight() / 2;
	if (cx < 0) cx = 0;

	CGame::GetInstance()->SetCamPos(cx, 0.0f /*cy*/);

	PurgeDeletedObjects();
}

void CPlayScene::Render()
{
	for (int i = 0; i < objects.size(); i++) objects[i]->Render();
}

void CPlayScene::Unload()
{
	for (int i = 0; i < objects.size(); i++) delete objects[i];
	objects.clear();
	player = NULL;
	DebugOut(L"[INFO] Scene %d unloaded! \n", id);
}

bool CPlayScene::IsGameObjectDeleted(const LPGAMEOBJECT& o) { return o == NULL; }

void CPlayScene::PurgeDeletedObjects()
{
	vector<LPGAMEOBJECT>::iterator it;
	for (it = objects.begin(); it != objects.end(); it++)
	{
		LPGAMEOBJECT o = *it;
		if (o->IsDeleted()) { delete o; *it = NULL; }
	}
	// remove_if swaps all deleted items to the end, then trim the vector
	objects.erase(
		std::remove_if(objects.begin(), objects.end(), CPlayScene::IsGameObjectDeleted),
		objects.end());
}
```

### 5.10. `main.cpp` — khung ứng dụng (đổi hằng số đầu file)

```cpp
#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <list>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"
#include "Animation.h"
#include "Animations.h"

#include "Mario.h"
#include "Brick.h"
#include "Goomba.h"
#include "Coin.h"
#include "Platform.h"
#include "SampleKeyEventHandler.h"
#include "AssetIDs.h"

#define WINDOW_CLASS_NAME L"SampleWindow"
#define MAIN_WINDOW_TITLE L"<NN> - <TenProject>"
#define WINDOW_ICON_PATH L"mario.ico"

#define BACKGROUND_COLOR D3DXCOLOR(200.0f/255, 200.0f/255, 255.0f/255, 0.0f)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

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

void Update(DWORD dt)
{
	CGame::GetInstance()->GetCurrentScene()->Update(dt);
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

	CGame::GetInstance()->GetCurrentScene()->Render();

	spriteHandler->End();
	pSwapChain->Present(0, 0);
}

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

	HWND hWnd = CreateWindow(WINDOW_CLASS_NAME, MAIN_WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, NULL, NULL, hInstance, NULL);

	if (!hWnd) 
	{
		OutputDebugString(L"[ERROR] CreateWindow failed");
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG frameStart = GetTickCount64();
	DWORD tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();
		DWORD dt = (DWORD)(now - frameStart);

		if (dt >= tickPerFrame)
		{
			frameStart = now;

			CGame::GetInstance()->ProcessKeyboard();			
			Update(dt);
			Render();

			CGame::GetInstance()->SwitchScene();
		}
		else
			Sleep(tickPerFrame - dt);	
	}
	return 1;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{
	HWND hWnd = CreateGameWindow(hInstance, nCmdShow, SCREEN_WIDTH, SCREEN_HEIGHT);

	SetDebugWindow(hWnd);

	LPGAME game = CGame::GetInstance();
	game->Init(hWnd, hInstance);
	game->InitKeyboard();

	//IMPORTANT: this is the only place where a hardcoded file name is allowed ! 
	game->Load(L"mario-sample.txt");  

	SetWindowPos(hWnd, 0, 0, 0, SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	Run();
	return 0;
}
```

### 5.11. `Game.h` — API công khai của `CGame` (giữ nguyên chữ ký)

```cpp
#define MAX_FRAME_RATE 100
#define KEYBOARD_BUFFER_SIZE 1024
#define KEYBOARD_STATE_SIZE 256

class CGame
{
	static CGame* __instance;
	// ... D3D10 device / swapchain / rendertarget / blend / sampler, DirectInput, keyStates, keyEvents ...
	LPKEYEVENTHANDLER keyHandler;
	float cam_x = 0.0f, cam_y = 0.0f;

	unordered_map<int, LPSCENE> scenes;
	int current_scene = -1;
	int next_scene = -1;

	void _ParseSection_SETTINGS(string line);
	void _ParseSection_SCENES(string line);

public:
	void Init(HWND hWnd, HINSTANCE hInstance);

	// (x,y) is at the CENTER of the image; rect NULL -> draw whole texture
	void Draw(float x, float y, LPTEXTURE tex, RECT* rect = NULL, float alpha = 1.0f, int sprite_width = 0, int sprite_height = 0);
	void Draw(float x, float y, LPTEXTURE tex, int l, int t, int r, int b, float alpha = 1.0f, int sprite_width = 0, int sprite_height = 0);

	LPTEXTURE LoadTexture(LPCWSTR texturePath);

	void InitKeyboard();
	int  IsKeyDown(int KeyCode);
	void ProcessKeyboard();
	void SetKeyHandler(LPKEYEVENTHANDLER handler) { keyHandler = handler; }

	void SetCamPos(float x, float y) { cam_x = x; cam_y = y; }
	void GetCamPos(float& x, float& y) { x = cam_x; y = cam_y; }

	LPSCENE GetCurrentScene() { return scenes[current_scene]; }
	void Load(LPCWSTR gameFile);           // đọc mario-sample.txt
	void SwitchScene();                    // gọi cuối frame
	void InitiateSwitchScene(int scene_id);// chỉ đặt next_scene

	void _ParseSection_TEXTURES(string line);
	static CGame* GetInstance();
};
typedef CGame* LPGAME;
```

> ⚠️ **Lưu ý:** file `Game.cpp` trong dữ liệu bạn tải lên **không đọc được** (lỗi encoding `0x92`, tức file lưu Windows-1252 chứ không phải UTF-8). Vì vậy mục này dựa vào `Game.h` + cách `main.cpp`/`PlayScene.cpp` gọi nó. Phần thân `Init`, `Draw`, `LoadTexture`, `InitKeyboard`, `ProcessKeyboard`, `Load`, `SwitchScene`… **hãy copy nguyên từ `Game.cpp` gốc**, đừng viết lại. Nếu muốn mình đưa bản chuẩn hóa, hãy gửi lại file đó dưới dạng UTF-8.

### 5.12. Tiện ích & debug (giữ nguyên)

```cpp
// Utils
vector<string> split(string line, string delimeter = "\t");
wstring ToWSTR(string st);
LPCWSTR ToLPCWSTR(string st);

// debug
void DebugOut(const wchar_t* fmt, ...);        // ra Output window của VS
void DebugOutTitle(const wchar_t* fmt, ...);   // ghi lên title cửa sổ
void SetDebugWindow(HWND hwnd);
```

### 5.13. Cấu hình project (`packages.config` + vcxproj)

`packages.config`
```xml
<?xml version="1.0" encoding="utf-8"?>
<packages>
  <package id="Microsoft.DXSDK.D3DX" version="9.29.952.8" targetFramework="native" />
</packages>
```

`.vcxproj` — các điểm quan trọng (giữ nguyên khi copy):
- `PlatformToolset` = `v143`, `CharacterSet` = `Unicode`, `ConfigurationType` = `Application`, `WindowsTargetPlatformVersion` = `10.0`.
- Cấu hình: `Debug|Win32`, `Release|Win32`, `Debug|x64`, `Release|x64`.
- Preprocessor: `WIN32;_DEBUG;_WINDOWS` (Debug) / `WIN32;NDEBUG;_WINDOWS` (Release); `ConformanceMode=true`; `SubSystem=Windows` (→ dùng `WinMain`).
- **Additional Dependencies:** `d3d10.lib;dxguid.lib;dinput8.lib`.
- Cài NuGet: *Tools > NuGet Package Manager > Package Manager Console* → `Install-Package Microsoft.DXSDK.D3DX`.
- Đổi `ProjectGuid` và `RootNamespace` cho project mới (tránh trùng GUID trong `.sln`).
- Đặt Working Directory trỏ về thư mục chứa `.txt` và `textures\` (vì code đọc đường dẫn tương đối).

---

## 6. Checklist "không cần suy nghĩ"

### 6.1. Tạo dự án mới từ template
1. Copy thư mục `05-SceneManager` → `NN-TenMoi`; đổi tên `.vcxproj`, `.vcxproj.filters`; đổi `ProjectGuid`/`RootNamespace`; thêm vào `.sln`.
2. Xóa `Scence.cpp`. Xóa object mẫu bạn không dùng (`Goomba`, `Coin`…), **giữ lại engine core** (Mục 1).
3. Sửa `MAIN_WINDOW_TITLE`, `SCREEN_WIDTH/HEIGHT`, `BACKGROUND_COLOR` trong `main.cpp`.
4. Sửa `mario-sample.txt`: `[SETTINGS]` (`start`, kích thước), `[SCENES]`, `[TEXTURES]`.
5. Đặt ảnh vào `textures\` (nhớ có `bbox.png` với id `-100` để vẽ bounding box).
6. Build thử với scene rỗng chỉ có player.

### 6.2. Thêm 1 loại object mới `Xxx`
1. `AssetIDs.h`: thêm `#define OBJECT_TYPE_XXX <số mới>` (+ ID sprite nếu cần).
2. Tạo `Xxx.h/.cpp` từ mẫu 5.3 (tĩnh) hoặc 5.4 (động). Đặt các `#define XXX_...` ở đầu `Xxx.h`.
3. Tạo `xxx.txt` (`[SPRITES]` + `[ANIMATIONS]`) — sprite id theo dải mới, animation id không trùng.
4. `PlayScene.cpp`: `#include "Xxx.h"` và thêm `case OBJECT_TYPE_XXX:` trong `_ParseSection_OBJECTS` (lấy thêm cột extra nếu có, ghi vào bảng 4.4).
5. Scene muốn dùng: thêm `xxx.txt` vào `[ASSETS]`, thêm dòng `<type> <x> <y> ...` vào `[OBJECTS]`.
6. Nếu Mario phản ứng với `Xxx`: thêm `#include "Xxx.h"` trong `Mario.cpp`, thêm nhánh `dynamic_cast<CXxx*>` trong `OnCollisionWith` và hàm `OnCollisionWithXxx(LPCOLLISIONEVENT e)` (khai báo private trong `Mario.h`).
7. Thêm 2 file vào `.vcxproj` + `.filters`.

### 6.3. Thêm 1 scene mới
1. Tạo `sceneNN.txt` (copy `scene01.txt`): `[ASSETS]` + `[OBJECTS]`, **Mario/player là dòng object đầu tiên**.
2. Thêm `NN	sceneNN.txt` vào `[SCENES]` của `mario-sample.txt`.
3. Nối từ scene khác bằng Portal: `50 l t r b NN`.

### 6.4. Thêm 1 animation / sprite
1. Mở file asset tương ứng, thêm dòng vào `[SPRITES]`: `id l t r b texture_id`.
2. Thêm dòng vào `[ANIMATIONS]`: `ani_id spr1 time1 spr2 time2 ...`.
3. Khai báo `#define ID_ANI_...` trong `.h` của object, và chọn nó trong `Render()`.

### 6.5. Thêm 1 state mới
1. `#define XXX_STATE_<TÊN> <số>` (bước 100; "nhả" = +1).
2. Thêm `case` trong `SetState` (đặt `vx/vy/ax`, `nx`, bbox adjust nếu cần) — **luôn gọi `CGameObject::SetState(state)` ở cuối/đầu**.
3. Thêm nhánh chọn animation trong `Render()`; nếu bbox khác thì xử lý trong `GetBoundingBox()`.

### 6.6. Cần object chặn một chiều (như nền Platform)
Override `IsDirectionColliable(float nx, float ny)` — trả `1` chỉ cho hướng cho phép (`nx==0 && ny==-1` = chỉ chặn từ trên).

---

## 7. Quy tắc vàng (DO / DON'T)

**DO**
- Đặt mọi ID mới vào `AssetIDs.h`; mọi tham số vật lý/bbox vào `.h` của object.
- Cho object tự tính `aniId` mỗi frame trong `Render()` từ `state`.
- Xóa object bằng `Delete()` (đặt cờ `isDeleted`), để `PurgeDeletedObjects()` dọn cuối frame.
- Gọi `CCollision::GetInstance()->Process(this, dt, coObjects)` **một lần** trong `Update` của object có va chạm, và cập nhật vị trí trong `OnNoCollision`.
- Đổi scene bằng `InitiateSwitchScene`, **không** gọi `SwitchScene` trực tiếp trong lúc đang Update.
- Dùng `DebugOut(L"[ERROR] ...")` khi không tìm thấy asset/ID.

**DON'T**
- Đừng hardcode đường dẫn file ngoài `game->Load(L"mario-sample.txt")`.
- Đừng rải biến toàn cục; dữ liệu dùng chung (texture, sprite, animation, game) đi qua Singleton `GetInstance()`.
- Đừng thêm thư viện ngoài D3DX/DirectInput/STL/Win32.
- Đừng dùng `const`/`enum class` cho hằng số game — giữ `#define`.
- Đừng đưa player xuống giữa danh sách `[OBJECTS]`.
- Đừng khai báo `Update(DWORD dt) {}` cho object mới: scene gọi `Update(dt, &coObjects)`, nên phải khai báo đúng chữ ký đó (xem mục 8, dòng 4).

---

## 8. Điểm yếu đã biết của template (nên né khi dùng làm khuôn)

| # | Vấn đề trong template | Cách xử lý đề xuất |
|---|---|---|
| 1 | `Scence.cpp` sai chính tả, include `Scence.h` không tồn tại | Xóa file; `Scene.h` đã định nghĩa đủ |
| 2 | `main.cpp` ghi `MAIN_WINDOW_TITLE L"04 - Collision"` ở bản 05 | Đổi tiêu đề theo project |
| 3 | `mario-sample.txt` liệt kê scene `0 intro.txt` không có trong repo, và comment `# type` không khớp cột `id file` | Chỉ liệt kê scene thực sự tồn tại |
| 4 | `Brick/Coin/Platform` khai báo `void Update(DWORD dt) {}` — **không override** `Update(DWORD, vector*)`, nên thực tế chạy hàm rỗng của lớp cha (vô tình đúng) | Với object mới, khai báo đúng `void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)` |
| 5 | `CPlayScene::Update` giả định player là `objects[0]` ("dirty way" theo chú thích gốc) | Luôn để player đầu tiên trong `[OBJECTS]`, hoặc tự tách danh sách |
| 6 | `CPlayScene::Unload` chỉ xóa object; `TODO` chú thích: chưa dọn sprite/animation/texture | Gọi `CSprites::Clear()`, `CAnimations::Clear()` khi đổi scene nếu thêm scene nhiều |
| 7 | `Render()` của Goomba/Platform/Portal đang gọi `RenderBoundingBox()` (debug) | Comment lại khi release |
| 8 | `ToWSTR`/`ToLPCWSTR` cấp phát bằng `new` không `delete` (rò rỉ nhỏ, chú thích gốc đã nghi ngờ) | Chấp nhận khi load scene; không gọi mỗi frame |
| 9 | Hằng `ID_SPRITE_MARIO_SMALL_BRACE` trùng `+500` với `SMALL_SIT` | Kiểm tra trùng ID khi thêm dải mới |
| 10 | `Game.cpp` lưu ở Windows-1252 (byte `0x92`) | Lưu lại UTF-8 hoặc giữ nguyên encoding khi copy |

---

## 9. Mẫu prompt để dùng guide này với Claude

> "Dựa theo **Architecture & Boilerplate Guide** (template 05-SceneManager): tạo object `CKoopas` (type = 3) đi qua lại, bị đá thành mai rùa khi Mario nhảy lên. Cho đủ: `AssetIDs.h`, `Koopas.h/.cpp`, `koopas.txt`, phần thêm vào `PlayScene.cpp`, dòng thêm vào `sceneNN.txt`, và đoạn xử lý trong `Mario.cpp`. Tuân thủ nghiêm quy tắc đặt tên, `#define`, TAB, `LP...`, và checklist 6.2."
