#ifndef __BOMB_H__
#define __BOMB_H__

#include "GameUtil.h"
#include "GameObject.h"

class Bomb : public GameObject {

public:
	Bomb(GLVector3f p, GLVector3f v, int fuse_ms = 1500);
	virtual ~Bomb(void);

	virtual void Update(int t);

private:
	int mFuseMillis;
	bool mExploded;
};
#endif