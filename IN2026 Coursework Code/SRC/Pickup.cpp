#include "Pickup.h"
#include "Spaceship.h"
#include "GameWorld.h"
#include "BoundingShape.h"
#include "GameObjectType.h"

Pickup::Pickup(const char* type_name, int ttl) : GameObject(type_name), mTimeToLive(ttl) {

}

Pickup::~Pickup(void) {

}
void Pickup::SetTargetShip(shared_ptr<Spaceship> ship) {
	mTargetShip = ship;
}

void Pickup::Update(int t) {
	GameObject::Update(t);

	mTimeToLive -= t;
	if (mTimeToLive <= 0 && mWorld) {
		mWorld->FlagForRemoval(GetThisPtr());
	}

}

bool Pickup::CollisionTest(shared_ptr<GameObject> o) {
	if (o->GetType() != GameObjectType("Bullet")) 
		return false;
	if (mBoundingShape.get() == NULL)
		return false;
	if (o->GetBoundingShape().get() == NULL)
		return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Pickup::OnCollision(const GameObjectList& objects) {
	bool hit = false;

	for (GameObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it) {

		if ((*it)->GetType() == GameObjectType("Bullet"))
		{
			hit = true;
			break;
		}
	}
	if (hit) {
		shared_ptr<Spaceship> ship = mTargetShip.lock();
		if (ship) {
			ApplyToShip(ship);
		}
	}
	if (mWorld)
		mWorld->FlagForRemoval(GetThisPtr());
}