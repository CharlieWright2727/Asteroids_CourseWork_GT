#include "Bomb.h"
#include "BombExplosion.h"
#include "Explosion.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "Sprite.h"
#include "BoundingSphere.h"
#include "GameWorld.h"

Bomb::Bomb(GLVector3f p, GLVector3f v, int fuse_ms)
    : GameObject("Bomb", p, v, GLVector3f(0, 0, 0), 0, 0),
    mFuseMillis(fuse_ms),
    mExploded(false)
{
}

Bomb::~Bomb(void)
{
}

void Bomb::Update(int t)
{
    GameObject::Update(t);

    if (mExploded) return;

    mFuseMillis -= t;
    if (mFuseMillis > 0) return;

    mExploded = true;

    Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
    shared_ptr<Sprite> explosion_sprite =
        make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
    explosion_sprite->SetLoopAnimation(false);

    shared_ptr<GameObject> explosion = make_shared<Explosion>();
    explosion->SetSprite(explosion_sprite);
    explosion->Reset();
    explosion->SetPosition(mPosition);
    explosion->SetRotation(mRotation);
    mWorld->AddObject(explosion);

    shared_ptr<GameObject> expl = make_shared<BombExplosion>();
    expl->SetBoundingShape(make_shared<BoundingSphere>(expl->GetThisPtr(), 22.0f));
    expl->SetPosition(mPosition);
    mWorld->AddObject(expl);

    mWorld->FlagForRemoval(GetThisPtr());
}