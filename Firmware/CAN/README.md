# STM32 CAN Manager

Lightweight CAN driver for STM32 HAL with interrupt-based RX and ring buffer.

## Setup

### 1. Add files
Copy into your project:
- Drivers/CAN/Inc/can_manager.h
- Drivers/CAN/Src/can_manager.c

---

### 2. Configure STM32 HAL version
Open can.manager.h and check the HAL include:
```
#include "stm32l4xx_hal.h"
```
Change ```stm32l4xx_hal.h``` to match your STM32 family if necessary


---

### 3. Include path

#### STM32CubeIDE (CubeMX project)
Add include directory:
- Drivers/CAN/Inc

(Project → Properties → C/C++ General → Paths and Symbols)

---

#### CMake project

Add to `CMakeLists.txt`:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/CAN/Src/can_manager.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/CAN/Inc
)
```

---

### 4. Include CAN manager
Add to ```main.c```  
in ```/* Includes */```:
```
#include "can_manager.h"
```

---

### 5. Initialize CAN
Add to ```main.c```  
in ```/* USER CODE BEGIN 2 */```:
```
CAN_Manager_Init(&hcan1, CAN1_RX0_IRQn);
```


---

### 6. Enable RX callback
Add to ```main.c```
in ```/* USER CODE BEGIN 4 */```:
```
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_Manager_RxCallback(hcan);
}
```

---

## Usage

### Send message
```
uint8_t data[1] = {42};
CAN_Manager_Send(0x200, data, 1);
```

---

### Read message
```
CAN_Message_t msg;

if (CAN_Manager_Read(&msg))
{
    // msg.id
    // msg.data[0..7]
}
```

---

### Set filter (optional)
```
CAN_Manager_SetFilter(0x100, 0x7FF); // exact match
```

---
### Physical connection
CAN communication requires a CAN transciever (e.g. SN65HVD230). The STM32 CAN uses logic-level signals and must NOT be connected directly to the CAN bus

Connect as follows:

| STM32 | Transceiver |
|---|---|
| STM32 CAN_TX | Transceiver TXD  |
| STM32 CAN_RX | Transceiver RXD  |
| STM32 3.3V   | Transceiver 3.3V |

| STM32/Transceiver | Shared |
|---|---|
| Transceiver CANH  | CANH   |
| Transceiver CANL  | CANL   |
| Tarnsceiver GND   | GND    |
| STM32 GND         | GND    |

---

## Notes
- RX uses ring buffer (16 messages)
- No message lost unless buffer full
- HAL-based, no RTOS required