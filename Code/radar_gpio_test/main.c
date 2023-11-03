/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cy_pdl.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/* Modifiable Macros*/
//#define PRINT_DEBUG_MSG

/* Constant defined Macros */
#define GPIO_INTERRUPT_PRIORITY          7u


/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_gpio_callback_data_t gpio_btn_callback_data;

/* Variable for storing character read from terminal */
uint8_t uart_read_value;

/* Define GPIO to be read */
cyhal_gpio_t RADAR_TD = P12_2; //D13
cyhal_gpio_t RADAR_PD = P12_1; //D12

/* Timing Variables */
uint32_t ticks_ms = 0;
bool ticks_overflowed = false;


/* Data Recording state*/
bool recording_data = false;


/*******************************************************************************
* Function Definitions
*******************************************************************************/
void handle_error(uint32_t status);

static uint32_t get_ticks_ms(void);
static void reset_ticks_ms(void);


/* Interrupts. Should not be used in the main code */
static void systick_isr(void);
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);



int main(void)
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif /* #if defined (CY_DEVICE_SECURE) */

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    handle_error(result);


    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                        CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,
                                        CY_RETARGET_IO_BAUDRATE);
    handle_error(result);

#ifdef PRINT_DEBUG_MSG
	printf("\x1b[2J\x1b[;H"); //Clear UART Screen
#endif /* PRINT_DEBUG_MSG */

	/* Initialise Timing */
	Cy_SysTick_Init (CY_SYSTICK_CLOCK_SOURCE_CLK_IMO ,(8000000/1000)-1);    // IMO freq/1000 = # of SysTick counts
	Cy_SysTick_SetCallback(0, systick_isr);



    /* Initialize the user button */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                    CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    handle_error(result);

    /* Configure GPIO interrupt for SW2 - used this to start and end the data collection*/
    gpio_btn_callback_data.callback = gpio_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN,
                                 &gpio_btn_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
                                 GPIO_INTERRUPT_PRIORITY, true);

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the Input Radar Pins */
    result = cyhal_gpio_init(RADAR_TD, CYHAL_GPIO_DIR_INPUT,
    						 CYHAL_GPIO_DRIVE_NONE, CYBSP_LED_STATE_OFF);
    result = cyhal_gpio_init(RADAR_PD, CYHAL_GPIO_DIR_INPUT,
    						 CYHAL_GPIO_DRIVE_NONE, CYBSP_LED_STATE_OFF);
    handle_error(result);

    /* Initialize the LED to inform user data recording */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    handle_error(result);


#ifdef PRINT_DEBUG_MSG
    printf("All good \r\n");
    printf("Press SW2 to start recording work timing data \r\n");
#endif /* PRINT_DEBUG_MSG */


    for (;;)
    {
    	while(recording_data == false) cyhal_system_delay_ms(10);

#ifdef PRINT_DEBUG_MSG
    	printf("START \r\n");
#endif /* PRINT_DEBUG_MSG */

    	printf("9 \r\n");
    	cyhal_gpio_write(CYBSP_USER_LED, 1);

    	bool TD_pin_result;
    	bool PD_pin_result;

    	reset_ticks_ms();
    	while(recording_data == true){
    		if(get_ticks_ms() > 1000){

    			reset_ticks_ms();

    			TD_pin_result = cyhal_gpio_read(RADAR_TD);
    			PD_pin_result = cyhal_gpio_read(RADAR_PD);

    			//cyhal_gpio_toggle(CYBSP_USER_LED);

#ifdef PRINT_DEBUG_MSG
    			printf("TD:%i PD:%i \r\n",TD_pin_result,PD_pin_result);
#else
    			printf("%i %i \r\n",TD_pin_result,PD_pin_result);
#endif /* PRINT_DEBUG_MSG */

    		}
    	}

    	cyhal_gpio_write(CYBSP_USER_LED, 0);
    	printf("9 \r\n");

#ifdef PRINT_DEBUG_MSG
    	printf("END \r\n");
#endif /* PRINT_DEBUG_MSG */

    	recording_data   = false;

    }

}


/**
 * @brief User defined error handling function
 *
 * @param Status status indicates success or failure
 */
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
    	printf("Error in init");
        CY_ASSERT(0);
    }
}

/**
 *  @brief GPIO interrupt handler for SW2
 */
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    recording_data = !recording_data;
}




/**
 * @brief ISR that increments global variable ticks_ms every 1ms,
 *        Also raises ticks_overflowed flag if ticks_ms has been reset to zero
 *        after the increment
 *
 */
static void systick_isr(void)
{
	ticks_ms++;
	if(ticks_ms == 0){
		ticks_overflowed = true;
	}
}

/**
 * @brief Get the number of milliseconds since ticks_ms was reset to zero
 *
 * @retval milliseconds [uint32_t]
 */
static uint32_t get_ticks_ms(void)
{
	return ticks_ms;
}

/**
 * @brief Reset global variable ticks_ms back to zero
 *
 */
static void reset_ticks_ms(void)
{
	ticks_ms = 0;
}


