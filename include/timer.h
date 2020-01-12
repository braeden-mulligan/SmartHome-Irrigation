volatile bool timer8_flag;
volatile bool timer16_flag;

void timer8_init(short period_ms, void (* volatile custom_routine)(void));

void timer8_start();
void timer8_stop();
