#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "bomb.h"
#include "BombExplosion.h"
#include "InvPickup.h"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;

	mState = STATE_MENU;
	mCurrentScore = 0;
	mEnteredName = "";

	mMenuShowingHS = false;
	mGameOverShowingHS = false;

	mExtraLivesEnabled = true;
	mNextLivesScore = 20;

	mBombReady = true;
	mBombCooldown = 0;

}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	// Create a spaceship and add it to the world

	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mGameOverLabel->SetVisible(false);
	mBombLabel->SetVisible(false);
	mINVLabel->SetVisible(false);


	CreateMenuGUI();
	LoadHighScores();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (mState == STATE_MENU)
	{
		switch (key)
		{
		case 13: // Enter
			StartGameplay();
			break;

		case 'i':
		case 'I':
			if (mInstructionsTextLabel && mInstructionsLabel)
			{

				bool show = !mInstructionsTextLabel->GetVisible();

				mInstructionsTextLabel->SetVisible(show);
				mInstructionsLabel->SetVisible(!show);
			}
			break;

		case 'h':
		case 'H':
			ShowMenuHS(!mMenuShowingHS);
			break;

		default:
			break;
		}
		return;
	}


	if (mState == STATE_ENTER_NAME)
	{
		if (key == 13) {
			if (!mEnteredName.empty()) {
				SaveHighScore(mEnteredName, mCurrentScore);
				ShowGameOverScreen();

			}
			return;
		}

		if (key == 8) // backspace 
		{
			if (!mEnteredName.empty()) {
				mEnteredName.erase(mEnteredName.size() - 1, 1);
				UpdateNameGUI();
			}
			return;
		}
		if (isalnum(key) && mEnteredName.size() < 10) {
			mEnteredName += key;
			UpdateNameGUI();
		}

		return;
	}
	if (mState == STATE_GAME_OVER)
	{
		switch (key)
		{
		case 'p':
		case 'P':
			RestartGame();
			break;

		case 'm':
		case 'M':
			ReturnToMenu();
			break;

		default:
			break;
		}
		return;
	}

	switch (key)
	{
	case ' ':
		if (mSpaceship) mSpaceship->Shoot();
		break;
	// add bomb ready clause later

	case 'b':
	case 'B':
		DropBomb();
		break;
	default:
		break;
	}
}
void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (mState != STATE_PLAYING || !mSpaceship) return;

	switch (key)
	{
		// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
		// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
		// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		// create difficulty stat and disable this on harder difficulty make forward thrust slowe and rotations imbalanced
	case GLUT_KEY_DOWN: mSpaceship->Thrust(-5); break;
		// Default case - do nothing

	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (mState != STATE_PLAYING || !mSpaceship) return;

	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		// Default case - do nothing
	case GLUT_KEY_DOWN: mSpaceship->Thrust(0); break;
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		if (mState == STATE_PLAYING)
		{
			shared_ptr<GameObject> explosion = CreateExplosion();
			explosion->SetPosition(object->GetPosition());
			explosion->SetRotation(object->GetRotation());
			mGameWorld->AddObject(explosion);
			mAsteroidCount--;
			if (mAsteroidCount <= 0)
			{
				SetTimer(500, START_NEXT_LEVEL);
			}
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mSpaceship->SetInvulnerable(3000);
		mGameWorld->AddObject(mSpaceship);
		UpdateINVGUI();
		SetTimer(100, INV_TICK);
	}
	


		if (value == START_NEXT_LEVEL)
		{
			if (mState == STATE_PLAYING) {
			mLevel++;
			int num_asteroids = 10 + 2 * mLevel;
			mSpaceship->SetInvulnerable(1000);
			UpdateINVGUI();
			SetTimer(100, INV_TICK);
			CreateAsteroids(num_asteroids);
			CreateInvPickup(0.0f);
		}
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
	}
	if (value == BOMB_COOLDOWN) {
		if (!mBombReady){

			mBombCooldown--;
			if (mBombCooldown <= 0) {
				mBombCooldown = 0;
				mBombReady = true;
			}
			else
			{
				SetTimer(1000, BOMB_COOLDOWN);
			}

			UpdateBombGUI();


		}
	}
	if (value == INV_TICK) {
		UpdateINVGUI();

		if (mSpaceship && mSpaceship->IsInvulnerable()) {
			SetTimer(100, INV_TICK);
		}
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	mSpaceship->SetInvulnerable(3000);
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

shared_ptr<GameObject> Asteroids::CreateInvPickup(GLVector3f pos) {
	shared_ptr<InvPickup> pickup = make_shared<InvPickup>(4000, 5000);

	pickup->SetBoundingShape(make_shared<BoundingSphere>(pickup->GetThisPtr(), 3.0f));
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spcaeship");
	shared_ptr<Sprite> pickup_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

	pickup->SetSprite(pickup_sprite);
	pickup->SetScale(0.05f);
	pickup->SetPosition(pos);

	return pickup;
}



void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	mBombLabel = make_shared<GUILabel>("Bomb: READY");
	mBombLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mBombLabel->SetVisible(false);
	shared_ptr<GUIComponent> bomb_component
		= static_pointer_cast<GUIComponent>(mBombLabel);
	mGameDisplay->GetContainer()->AddComponent(bomb_component, GLVector2f(0.0f, 0.94f));

	mINVLabel = make_shared<GUILabel>("");
	mINVLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mINVLabel->SetVisible(false);
	shared_ptr<GUIComponent> invulnerable_component
		= static_pointer_cast<GUIComponent>(mINVLabel);
	mGameDisplay->GetContainer()->AddComponent(invulnerable_component, GLVector2f(0.0f, 0.88f));




	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

}
void Asteroids::CreateMenuGUI() {

	mTitleLabel = make_shared<GUILabel>("ASTEROIDS");
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);

	mStartLabel = make_shared<GUILabel>("Press Enter to Start");
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);

	mInstructionsLabel = make_shared<GUILabel>("I - Instructions");
	mInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);

	mInstructionsTextLabel = make_shared<GUILabel>(" L/R = Rotate, U/D = Move, Space = shoot ");
	mInstructionsTextLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInstructionsTextLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mInstructionsTextLabel->SetVisible(false);

	mHighScoreLabel = make_shared<GUILabel>("H - High Scores");
	mHighScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mHighScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);

	mEnterNameLabel = make_shared<GUILabel>("Enter name:");
	mEnterNameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mEnterNameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mEnterNameLabel->SetVisible(false);

	mTypedNameLabel = make_shared<GUILabel>("");
	mTypedNameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTypedNameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mTypedNameLabel->SetVisible(false);

	mHighScoreTableLabel = make_shared<GUILabel>("");
	mHighScoreTableLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mHighScoreTableLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mHighScoreTableLabel->SetVisible(false);

	mGameOverOptionsLabel = make_shared<GUILabel>("P play again   M main menu");
	mGameOverOptionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverOptionsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mGameOverOptionsLabel->SetVisible(false);

	mGameOverScoresLabel = make_shared<GUILabel>("");
	mGameOverScoresLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverScoresLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mGameOverScoresLabel->SetVisible(false);


	for (int i = 0; i < 10; i++)
	{
		shared_ptr<GUILabel> row = make_shared<GUILabel>("");
		row->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		row->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
		row->SetVisible(false);

		mHSRows.push_back(row);
		float y = 0.75f - (i * 0.05f);
		mGameDisplay->GetContainer()->AddComponent(
			static_pointer_cast<GUIComponent>(row), GLVector2f(0.5f, y));

	}

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.80f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mStartLabel), GLVector2f(0.5f, 0.50f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsLabel), GLVector2f(0.5f, 0.30f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mInstructionsTextLabel), GLVector2f(0.5f, 0.30f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mHighScoreLabel), GLVector2f(0.5f, 0.20f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mHighScoreTableLabel), GLVector2f(0.5f, 0.20f));



	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mEnterNameLabel), GLVector2f(0.5f, 0.45f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mTypedNameLabel), GLVector2f(0.5f, 0.38f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mGameOverScoresLabel), GLVector2f(0.5f, 0.35f));

	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mGameOverOptionsLabel), GLVector2f(0.5f, 0.18f));
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mGameOverLabel), GLVector2f(0.5f, 0.95f));
}
void Asteroids::HideMenuGUI() {
	if (mTitleLabel) mTitleLabel->SetVisible(false);
	if (mStartLabel) mStartLabel->SetVisible(false);
	if (mInstructionsLabel) mInstructionsLabel->SetVisible(false);
	if (mHighScoreLabel) mHighScoreLabel->SetVisible(false);

	if (mInstructionsTextLabel) mInstructionsTextLabel->SetVisible(false);

	if (mEnterNameLabel) mEnterNameLabel->SetVisible(false);
	if (mTypedNameLabel) mTypedNameLabel->SetVisible(false);

	if (mGameOverOptionsLabel) mGameOverOptionsLabel->SetVisible(false);
	if (mGameOverScoresLabel) mGameOverScoresLabel->SetVisible(false);

	if (mHighScoreTableLabel) mHighScoreTableLabel->SetVisible(false);

	for (size_t i = 0; i < mHSRows.size(); i++)
		mHSRows[i]->SetVisible(false);


	mMenuShowingHS = false;


}
void Asteroids::ShowMenuGUI()
{
	if (mTitleLabel) mTitleLabel->SetVisible(true);
	if (mStartLabel) mStartLabel->SetVisible(true);
	if (mInstructionsLabel) mInstructionsLabel->SetVisible(true);

	if (mHighScoreLabel) mHighScoreLabel->SetVisible(!mMenuShowingHS);
	if (mHighScoreTableLabel) mHighScoreTableLabel->SetVisible(mMenuShowingHS);


	if (mInstructionsTextLabel) mInstructionsTextLabel->SetVisible(false);


	if (mEnterNameLabel) mEnterNameLabel->SetVisible(false);

	if (mTypedNameLabel) mTypedNameLabel->SetVisible(false);

	if (mGameOverOptionsLabel) mGameOverOptionsLabel->SetVisible(false);

	if (mGameOverScoresLabel) mGameOverScoresLabel->SetVisible(false);
	


}
void Asteroids::StartGameplay() {
	if (mState == STATE_PLAYING) return;

	mState = STATE_PLAYING;
	mEnteredName = "";
	mCurrentScore = 0;
	mNextLivesScore = 500;
	mGameOverShowingHS = false;
	HideMenuGUI();

	mPlayer.Reset(3);
	mScoreKeeper.Reset();

	mBombReady = true;
	mBombCooldown = 0;

	UpdateLivesGUI(mPlayer.GetLives());
	UpdateBombGUI();

	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);
	mGameOverLabel->SetVisible(false);
	mBombLabel->SetVisible(true);
	

	mGameWorld->AddObject(CreateSpaceship());

	UpdateINVGUI();
	SetTimer(100, INV_TICK);


}


void Asteroids::NameEntry() {

	mState = STATE_ENTER_NAME;
	mEnteredName = "";

	mGameOverLabel->SetVisible(true);
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mBombLabel->SetVisible(false);
	mINVLabel->SetVisible(false);

	if (mEnterNameLabel) mEnterNameLabel->SetVisible(true);
	if (mTypedNameLabel) mTypedNameLabel->SetVisible(true);

	if (mGameOverScoresLabel) mGameOverScoresLabel->SetVisible(false);
	if (mGameOverOptionsLabel) mGameOverOptionsLabel->SetVisible(false);

	UpdateNameGUI();


}

void Asteroids::UpdateNameGUI() {

	if (mTypedNameLabel) {
		std::ostringstream msg;
		msg << mEnteredName;
		mTypedNameLabel->SetText(msg.str());
	}
}

void Asteroids::UpdateINVGUI() {
	if (!mINVLabel || !mSpaceship) return;
	if (mSpaceship->IsInvulnerable()) {
		int m = mSpaceship->GetInvulnerableTime();
		int t = (m + 99) / 100;

		std::ostringstream msg_stream;
		msg_stream << "INVULNERABLE: " << (t / 10) << "." << (t % 10) << "s";

		mINVLabel->SetText(msg_stream.str());
		mINVLabel->SetVisible(true);
	}
	else {
		mINVLabel->SetVisible(false);
	}
}

void Asteroids::UpdateLivesGUI(int lives) {
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives;
	mLivesLabel->SetText(msg_stream.str());
	
}

void Asteroids::LivesCheck(int score) {
	if (!mExtraLivesEnabled) return;

	while (score >= mNextLivesScore)
	{
		mPlayer.AddLife(1);
		UpdateLivesGUI(mPlayer.GetLives());
		mNextLivesScore += 500;
	}
}


void Asteroids::OnScoreChanged(int score)
{
	mCurrentScore = score;

	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);

	LivesCheck(score);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	UpdateLivesGUI(lives_left);

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else
	{
		if (IsTopTenScore(mCurrentScore)) {
			NameEntry();
		}
		else
		{
			ShowGameOverScreen();
		}
	}
}

bool Asteroids::IsTopTenScore(int score)
{
	LoadHighScores();

	if (mHighScores.size() < 10) return true;
	return score > mHighScores.back().score;
}

void Asteroids::LoadHighScores()
{
	mHighScores.clear();

	std::ifstream file("highscores.txt");
	if (!file) return;

	string name;
	int score;

	while (file >> name >> score)
	{
		HighScoreEntry entry;
		entry.name = name;
		entry.score = score;
		mHighScores.push_back(entry);
	}

	std::sort(mHighScores.begin(), mHighScores.end(),
		[](const HighScoreEntry& a, const HighScoreEntry& b)
		{
			return a.score > b.score;
		});

	if (mHighScores.size() > 10)
		mHighScores.resize(10);
}

void Asteroids::WriteHighScores()
{
	std::ofstream file("highscores.txt", std::ios::trunc);

	for (size_t i = 0; i < mHighScores.size(); ++i)
	{
		file << mHighScores[i].name << " " << mHighScores[i].score << std::endl;
	}
}

void Asteroids::SaveHighScore(const string& name, int score)
{
	LoadHighScores();

	HighScoreEntry entry;
	entry.name = name;
	entry.score = score;
	mHighScores.push_back(entry);

	std::sort(mHighScores.begin(), mHighScores.end(),
		[](const HighScoreEntry& a, const HighScoreEntry& b)
		{
			return a.score > b.score;
		});

	if (mHighScores.size() > 10)
		mHighScores.resize(10);

	WriteHighScores();
}

void Asteroids::UpdateHighScoreTableText()
{
	LoadHighScores();

	std::ostringstream msg;
	msg << "top 10 ";

	for (size_t i = 0; i < mHighScores.size(); ++i)
	{
		std::ostringstream line;

		if (i < (int)mHighScores.size())
			line << (i + 1) << ". " << mHighScores[i].name << " " << mHighScores[i].score;
		else
			line << (i + 1) << ". -----";

		if (i < (int)mHSRows.size())
			mHSRows[i]->SetText(line.str());
	}
}

void Asteroids::ShowMenuHS(bool show)
{
	mMenuShowingHS = show;
	UpdateHighScoreTableText();

	if (mTitleLabel) mTitleLabel->SetVisible(!show);
	if (mStartLabel) mStartLabel->SetVisible(!show);
	if (mInstructionsLabel) mInstructionsLabel->SetVisible(!show);
	if (mInstructionsTextLabel) mInstructionsTextLabel->SetVisible(false);
	if (mHighScoreLabel) mHighScoreLabel->SetVisible(!show);
	if (mHighScoreTableLabel) mHighScoreTableLabel->SetVisible(show);

	for (size_t i = 0; i < mHSRows.size(); i++)
		mHSRows[i]->SetVisible(show);

}

void Asteroids::ShowGameOverScreen()
{
	mState = STATE_GAME_OVER;
	UpdateHighScoreTableText();

	ClearObjects();
	
	CreateAsteroids(10);
	

	mGameOverLabel->SetVisible(true);
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mBombLabel->SetVisible(false);
	mINVLabel->SetVisible(false);

	if (mEnterNameLabel) mEnterNameLabel->SetVisible(false);
	if (mTypedNameLabel) mTypedNameLabel->SetVisible(false);

	if (mGameOverScoresLabel) mGameOverScoresLabel->SetVisible(true);
	if (mGameOverOptionsLabel) mGameOverOptionsLabel->SetVisible(true);

	for (size_t i = 0; i < mHSRows.size(); i++)
		mHSRows[i]->SetVisible(true);
}

void Asteroids::ReturnToMenu()
{
	mState = STATE_MENU;
	mEnteredName = "";
	mCurrentScore = 0;
	mMenuShowingHS = false;

	mGameOverLabel->SetVisible(false);
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mBombLabel->SetVisible(false);
	mINVLabel->SetVisible(false);


	for (size_t i = 0; i < mHSRows.size(); i++)
		mHSRows[i]->SetVisible(false);

	CreateAsteroids(10);
	ShowMenuGUI();
}

void Asteroids::RestartGame()
{
	mState = STATE_MENU;
	mEnteredName = "";
	mCurrentScore = 0;
	mMenuShowingHS = false;

	mGameOverLabel->SetVisible(false);
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mBombLabel->SetVisible(false);
	mINVLabel->SetVisible(false);

	CreateAsteroids(10);
	StartGameplay();
}

void Asteroids::ClearObjects()
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	
	explosion->SetPosition(GLVector3f(0,0,0));
	explosion->SetRotation(0);
	mGameWorld->AddObject(explosion);

	shared_ptr<GameObject> expl = make_shared<BombExplosion>();
	expl->SetBoundingShape(make_shared<BoundingSphere>(expl->GetThisPtr(), 500.0f));
	expl->SetPosition(GLVector3f(0, 0, 0));
	mGameWorld->AddObject(expl);

	mAsteroidCount = 0;
}

void Asteroids::UpdateBombGUI()
{
	if (!mBombLabel) return;

	std::ostringstream msg_stream;
	if (mBombReady)
	{
		msg_stream << "Bomb: READY";
	}
	else
	{
		msg_stream << "Bomb: " << mBombCooldown << "s";
	}

	mBombLabel->SetText(msg_stream.str());
}

void Asteroids::DropBomb()
{
	if (mState != STATE_PLAYING) return;
	if (!mSpaceship) return;
	if (!mBombReady) return;

	mBombReady = false;
	mBombCooldown = 10;
	UpdateBombGUI();

	mGameWorld->AddObject(CreateBomb());
	SetTimer(1000, BOMB_COOLDOWN);
}


shared_ptr<GameObject> Asteroids::CreateBomb()
{
	GLVector3f ship_pos = mSpaceship->GetPosition();
	GLVector3f ship_vel = mSpaceship->GetVelocity();
	float angle = mSpaceship->GetAngle();

	GLVector3f heading(cos(DEG2RAD * angle), sin(DEG2RAD * angle), 0);
	heading.normalize();

	GLVector3f back_dir = heading * -2.0f;
	GLVector3f bomb_position = ship_pos + (back_dir * 6.0f);
	GLVector3f bomb_velocity = ship_vel + (back_dir * 15.0f);

	shared_ptr<GameObject> bomb = make_shared<Bomb>(bomb_position, bomb_velocity, 1500);

	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
	shared_ptr<Sprite> bomb_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	bomb_sprite->SetLoopAnimation(true);

	bomb->SetSprite(bomb_sprite);
	bomb->SetScale(0.08f);

	return bomb;
}






shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}