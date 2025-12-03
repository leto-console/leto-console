#include "ProjectSettings/ProjectSettings.hpp"

// --------------------------------------------------

Storage SystemStorage;

// --------------------------------------------------

static uint16_t USER_ADDR[]
{
	0x0400,
	0x0800,
	0x0C00,
	0x1000,
	0x1400,
	0x1800,
	0x1C00,
};

// --------------------------------------------------

// Адрес: 0x0000,	Размер: 4 Б
StoredDataCell<uint32_t>		StartsCount			(SystemSettingsStart + 0x00, SystemStorage);
// Адрес: 0x0004:0, Размер: 1 бит
StoredDataCell<bool>			EncoderReverse		(SystemSettingsStart + 0x04, 0, SystemStorage);
// Адрес: 0x0004:1, Размер: 1 бит
StoredDataCell<bool>			UARTConsoleOnStart	(SystemSettingsStart + 0x04, 1, SystemStorage);
// Адрес: 0x0004:2, Размер: 1 бит
StoredDataCell<bool>			EnableSnowfall		(SystemSettingsStart + 0x04, 2, SystemStorage);
// Адрес: 0x0004:3, Размер: 1 бит
StoredDataCell<bool>			DebugMode			(SystemSettingsStart + 0x04, 3, SystemStorage);

// Адрес: 0x0008,	Размер: 4 Б
StoredDataCell<uint32_t>		SerialNumber		(SystemSettingsStart + 0x08, SystemStorage);


namespace NRF24L01
{
// Адрес: 0x0010,	Размер: 1 Б
StoredDataCell<uint8_t>			RxPipe				(SystemSettingsStart + 0x10, SystemStorage);
// Адрес: 0x0011,	Размер: 1 Б
StoredDataCell<uint8_t>			TxPipe				(SystemSettingsStart + 0x11, SystemStorage);
// Адрес: 0x0012,	Размер:	1 Б
StoredDataCell<uint8_t>			Channel				(SystemSettingsStart + 0x12, SystemStorage);
}
