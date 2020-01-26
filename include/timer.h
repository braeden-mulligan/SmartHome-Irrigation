volatile bool timer8_flag;
volatile bool timer16_flag;

void timer8_init(uint16_t period_ms, void (* volatile custom_routine)(void));
void timer8_start();
void timer8_stop();

void timer16_init(uint16_t period_s, void (* volatile custom_routine)(void));
void timer16_start();
void timer16_stop();
