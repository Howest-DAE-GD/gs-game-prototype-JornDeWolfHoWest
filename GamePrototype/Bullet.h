#pragma once
class Bullet
{
public:
	Bullet(Point2f position, Point2f velocity, int m_Health);
	Point2f m_Position{};
	Point2f m_Velocity{};
	int m_Health{};
	int m_Size{};
	void Move(float elapsedSec);
	void Draw() const;
};

