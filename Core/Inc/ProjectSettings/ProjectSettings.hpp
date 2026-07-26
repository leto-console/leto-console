/*
 * ProjectSetting.hpp
 *
 *  Created on: Dec 12, 2025
 *      Author: Timur
 */

#ifndef INC_SETTINGS_PROJECT_SETTING_HPP_
#define INC_SETTINGS_PROJECT_SETTING_HPP_

#include <Storage/StoredDataCell.hpp>
#include <Storage/Storage.hpp>
#include <Storage/NamedSpace.hpp>
#include <Storage/NamedDataCell.hpp>

#include <Data/IDataCell.hpp>

#include <stdint.h>
#include <Data/StaticText.hpp>
#include <Time/DateTime.hpp>

// --------------------------------------------------

// Держатель настроек
extern Storage SystemStorage;

// -------------------- Системные настройки --------------------

constexpr uint16_t AuthHandlerStart = 0x00;
constexpr uint16_t SystemSettingsStart = 0x32;

// Количество запусков устройства
extern StoredDataCell<uint32_t>			StartsCount;

// Настройка энкодера
extern StoredDataCell<bool>				EncoderReverse;

// Включение консоли по UART
extern StoredDataCell<bool>				UARTConsoleOnStart;

// Включение снегопада
extern StoredDataCell<bool>				EnableSnowfall;

// Режим отладки
extern StoredDataCell<bool>				DebugMode;

// Серийный номер устройства
extern StoredDataCell<uint32_t>			SerialNumber;

namespace NRF24L01
{
	extern StoredDataCell<uint8_t> RxPipe;
	extern StoredDataCell<uint8_t> TxPipe;
	extern StoredDataCell<uint8_t> Channel;
}

namespace RTC_Settings
{
	extern StoredDataCell<DateTimeStruct> LastExactDateTime;
	extern StoredDataCell<int32_t> PPM;
}

#endif
