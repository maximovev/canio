#pragma once

#include <stdint.h>

namespace maxssau
{

#define	CANIO_VERSION									0x0001

#define	CANIO_OPERATION_FAIL							0
#define	CANIO_OPERATION_SUCCESS							1
#define	CANIO_OPERATION_IN_PROGRESS						2
#define	CANIO_OPERATION_ERROR_OVERLOAD_BUFFER			3
#define	CANIO_OPERATION_ERROR_UNDERLOAD_BUFFER			4
#define	CANIO_OPERATION_SUCCESS_SEND					5
#define	CANIO_OPERATION_SUCCESS_RECEIVE					6
#define	CANIO_OPERATION_ERROR_INVALID_CRC				7
#define	CANIO_OPERATION_ERROR_PROTOCOL					8


#define	CANIO_COMMAND_TYPE_PING							0x0000	//	пинг устройства
#define	CANIO_COMMAND_TYPE_PING_ANSWER					0x0001	//	ответ устройства на пинг
#define	CANIO_COMMAND_START_UPLOAD						0x0002	//	старт отправки данных
#define	CANIO_COMMAND_START_DOWNLOAD					0x0003	//	старт приёма данных
#define	CANIO_COMMAND_START_UPLOAD_ANSWER				0x0004	//	старт отправки данных, ответ
#define	CANIO_COMMAND_START_DOWNLOAD_ANSWER				0x0005	//	старт приёма данных, ответ
#define	CANIO_COMMAND_RECEIVE_DATA						0x0006	//	приём данных
#define	CANIO_COMMAND_SEND_DATA							0x0007	//	отправка данных
#define	CANIO_COMMAND_ERROR								0x0008	//	если пошло что то не так отправляем ошибку и описание
#define	CANIO_COMMAND_DOWNLOAD_COMPLETE					0x0009	//	успешное считывание блока
#define	CANIO_COMMAND_UPLOAD_COMPLETE					0x000A	//	успешная запись блока



#pragma pack(push, 1)

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

typedef struct
{
	uint16_t	current_state;
	uint16_t	current_operation;

	uint32_t	rx_buffer_index;
	uint32_t	tx_buffer_index;


} canio_machinestate;

typedef struct
{
	uint16_t	version;
	uint32_t	buffer_size;
	uint32_t	last_timestamp_tx;
	uint32_t	last_timestamp_rx;
	uint16_t	error_number;
} canio_remote_node;

#pragma pack(pop)

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

	uint32_t	GetArraySize()
	{
		return size;
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

	bool ParseCANMessage(uint32_t can_id, uint8_t* data, uint8_t interface, uint32_t timestamp)
	{
		bool result=false;

		for(int i=0;i<8;i++)
		{
			rx_command.bytes[i]=data[i];
		}

		if(can_id==can_rx_addr_commands)
		{

			remote_node.last_timestamp_rx=timestamp;

			switch(rx_command.command.command)
			{
				case CANIO_COMMAND_TYPE_PING:
				{
					/*	На команду пинга посылаем ответ, с версией модуля и размером приёмного буфера
					 */
					tx_command.command.command=CANIO_COMMAND_TYPE_PING_ANSWER;
					tx_command.command.user_code=CANIO_VERSION;
					tx_command.command.user_data=rx_data->GetArraySize();
					state.current_operation=CANIO_COMMAND_TYPE_PING;
					state.current_state=CANIO_OPERATION_SUCCESS;
					remote_node.last_timestamp_tx=timestamp;
					SendCANMessage(interface,can_tx_addr_commands, tx_command.bytes);
				}break;
				case CANIO_COMMAND_TYPE_PING_ANSWER:
				{
					/*	в ответном сообщении читаем версию и размер буфера
					 */
					remote_node.version=rx_command.command.user_code;
					remote_node.buffer_size=rx_command.command.user_data;
				}break;
				case CANIO_COMMAND_START_UPLOAD:
				{
					/*	старт отправки данных из буфера. Команад подготавливает приёмник к приёму данных и
					 * 	отвечает передатчику готовность приёма данных
					 *
					 * 	в секции user_data отправляем CRC пакета
					 */
					state.current_operation=CANIO_COMMAND_RECEIVE_DATA;
					state.rx_buffer_index=0;
					state.current_state=CANIO_OPERATION_SUCCESS;
					tx_command.command.command=CANIO_COMMAND_START_UPLOAD_ANSWER;
					tx_command.command.user_data=0;
					SendCANMessage(interface,can_tx_addr_commands, tx_command.bytes);
				}break;
				case CANIO_COMMAND_START_UPLOAD_ANSWER:
				{
					/*	Устройство готово к приёму и переключилось на приём данных
					 *
					 */
					state.current_operation=CANIO_COMMAND_SEND_DATA;
				}break;
				case CANIO_COMMAND_START_DOWNLOAD:
				{
					/*	Готовимся к приёму данных
					 *
					 */
					state.current_operation=CANIO_COMMAND_SEND_DATA;
					state.tx_buffer_index=0;
					state.current_state=CANIO_OPERATION_SUCCESS;
					tx_command.command.command=CANIO_COMMAND_START_DOWNLOAD_ANSWER;
					tx_command.command.user_data=crc_tx;
					SendCANMessage(interface,can_tx_addr_commands, tx_command.bytes);
					remote_node.last_timestamp_tx=timestamp;
				}break;
				case CANIO_COMMAND_START_DOWNLOAD_ANSWER:
				{
					state.current_operation=CANIO_COMMAND_RECEIVE_DATA;
				}break;
				case CANIO_COMMAND_ERROR:
				{
					state.current_state=CANIO_COMMAND_ERROR;
					remote_node.error_number=rx_command.command.user_code;
				}break;
			}
		}

		if(can_id==can_rx_addr_data)
		{
			switch(state.current_operation)
			{
				case CANIO_COMMAND_RECEIVE_DATA:
				{
					/*	заполняем приёмный буфер
					 *
					 */
					if(state.current_operation==CANIO_COMMAND_RECEIVE_DATA)
					{
						if(state.rx_buffer_index<rx_data->GetArraySize())
						{
							for(int i=0;i<8;i++)
							{
								rx_data->SetElement(data[i], state.rx_buffer_index+i);
							}
							state.rx_buffer_index=state.rx_buffer_index+8;
							if(state.rx_buffer_index==rx_data->GetArraySize())
							{
								// receive complete
								uint32_t receive_crc=CalculateCRC(rx_data);
								if(receive_crc==crc_rx)
								{
									tx_command.command.command=CANIO_COMMAND_DOWNLOAD_COMPLETE;
								}
								else
								{
									tx_command.command.command=CANIO_COMMAND_ERROR;
									tx_command.command.user_code=CANIO_OPERATION_ERROR_INVALID_CRC;
								}
								SendCANMessage(interface,can_tx_addr_commands, tx_command.bytes);
							}
						}
					}
					else
					{
						tx_command.command.command=CANIO_COMMAND_ERROR;
						tx_command.command.user_code=CANIO_OPERATION_ERROR_PROTOCOL;
						state.current_state=CANIO_OPERATION_ERROR_PROTOCOL;
					}
				}break;
				default:
				{
					tx_command.command.command=CANIO_COMMAND_ERROR;
					tx_command.command.user_code=CANIO_OPERATION_ERROR_PROTOCOL;
					state.current_state=CANIO_OPERATION_ERROR_PROTOCOL;
				}break;
			}
		}

		return result;
	}



private:

	uint32_t CalculateCRC(CanIO_Array<uint8_t> *data)
	{

	}



	bool SendCANMessage(uint8_t interface, uint8_t can_id, uint8_t *data)
	{
		bool result=false;
		if(can_send_message!=nullptr)
		{
			can_send_message(interface,can_id,data);
			result=true;
		}
		return result;
	}

	uint32_t	can_rx_addr_commands;
	uint32_t	can_tx_addr_commands;
	uint32_t	can_rx_addr_data;
	uint32_t	can_tx_addr_data;

	uint32_t	crc_rx;
	uint32_t	crc_tx;

	CanIO_Array<uint8_t>	*rx_data;
	CanIO_Array<uint8_t>	*tx_data;

	canio_command			rx_command;
	canio_command			tx_command;

	canio_machinestate		state;

	canio_remote_node		remote_node;
};

}
