/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#define BAUD 9600
#define BAUD_EQ (F_CPU / 16 / BAUD - 1)
#define UART_BUFFER_SIZE 32

void UART_init();

//bool tx_buffer_append(char c);

//void UART_transmit(char byte);

//void UART_write();

// Call this function as wrapper for sending serial data.
// Checks long strings for buffer overflow. 
void serial_write(char* text);
