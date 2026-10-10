#include "ProjectScenes/MainScene.hpp"

#include <Input/SystemInputID.hpp>

#include <Input/ButtonEvent.hpp>
#include <Time/Timer.hpp>

#include <Bitmaps/Eyes.hpp>

#include <Data/StaticList.hpp>
#include <DrawFunctions/DrawBitmap.hpp>
#include <Auth/AuthHandler.hpp>
#include <SceneManager/ISceneManager.hpp>

// ====================================================================================================

#include <System/SystemMode.hpp>
#include <Data/LangText.hpp>

struct MainMenuDef
{
	LangText<2> name;
	SceneID ID;
	int exclude_mode;
};

static constexpr MainMenuDef menu_def[]
{
	{ {"GAMES", 	{ Translation::RUS("ИГРЫ") }},			SceneID::GAMES_CENTER, (int) SystemMode::ADMIN },
	{ {"SETTINGS", 	{ Translation::RUS("НАСТРОЙКИ") }},		SceneID::SETTINGS },
	{ {"SETTINGS2", { Translation::RUS("НАСТРОЙКИ2") }},	SceneID::SETTINGS2, (int) SystemMode::USER },
	{ {"ACCOUNT", 	{ Translation::RUS("АККАУНТ") }},		SceneID::SETTING_ACCOUNT },
	{ {"SYSTEM", 	{ Translation::RUS("СИСТЕМА") }},		SceneID::SYSTEM },
	{ {"EEPROM"},											SceneID::EEPROM, (int) SystemMode::USER },
	{ {"FILES", 	{ Translation::RUS("ФАЙЛЫ") }},			SceneID::FILE_MANAGER },
	{ {"DEBUG"},											SceneID::DEBUG_SCENE },
};

static constexpr LangText<2> txt_EXIT		{ "EXIT", 		{ Translation::RUS("ВЫЙТИ") } };
static constexpr LangText<2> txt_ARE_YOU	{ "ARE YOU", 	{ Translation::RUS("ВЫ") } };
static constexpr LangText<2> txt_SURE		{ "SURE?", 		{ Translation::RUS("УВЕРЕНЫ?") } };
static constexpr LangText<2> txt_M_question	{ "M?", 		{ Translation::RUS("А?") } };
static constexpr LangText<2> txt_YES		{ "YES", 		{ Translation::RUS("ДА") } };
static constexpr LangText<2> txt_ABSOLUT	{ "ABSOLUTELY", { Translation::RUS("АБСОЛЮТНО") } };
static constexpr LangText<2> txt_NO			{ "NO", 		{ Translation::RUS("НЕТ") } };
static constexpr LangText<2> txt_IDN		{ "FUCK KNOWS", { Translation::RUS("ХЗ") } };

MainScene::MainScene(ISceneManager* scene_manager) : CommonScene{scene_manager}
{
	for (const MainMenuDef& def : menu_def)
	{
		if (scene_manager->IsExists((uint32_t) def.ID) && !((int) GetSystemMode() & def.exclude_mode))
			menu.AppendMenuItem(def.name.Text(), def.ID);
	}

	menu.InitBaseCatchers();
	menu.AppendMenuItem(txt_EXIT.Text(), SceneID::LOGOUT);
	menu.SetResetOnShow(false);
	menu.EnableReadyLogic();
	menu.Enable();

	exit_question.InitBaseCatchers();
	exit_question.SetText(0, txt_ARE_YOU.Text());
	exit_question.SetText(1, txt_SURE.Text());
	exit_question.SetText(2, txt_M_question.Text());
	exit_question.SetText(3, "Ы?");
	exit_question.AppendMenuItem(txt_YES.Text(), true);
	exit_question.AppendMenuItem(txt_ABSOLUT.Text(), true);
	exit_question.AppendMenuItem(txt_NO.Text(), false);
	exit_question.AppendMenuItem(txt_IDN.Text(), false);
	exit_question.SetPosition({64, 0});
	exit_question.Disable();

	AddObject(&exit_question);
	AddObject(&menu);
}

void MainScene::Loop()
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

	SceneID scene;
	if (menu.IsResultParamReady(scene))
	{
		if (scene == SceneID::LOGOUT)
			exit_question.Enable();
		else
			scene_manager->SwitchScene(scene);
		menu.SubmitReady();
	}
}

bool MainScene::ProcessInput(const AppEvent& event)
{
	if (CommonScene::ProcessInput(event)) 
		return true;

	return true;
}

