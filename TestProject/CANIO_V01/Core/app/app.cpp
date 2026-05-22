/*
 * app.c
 *
 *  Created on: May 19, 2026
 *      Author: e.maksimov
 */



#include "app.h"
#include "settings.h"
#include "canio.h"

using namespace maxssau;

uint8_t	_rx_buff[CANIO_RX_BUFF_SIZE];
uint8_t	_tx_buff[CANIO_TX_BUFF_SIZE];

CanIO_Array<uint8_t>	rx_buff(_rx_buff,CANIO_RX_BUFF_SIZE);
CanIO_Array<uint8_t>	tx_buff(_tx_buff,CANIO_TX_BUFF_SIZE);;

CanIO canio_object(
		CANIO_ADDR_RX_COMMANDS
		,CANIO_ADDR_RX_DATA
		,CANIO_ADDR_TX_COMMANDS
		,CANIO_ADDR_TX_DATA
		,&rx_buff,&tx_buff
		,&SendCanMessage
		,&CANIOParseAnswer
		);

void SendCanMessage(uint8_t interface, uint8_t can_id, uint8_t *data)
{

}

void CANIOParseAnswer(uint16_t command, uint16_t user_data1,uint32_t user_data2)
{

}

void app()
{
	canio_object.can_send_message=&SendCanMessage;
	while(1);
}


