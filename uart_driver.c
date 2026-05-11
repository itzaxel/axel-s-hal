#include "uart_driver.h"

void UART_Transmit(USART_TypeDef *port, uint8_t *data, uint32_t len) {
    if (data == NULL) return;
    for (uint32_t i = 0; i < len; i++) {
        while (!LL_USART_IsActiveFlag_TXE(port)) {}
        LL_USART_TransmitData8(port, *data++);
    }
    while (!LL_USART_IsActiveFlag_TC(port));
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

void UART1_SetBaudRate(uint32_t baud) {
    LL_RCC_ClocksTypeDef rcc_clocks;
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);
    if (baud == LL_USART_GetBaudRate(
      USART1,
      rcc_clocks.PCLK2_Frequency,
      LL_USART_OVERSAMPLING_16 )
      ) {return;}

    while (!LL_USART_IsActiveFlag_TC(USART1)) {}
    LL_USART_Disable(USART1);
    LL_USART_SetBaudRate(
      USART1,
      rcc_clocks.PCLK2_Frequency,
      LL_USART_OVERSAMPLING_16,
      baud);
    LL_USART_Enable(USART1);
}

void LSB_Encode(uint8_t byte, uint8_t *out_buffer) { // For 1-Wire protocol
    for (int i = 0; i < 8; i++) {
        if (byte & (1 << i)) {
            out_buffer[i] = 0xFF;
        } else {
            out_buffer[i] = 0x00;
        }
    }
}

uint8_t LSB_Decode(uint8_t *ow_rx_buffer) {
    uint8_t byte = 0x00;
    for (int i = 0; i < 8; i++) {
        byte>>=1;
        if (ow_rx_buffer[i] >= 0xFE) {
            byte|= 0x80;
        }
    }
    return byte;
}
