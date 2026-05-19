#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define CAN_RX_QUEUE_SIZE 16

typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
} CAN_Message_t;

void CAN_Manager_Init(CAN_HandleTypeDef *hcan,
                      IRQn_Type rx_irq);

HAL_StatusTypeDef CAN_Manager_Send(uint32_t id,
                                   uint8_t *data,
                                   uint8_t len);

uint8_t CAN_Manager_Read(CAN_Message_t *msg);

HAL_StatusTypeDef CAN_Manager_SetFilter(uint32_t id,
                                        uint32_t mask);

void CAN_Manager_RxCallback(CAN_HandleTypeDef *hcan);

#endif