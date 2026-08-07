#include "ProjectScenes/EEPROMScene.hpp"

#include <Input/SystemInputID.hpp>

#include <Input/EncoderEvent.hpp>

#include <Graphics/DefaultFont.hpp>
#include <SSD1306/SSD1306_Properties.hpp>

#include <Data/StaticText.hpp>
#include <DrawFunctions/DrawLine.hpp>
#include <DrawFunctions/DrawText.hpp>

#include <cstdio>

// Страница с информацией в EEPROM 
class EEPROMReadScene : public IScene
{
protected:
	EEPROM_24C& eeprom;
	enum { EEPROM_PAGE_SIZE = 2048 };
	bool needToUpdate{ true };

	uint8_t data[EEPROM_PAGE_SIZE]{};
	uint32_t startPos{};
	bool charMode{};

	virtual void Load()
	{
		for (uint16_t i = 0; i < EEPROM_PAGE_SIZE; ++i)
			data[i] = 0xFC;

		eeprom.Read(0,	0,						&data[0],						EEPROM_PAGE_SIZE / 2);
		eeprom.Read(0,	EEPROM_PAGE_SIZE / 2,	&data[EEPROM_PAGE_SIZE / 2],	EEPROM_PAGE_SIZE / 2);
	}

	void ScrollDown()
	{
		if (startPos <= EEPROM_PAGE_SIZE - (8 + 1) * 8) startPos += 8;
	}

	void ScrollUp()
	{
		if (startPos > 7) startPos -= 8;
		else startPos = 0;
	}

public:
	EEPROMReadScene(ISceneManager* scene_manager, EEPROM_24C* eeprom)
		: IScene{scene_manager}, eeprom{ *eeprom }
	{
	}
	void DrawByteLine(IScreen& screen, Point2_i point, uint8_t* data, uint32_t size)
	{
		using namespace DrawFunctions;

		StaticText32 text{};
		RGBColor text_clr, back_clr;

		for (size_t i = 0; i < size; ++i)
		{
#ifdef USE_ST7735
			text_clr = data[i] == 0 ? WhiteColor : LightGreenColor;
			back_clr = data[i] == 0 ? BlackColor : DarkRedColor;
#else
			text_clr = WhiteColor;
			back_clr = BlackColor;
#endif

			snprintf(text.CharPtr(), 3, "%02X", data[i]);
			DrawText(screen, point, text, text_clr, back_clr, false, &Default_Font_7x7);
			point.x += 14;
		}

		//return text;
	}
	StaticText32 GetPosText(uint32_t pos)
	{
		char text[3]{};
		pos %= 0x100;
		snprintf(text, sizeof(text), "%02X", (unsigned int) pos);
		return text;
	}
	void OnShow() override
	{
		startPos = 0;
		charMode = false;
		Load();
	}
	void Draw(IScreen& screen) override
	{
		using namespace DrawFunctions;

		constexpr int left_offset = 17;
		constexpr int font_width = 7;
		constexpr int line_height = 8;
#ifdef USE_ST7735
		constexpr RGBColor line_num_clr = GrayColor;
#else
		constexpr RGBColor line_num_clr = WhiteColor;
#endif
		
		for (uint8_t i = 0; i < 8; ++i)
		{
			DrawText(screen, {0, i * line_height}, GetPosText(startPos + i * 8), line_num_clr, BlackColor, false, &Default_Font_7x7);
			if (!charMode)
			{
				DrawByteLine(screen, {left_offset, i * line_height}, data + startPos + i * 8, 8);
			}
			else
			{
				for (uint8_t byte = 0; byte < 8; ++byte)
				{
					char symbol = (data + startPos + i * 8)[byte];
					DrawChar(screen, { left_offset + byte * 2 * font_width + font_width / 2, i * line_height }, &symbol, WhiteColor, BlackColor, false, &Default_Font_7x7);
				}
			}
		}

		int line_x = left_offset - 4;
		DrawLine(screen, {line_x, 0}, {line_x, SSD1306_Height}, WhiteColor);

		for (uint8_t i = 0; i < 8; ++i)
		{
			line_x = left_offset + font_width * 2 * i - 1;
			DrawLine(screen, {line_x, 0}, {line_x, SSD1306_Height}, WhiteColor);
		}
	}
	bool ProcessInput(const AppEvent& event) override 
	{ 
		if (IsSystemEnterEvent(event))
		{
			charMode = !charMode;
			return true;
		}
		else if (IsSystemPrevEvent(event))
		{
			ScrollUp();
			return true;
		}
		else if (IsSystemNextEvent(event))
		{
			ScrollDown();
			return true;
		}

		return false; 
	}
	void Loop() override { }
};
//
//#include "ProjectSettings/ProjectSettings.hpp"
//
//class SaveReadScene : public EEPROMReadScene
//{
//public:
//	SaveReadScene(EEPROM_24C& eeprom)
//		: EEPROMReadScene{ eeprom }
//	{
//	}
//
//	void LoadPage(uint8_t page) override
//	{
//		BattleshipSave save{};
//		Battleship::GameSave.Get(save);
//
//		for (uint16_t i = 0; i < sizeof(save); ++i)
//			data[0][i] = ((uint8_t*) &save)[i];
//	}
//};

// ----------------------------------------------------------------------------------------------------

EEPROMScene::EEPROMScene(ISceneManager* scene_manager, EEPROM_24C* eeprom) 
	: CommonMenuScene{ scene_manager, "-----EEPROM-----" }
{
	menu.AppendMenuItem("Смотреть", scene_manager->GetCommonAllocator().Make<EEPROMReadScene>(scene_manager, eeprom));
//	menu.AppendMenuItem("Сохранение", CommonAllocator.Make<SaveReadScene>(eeprom));
	menu.AppendMenuItem("Изменить", nullptr);
}

// ----------------------------------------------------------------------------------------------------
