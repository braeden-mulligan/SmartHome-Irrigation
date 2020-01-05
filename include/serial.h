/*
Author: Braeden Mulligan
        braeden.mulligan@gmail.com
*/

// Set this flag to true to use interrupt-driven serial transmissions.
// Otherwise compile with a synchronous, busy-wait blocking design.
#define TX_INTERRUPT true 
//TODO: timouts for non-interrupt transmits
//TODO interrupt driven vs polling/timeouts for receive

#define BAUD 9600
#define BAUD_PRESCALE (F_CPU / 16 / BAUD - 1)

#define TX_BUFFER_SIZE 64
#define RX_BUFFER_SIZE 64

void UART_init(bool tx, bool rx);

void tx_buffer_erase();

void serial_putc(char byte);

// Call this function as wrapper for writing serial data.
// Checks long strings for buffer overflow. 
void serial_puts(char* text);

void serial_print();

char serial_getc(bool peek);

// Read exactly n bytes from the RX buffer, add a null terminator and then
//   write into read_buffer. If n < 1, read the whole buffer.
// <read_string> should be of length at least n + 1.
void serial_gets(char* read_string, short n); 

uint8_t rx_available();
