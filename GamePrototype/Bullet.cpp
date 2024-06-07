#include "pch.h"
#include "Bullet.h"
#include "Utils.h"

Bullet::Bullet(Point2f position, Point2f velocity, int health) : m_Position(position), m_Velocity(velocity), m_Size(5), m_Health(health)
{
}

void Bullet::Move(float elapsedSec)
{
	m_Position.x += m_Velocity.x * elapsedSec;
	m_Position.y += m_Velocity.y * elapsedSec;
}

void Bullet::Draw() const
{
	utils::SetColor(Color4f{0.7,0,0, 1});
	utils::FillEllipse(m_Position, m_Size, m_Size);
}
