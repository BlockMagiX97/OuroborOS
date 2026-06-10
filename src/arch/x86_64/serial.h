#include <stdint.h>
#include <libk/typedef.h>
#include <stdbool.h>

#define PORT_COM1 0x3f8

err_t init_serial(uint16_t port);
bool serial_received(uint16_t port);
char read_serial(uint16_t port);
bool is_transmit_empty(uint16_t port);
void write_serial(uint16_t port, uint8_t a);
