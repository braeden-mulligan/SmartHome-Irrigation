/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#define BAUD 9600
#define BAUD_EQ (F_CPU / 16 / BAUD - 1)
#define UART_BUFFER_SIZE 128

void UART_init();

//bool tx_buffer_append(char c);

void tx_buffer_erase();

//void UART_transmit(char byte);

// Flush tx_buffer.
void UART_write();

// Call this function as wrapper for sending serial data.
// Checks long strings for buffer overflow. 
void serial_put(char* text);
