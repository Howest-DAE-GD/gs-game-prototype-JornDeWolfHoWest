#include "pch.h"
#include "Zombie.h"
#include "utils.h"

Zombie::Zombie(Point2f position, int health) : m_Position(position), m_IsAlive(true), m_Health(health)
{
}

Zombie::~Zombie()
{
}

void Zombie::SetAliveState(bool isAlive)
{
	m_IsAlive = isAlive;
}

void Zombie::Draw() const
{
	if (not m_IsAlive)
		utils::SetColor(Color4f{ 0,0.2,0,1 });
	else
	{
		/*if (m_Speed < 0)
			utils::SetColor(Color4f{0,0,0,1});
			*/
		if (m_Health < 5)
			utils::SetColor(Color4f{ 0.1,0.8,0.1,1 });
		else if (m_Health < 10)
			utils::SetColor(Color4f{ 0.3,0.8,0.3,1 });
		else if (m_Health < 20)
			utils::SetColor(Color4f{ 0.5,0.8,0.5,1 });
		else if (m_Health < 30)
			utils::SetColor(Color4f{ 0.8,1,0.8,1 });
		else
			utils::SetColor(Color4f{ 1,1,1,1 });		
	}
	utils::FillEllipse(m_Position, m_Size, m_Size);
}

void Zombie::Move(Point2f playerPosition, float elapsedSeconds)
{
	float angle = atan2(playerPosition.y - m_Position.y, playerPosition.x - m_Position.x);
	m_Position.x += cos(angle) * m_Speed * elapsedSeconds;
	m_Position.y += sin(angle) * m_Speed * elapsedSeconds;
}

bool Zombie::DeadTimeAdd(float& elapsedTime)
{
	m_DeadTimer += elapsedTime;
	if (m_DeadTimer >= m_TimeNeededBeforeDeletion)
		return true;
	return false;
}
