
#define A6281_CLOCK_PIN PORTD0
#define A6281_ENABLE_PIN PORTD1
#define A6281_LATCH_PIN PORTD2
#define A6281_DATA_PIN PORTD3
#define LED_OUTPUT_REGISTER DDRD
#define LED_PORT PORTD


#include "tpad4-mega.h"
#include "led.h"
#include <stdlib.h>


uint8_t bootloader_force;

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t tpad_CDC_Interface =
  {
    .Config =
      {
        .ControlInterfaceNumber   = 0,
        .DataINEndpoint           =
          {
            .Address          = CDC_TX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .DataOUTEndpoint =
          {
            .Address          = CDC_RX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .NotificationEndpoint =
          {
            .Address          = CDC_NOTIFICATION_EPADDR,
            .Size             = CDC_NOTIFICATION_EPSIZE,
            .Banks            = 1,
          },
      },
  };

int ParserState[4];

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
static FILE USBSerialStream;
static bool connected = false, wasConnected = false;

int button = 0;
int buttonChannels[4] = { ADC_CHANNEL0, ADC_CHANNEL1, ADC_CHANNEL4, ADC_CHANNEL5 };
uint16_t results[4];
int buttonMapping[4] = { 2, 3, 1, 0 };
bool reading = false;

uint16_t colors[4][3] = {
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0}
};

void updateLeds(void) {
  for (int i = 0; i<4; i++) {
    led_send(
      led_createColor(
        colors[buttonMapping[i]][0],
        colors[buttonMapping[i]][1],
        colors[buttonMapping[i]][2]
      )
    );
  }

  led_latch();
}


int parserPos = 0, bufferPos;
char colorBuffer[8];
void parseChunk(uint16_t byte) {
  char current = (char)byte;

  if (current == '!') {
    bootloader_force = 0xbb;
    wdt_enable(WDTO_250MS);
    for (;;);
  } else if (current == ',' || current == '\n') {
    ParserState[parserPos] = atoi(colorBuffer);

    memset(colorBuffer, 0, sizeof(colorBuffer));
    bufferPos = 0;
    parserPos++;
  } else {
    colorBuffer[bufferPos] = current;
    bufferPos++;

    // avoid buffer overruns
    if (bufferPos > 7) {
      bufferPos = 0;
      memset(colorBuffer, 0, sizeof(colorBuffer));
      // bail on this parse
      parserPos = 0;
    }
  }

  if (parserPos > 3) {
    parserPos = 0;
    bufferPos = 0;

    colors[ParserState[0]][0] = ParserState[1];
    colors[ParserState[0]][1] = ParserState[2];
    colors[ParserState[0]][2] = ParserState[3];
    memset(ParserState, 0, sizeof(ParserState));
  }
}

/*

  Allow a reset into the (adafruit CDC) bootloader

  + CDC bootloader source https://github.com/adafruit/lufa-lib/blob/master/trunk/Bootloaders/CDC/BootloaderCDC.c#L139-142
  + source of this code https://groups.google.com/d/topic/lufa-support/-w1pP0K3Elk/discussion

*/
void __bootloader_test(void)
        __attribute__ ((naked))
        __attribute__ ((section (".init0")));
void __bootloader_test(void)
{
        __asm volatile ("    lds r24, bootloader_force\n"
                        "    cpi r24, 0xbb\n"
                        "    brne 1f\n"
                        "    ldi r25, 0x00\n"
                        "    sts bootloader_force, r25\n"
                        "    ret\n"
                        "1:\n"
                        : : : "memory");
}


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{

  SetupHardware();
  uint16_t bytesAvailable;

  CDC_Device_CreateStream(&tpad_CDC_Interface, &USBSerialStream);
  sei();

  for (;;)
  {

    //
    // Read pressures from 16 buttons
    //
    ButtonStates();

    //
    // Apply values to the buttons
    //
    updateLeds();

    if (connected && !wasConnected) {
      wasConnected = true;
      fputs("tpad\n", &USBSerialStream);
      fputs("{\"name\": \"tpad4-mega\", \"leds\":true, \"pads\" : 4, \"version\" : \"0.0.3\" }\n", &USBSerialStream);
    } else if (!connected && wasConnected) {

      // reset colors
      for (int i = 0; i<4; i++) {
        colors[i][0] = 0;
        colors[i][1] = 0;
        colors[i][2] = 0;
      }

      wasConnected = false;
      parserPos = 0;
    } else if ((bytesAvailable = CDC_Device_BytesReceived(&tpad_CDC_Interface))) {
      while (bytesAvailable--) {
        parseChunk(CDC_Device_ReceiveByte(&tpad_CDC_Interface));
      }
    }

    CDC_Device_USBTask(&tpad_CDC_Interface);
    USB_USBTask();
  }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);
  USB_Init();

  //
  // Setup sensor power pins
  //
  DDRB = 0;

  led_setup();

  // Configure ADC
  ADC_Init(ADC_PRESCALE_32 | ADC_FREE_RUNNING);

  ADC_SetupChannel(0);
  ADC_SetupChannel(1);
  ADC_SetupChannel(4);
  ADC_SetupChannel(5);

  PORTB = 0xFF;
}

void ButtonStates(void) {
  char buffer[64];

  uint16_t result;
  if (!reading) {
    ADC_StartReading(ADC_REFERENCE_AVCC | ADC_LEFT_ADJUSTED | buttonChannels[button]);
    reading = true;
  } else if (ADC_IsReadingComplete()) {
    result = ADC_GetResult();

    if (result < 150) {
      result = 0;
    }

    if (results[button] != result) {


      if (!connected) {
        if (result < 1000) {
          colors[button][0] = 0;
          colors[button][1] = 0;
          colors[button][2] = 0;
        } else if (result < 8000) {
          colors[button][0] = 1023;
          colors[button][1] = 0;
          colors[button][2] = 0;
        } else if (result < 13000) {
          colors[button][0] = 0;
          colors[button][1] = 1023;
          colors[button][2] = 0;
        } else {
          colors[button][0] = 0;
          colors[button][1] = 0;
          colors[button][2] = 1023;
        }
      // ignore the bounces at the bottom of the pressure range
      } else {
        sprintf(buffer, "%d,%u\n", button, result);
        fputs(buffer, &USBSerialStream);
      }

      results[button] = result;
    }

    button++;
    if (button > 3) {
      button = 0;
    }
    reading = false;
  }
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
  bool ConfigSuccess = true;
  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&tpad_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) {
  CDC_Device_ProcessControlRequest(&tpad_CDC_Interface);
}

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) {
  connected = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR);
}

