#ifndef __BOMBEXPLOSION_H__
#define __BOMBEXPLOSION_H__

#include "GameUtil.h"
#include "GameObject.h"

class BombExplosion : public GameObject {
public: 
	BombExplosion();
	virtual ~BombExplosion(void);
	virtual void Update(int t);
	virtual bool CollisionTest(shared_ptr<GameObject> o);
	virtual void OnCollision(const GameObjectList& objects);

private:
		int mTimeLeft;
};
#endif