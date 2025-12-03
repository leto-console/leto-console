/*
 * WebTask.hpp
 *
 *  Created on: Dec 10, 2025
 *      Author: Timur
 */

#ifndef INC_WEB_WEB_QUEUE_RX_TX_TASK_HPP_
#define INC_WEB_WEB_QUEUE_RX_TX_TASK_HPP_

#include <TaskHandler/PriorityTask.hpp>
#include <LetoAPI_V1_System/Web/WebPacket.hpp>
#include <Time/Timer.hpp>

#ifdef USE_HAL_DRIVER

#include <HAL_include/HAL.hpp>
#include <NRF24L01/nrf24l01.h>

#elif defined(_WIN32)

#include <UART/WinPipeUART.hpp>

#endif

/*
	Задача для отправки из очереди web_out
	и получения в очередь web_in.
	В HAL получение реализовано с помощью IRQ
*/
class WebQueueRxTxTask : public PriorityTask
{
protected:
#if defined(USE_HAL_DRIVER)
	UART_HandleTypeDef* huart_web = nullptr;
#elif defined(_WIN32)
	WinPipeUART* virtual_uart = nullptr;
#endif

	Timer common_retries_timer;
	Timer waiting_ack_timer;

	bool ping_mode{};
	Timer ping_timer;
	Timer buffer_wait_timer;

	bool DoTx(uint8_t channel, WebPacket_V1& msg_out);

	void DoRxUART();
	bool DoTxUART(WebPacket_V1& msg_out);

	uint8_t nrf_chan = 0;
	uint8_t rx_pipe_idx = 0;
	uint32_t tx_pipe = 0;
	void DoRxNRF();
	bool DoTxNRF(WebPacket_V1& msg_out);

public:
	WebQueueRxTxTask(const StaticText32& name, uint32_t period_ms, uint8_t priority);
	~WebQueueRxTxTask();

#if defined(USE_HAL_DRIVER)
	void Init(UART_HandleTypeDef* huart_web);
#elif defined(_WIN32)
	void Init(bool server, std::string pipe_name);
#endif

	bool Do() override;
};

#endif
