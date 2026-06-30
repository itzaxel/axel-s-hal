#include "uart_driver.h"
#define TIMEOUT 10

volatile uint8_t UART1_DMA_RX_State = 0;
volatile uint8_t UART1_DMA_TX_State = 0;

UART_Condition UART_Transmit(USART_TypeDef *port, uint8_t *data, uint32_t len) {
    if (data == NULL) return BUF_EMPTY;
    for (uint32_t i = 0; i < len; i++) {
        uint32_t tick = HAL_GetTick();
        while (!LL_USART_IsActiveFlag_TXE(port)){
            if (HAL_GetTick()-tick>TIMEOUT) {return UART_TIMEOUT;}
        }
        LL_USART_TransmitData8(port, *data++);
    }
    uint32_t tick = HAL_GetTick();
    while (!LL_USART_IsActiveFlag_TC(port)){
        if (HAL_GetTick()-tick>TIMEOUT) {return UART_TIMEOUT;}
    }
    return UART_OK;
}

UART_Condition UART_Receive(USART_TypeDef *port, uint8_t *output, uint32_t len) {
    if (output == NULL) return BUF_EMPTY;
    for (uint32_t i = 0; i < len; i++) {
        uint32_t tick = HAL_GetTick();
        while (!LL_USART_IsActiveFlag_RXNE(port))
        {
            if (HAL_GetTick()-tick>TIMEOUT) {return UART_TIMEOUT;}
        }
        if (LL_USART_IsActiveFlag_ORE(port)) {
            LL_USART_ClearFlag_ORE(port);
            return UART_ERROR;
        }

        output[i] = LL_USART_ReceiveData8(port);
    }
    return UART_OK;
}

void UART1_Receive_DMA(uint8_t *buffer, uint32_t size) {
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);

    LL_DMA_ClearFlag_TC2(DMA2);
    LL_DMA_ClearFlag_TE2(DMA2);

    // Peripheral TO Memory
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)buffer);
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2, LL_USART_DMA_GetRegAddr(USART1));
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, size);


    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);
    LL_USART_EnableDMAReq_RX(USART1);
}

void UART1_Transmit_DMA(uint8_t *buffer, uint16_t size) {
    if (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_7)) {return;}

    LL_DMA_ClearFlag_TC7(DMA2);

    // Memory TO Peripheral
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)buffer);
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, LL_USART_DMA_GetRegAddr(USART1));
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_7, size);

    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_7);
    LL_USART_EnableDMAReq_TX(USART1);
}

UART_Condition UART_SetBaudRate(uint32_t baud, USART_TypeDef *port) {
    LL_RCC_ClocksTypeDef rcc_clocks;
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);
    uint32_t clock = 0;
    if (port == USART1 || port == USART6)
    {
        clock = rcc_clocks.PCLK2_Frequency;
    }else
    {
        clock = rcc_clocks.PCLK1_Frequency;
    }
    if (baud == LL_USART_GetBaudRate(
      port,
      clock,
      LL_USART_OVERSAMPLING_16 )
      ) {return UART_OK;}
    uint32_t tick = HAL_GetTick();
    while (!LL_USART_IsActiveFlag_TC(port)){
        if (HAL_GetTick()-tick>TIMEOUT){
            return UART_TIMEOUT;
        }
    }
    LL_USART_Disable(port);
    LL_USART_SetBaudRate(
      port,
      clock,
      LL_USART_OVERSAMPLING_16,
      baud);
    LL_USART_Enable(port);
    return UART_OK;
}


// for 1-wire
void LSB_Encode(uint8_t byte, uint8_t *out_buffer) {
    for (int i = 0; i < 8; i++) {
        if (byte & (1 << i)) {
            out_buffer[i] = 0xFF;
        } else {
            out_buffer[i] = 0x00;
        }
    }
}

uint8_t LSB_Decode(const uint8_t *ow_rx_buffer) {
    uint8_t byte = 0x00;
    for (int i = 0; i < 8; i++) {
        byte>>=1;
        if (ow_rx_buffer[i] >= 0xFE) {
            byte|= 0x80;
        }
    }
    return byte;
}