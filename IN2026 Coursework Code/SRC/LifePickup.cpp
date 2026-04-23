#include "LifePickup.h"
#include "Asteroids.h"
#include "Spaceship.h"

LifePickup::LifePickup(int ttl) : Pickup("LifePickup", ttl), mGame(NULL) {

	mAngle = rand() % 360;
	mRotation = 0; // rand() % 90;
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	mVelocity.x = 12.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 12.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}

LifePickup::~LifePickup(void) {

}

void LifePickup::SetAsteroidsTarget(Asteroids* game) {
	mGame = game;
}

void LifePickup::ApplyToShip(shared_ptr<Spaceship> ship) {
	if (mGame) {
		mGame->GrantLife();
	}
}