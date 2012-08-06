#include <avr/io.h>
#include "led.h"
// colorPacket returns a a6281Packet for setting color brightnesses
//
// red, green, and blue are brightness values from 0 to 1023. 0 is off, and
// 1023 is brightest.
a6281Packet led_createColor(unsigned int red, unsigned int green, unsigned int blue)
{
    //Make a packet and initialize all of the bits to zero.
    a6281Packet shiftbrite_packet = {value:0};

    shiftbrite_packet.red   = (red>1023) ? 1023 : red;
    shiftbrite_packet.green = (green>1023) ? 1023 : green;
    shiftbrite_packet.blue  = (blue>1023) ? 1023 : blue;

    return shiftbrite_packet;
}


// commandPacket returns a a6281Packet for sending commands to the A6281.
//
// redDotCorrect, greenDotCorrect, and blueDotCorrect lets you control what
//   percentage of current is flowing to each color diode.
//   Refer to page 8 of the datasheet for more information.
// clockMode lets you set the PWM frequency for the diodes.
//   Refer to page 7 of the datasheet for more information.
a6281Packet led_createPacket(unsigned int redDotCorrect, unsigned int greenDotCorrect,
                               unsigned int blueDotCorrect, unsigned char clockMode)
{
    //Make a packet and initialize all of the bits to zero.
    a6281Packet shiftbrite_packet = {value:0};

    shiftbrite_packet.redDotCorrect   = redDotCorrect;
    shiftbrite_packet.greenDotCorrect = greenDotCorrect;
    shiftbrite_packet.blueDotCorrect  = blueDotCorrect;
    shiftbrite_packet.clockMode = clockMode;
    shiftbrite_packet.command = 1;

    return shiftbrite_packet;
}

void led_send(a6281Packet shiftbrite_packet)
{
    int i;
    for(i = 1; i < 32 + 1; i++)
    {
        //Set the appropriate Data In value according to the packet.
        if ((shiftbrite_packet.value >> (32 - i)) & 1)
            LED_PORT |= (1 << A6281_DATA_PIN);
        else
            LED_PORT &= ~(1 << A6281_DATA_PIN);

        //Toggle the clock bit twice.
        LED_PORT ^= (1 << A6281_CLOCK_PIN);
        LED_PORT ^= (1 << A6281_CLOCK_PIN);
    }
}

void led_latch()
{
    // Set Latch high
    LED_PORT |= (1 << A6281_LATCH_PIN);
    // Set Latch low
    LED_PORT &= ~(1 << A6281_LATCH_PIN);
}

void led_setup() {
  // Set all to outputs.
  LED_OUTPUT_REGISTER |= (1 << A6281_DATA_PIN); // Data In
  LED_OUTPUT_REGISTER |= (1 << A6281_LATCH_PIN); // Latch
  LED_OUTPUT_REGISTER |= (1 << A6281_ENABLE_PIN); // Enable
  LED_OUTPUT_REGISTER |= (1 << A6281_CLOCK_PIN); // Clock

    // Set the Enable output low to enable the ShiftBrites.
  LED_PORT &= ~(1 << A6281_ENABLE_PIN);
}