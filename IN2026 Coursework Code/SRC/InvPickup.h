#ifndef __INVPICKUP_H__
#define __INVPICKUP_H__

#include"Pickup.h"

class InvPickup : public Pickup {
public:
	InvPickup(int ttl = 4000, int duration_ms = 5000);
	virtual ~InvPickup(void);

protected:
	virtual void ApplyToShip(shared_ptr<Spaceship> ship);
private:
	int mDurationMs;
};

#endif 
