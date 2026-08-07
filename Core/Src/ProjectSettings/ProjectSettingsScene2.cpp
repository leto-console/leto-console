#include "ProjectSettings/ProjectSettingsScene2.hpp"

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

class Settings_2;

namespace Setting_2
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
#include <System/SystemRestart.hpp>

#include <cstdio>

class Settings_2 : public SettingsContainer
{
protected:
	static ButtonCallInstanceSettingUI<Settings_2>* clear_button;

	enum { CLEAR_BUFFER_SIZE = 256 };
	const char clear_buffer[CLEAR_BUFFER_SIZE]{};

	DialogParamMenu<bool, 4> clear_question;
	DialogParamMenu<bool, 4> reset_question;
	int part = -1;
	const int count_parts = 32 * 1024 / CLEAR_BUFFER_SIZE;

	void OnClear()
	{
		clear_question.Enable();
	}

	void OnTest()
	{
		SystemSceneManager::Instance().SwitchScene(SceneID::TEST_SCENE);
	}

	void OnRestart()
	{
		SystemRestart();
	}

    void InitClearQuestions()
    {
        clear_question.SetText(0, "ВНИМАНИЕ!");
		clear_question.SetText(1, "ПОЛНЫЙ СБРОС");
		clear_question.SetText(2, "EEPROM");
		clear_question.AppendMenuItem("ОТМЕНИТЬ", false);
		clear_question.AppendMenuItem("ПРОДОЛЖИТЬ", true);
		clear_question.SetPosition({64, 0});

		reset_question.SetText(1, "ПЕРЕЗАГРУЗИТЕ");
		reset_question.SetText(2, "КОНСОЛЬ");
		reset_question.SetPosition({64, 0});

        clear_button = AddSetting<ButtonCallInstanceSettingUI<Settings_2>>("СБРОСИТЬ", Point2_i{-1, -1});
		clear_button->InitScene(this, &Settings_2::OnClear);
    }

public:
	Settings_2(ISceneManager* scene_manager) : SettingsContainer{ "", &scene_manager->GetCommonAllocator() }
	{
		using namespace Setting_2;

		StaticListView<ListSettingItem<bool>> yes_no = YesNoList;
		StaticListView<ListSettingItem<bool>> on_off = OnOffList;

        AddSetting<ValueEditableSettingUI<uint32_t>>("Ser.Num:", Point2_i{-1, -1}, &SerialNumber, "%d", 0, 10000, 1, false);
		AddSetting<ButtonCallInstanceSettingUI<Settings_2>>("ТЕСТ", Point2_i{-1, -1}, this, &Settings_2::OnTest);
		AddSetting<ButtonCallInstanceSettingUI<Settings_2>>("РЕСТАРТ", Point2_i{-1, -1}, this, &Settings_2::OnRestart);

        InitClearQuestions();
	}

	void OnShow() override
	{
		clear_question.OnShow();
		SettingsContainer::OnShow();
	}

	void Draw(IScreen& screen, Point2_i offset = {}) override
	{
		using namespace DrawFunctions;

		if (part >= 0)
		{
			static StaticText32 empty_text = "                ";
			static StaticText32 process_text = "ПРОЦЕСС ОЧИСТКИ";

			DrawText(screen, {0, 0}, empty_text);
			DrawText(screen, {0, 8}, empty_text);
			DrawText(screen, {0, 16}, empty_text);
			DrawText(screen, {0, 24}, empty_text);

			StaticText32 progress{};
			snprintf(progress.CharPtr(), progress.Capacity(), "%d procent", 100 * part / count_parts);

			DrawText(screen, {0, 0}, process_text);
			DrawText(screen, {0, 8}, progress);
		}
		else
		{
			SettingsContainer::Draw(screen, offset);
		}

		clear_question.Draw(screen);
		reset_question.Draw(screen);
	}

	bool ProcessInput(const AppEvent& event) override
	{
		if (part >= 0)
			return true;

		if (clear_question.IsCaptured() && clear_question.ProcessInput(event))
			return true;

		return SettingsContainer::ProcessInput(event);
	}

	void Loop() override
	{
		SettingsContainer::Loop();

		bool answer{};
		if (clear_question.IsResultReady(answer))
		{
			if (answer)
				part = 0;
			clear_question.Disable();
		}

		if (part >= 0 && part < count_parts)
		{
			SystemStorage.Write(part * CLEAR_BUFFER_SIZE, clear_buffer, CLEAR_BUFFER_SIZE);
			part++;
		}

		if (part == count_parts && !reset_question.IsActive())
		{
			reset_question.Enable();
		}
	}
};

ButtonCallInstanceSettingUI<Settings_2>* Settings_2::clear_button{};

// ====================================================================================================

ProjectSettingsScene2::ProjectSettingsScene2(ISceneManager* scene_manager) : CommonScene{scene_manager}
{
	AddObject<Settings_2>(scene_manager)->Enable();
}

bool ProjectSettingsScene2::ProcessInput(const AppEvent& event)
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
