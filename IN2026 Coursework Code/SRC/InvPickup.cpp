#include "InvPickup.h"

#include "Spaceship.h"

InvPickup::InvPickup(int ttl, int duration_ms) : Pickup("InvPickup", ttl), mDurationMs(duration_ms) {
	mAngle = rand() % 360;
	mVelocity.x = 6.0f * mAngle;
	mVelocity.y = 6.0f * mAngle;
	mVelocity.z = 0.0f;
}

InvPickup::~InvPickup(void) {

}

void InvPickup::ApplyToShip(shared_ptr<Spaceship> ship) {
	ship->SetInvulnerable(mDurationMs);
}