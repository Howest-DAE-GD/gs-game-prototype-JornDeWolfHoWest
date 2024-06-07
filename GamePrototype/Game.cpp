#include "pch.h"
#include "Game.h"
#include "Utils.h"
using namespace utils;
#include "Bullet.h"
#include "Zombie.h"
#include "Texture.h"

Game::Game( const Window& window ) 
	:BaseGame{ window }
{
	Initialize();
}

Game::~Game( )
{
	Cleanup( );
}

void Game::Initialize( )
{
	FullReset();
}

void Game::Cleanup( )
{
	DeleteEverything();
}

void Game::Update( float elapsedSec )
{
	const Uint8 *pStates = SDL_GetKeyboardState( nullptr );

	if (pStates[SDL_SCANCODE_R])
	{
		FullReset();
	}

	if (m_Score > m_HighScore)
		m_HighScore = m_Score;
	if (m_IsDead or m_IsRespawning or m_FullDead)
	{
		m_Timer += elapsedSec;
		if (m_Timer > 5)
			m_Timer = 5;

		if (m_Timer >= 3.f and m_IsDead)
		{
			m_IsDead = false;
			m_IsRespawning = true;
			m_Timer = 0;
		}
		if (m_Timer >= 4.f and m_IsRespawning)
		{
			if (m_WavesSurvived < 8 + m_TimesDead)
			{
				m_IsRespawning = false;
				m_FullDead = true;
				return;
			}
			m_IsRespawning = false;
			DeleteEverything();
			++m_TimesDead;

			m_TimeBetweenSpawns = 10;
			m_Timer = 0;
			m_Score = 0;
			m_WavesSurvived = 0;
			int timesDead{};
			if (m_TimesDead > 5)
				timesDead = 5;
			m_CostNextDamage = 100;
			m_CostNextFireRate = 100;
			m_LevelDamage = 1;
			m_LevelFireRate = 1;
		}
		return;
	}

	if (m_FullDead)
		return;
#pragma region Zombies & Bullets


	m_ZombiesRemaining = 0;
	for (Zombie* zombie : m_ArrPtrZombies)
	{
		if (zombie == nullptr)
			continue;
		if (zombie->m_IsAlive)
		{
			++m_ZombiesRemaining;
		}
	}
	m_TimeRemainingTillNextSpawn -= elapsedSec;
	if (m_TimeRemainingTillNextSpawn <= 0 || m_ZombiesRemaining == 0)
	{
		m_TimeBetweenSpawns -= 0.25f;
		if (m_TimeBetweenSpawns <= 2.f)
			m_TimeBetweenSpawns = 2.f;

		++m_WavesSurvived;
		m_SpawnAmount = 4 + int(float(m_WavesSurvived) / 5.f);
		if (m_SpawnAmount > 5)
			m_SpawnAmount = 5;
		float offset{ 200 };
		int spawnPosition{};
		int xSize{ int(GetViewPort().width + offset*2) };
		int ySize{ int(GetViewPort().height + offset*2) };
		m_TimeRemainingTillNextSpawn = m_TimeBetweenSpawns;
		int health{ 3 + int(m_WavesSurvived / 5) };
		for (int i = 0; i < m_SpawnAmount; i++)
		{
			spawnPosition = rand() % 4;
			switch (spawnPosition)
			{
			case 0:
				m_ArrPtrZombies.push_back(new Zombie{ Point2f{ -offset, float(rand() % ySize - offset)}, health * (m_TimesDead+1)});
				break;
			case 1:
				m_ArrPtrZombies.push_back(new Zombie{ Point2f{ GetViewPort().width+offset, float(rand() % ySize - offset)}, health * (m_TimesDead + 1) });
				break;
			case 2:
				m_ArrPtrZombies.push_back(new Zombie{ Point2f{ float(rand() % xSize - offset), -offset }, health * (m_TimesDead + 1) });
				break;
			case 3:
				m_ArrPtrZombies.push_back(new Zombie{ Point2f{ float(rand() % xSize - offset), GetViewPort().height+offset }, health * (m_TimesDead + 1) });
				break;
			}

		}
	}
	// Zombies
	for (int index{}; index < m_ArrPtrZombies.size(); ++index)
	{
		Zombie* zombie = m_ArrPtrZombies[index];
		if (zombie == nullptr)
			continue;
		if (not zombie->m_IsAlive)
		{
			if (zombie->DeadTimeAdd(elapsedSec))
			{
				delete zombie;
				m_ArrPtrZombies[index] = nullptr;
			}
			continue;
		}
		zombie->Move(m_PlayerPosition, elapsedSec);
		bool bulletsToDelete{ false };
		for (int index = 0; index < m_ArrPtrBullets.size(); ++index)
		{
			if (utils::IsOverlapping(Circlef{ zombie->m_Position, float(zombie->m_Size) }, Circlef{ m_ArrPtrBullets[index]->m_Position, float(m_ArrPtrBullets[index]->m_Size) })) {

				int zombieHealth = zombie->m_Health;
				zombie->m_Health -= m_ArrPtrBullets[index]->m_Health;

				m_ArrPtrBullets[index]->m_Health -= zombieHealth;
				if (zombie->m_Health <= 0)
				{
					m_Score += 10*((m_TimesDead*1.5)+1);
					zombie->m_IsAlive = false;
				}
				if (m_ArrPtrBullets[index]->m_Health <= 0)
				{
					delete m_ArrPtrBullets[index];
					m_ArrPtrBullets[index] = nullptr;
					bulletsToDelete = true;
				}
			}
		}
		if (bulletsToDelete)
		{
			for (int index = 0; index < m_ArrPtrBullets.size();)
			{
				if (m_ArrPtrBullets[index] == nullptr)
				{
					m_ArrPtrBullets.erase(m_ArrPtrBullets.begin() + index);
					continue;
				}
				++index;
			}
		}
	}
	// Zombies deletion
	for (int index{}; index < m_ArrPtrZombies.size();)
	{
		if (m_ArrPtrZombies[index] == nullptr)
		{
			m_ArrPtrZombies.erase(m_ArrPtrZombies.begin() + index);
			continue;
		}
		++index;
	}
	// Bullets
	for (int index = 0; index < m_ArrPtrBullets.size(); ++index)
	{
		m_ArrPtrBullets[index]->Move(elapsedSec);
		if (not IsOverlapping(Rectf{ 0, 0, GetViewPort().width, GetViewPort().height }, Circlef{ m_ArrPtrBullets[index]->m_Position, 5 })) {
			delete m_ArrPtrBullets[index];
			m_ArrPtrBullets[index] = nullptr;
		}
	}
	// Bullets deletion
	for (int index = 0; index < m_ArrPtrBullets.size();)
	{
		if (m_ArrPtrBullets[index] == nullptr)
		{
			m_ArrPtrBullets.erase(m_ArrPtrBullets.begin() + index);
			continue;
		}
		++index;
	}

	for (int index{}; index < m_ArrPtrZombies.size(); ++index)
	{
		if (not m_ArrPtrZombies[index]->m_IsAlive)
			continue;
		if (utils::IsOverlapping(Circlef(m_PlayerPosition, 15), Circlef(m_ArrPtrZombies[index]->m_Position, 15)))
		{
			m_IsDead = true;
			break;
		}
	}
#pragma endregion
	int speed{ 200 };
	// Check keyboard state
	if ( pStates[SDL_SCANCODE_W] )
	{
		m_PlayerPosition.y += speed * elapsedSec;
	}
	if (pStates[SDL_SCANCODE_S])
	{
		m_PlayerPosition.y -= speed * elapsedSec;
	}
	if (pStates[SDL_SCANCODE_A])
	{
		m_PlayerPosition.x -= speed * elapsedSec;
	}
	if (pStates[SDL_SCANCODE_D])
	{
		m_PlayerPosition.x += speed * elapsedSec;
	}
	m_ShootingCooldown -= elapsedSec;
	if (m_ShootingCooldown <= 0)
		m_ShootingCooldown = 0;

	if ((pStates[SDL_SCANCODE_F] or pStates[SDL_SCANCODE_SPACE] or m_MouseDown) and m_ShootingCooldown == 0)
	{
		AddBullet(m_PlayerPosition);
		m_ShootingCooldown = 0.5f - (float((m_LevelFireRate-1)*10)/100);
	}
}

void Game::Draw() const
{
	int size{ 48 };
	Texture* texture{};
	ClearBackground();

	texture = new Texture("WASD to move", "C&CRedAlert.ttf", 34, Color4f{ .8f, .8f, .8f , .8f });
	texture->Draw(Point2f{ 20, 100 });
	delete texture;
	texture = new Texture("Left click, F or SPACE to shoot", "C&CRedAlert.ttf", 34, Color4f{ .8f, .8f, .8f , .8f });
	texture->Draw(Point2f{ 20, 100-34 });
	delete texture;
	texture = new Texture("1/2 to upgrade damage/fire-rate respectively", "C&CRedAlert.ttf", 34, Color4f{ .8f, .8f, .8f , .8f });
	texture->Draw(Point2f{ 20, 100-34*2 });
	delete texture;
	if (m_WavesSurvived >= 8 + m_TimesDead)
	{
		texture = new Texture("You have passed enough levels to rebirth.", "C&CRedAlert.ttf", 34, Color4f{ .8f, .1f, .1f , .8f });
		texture->Draw(Point2f{ 20, 100 + 34 });
		delete texture;
	}
	for (Zombie* zombie : m_ArrPtrZombies)
	{
		if (not zombie->m_IsAlive)
			zombie->Draw();
	}
	for (Zombie* zombie : m_ArrPtrZombies)
	{
		if (zombie->m_IsAlive)
			zombie->Draw();
	}
	SetColor(Color4f{ 0, 1, 1, 1});
	FillEllipse(m_PlayerPosition, 15, 15);

	for (Bullet* bullet : m_ArrPtrBullets)
	{
		bullet->Draw();
	}
	
	int offset{};
	texture = new Texture("Score: " + std::to_string(m_Score), "C&CRedAlert.ttf", size, Color4f{1, 1, 1, 1});
	texture->Draw(Point2f{ GetViewPort().width - texture->GetWidth() - 10, GetViewPort().height - size - 10});
	offset = texture->GetHeight();
	delete texture;

	texture = new Texture("Highscore: " + std::to_string(m_HighScore), "C&CRedAlert.ttf", size, Color4f{1, 1, 1, 1});
	texture->Draw(Point2f{ GetViewPort().width - texture->GetWidth() - 10, GetViewPort().height - size - 10 - offset });
	delete texture;

	texture = new Texture( "Zombies Remaining: " + std::to_string(m_ZombiesRemaining), "C&CRedAlert.ttf", size, Color4f{1, 1, 1, 1});
	texture->Draw(Point2f{ 10, GetViewPort().height - size - 10 });
	offset = texture->GetHeight();
	delete texture;

	texture = new Texture("Waves Survived: " + std::to_string(m_WavesSurvived), "C&CRedAlert.ttf", size, Color4f{ 1, 1, 1, 1 });
	texture->Draw(Point2f{ 10, GetViewPort().height - size - 10 - offset });
	offset += texture->GetHeight();
	delete texture;

	if (m_TimesDead != 0)
	{
		texture = new Texture("Times reborn: " + std::to_string(m_TimesDead), "C&CRedAlert.ttf", size, Color4f{ 1, 1, 1, 1 });
		texture->Draw(Point2f{ 10, GetViewPort().height - size - 10 - offset });
		offset += texture->GetHeight();
		delete texture;
	}

	float previousHeight{};
	texture = new Texture("2: (lvl " + std::to_string(m_LevelFireRate) + ")Upgrade firerate: " + std::to_string(m_CostNextFireRate), "C&CRedAlert.ttf", size, Color4f{ 1, 1, 1, 1 });
	previousHeight = texture->GetHeight();
	texture->Draw(Point2f{ GetViewPort().width - texture->GetWidth() - 10, 10 });
	delete texture;
	texture = new Texture("1: (lvl " + std::to_string(m_LevelDamage) + ") Upgrade damage: " + std::to_string(m_CostNextDamage), "C&CRedAlert.ttf", size, Color4f{1, 1, 1, 1});
	texture->Draw(Point2f{ GetViewPort().width - texture->GetWidth() - 10, 10 + previousHeight + 10 });
	delete texture;

	if (m_IsDead or (m_IsRespawning and m_Timer < 3))
	{
		texture = new Texture("YOU DIED", "C&CRedAlert.ttf", 64, Color4f{1, 1, 1, 1});
		texture->Draw(Point2f{ GetViewPort().width/2 - texture->GetWidth()/2, GetViewPort().height / 2 - texture->GetHeight() / 2 });
		delete texture;
	}
	if (m_IsRespawning)
	{
		utils::SetColor(Color4f{ 1,1,1,1 });
		int size{ int(m_Timer * 800) };
		if (m_Timer > 3.f)
		{
			size = 2000 - (m_Timer - 3) * 2000;
		}
		utils::FillEllipse(m_PlayerPosition.x, m_PlayerPosition.y, size, size);
		texture = new Texture("OR DID YOU?", "C&CRedAlert.ttf", 64, Color4f{ 0, 0, 0, 1 });
		texture->Draw(Point2f{ GetViewPort().width / 2 - texture->GetWidth() / 2, GetViewPort().height / 2 - texture->GetHeight() / 2 });
		delete texture;
		
	}
	if (m_Timer >= 5.f)
	{
		texture = new Texture("Yes you did.", "C&CRedAlert.ttf", 48, Color4f{ 0.7f, 0, 0, 1 });
		texture->Draw(Point2f{ GetViewPort().width / 2 - texture->GetWidth() / 2, GetViewPort().height / 2 - texture->GetHeight() / 2 +30});
		delete texture;
		texture = new Texture("Not enough waves passed.", "C&CRedAlert.ttf", 24, Color4f{ 0.7f, 0, 0, 1 });
		texture->Draw(Point2f{ GetViewPort().width / 2 - texture->GetWidth() / 2, GetViewPort().height / 2 - texture->GetHeight() / 2 -30});
		delete texture;
	}
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	//std::cout << "KEYDOWN event: " << e.keysym.sym << std::endl;
	switch ( e.keysym.sym )
	{
	case SDLK_1:
	case SDLK_r:
		if (m_CostNextDamage <= m_Score)
		{
			++m_LevelDamage;
			m_Score -= m_CostNextDamage;
			m_CostNextDamage *= 2;
		}
	case SDLK_2:
	case SDLK_t:
		if (m_CostNextFireRate <= m_Score)
		{

			++m_LevelFireRate;
			m_Score -= m_CostNextFireRate;
			m_CostNextFireRate *= 2;
		}
		break;
		break;
	}
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
	//std::cout << "KEYUP event: " << e.keysym.sym << std::endl;
	
}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	m_CursorPosition.x = float(e.x);
	m_CursorPosition.y = float(e.y);
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	m_MouseDown = true;
	//std::cout << "MOUSEBUTTONDOWN event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
	
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	m_MouseDown = false;
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ClearBackground( ) const
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}

void Game::AddBullet(Point2f position)
{
	int speed{ 350 };
	float angle = atan2(m_CursorPosition.y - position.y, m_CursorPosition.x - position.x);
	Point2f velocity{speed * cosf(angle) , speed * sinf(angle)};
	m_ArrPtrBullets.push_back(new Bullet{ position, velocity, m_LevelDamage });
}

void Game::DeleteEverything()
{
	for (Bullet* bulletToDelete : m_ArrPtrBullets)
	{
		if (bulletToDelete != nullptr)
			delete bulletToDelete;
	}
	m_ArrPtrBullets.clear();
	for (Zombie* zombieToDelete : m_ArrPtrZombies)
	{
		if (zombieToDelete != nullptr)
			delete zombieToDelete;
	}
	m_ArrPtrZombies.clear();
}

void Game::FullReset()
{
	m_TimeBetweenSpawns = 10;
	m_Timer = 0;
	m_IsDead = false;
	m_IsRespawning = false;
	m_FullDead = false;
	m_PlayerPosition = Point2f{ GetViewPort().width/2, GetViewPort().height/2};
	m_TimesDead = 0;
	m_Score = 0;
	m_HighScore = 0;
	m_WavesSurvived = 0;
	m_CostNextDamage = 100;
	m_CostNextFireRate = 100;
	m_LevelDamage = 1;
	m_LevelFireRate = 1;
	DeleteEverything();
}
