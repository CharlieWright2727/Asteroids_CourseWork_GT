#include "BombExplosion.h"
#include "GameWorld.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GameObjectType.h"

BombExplosion::BombExplosion()
    : GameObject("BombExplosion"), mTimeLeft(50)
{
}

BombExplosion::~BombExplosion(void)
{
}

void BombExplosion::Update(int t)
{
    GameObject::Update(t);

    mTimeLeft -= t;
    if (mTimeLeft <= 0 && mWorld)
    {
        mWorld->FlagForRemoval(GetThisPtr());
    }
}

bool BombExplosion::CollisionTest(shared_ptr<GameObject> o)
{
    if (o->GetType() != GameObjectType("Asteroid") &&
        o->GetType() != GameObjectType("Spaceship"))
        return false;

    if (mBoundingShape.get() == NULL) return false;

    if (o->GetBoundingShape().get() == NULL) return false;

    return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void BombExplosion::OnCollision(const GameObjectList& objects)
{
    for (GameObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
        if ((*it)->GetType() == GameObjectType("Asteroid"))
            
        {
            mWorld->FlagForRemoval((*it)->GetThisPtr());
        }
    }

    mWorld->FlagForRemoval(GetThisPtr());
}