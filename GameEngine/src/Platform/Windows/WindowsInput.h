#pragma once

#include "Engine/Input.h"

namespace GameEngine {

	class WindowsInput : public Input
	{
	protected:
		// virtual means everything down the hierarchy will be virtual
		// so bool IsKeyPressedImpl(int keycode) override; is still virtual
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePosImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}