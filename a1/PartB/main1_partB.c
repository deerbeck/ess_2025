#include <xmc_common.h>
#include <xmc_gpio.h>
#include <xmc_ccu4.h>
#include <xmc_scu.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// function prototypes
void SysTick_Handler(void);
void sendMorseCode(const char *message);
uint32_t get_elapsed_time_ms(void);
void delay_ms(uint32_t delay);

// macros
// TIMER
#define TICKS_PER_SECOND 1000 // 1 second = 1000 ms

// MORSE
#define DOT_DURATION 100 // 100 ms
#define DASH_DURATION (3 * DOT_DURATION)
#define INTER_ELEMENT_SPACE DOT_DURATION
#define INTER_CHARACTER_SPACE (3 * DOT_DURATION)
#define INTER_WORD_SPACE (7 * DOT_DURATION)

// BUTTONS
#define BUTTON1_PORT XMC_GPIO_PORT1
#define BUTTON1_PIN 14
#define BUTTON2_PORT XMC_GPIO_PORT1
#define BUTTON2_PIN 15
#define LED1_PORT XMC_GPIO_PORT1
#define LED1_PIN 1
#define LED2_PORT XMC_GPIO_PORT1
#define LED2_PIN 0

// Global variables
volatile uint32_t ms_counter = 0;                 // Global counter for milliseconds
volatile uint32_t button1_timestamps[2] = {0, 0}; // Store last two Button1 press timestamps
volatile uint8_t button1_press_count = 0;         // Count Button1 presses
volatile uint8_t is_transmitting = 0;             // Flag to indicate Morse code transmission
volatile uint8_t button_lock = 0;                 // Mutex-like lock for button handling

// function called every 1 ms
void SysTick_Handler(void)
{
    ms_counter++;
}

uint32_t get_elapsed_time_ms()
{
    return ms_counter; // Return the current value of the millisecond counter
}

void delay_ms(uint32_t delay)
{
    uint32_t start_time = get_elapsed_time_ms();
    while ((get_elapsed_time_ms() - start_time) < delay)
    {
        // Wait for the specified delay
    }
}

void sendMorseCode(const char *message)
{
    const char *morseCode[] = {
        ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
        "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};

    const XMC_GPIO_CONFIG_t LED_config = {
        .mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
        .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
        .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE};

    XMC_GPIO_Init(LED1_PORT, LED1_PIN, &LED_config);


    for (int i = 0; i < strlen(message); i++)
    {
        if (message[i] == ' ')
        {
            delay_ms(INTER_WORD_SPACE);
        }
        else
        {
            char c = toupper(message[i]);
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
            {
                const char *morse = (c >= 'A' && c <= 'Z') ? morseCode[c - 'A'] : morseCode[c - '0' + 26];
                for (int j = 0; j < strlen(morse); j++)
                {
                    if (morse[j] == '.')
                    {
                        XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, LED1_PIN);
                        delay_ms(DOT_DURATION);
                        XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, LED1_PIN);
                    }
                    else if (morse[j] == '-')
                    {
                        XMC_GPIO_SetOutputHigh(XMC_GPIO_PORT1, LED1_PIN);
                        delay_ms(DASH_DURATION);
                        XMC_GPIO_SetOutputLow(XMC_GPIO_PORT1, LED1_PIN);
                    }
                    // only seperate elements, when not last element of character
                    if (j < strlen(morse) - 1)
                    {
                        delay_ms(INTER_ELEMENT_SPACE);
                    }
                }
                // only separate characters, when not last character of message
                if (i < strlen(message) - 1 && message[i + 1] != ' ')
                {
                    delay_ms(INTER_CHARACTER_SPACE);
                };
            }
        }
    }
}

int main(void)
{
    // init timer
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

    // Configure buttons
    XMC_GPIO_CONFIG_t button_config = {
        .mode = XMC_GPIO_MODE_INPUT_TRISTATE,
        .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
        .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE};

    XMC_GPIO_Init(BUTTON1_PORT, BUTTON1_PIN, &button_config);
    XMC_GPIO_Init(BUTTON2_PORT, BUTTON2_PIN, &button_config);

    // Assume buttons are not pressed initially
    uint8_t button1_prev_state = 1;
    uint8_t button2_prev_state = 1;

    while (1)
    {
        // Poll Button1
        uint8_t button1_current_state = XMC_GPIO_GetInput(BUTTON1_PORT, BUTTON1_PIN);
        if (button1_prev_state == 1 && button1_current_state == 0) // Falling edge
        {
            uint32_t current_time = get_elapsed_time_ms();
            button1_timestamps[1] = button1_timestamps[0];
            button1_timestamps[0] = current_time;
            button1_press_count++;

            if (!is_transmitting)
            {
                is_transmitting = 1;
                sendMorseCode("I CAN MORSE");
                is_transmitting = 0;
            }
        }
        button1_prev_state = button1_current_state;

        // Button 2
        uint8_t button2_current_state = XMC_GPIO_GetInput(BUTTON2_PORT, BUTTON2_PIN);
        if (button2_prev_state == 1 && button2_current_state == 0) // Falling edge
        {
            uint32_t time_to_send = 0;
            if (button1_press_count == 0)
            {
                time_to_send = 0;
            }
            else if (button1_press_count == 1)
            {
                time_to_send = button1_timestamps[0];
            }
            else
            {
                time_to_send = button1_timestamps[0] - button1_timestamps[1];
            }

            char time_str[12];
            sprintf(time_str, "%lu", (time_to_send));

            if (!is_transmitting)
            {
                is_transmitting = 1;
                sendMorseCode(time_str);
                is_transmitting = 0;
            }
        }
        button2_prev_state = button2_current_state;
    }

    return 0;
}