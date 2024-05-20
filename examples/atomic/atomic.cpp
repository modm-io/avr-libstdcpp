/*
 * Copyright (c) 2022, Christopher Kormanyos
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <atomic>
#include <cstdint>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

static void app_hw_init();

std::atomic<std::uint16_t> sequence;

int main()
{
	// Initialize the application hardware. This includes WDT, PORTB.5 and TIMER0.
	app_hw_init();

	for(;;)
	{
		// Toggle the LED on portb.5.
		PINB = (1U << PORTB5);

		sequence++;
	}
}

static void app_hw_init()
{
	// Initialize the application including WDT, PORTB.5 and TIMER0

	// We will now disable the watchdog.
	// Service the watchdog just to be sure to avoid pending timeout.
	wdt_reset();

	// Clear WDRF in MCUSR.
	MCUSR &= ~(1U << WDRF);

	// Write logical one to WDCE and WDE.
	// Keep the old prescaler setting to prevent unintentional time-out.
	WDTCSR |= (1U << WDCE) | (1U << WDE);

	// Turn off the WDT.
	WDTCSR = 0x00;

	// We will now initialize PORTB.5 to be used as an LED driver port.
	// Set PORTB.5 value to low.
	PORTB &= ~(1U << PORTB5);

	// Set PORTB.5 direction to output.
	DDRB |= (1U << DDB5);

	// We will now initialize the TIMER0 clock and interrupt.
	// Clear the TIMER0 overflow flag.
	TIFR0 = static_cast<std::uint8_t>(1U << TOV0);

	// Enable the TIMER0 overflow interrupt.
	TIMSK0 = static_cast<std::uint8_t>(1U << TOIE0);

	// Set the TIMER0 clock source to f_osc/8 = 2MHz and begin counting.
	TCCR0B = static_cast<std::uint8_t>(1U << CS01);

	// Enable all interrupts.
	sei();
}



ISR(TIMER0_OVF_vect)
{
	sequence++;
}
