#pragma once

namespace Engine
{
	class Timestep
	{
	public:
		Timestep() {}
		Timestep(float time);

		inline float GetSeconds() const { return m_Time; }
		inline float GetMiliseconds() const { return m_Time * 1000.0f; }

		operator float() { return m_Time; }
	private:
		// In seconds
		float m_Time = 0.0f;
	};
}