#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "main.h"

void UART_Transmit(USART_TypeDef *port, uint8_t *data, uint32_t len);
void UART1_SetBaudRate(uint32_t baud);
void UART1_Transmit_DMA(uint8_t *buffer, uint16_t size);
void UART1_Receive_DMA(uint8_t *buffer, uint32_t size);
void LSB_Serialize(uint8_t byte, uint8_t *out_buffer);
uint8_t LSB_Decode(uint8_t *ow_rx_buffer);

#endif