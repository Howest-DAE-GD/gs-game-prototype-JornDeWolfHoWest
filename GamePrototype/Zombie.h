#pragma once
class Zombie
{
public:
	Zombie(Point2f position, int health);
	~Zombie();
	void SetAliveState(bool isAlive);
	void Draw() const;
	Point2f m_Position{ };
	bool m_IsAlive{ };
	int m_Speed{ 150 };
	int m_Size{ 15 };
	int m_Health{ 3 };
	void Move(Point2f playerPosition, float elapsedSeconds);
	bool DeadTimeAdd(float& elapsedTime);
	float m_DeadTimer{0};
	float m_TimeNeededBeforeDeletion{20};
private:
};

