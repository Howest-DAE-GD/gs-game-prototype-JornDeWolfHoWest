#pragma once
#include "BaseGame.h"
#include "vector"
class Bullet;
class Person;
class Zombie;
class Game : public BaseGame
{
public:
	explicit Game( const Window& window );
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game( Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	// http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-override
	~Game();

	void Update( float elapsedSec ) override;
	void Draw( ) const override;

	// Event handling
	void ProcessKeyDownEvent( const SDL_KeyboardEvent& e ) override;
	void ProcessKeyUpEvent( const SDL_KeyboardEvent& e ) override;
	void ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e ) override;
	void ProcessMouseDownEvent( const SDL_MouseButtonEvent& e ) override;
	void ProcessMouseUpEvent( const SDL_MouseButtonEvent& e ) override;

private:

	// FUNCTIONS
	void Initialize();
	void Cleanup( );
	void ClearBackground( ) const;
	void AddBullet(Point2f position);
	void DeleteEverything();

	Point2f m_CursorPosition{};
	Point2f m_PlayerPosition{};
	std::vector<Zombie*> m_ArrPtrZombies{};
	std::vector<Bullet*> m_ArrPtrBullets{};

	float m_TimeBetweenSpawns{ 10 };
	int m_SpawnAmount{ };
	float m_TimeRemainingTillNextSpawn{ };
	int m_ZombiesRemaining{};
	int m_Score{};
	int m_HighScore{};
	int m_WavesSurvived{0};

	int m_CostNextDamage{ 100 };
	int m_CostNextFireRate{ 100 };
	int m_LevelDamage{ 1 };
	int m_LevelFireRate{ 1 };

	float m_ShootingCooldown{};

	float m_Timer{};
	bool m_IsDead{};
	bool m_IsRespawning{};
	int m_TimesDead{};
	int m_FullDead{};

	void FullReset();
	bool m_MouseDown{};
};