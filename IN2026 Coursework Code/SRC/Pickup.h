#ifndef __PICKUP_H__
#define __PICKUP_H__

#include "GameUtil.h"
#include "GameObject.h"

class Spaceship;

class Pickup : public GameObject {

public: 
	Pickup(const char* type_name, int ttl = 4000);
	virtual ~Pickup(void);
	virtual void Update(int t);
	virtual bool CollisionTest(shared_ptr<GameObject> o);
	virtual void OnCollision(const GameObjectList& objects);

	void SetTargetShip(shared_ptr<Spaceship> ship);

protected:
	virtual void ApplyToShip(shared_ptr<Spaceship> ship) = 0;

	int mTimeToLive;
	weak_ptr<Spaceship> mTargetShip;
};
#endif