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
};

#include <UI/Menu/DialogMenu.hpp>
#include <cstdio>

class Settings_1 : public SettingsContainer
{
public:
	Settings_1() : SettingsContainer{ "", &CommonAllocator }
	{
		using namespace Setting_1;

		StaticListView<ListSettingItem<bool>> yes_no = YesNoList;
		StaticListView<ListSettingItem<bool>> on_off = OnOffList;

		AddSetting<ValueSettingUI<uint32_t>>("ЗАПУСКОВ", Point2_i{-1, -1}, &StartsCount, "%d");
		AddSetting<ListEditableSettingUI<bool>>("ИНВ.ЭНК", Point2_i{-1, -1}, &EncoderReverse, yes_no, false);
		AddSetting<ListEditableSettingUI<bool>>("UART", Point2_i{-1, -1}, &UARTConsoleOnStart, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("DEBUG", Point2_i{-1, -1}, &DebugMode, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("FPS", Point2_i{-1, -1}, &EnableFPS_Setting, on_off, false);
		AddSetting<ListEditableSettingUI<bool>>("SNOW", Point2_i{-1, -1}, &EnableSnowfall, on_off, false);
	}
};

// ====================================================================================================

ProjectSettingsScene::ProjectSettingsScene(ISceneManager* scene_manager) : CommonScene{scene_manager}
{
	AddObject<Settings_1>()->Enable();
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
