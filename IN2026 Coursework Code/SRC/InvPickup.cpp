#include "InvPickup.h"

#include "Spaceship.h"

InvPickup::InvPickup(int ttl, int duration_ms) : Pickup("InvPickup", ttl), mDurationMs(duration_ms) {
	
	mAngle = rand() % 360;
	mRotation = 0; // rand() % 90;
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	mVelocity.x = 12.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 12.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}

InvPickup::~InvPickup(void) {

}

void InvPickup::ApplyToShip(shared_ptr<Spaceship> ship) {
	ship->SetInvulnerable(mDurationMs);
}