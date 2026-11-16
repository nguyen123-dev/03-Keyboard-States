#include "GameObject.h"
#include "debug.h"
class Tank : public CGameObject {
protected:
	float Vx;
	float Vy;
public:
	Tank(float x, float y);
	void Render();
	void Update(DWORD dt);
	void SetState(int state);
};