#ifndef __LIFEPICKUP_H__
#define __LIFEPICKUP_H__

#include"Pickup.h"

class Asteroids;

class LifePickup : public Pickup {
public:
	LifePickup(int ttl = 4000);
	virtual ~LifePickup(void);

	void SetAsteroidsTarget(Asteroids* game);

protected:
	virtual void ApplyToShip(shared_ptr<Spaceship> ship);
private:
	Asteroids* mGame;
};

#endif 
