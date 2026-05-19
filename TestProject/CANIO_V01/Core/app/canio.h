#pragma once

#include <stdint.h>

#define	CANIO_COMMAND_TYPE_PING				0x0000



typedef union
{
	struct
	{
		uint16_t	command;
		uint16_t	user_code;
		uint32_t	user_data;
	}command;
	uint8_t	bytes[sizeof(command)];
} canio_command;

/*	Безопасная работа с массивами
 *
 */
template <typename ClassType>class CanIO_Array
{
public:
	CanIO_Array(
			ClassType*	_array,
			uint32_t	_size
			){
		array=_array;
		size=_size;
	}

	bool SetElement(ClassType data, uint32_t index)
	{
		bool result=false;
		if(index<size)
		{
			array[index]=data;
			result=true;
		}
		return result;
	}

	ClassType GetElement(uint32_t index)
	{
		ClassType result=nullptr;
		if(index<size)
		{
			result=array[index];
		}
		return result;
	}
private:
	ClassType*	array;
	uint32_t	size;
};

class CanIO
{
public:

	void 		(*can_send_message)(uint8_t interface, uint8_t can_id, uint8_t *data);
	void		(*parse_result)(uint16_t command, uint16_t user_data1,uint32_t user_data2);

	CanIO()
	{

	}

	CanIO(uint32_t rx_addr_commands
			,uint32_t rx_addr_data
			,uint32_t tx_addr_commands
			,uint32_t tx_addr_data
			, CanIO_Array<uint8_t> *_rx_data
			,CanIO_Array<uint8_t> *_tx_data
			,void (*can_send_message_function)(uint8_t interface, uint8_t can_id, uint8_t *data)
			,void (*parse_result_function)(uint16_t command, uint16_t user_data1,uint32_t user_data2)
	)
	{
		init(rx_addr_commands,rx_addr_data,tx_addr_commands,tx_addr_data,_rx_data,_tx_data, can_send_message_function, parse_result_function);
	}

	bool init(
			uint32_t rx_addr_commands
			,uint32_t rx_addr_data
			,uint32_t tx_addr_commands
			,uint32_t tx_addr_data
			,CanIO_Array<uint8_t> *_rx_data
			,CanIO_Array<uint8_t> *_tx_data
			,void (*can_send_message_function)(uint8_t interface, uint8_t can_id, uint8_t *data)
			,void (*parse_result_function)(uint16_t command, uint16_t user_data1,uint32_t user_data2)
			)
	{
		bool result=false;

		can_rx_addr_commands=rx_addr_commands;
		can_tx_addr_commands=tx_addr_commands;

		can_rx_addr_data=rx_addr_data;
		can_tx_addr_data=tx_addr_data;

		rx_data=_rx_data;
		tx_data=_tx_data;

		can_send_message=can_send_message_function;
		parse_result=parse_result_function;

		return result;
	}

	bool ParseCANMessage(uint32_t can_id, uint8_t* data)
	{
		bool result=false;


		return result;
	}

private:
	uint32_t	can_rx_addr_commands;
	uint32_t	can_tx_addr_commands;
	uint32_t	can_rx_addr_data;
	uint32_t	can_tx_addr_data;

	CanIO_Array<uint8_t>	*rx_data;
	CanIO_Array<uint8_t>	*tx_data;

};
