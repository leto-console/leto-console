#include "ProjectSettings/ProjectSettingsScene.hpp"

#include <Input/SystemInputID.hpp>

#include <UI/ListSettingUI.hpp>
#include <UI/TextSettingUI.hpp>
#include <UI/ValueSettingUI.hpp>
#include <UI/ButtonSettingUI.hpp>

#include "ProjectSettings/ProjectSettings.hpp"
#include <SceneManager/SystemSceneSettings.hpp>

#include <Data/StaticText.hpp>
#include <Data/StaticList.hpp>

#include <DrawFunctions/DrawText.hpp>
#include <Data/LangText.hpp>

// ====================================================================================================

class Settings_1;

namespace Setting_1
{
	static StaticList<ListSettingItem<bool>, 2> YesNoList
	{
		{ "ДА", true },
		{ "НЕТ", false },
	};

	static StaticList<ListSettingItem<bool>, 2> OnOffList
	{
		{ "ВКЛ", true },
		{ "ВЫКЛ", false },
	};

	static StaticList<ListSettingItem<uint8_t>, 2> LanguagesList
	{
		{ "English", LETO_LANG_V1_ENG },
		{ "Русский", LETO_LANG_V1_RUS }
	};

	static constexpr LangText<2> txt_launches	{ "LAUNCHES", 	{Translation::RUS("ЗАПУСКОВ")} };
	static constexpr LangText<2> txt_inverse	{ "INV.ENC", 	{Translation::RUS("ИНВ.ЭНК")} };
	static constexpr LangText<2> txt_lang		{ "LANG", 	{Translation::RUS("ЯЗЫК")} };
};

#include <UI/Menu/DialogMenu.hpp>
#include <cstdio>

class Settings_1 : public SettingsContainer
{
public:
	Settings_1(ISceneManager* scene_manager) : SettingsContainer{ "", &scene_manager->GetCommonAllocator() }
	{
		using namespace Setting_1;

		// TODO: Придумать способ удобно переводить на разные языки такие штуки
		StaticListView<ListSettingItem<bool>> yes_no = YesNoList;
		StaticListView<ListSettingItem<bool>> on_off = OnOffList;
		StaticListView<ListSettingItem<uint8_t>> sys_lang = LanguagesList;

		AddSetting<ValueSettingUI<uint32_t>>(txt_launches.Text(), Point2_i{-1, -1}, &StartsCount, "%d");
		AddSetting<ListEditableSettingUI<bool>>(txt_inverse.Text(), Point2_i{-1, -1}, &EncoderReverse, yes_no, false);
		AddSetting<ListEditableSettingUI<bool>>("UART", Point2_i{-1, -1}, &UARTConsoleOnStart, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("DEBUG", Point2_i{-1, -1}, &DebugMode, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("FPS", Point2_i{-1, -1}, &EnableFPS_Setting, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("SNOW", Point2_i{-1, -1}, &EnableSnowfall, on_off, false);
		AddSetting<ListEditableSettingUI<uint8_t>>(txt_lang.Text(), Point2_i{-1, -1}, &SystemLanguage, sys_lang, false);
	}
};

// ====================================================================================================

ProjectSettingsScene::ProjectSettingsScene(ISceneManager* scene_manager) : CommonScene{scene_manager}
{
	AddObject<Settings_1>(scene_manager)->Enable();
}

bool ProjectSettingsScene::ProcessInput(const AppEvent& event)
{
	if (CommonScene::ProcessInput(event))
		return true;

	if (IsSystemReturnEvent(event))
	{
		SystemSceneManager::Instance().SwitchScene(SceneID::MAIN);
		return true;
	}

	return false;
}
