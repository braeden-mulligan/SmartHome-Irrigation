/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#define BAUD 9600
#define BAUD_EQ (F_CPU / 16 / BAUD - 1)
#define UART_BUFFER_SIZE 128

void UART_init();

void tx_buffer_erase();

void UART_write();

void serial_put(char* text);
