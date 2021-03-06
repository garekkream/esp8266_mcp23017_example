
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "stdout/stdout.h"
#include "c_types.h"
#include "mcp23017/mcp23017.h"
#include "mcp23017_basictest.h"

#define user_procTaskPeriod      1000
static volatile os_timer_t loop_timer;
static uint8_t deviceAddr = 0;
static MCP23017_Self mcpSelf;

static void nop_procTask(os_event_t *events);
static void loop(void *timer_arg);
static void setup(void *timer_arg);

/**
 * This is the main user program loop
 */
static void ICACHE_FLASH_ATTR
loop(void *timer_arg) {
  static uint8_t i = 0;
  bool rv = false;

  mcp23017_pinModeAB(&mcpSelf, deviceAddr, MCP23017_INPUT);
  mcp23017_pinModeAB(&mcpSelf, deviceAddr, MCP23017_OUTPUT);
  os_printf("mcp23017_pinMode(0, MCP23017_OUTPUT);\n");
  os_delay_us(10000);

  rv = mcp23017_digitalWrite(&mcpSelf, deviceAddr, 0, i&1);
  //rv = mcp23017_digitalWriteA(&mcpSelf, deviceAddr, i&1);
  os_printf("mcp23017_digitalWrite(deviceAddr, 0, %d)=%s\n", i&1, rv?"ACK":"NACK");
  bool sample = false;
  rv = mcp23017_digitalRead(&mcpSelf, deviceAddr, 0, &sample);
  //rv = mcp23017_digitalReadA(&mcpSelf, deviceAddr, &sample);
  os_printf("mcp23017_digitalRead(deviceAddr, 0)=%s sample=%d;\n\n",  rv?"ACK":"NACK", sample);
  os_delay_us(10000);
  i += 1;
}

/**
 * Setup program. When user_init runs the debug printouts will not always
 * show on the serial console. So i run the inits in here, 2 seconds later.
 */
static void ICACHE_FLASH_ATTR
setup(void *timer_arg) {
  // setup stuff
  i2c_master_gpio_init();
  i2c_master_init();
  os_printf("i2c_master_gpio_init: I2C_MASTER_SDA_GPIO=GPIO%d, I2C_MASTER_SCL_GPIO=GPIO%d, I2C_MASTER_HALF_CYCLE=%d\n", I2C_MASTER_SDA_GPIO, I2C_MASTER_SCL_GPIO, I2C_MASTER_HALF_CYCLE);

  mcp23017_init(&mcpSelf);

  // run some tests
  mcp23017_basictest(&mcpSelf);

  // Start loop timer
  os_timer_disarm(&loop_timer);
  os_timer_setfn(&loop_timer, (os_timer_func_t *) loop, NULL);
  os_timer_arm(&loop_timer, 1000, 1);
}

void user_init(void) {

  // Make uart0 work with just the TX pin. Baud:115200,n,8,1
  // The RX pin is now free for GPIO use.
  stdout_init();

  // turn off WiFi for this console only demo
  wifi_station_set_auto_connect(false);
  wifi_station_disconnect();

  // Run setup() 3 seconds from now
  os_timer_disarm(&loop_timer);
  os_timer_setfn(&loop_timer, (os_timer_func_t *) setup, NULL);
  os_timer_arm(&loop_timer, 3000, false);

  os_printf("\nSystem started ...\n");
}
