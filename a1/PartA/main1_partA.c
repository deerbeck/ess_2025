#include <xmc_common.h>
#include <xmc_gpio.h>
#include <xmc_ccu4.h>
#include <xmc_scu.h>
#include <string.h>
#include <ctype.h>

// function prototypes
void SysTick_Handler(void);
uint32_t get_elapsed_time_ms(void);
void delay_ms(uint32_t delay);
void sendMorseCode(const char *message);

// macros
// TIMER
#define TICKS_PER_SECOND 1000 // 1 second = 1000 ms

// MORSE
#define DOT_DURATION 100 // 100 ms
#define DASH_DURATION (3 * DOT_DURATION)
#define INTER_ELEMENT_SPACE DOT_DURATION
#define INTER_CHARACTER_SPACE (3 * DOT_DURATION)
#define INTER_WORD_SPACE (7 * DOT_DURATION)

// PINS
#define LED1_PORT XMC_GPIO_PORT1
#define LED1_PIN 1
#define LED2_PORT XMC_GPIO_PORT1
#define LED2_PIN 0

volatile uint32_t ms_counter = 0; // Global counter for milliseconds

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

    XMC_GPIO_Init(XMC_GPIO_PORT1, LED1_PIN, &LED_config);

    while (1)
    {
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
        delay_ms(5000);
    }
}

int main(void)
{
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND); // Configure SysTick to tick every 100 ms
    sendMorseCode("I CAN MORSE");
    return 0;
}