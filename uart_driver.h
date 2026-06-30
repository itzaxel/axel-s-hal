#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "main.h"

extern volatile uint8_t UART1_DMA_RX_State;
extern volatile uint8_t UART1_DMA_TX_State;

typedef enum {
    UART_OK = 0,
    UART_TIMEOUT,
    BUF_EMPTY,
    UART_ERROR
} UART_Condition;

UART_Condition UART_Transmit(USART_TypeDef *port, uint8_t *data, uint32_t len);
UART_Condition UART_Receive(USART_TypeDef *port, uint8_t *data, uint32_t len);
UART_Condition UART_SetBaudRate(uint32_t baud, USART_TypeDef *port);
void UART1_Transmit_DMA(uint8_t *buffer, uint16_t size);
void UART1_Receive_DMA(uint8_t *buffer, uint32_t size);
void LSB_Encode(uint8_t byte, uint8_t *out_buffer);
uint8_t LSB_Decode(const uint8_t *ow_rx_buffer);

#endif