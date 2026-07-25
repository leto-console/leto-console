#include "ProjectScenes/MainScene.hpp"

#include <Input/SystemInputID.hpp>

#include <Input/ButtonEvent.hpp>
#include <SceneManager/SceneManager.hpp>
#include <Time/Timer.hpp>

#include <Bitmaps/Eyes.hpp>

#include <Data/StaticList.hpp>
#include <DrawFunctions/DrawBitmap.hpp>
#include <Auth/AuthHandler.hpp>

// ====================================================================================================

#include <System/SystemMode.hpp>

struct MainMenuDef
{
	const char* name;
	SceneID ID;
	int exclude_mode;
};

static const MainMenuDef menu_def[]
{
	{ "ИГРЫ",		SceneID::GAMES_CENTER, (int) SystemMode::ADMIN },
	{ "НАСТРОЙКИ",	SceneID::SETTINGS },
	{ "НАСТРОЙКИ2",	SceneID::SETTINGS2, (int) SystemMode::USER },
	{ "АККАУНТ",	SceneID::SETTING_ACCOUNT },
	{ "СИСТЕМА",	SceneID::SYSTEM },
	{ "EEPROM",		SceneID::EEPROM, (int) SystemMode::USER },
	{ "ФАЙЛЫ",		SceneID::FILE_MANAGER },
	{ "DEBUG",		SceneID::DEBUG_SCENE },
};

MainScene::MainScene()
{
	for (const MainMenuDef& def : menu_def)
	{
		if (SceneManager::Instance().IsExists((uint32_t) def.ID) && !((int) GetSystemMode() & def.exclude_mode))
			menu.AppendMenuItem(def.name, def.ID);
	}

	menu.InitBaseCatchers();
	menu.AppendMenuItem("ВЫЙТИ", SceneID::LOGOUT);
	menu.SetResetOnShow(false);
	menu.Enable();

	exit_question.SetText(0, "ВЫ");
	exit_question.SetText(1, "УВЕРЕНЫ?");
	exit_question.SetText(2, "А?");
	exit_question.SetText(3, "Ы?");
	exit_question.AppendMenuItem("ДА", true);
	exit_question.AppendMenuItem("АБСОЛЮТНО", true);
	exit_question.AppendMenuItem("НЕТ", false);
	exit_question.AppendMenuItem("ХЗ", false);
	exit_question.SetPosition({64, 0});
	exit_question.Disable();

	AddObject(&exit_question);
	AddObject(&menu);
}

bool MainScene::Loop()
{
	CommonScene::Loop();
	
	bool yes;
	if (exit_question.IsResultReady(yes))
	{
		if (yes)
		{
			SetSystemMode(SystemMode::AUTH);
			AuthHandler::Instance().Logout();
		}
		exit_question.Disable();
	}

	return true;
}

bool MainScene::ProcessInput(const AppEvent& event)
{
	if (CommonScene::ProcessInput(event)) 
		return true;

	if (IsSystemEnterEvent(event))
	{
		if (menu.GetCurrentParam() == SceneID::LOGOUT)
			exit_question.Enable();
		else
			SceneManager::Instance().SwitchScene(menu.GetCurrentParam());
		return true;
	}

	return true;
}

