/*
 * app.h
 *
 *  Created on: May 19, 2026
 *      Author: e.maksimov
 */

#ifndef APP_APP_H_
#define APP_APP_H_

#include "main.h"

void app();
void SendCanMessage(uint8_t interface, uint8_t can_id, uint8_t *data);
void CANIOParseAnswer(uint16_t command, uint16_t user_data1,uint32_t user_data2);


#endif /* APP_APP_H_ */
