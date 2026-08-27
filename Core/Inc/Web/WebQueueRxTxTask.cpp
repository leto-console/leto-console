#include "Web/WebQueueRxTxTask.hpp"

#include <LetoAPI_V1_System/Web/WebPacketParser.hpp>
#include <LetoAPI_V1_System/Web/WebManager.hpp>

#include <ProjectSettings/ProjectSettings.hpp>
#include <NRF24L01/NRF24L01_data.hpp>


WebQueueRxTxTask::WebQueueRxTxTask(const StaticText32& name, uint32_t period_ms, uint8_t priority)
	: PriorityTask{ name, period_ms, priority }
{	
	buffer_wait_timer.Start(1000);
}

WebQueueRxTxTask::~WebQueueRxTxTask()
{
#if defined(_WIN32)
	if (virtual_uart)
		delete virtual_uart;
#endif
}

#if defined(USE_HAL_DRIVER)
void WebQueueRxTxTask::Init(UART_HandleTypeDef* huart_web)
{
	this->huart_web = huart_web;
}
#elif defined(_WIN32)
void WebQueueRxTxTask::Init(bool server, std::string pipe_name)
{
	this->virtual_uart = new WinPipeUART(server, pipe_name);
}
#endif

// ====================================================================================================

void WebQueueRxTxTask::UpdateSettings()
{
	uint8_t pipe;
	if (NRF24L01::RxPipe.Get(pipe) && pipe > 0 && pipe < 6)
	{
		rx_pipe_idx = pipe;
	}

	if (NRF24L01::TxPipe.Get(pipe) && pipe > 0 && pipe < 6)
	{
		tx_pipe = NRF24L01::PipeAddress[pipe];
	}

	uint8_t chan;
	if (!NRF24L01::Channel.Get(chan))
		chan = 32;
	
	if (nrf_chan != chan)
	{
		nrf_chan = chan;
#if defined(USE_HAL_DRIVER)
		setChannel(nrf_chan);
#endif
	}
}

bool WebQueueRxTxTask::DoTx(uint8_t channel, WebPacket_V1 &msg_out)
{
	bool success = false;
	if (!channel || channel == WC_CHANNEL_UART)
		success = DoTxUART(msg_out) || success;
	if (!channel || channel == WC_CHANNEL_NRF)
		success = DoTxNRF(msg_out) || success;
	return success;
}

void WebQueueRxTxTask::DoRxUART()
{
#if defined(_WIN32)
	if (virtual_uart)
		virtual_uart->poll();

	// Приём данных
	std::vector<uint8_t> rx;
	if (virtual_uart && virtual_uart->receive(rx))
	{
		for (uint8_t b : rx)
			WebPacketParser::CallbackByte(WC_CHANNEL_UART, b);
	}
#endif
}

bool WebQueueRxTxTask::DoTxUART(WebPacket_V1& msg_out)
{
#if defined(USE_HAL_DRIVER)
	if (huart_web)
		HAL_UART_Transmit(huart_web, (uint8_t*)&msg_out, sizeof(msg_out), 100);
#elif defined(_WIN32)
	if (virtual_uart && virtual_uart->isConnected())
		virtual_uart->send(&msg_out, sizeof(msg_out));
#endif
	// Заглушка, потому что пока не было обнаружено особых проблем
	return true;
}

void WebQueueRxTxTask::DoRxNRF()
{
#if defined(USE_HAL_DRIVER)
	if (!isPowerUp())
		return;

	openReadingPipe(1, NRF24L01::PipeAddress[rx_pipe_idx]);
	startListening();

	uint8_t nrf_data[32] = { 0, }; // буфер указываем максимального размера
	uint8_t pipe_num = 0;

	while (available(&pipe_num) && isPowerUp()) // проверяем пришло ли что-то
	{
		uint8_t count = getDynamicPayloadSize(); // смотрим сколько байт прилетело
		read(&nrf_data, count); // Читаем данные в массив nrf_data и указываем сколько байт читать

		// Единственный приемный пайп
		if (pipe_num == 1)
		{
			printf("[RX%d]        : ", pipe_num);

			for (int i = 0; i < count; ++i)
				printf("%02X ", nrf_data[i]);
			printf("\n");

			for (int i = 0; i < count; ++i)
				WebPacketParser::CallbackByte(WC_CHANNEL_NRF, nrf_data[i]);
		}
		else
		{
			printf("Unknown pipe\n");
		}
	}
#endif
}

bool WebQueueRxTxTask::DoTxNRF(WebPacket_V1& msg_out)
{
#if defined(USE_HAL_DRIVER)
	// Если питание выключено - считаем, что сообщение успешно потеряно
	if (!isPowerUp())
		return true;

	stopListening();
	openWritingPipe(tx_pipe);
    bool w = write(&msg_out, sizeof(msg_out)); // отправляем данные
	printf("[TX] (res: %d): ", w);
	
	for (unsigned i = 0; i < sizeof(msg_out); ++i)
		printf("%02X ", ((uint8_t*)&msg_out)[i]);
	printf("\n");

	startListening();

	return w;
#else
	// Заглушка под WIN/Linux
	return true;
#endif
}


// ====================================================================================================

bool WebQueueRxTxTask::Do()
{
	UpdateSettings();
	DoRxUART();
	DoRxNRF();

	WebManager_V1::Instance().Loop();

	if (ping_mode)
	{
		if (ping_timer.Expired())
		{
			static WebPacket_V1 ping_msg{};
			ping_msg.port = WC_PORT_PING;
			ping_mode = !DoTxNRF(ping_msg);
			ping_timer.Start();
		}
	}

	WebPacket_BufferItem info;

	while (WebManager_V1::Instance().CheckBuffer(info))
	{
		info.packet.CalcCRC16();

		if (!DoTx(info.channel, info.packet))
		{
			if (buffer_wait_timer.Expired())
			{
				ping_mode = true;
				ping_timer.Start(200);
			}
			return true;
		}

		buffer_wait_timer.Start();
		WebManager_V1::Instance().NextBuffer();
	}

	return true;
}
