#include <arch/curr/serial.h>
#include <arch/curr/spinlock.h>
#include <libk/typedef.h>

err_t init_debug_output();
// dont use much
void send_debug_output_char(char ch);
// prevent fragmenting msg when multiprocessing (locks before msg and unlocks after)
void send_debug_output(const char *str);



// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void lock_debug_output(flags_t *flags);
// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void send_debug_output_no_lock(const char *str);
// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void unlock_debug_output(flags_t *flags);

