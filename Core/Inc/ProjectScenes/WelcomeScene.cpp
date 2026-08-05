#include "WelcomeScene.hpp"

#include "ProjectScenes/SceneID.hpp"
#include <Graphics/DefaultFont.hpp>
#include <Auth/AuthHandler.hpp>
#include <DrawFunctions/DrawRectangle.hpp>
#include <DrawFunctions/DrawText.hpp>
#include <SceneManager/ISceneManager.hpp>

WelcomeScene::WelcomeScene(ISceneManager* scene_manager) : CommonScene{scene_manager}
{
}

void WelcomeScene::OnShow()
{
	timer.Start(1500);

	Account account;
	if (AuthHandler::Instance().GetCurrentAccount(account))
		name = account.Name;
}

void WelcomeScene::Draw(IScreen& screen)
{
	using namespace DrawFunctions;

	StaticText32 text = "ДОБРО";
	DrawText(screen, {64 - TextWidth(text, &Default_Font_7x7) / 2, 16}, text, WhiteColor, BlackColor, false, &Default_Font_7x7);
	text = "ПОЖАЛОВАТЬ";
	DrawText(screen, {64 - TextWidth(text, &Default_Font_7x7) / 2, 24}, text, WhiteColor, BlackColor, false, &Default_Font_7x7);

	DrawRectangle(
		screen,
		{ 64 - TextWidth(name) / 2, 40 - 1 },
		{ 64 + TextWidth(name) / 2, 40 + Default_Font_8x8.GetHeight() },
		WhiteColor
	);

	DrawText(screen, {64 - TextWidth(name) / 2, 40}, name, WhiteColor, BlackColor, true, &Default_Font_8x8);
}

bool WelcomeScene::Loop()
{
	if (timer.Expired())
		scene_manager->SwitchScene(SceneID::EYES);

	return true;
}

bool WelcomeScene::ProcessInput(const AppEvent& event)
{
	if (ButtonEvent::IsPressed(event))
	{
		scene_manager->SwitchScene(SceneID::EYES);
		return true;
	}

	return false;
}

