#include "ProjectScenes/SystemScene.hpp"
#include "ProjectScenes/CreateCharInfo.hpp"
#include <Input/SystemInputID.hpp>

#include <TaskHandler/PriorityTaskSheduler.hpp>
#include <Time/TimeUtils.hpp>
#include <DrawFunctions/DrawText.hpp>

// Страница с информацией о текущих задачах
class TasksScene : public IScene
{
protected:
	int start_from = 0;

public:
	void Draw(IScreen& screen) override
	{
		int i = 0, skip = start_from;
		for (const PriorityTask* task : PriorityTaskSheduler::GetList())
		{
			if (skip > 0) { skip--; continue; }
			DrawFunctions::DrawText(screen, {0, (i++) * 8}, CreateCharInfo(task->GetName(), task->GetAverageDurationMs(), 2));
			if (i >= 8) break;
		}
	}
	bool ProcessInput(const AppEvent& event) override
	{
		if (IsSystemPrevEvent(event))
		{
			if (start_from > 0) start_from--;
			return true;
		}
		else if (IsSystemNextEvent(event))
		{
			if (start_from < (int)PriorityTaskSheduler::GetList().Count() - 8) start_from++;
			return true;
		}
		return false;
	}
	bool Loop() override { return true; }
};

#include <Utils/random.hpp>
#include <Time/DateTime.hpp>
#include <Time/CurrentTimeCell.hpp>
#include <UI/Menu/Menu.hpp>
#include <UI/DateTimeSettingUI.hpp>
#include <UI/ValueSettingUI.hpp>
#include <UI/ListSettingUI.hpp>
#include <UI/SettingsContainer.hpp>
#include <UI/ButtonSettingUI.hpp>


class TimeScene : public IScene
{
protected:
	SettingsContainer settings;
	CurrentTimeCell cell_ms, cell_mks;

	ISettingUI *time, *date;
	ISettingUI *edit_time, *edit_date;
	ISettingUI *button;

	void OnChange()
	{
		time->SetActive(false);
		date->SetActive(false);
		edit_time->SetActive(true);
		edit_date->SetActive(true);
		button->SetActive(false);
	}

public:
	TimeScene(): settings { "===== ВРЕМЯ =====", &CommonAllocator }, cell_ms{ false }, cell_mks{ true }
	{
		time = settings.AddSetting<DateTimeSettingUI>("ВРЕМЯ", Point2_i{-1, -1}, DateTimeSettingUI::TIME);
		date = settings.AddSetting<DateTimeSettingUI>("ДАТА", Point2_i{-1, -1}, DateTimeSettingUI::DATE);

		edit_time = settings.AddSetting<DateTimeEditableSettingUI>(">ВРЕМЯ", Point2_i{-1, -1}, DateTimeSettingUI::TIME);
		edit_date = settings.AddSetting<DateTimeEditableSettingUI>(">ДАТА", Point2_i{-1, -1}, DateTimeSettingUI::DATE);
		edit_time->SetActive(false);
		edit_date->SetActive(false);

		settings.AddSetting<ValueSettingUI<uint32_t>>("MS", Point2_i{-1, -1}, &cell_ms, "%" PRIu32);
		settings.AddSetting<ValueSettingUI<uint32_t>>("MKS", Point2_i{-1, -1}, &cell_mks, "%" PRIu32);
		button = settings.AddSetting<ButtonCallInstanceSettingUI<TimeScene>>("ИЗМЕНИТЬ", Point2_i{-1, -1}, this, &TimeScene::OnChange);
		settings.Enable();

		AddObject(&settings);
	}

	void Draw(IScreen& screen) override {  }

	bool ProcessInput(const AppEvent& event) override
	{
		return IsSystemPrevEvent(event) || IsSystemNextEvent(event);
	}
	
	bool Loop() override { return true; }
};

class RandomScene : public IScene
{
protected:
	float R{};
	int R_int{};

public:
	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		StaticText32 debug_str{};

		Point2_i p;

		snprintf(debug_str.CharPtr(), debug_str.Capacity(), "RAND: %0.6f", R);
		DrawText(screen, p, debug_str);
		p.y += 8;

		snprintf(debug_str.CharPtr(), debug_str.Capacity(), "RAND_INT: %d", R_int);
		DrawText(screen, p, debug_str);
	}

	bool ProcessInput(const AppEvent& event) override
	{
		if (IsSystemEnterEvent(event))
		{
			R = RandomFloat();
			R_int = Random(0, 9);
			return true;
		}
		else if (IsSystemPrevEvent(event) || IsSystemNextEvent(event))
		{
			return true;
		}
		return false;
	}
	bool Loop() override { return true; }
};


#include <LetoAPI_V1_System/Web/WebDevicesList.hpp>
#include <ExtDevice/UI/UI_ExtDeviceStatus.hpp>
#include <SDCard/SDCard_ExtDevice.hpp>
#include <Graphics/DefaultFont.hpp>

class DevicesScene : public IScene
{
	UI_ExtDeviceStatus ui_status;
public:
	DevicesScene() : ui_status{ sdcard_extdev }
	{
		ui_status.SetFont(&Default_Font_7x7_small);
	}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		uint8_t idx = 0;
		for (WebDeviceInfo_ListItem& info : WebDevicesList)
		{
			StaticText32 str{};
			snprintf(str.CharPtr(), str.Capacity(), "ch[%d] %s", info.device.channel, info.device.web_name);
			DrawText(screen, {0, idx * 16}, str);
			snprintf(str.CharPtr(), str.Capacity(), "%" PRIu32, info.last_activity);
			DrawText(screen, {0, idx * 16 + 8}, str);
			if (info.device.app_id)
			{
				snprintf(str.CharPtr(), str.Capacity(), "APP %X", info.device.app_id);
				DrawText(screen, {0, idx * 16 + 16}, str);
			}

			idx++;
			if (idx >= 3)
				break;
		}
		idx++;
		ui_status.Draw(screen, {0, idx * 16});
	}
	bool ProcessInput(const AppEvent& event) override 
	{
		ui_status.ProcessInput(event);
		return false; 
	}
	bool Loop() override { return true; }
};

#include <LetoAPI_V1_System/Web/WebManager.hpp>
#include <LetoAPI_V1_System/Lobby/LobbyManager.hpp>
#include <System/DeviceID.hpp>

#include <Data/StaticList.hpp>

#include <GamesSupport/Lobby/LobbyData.hpp>
#include <GamesSupport/Lobby/LobbyDataProcessor.hpp>

#include <cstring>

struct AppEvent_LobbyData : public LobbyData
{
public:
	AppEvent_LobbyData(uint8_t id, uint32_t period) : LobbyData{ id, period } { }

	void SetData(AppEvent event) { send_event = event; }
	void GetData(AppEvent& event) { event = get_data; }

protected:
	AppEvent send_event;
	AppEvent get_data;

	bool OnSend(void* data, uint32_t size) override
	{
		if (size < sizeof(AppEvent)) return false;
		memcpy(data, &send_event, sizeof(AppEvent));
		return true;
	}

	void OnReceive(uint32_t /*id*/, const void* data, uint32_t size) override
	{
		if (size < sizeof(AppEvent)) return;
		memcpy(&get_data, data, sizeof(AppEvent));
	}
};

static LobbyDataProcessor* data_processor{};
static AppEvent_LobbyData app_event_data(0x01, 100);

void LobbyCallback(uint32_t id, const void* data, uint32_t size)
{
	if (data_processor)
		data_processor->ReceiveData(id, data, size);
}

class LobbySettingUI : public ISettingUI
{
protected:
	LobbyDataProcessor processor;

	StaticText32 CurrentValueRepr() override
	{
		return "";
	}

	void UpdateCurrentValue()
	{
	}

public:
	LobbySettingUI(StaticText32 name, Point2_i position)
		: ISettingUI{ name, position }
	{
		delimiter = false;

		processor.RegLobbyData(&app_event_data);
		data_processor = &processor;
	}
	~LobbySettingUI()
	{
		data_processor = nullptr;
	}

	void Draw(IScreen& screen, Point2_i offset = {}) override
	{
		LobbyConnection_V1 lobby;
		if (!LobbyManager_V1::Instance().GetActiveLobby(&lobby))
			return;

		snprintf(name.CharPtr(), name.Capacity(), "[%s] Lobby: %d", lobby.owner == GetDeviceID() ? "O" : "M", lobby.connected);

		ISettingUI::Draw(screen, offset);
		if (!IsCaptured())
			return;

		Point2_i start {30, 60};
		DrawFunctions::DrawOutlinedRectangle(screen, start, start + Point2_i{100, 60}, CyanColor, WhiteColor);

		StaticText32 str{};
		uint8_t i = 0;

		snprintf(str.CharPtr(), str.Capacity(), "O: %d", lobby.owner);
		DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		i++;

		AppEvent get_event;
		app_event_data.GetData(get_event);

		if (IsSystemNextEvent(get_event))
		{
			snprintf(str.CharPtr(), str.Capacity(), "Next");
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
		else if (IsSystemPrevEvent(get_event))
		{
			snprintf(str.CharPtr(), str.Capacity(), "Prev");
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}

		i++;

		for (uint32_t j = 0; j < lobby.connected; ++j, ++i)
		{
			snprintf(str.CharPtr(), str.Capacity(), "M[%d]: %d", j, lobby.members[j]);
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
	}

	bool ProcessInput(const AppEvent& event) override
	{
		// Если не выделен и не захвачен, обработка передается следующему элементу в списке
		// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
		if (!IsSelected())
			return false;

		if (!IsCaptured())
		{
			if (IsSystemEnterEvent(event, true))
			{
				Capture();
				return true;
			}
			return false;
		}

		if (IsSystemLeftEvent(event, true))
			Capture(false);

		app_event_data.SetData(event);

		return true;
	}

	void Loop() override { processor.Loop(); }
};

class LobbyNearSettingUI : public ISettingUI
{
protected:
	LobbyInfo_V1 lobbies_info[8];
	uint8_t lobbies_cnt;

	StaticText32 CurrentValueRepr() override
	{
		return "";
	}

	void UpdateCurrentValue()
	{
	}

public:
	LobbyNearSettingUI(StaticText32 name, Point2_i position)
		: ISettingUI{ name, position }
	{
		delimiter = false;
	}

	void Draw(IScreen& screen, Point2_i offset = {}) override
	{
		if (!IsCaptured())
		{
			ISettingUI::Draw(screen, offset);
			return;
		}

		StaticText32 str{};

		lobbies_cnt = LobbyManager_V1::Instance().GetLobbiesNear(lobbies_info, 8);

		Point2_i start {30, 30};
		DrawFunctions::DrawOutlinedRectangle(screen, start, {130, 90}, CyanColor, WhiteColor);
		for (uint8_t i = 0; i < lobbies_cnt; ++i)
		{
			snprintf(str.CharPtr(), str.Capacity(), "-Lobby %d", lobbies_info[i].owner);
			DrawFunctions::DrawText(screen, start + Point2_i{8, 8 + i * 8}, str, CyanColor);
		}
		
		// Отрисовка доступных лобби
	}

	bool ProcessInput(const AppEvent& event) override
	{
		// Если не выделен и не захвачен, обработка передается следующему элементу в списке
		// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
		if (!IsSelected())
			return false;

		if (!IsCaptured())
		{
			if (IsSystemEnterEvent(event, true))
			{
				Capture();
				return true;
			}
			return false;
		}

		if (IsSystemEnterEvent(event))
		{
			LobbyManager_V1::Instance().JoinLobby(lobbies_info[0], &LobbyCallback);
		}

		if (IsSystemLeftEvent(event, true))
			Capture(false);

		return true;
	}
};

void CreateLobbyBtnCallback()
{
	LobbyManager_V1::Instance().CreateLobby(nullptr, 0x1, 3, &LobbyCallback);
}

void QuitLobbyBtnCallback()
{
	LobbyManager_V1::Instance().QuitLobby();
}

class WebScene : public IScene
{
protected:
	SettingsContainer settings;
	const uint16_t H = 8;

	ISettingUI* lobby{};
	ISettingUI* create{};
	ISettingUI* quit{};

public:
	WebScene() : settings { "===== СВЯЗЬ =====", &CommonAllocator }
	{
		create = settings.AddSetting<ButtonSettingUI>("СОЗДАТЬ", Point2_i{0, 4 * H}, &CreateLobbyBtnCallback);
		quit = settings.AddSetting<ButtonSettingUI>("ВЫЙТИ", Point2_i{0, 4 * H}, &QuitLobbyBtnCallback);
		quit->SetActive(false);

		settings.AddSetting<LobbyNearSettingUI>("РЯДОМ", Point2_i{0, 5 * H});

		lobby = settings.AddSetting<LobbySettingUI>("ЛОББИ", Point2_i{0, 6 * H});
		lobby->SetActive(false);

		settings.Enable();
		AddObject(&settings);
	}

	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		StaticText32 str{};
		snprintf(str.CharPtr(), str.Capacity(), "Buffer: %d", WebManager_V1::Instance().BufferPPS());
		DrawText(screen, {0, 1 * H}, str);

		snprintf(str.CharPtr(), str.Capacity(), "Connections: %d", WebManager_V1::Instance().GetConnections(nullptr, 0));
		DrawText(screen, {0, 2 * H}, str);

		LobbyConnection_V1 connection;
		if (LobbyManager_V1::Instance().GetActiveLobby(&connection))
		{
			lobby->SetActive(true);
			//snprintf(str.CharPtr(), str.Capacity(), "[%s] Lobby: %d", connection.owner == GetDeviceID() ? "O" : "M", connection.connected);
			//DrawText(screen, {0, 3 * H}, str);
			create->SetActive(false);
			quit->SetActive(true);
		}
		else
		{
			lobby->SetActive(false);
			create->SetActive(true);
			quit->SetActive(false);
		}
	}
	bool ProcessInput(const AppEvent& event) override { return false; }
	bool Loop() override { return true; }
};

#include <NRF24L01/NRF24L01_data.hpp>
#include <ProjectSettings/ProjectSettings.hpp>

namespace NRF24L01_UI
{
	static StaticList<ListSettingItem<uint8_t>, 6> pipes
	{
		{ "1", 1 },
		{ "2", 2 },
		{ "3", 3 },
		{ "4", 4 },
		{ "5", 5 },
		{ "6", 6 },
	};

	class ScanSettingUI : public ISettingUI
	{
	protected:
		StaticText32 CurrentValueRepr() override
		{
			return "";
		}

		void UpdateCurrentValue()
		{
		}

	public:
		ScanSettingUI(StaticText32 name, Point2_i position)
			: ISettingUI{ name, position }
		{
			delimiter = false;
		}

		void Draw(IScreen& screen, Point2_i offset = {}) override
		{
			if (!IsCaptured())
				ISettingUI::Draw(screen, offset);
			else
			{
				NRF24L01::Scanner.Draw(screen, offset);
			}
		}

		bool ProcessInput(const AppEvent& event) override
		{
			// Если не выделен и не захвачен, обработка передается следующему элементу в списке
			// + в инкапсуляции состояния, и отсутствии необходимости извне опрашивать состояние кнопки
			if (!IsSelected())
				return false;

			if (!IsCaptured())
			{
				if (IsSystemEnterEvent(event, true))
				{
					Capture();
					NRF24L01::Scanner.SetActive(true);
					return true;
				}
				return false;
			}

			if (IsSystemLeftEvent(event, true))
				Capture(false);

			return true;
		}
	};
};

class NRF24L01ReadScene : public IScene
{
protected:
	SettingsContainer settings;

public:
	NRF24L01ReadScene() : settings { NRF24L01::PVariant.GetOrDefault() ? "====NRF24L01P===" : "====NRF24L01====", &CommonAllocator }
	{
		struct
		{
			StaticText32 name;
			IDataCell<uint8_t>* cell;
		} 
		ReadItems[]
		{
			{ "CONFIG",	&NRF24L01::Config },
			{ "STATUS",	&NRF24L01::Status },
			{ "SETUP",	&NRF24L01::Setup },
			{ "RETR",	&NRF24L01::SetupRetr },
			{ "FIFO",	&NRF24L01::FifoStatus },
		};

		for (auto item : ReadItems)
		{
			settings.AddSetting<ValueSettingUI<uint8_t>>(item.name, Point2_i{-1, -1}, item.cell, "%02X");
		}
		
		settings.Enable();
		AddObject(&settings);
	}

	bool Loop() override
	{
		settings.UpdateCurrentValue();
		return true;
	}
};


class NRF24L01SettingScene : public IScene
{
protected:
	SettingsContainer settings;

public:
	NRF24L01SettingScene() : settings { NRF24L01::PVariant.GetOrDefault() ? "====NRF24L01P===" : "====NRF24L01====", &CommonAllocator }
	{
		using namespace NRF24L01_UI;

		StaticListView<ListSettingItem<uint8_t>> pipes_view = pipes;

		settings.AddSetting<ListEditableSettingUI<uint8_t>>("RX-PIPE", Point2_i{-1, -1}, &NRF24L01::RxPipe, pipes_view, false);
		settings.AddSetting<ListEditableSettingUI<uint8_t>>("TX-PIPE", Point2_i{-1, -1}, &NRF24L01::TxPipe, pipes_view, false);
		settings.AddSetting<ValueEditableSettingUI<uint8_t>>("CHANNEL", Point2_i{-1, -1}, &NRF24L01::Channel, "%d", 0, 127, 1, false);

		if (NRF24L01::PVariant.GetOrDefault())
			settings.AddSetting<ScanSettingUI>("СКАНИРОВАТЬ", Point2_i{-1, -1});
		
		settings.Enable();
		AddObject(&settings);
	}
};

//class NRF24L01Scene : public CommonMenuScene
//{
//public:
//	NRF24L01Scene() : CommonMenuScene{ "====NRF24L01P===", 4, (uint8_t) SceneID::SYSTEM }
//	{
//		menu.AppendMenuItem("Регистры", new NRF24L01ReadScene);
//		menu.AppendMenuItem("Настройки", new NRF24L01SettingScene);
//	}
//};

#include <UI/CapacitySettingUI.hpp>
#include <SceneManager/SceneManager.hpp>
#include <TaskHandler/PriorityTaskSheduler.hpp>

class CapacitySettingScene : public IScene
{
protected:
	SettingsContainer settings;

public:
	CapacitySettingScene() : settings{ "ПАМЯТЬ", &CommonAllocator }
	{
		settings.AddSetting<CapacitySettingUI>("Scenes", Point2_i{-1, -1}, &CommonAllocator, CapacitySettingsStyle::STYLE_1);
		settings.AddSetting<CapacitySettingUI>("Scenes", Point2_i{-1, -1}, &CommonAllocator, CapacitySettingsStyle::STYLE_3);
		settings.AddSetting<CapacitySettingUI>("Tasks", Point2_i{-1, -1}, &CommonAllocator, CapacitySettingsStyle::STYLE_1);
		settings.AddSetting<CapacitySettingUI>("Tasks", Point2_i{-1, -1}, &CommonAllocator, CapacitySettingsStyle::STYLE_3);

		settings.Enable();
		AddObject(&settings);
	}

	bool Loop() override
	{
		settings.UpdateCurrentValue();
		return true;
	}

};

// ----------------------------------------------------------------------------------------------------

SystemScene::SystemScene() : CommonMenuScene{ "-----СИСТЕМА----", 8 }
{
	menu.AppendMenuItem("Задачи",		CommonAllocator.Make<TasksScene>());
	menu.AppendMenuItem("Устройства",	CommonAllocator.Make<DevicesScene>());
	menu.AppendMenuItem("Связь",		CommonAllocator.Make<WebScene>());
	menu.AppendMenuItem("Память",		CommonAllocator.Make<CapacitySettingScene>());
	menu.AppendMenuItem("NRF24L01-R",	CommonAllocator.Make<NRF24L01ReadScene>());
	menu.AppendMenuItem("NRF24L01-S",	CommonAllocator.Make<NRF24L01SettingScene>());
	menu.AppendMenuItem("Время",		CommonAllocator.Make<TimeScene>());
	menu.AppendMenuItem("Рандом",		CommonAllocator.Make<RandomScene>());
}

// ----------------------------------------------------------------------------------------------------

