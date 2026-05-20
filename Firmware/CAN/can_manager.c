#include "can_manager.h"
#include <string.h>

static CAN_HandleTypeDef *canHandle;

/* RX queue */
static CAN_Message_t rxQueue[CAN_RX_QUEUE_SIZE];

static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

static uint8_t Queue_IsEmpty(void)
{
    return head == tail;
}

static uint8_t Queue_IsFull(void)
{
    return ((head + 1) % CAN_RX_QUEUE_SIZE) == tail;
}

void CAN_Manager_Init(CAN_HandleTypeDef *hcan,
                      IRQn_Type rx_irq)
{
    canHandle = hcan;

    CAN_Manager_SetFilter(0x000, 0x000);

    HAL_CAN_Start(canHandle);

    HAL_CAN_ActivateNotification(
        canHandle,
        CAN_IT_RX_FIFO0_MSG_PENDING
    );

    HAL_NVIC_SetPriority(rx_irq, 0, 0);
    HAL_NVIC_EnableIRQ(rx_irq);
}

HAL_StatusTypeDef CAN_Manager_SetFilter(uint32_t id,
                                        uint32_t mask)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;

    filter.FilterIdHigh = (id << 5);
    filter.FilterIdLow  = 0;

    filter.FilterMaskIdHigh = (mask << 5);
    filter.FilterMaskIdLow  = 0;

    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;

    return HAL_CAN_ConfigFilter(canHandle, &filter);
}

HAL_StatusTypeDef CAN_Manager_Send(uint32_t id,
                                   uint8_t *data,
                                   uint8_t len)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    txHeader.StdId = id;
    txHeader.IDE   = CAN_ID_STD;
    txHeader.RTR   = CAN_RTR_DATA;
    txHeader.DLC   = len;

    return HAL_CAN_AddTxMessage(
        canHandle,
        &txHeader,
        data,
        &txMailbox
    );
}

uint8_t CAN_Manager_Read(CAN_Message_t *msg)
{
    if (Queue_IsEmpty())
        return 0;

    *msg = rxQueue[tail];

    tail = (tail + 1) % CAN_RX_QUEUE_SIZE;

    return 1;
}

void CAN_Manager_RxCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    if (HAL_CAN_GetRxMessage(
            hcan,
            CAN_RX_FIFO0,
            &rxHeader,
            rxData) != HAL_OK)
    {
        return;
    }

    if (Queue_IsFull())
    {
        /* Optional:
           overwrite oldest
           or discard newest

           Currently:
           discard newest
        */
        return;
    }

    rxQueue[head].id = rxHeader.StdId;
    rxQueue[head].dlc = rxHeader.DLC;

    memcpy(
        rxQueue[head].data,
        rxData,
        rxHeader.DLC
    );

    head = (head + 1) % CAN_RX_QUEUE_SIZE;
}