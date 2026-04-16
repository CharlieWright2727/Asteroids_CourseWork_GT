#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameSession.h"
#include "GameUtil.h"
#include "IGameWorldListener.h"
#include "IKeyboardListener.h"
#include "IPlayerListener.h"
#include "IScoreListener.h" 
#include "Player.h"
#include "ScoreKeeper.h"

#include <vector>

class GameObject;
class Spaceship;
class GUILabel;


enum GameState
{
	STATE_MENU,
	STATE_PLAYING,
	STATE_ENTER_NAME,
	STATE_GAME_OVER
};

struct HighScoreEntry
{
	string name;
	int score;
};

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	shared_ptr<GUILabel> mTitleLabel;
	shared_ptr<GUILabel> mStartLabel;
	shared_ptr<GUILabel> mInstructionsLabel;
	shared_ptr<GUILabel> mInstructionsTextLabel;
	shared_ptr<GUILabel> mHighScoreLabel;
	shared_ptr<GUILabel> mHighScoreTableLabel;

	shared_ptr<GUILabel> mEnterNameLabel;
	shared_ptr<GUILabel> mTypedNameLabel;

	shared_ptr<GUILabel> mGameOverOptionsLabel;
	shared_ptr<GUILabel> mGameOverScoresLabel;

	GameState mState;

	int mCurrentScore;
	string mEnteredName;
	vector<HighScoreEntry> mHighScores;

	bool mMenuShowingHS;
	bool mGameOverShowingHS;





	uint mLevel;
	uint mAsteroidCount;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);

	void ShowMenuGUI();
	void NameEntry();
	void SaveHighScore(const string& name, int score);
	void LoadHighScores();
	void WriteHighScores();
	void UpdateHighScoreTableText();
	bool IsTopTenScore(int score);
	void ShowMenuHS(bool show);
	void ShowGameOverScreen();
	void ReturnToMenu();
	void RestartGame();
	void ClearObjects();
	void UpdateNameGUI();

	void CreateMenuGUI();
	void HideMenuGUI();
	void StartGameplay();


	shared_ptr<GameObject> CreateExplosion();
	
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif