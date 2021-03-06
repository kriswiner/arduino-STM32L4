/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include <stdio.h>

#include "stm32l4xx.h"

#include "armv7m.h"

#include "stm32l4_gpio.h"
#include "stm32l4_servo.h"
#include "stm32l4_system.h"

static void stm32l4_servo_event_callback(void *context, uint32_t events)
{
    stm32l4_servo_t *servo = (stm32l4_servo_t*)context;
    stm32l4_servo_schedule_t *active, *pending;
    unsigned int index;

    if (events & TIMER_EVENT_PERIOD)
    {
	pending = servo->pending;
	servo->pending = NULL;

	if (pending)
	{
	    servo->active = pending;
	}

	active = servo->active;

	if (active->entries != 0)
	{
	    if (servo->period == active->period)
	    {
		active->slot[0].GPIO->BSRR = active->slot[0].mask;
	    
		stm32l4_timer_compare(&servo->timer, TIMER_CHANNEL_1, active->slot[0].offset);
	    }
	    else
	    {
		servo->period = active->period;

		stm32l4_timer_stop(&servo->timer);
		stm32l4_timer_configure(&servo->timer, servo->prescaler -1, servo->period -1, 0);
		stm32l4_timer_channel(&servo->timer, TIMER_CHANNEL_1, active->slot[0].offset, TIMER_CONTROL_COMPARE_TIMING);
		stm32l4_timer_start(&servo->timer, false);

		active->slot[0].GPIO->BSRR = active->slot[0].mask;

		servo->state = SERVO_STATE_ACTIVE;
	    }
	}
	else
	{
	    stm32l4_timer_stop(&servo->timer);

	    servo->active = NULL;

	    servo->state = SERVO_STATE_READY;
	}

	servo->index = 0;

	if (pending && (servo->events & SERVO_EVENT_UPDATE))
	{
	    (*servo->callback)(servo->context, SERVO_EVENT_UPDATE);
	}
    }
    else
    {
	index = servo->index;
	active = servo->active;

	if (index != active->entries)
	{
	    active->slot[index].GPIO->BRR = active->slot[index].mask;
	    
	    index++;

	    if (index != active->entries)
	    {
		active->slot[index].GPIO->BSRR = active->slot[index].mask;

		stm32l4_timer_compare(&servo->timer, TIMER_CHANNEL_1, active->slot[index].offset);
	    }
	    else
	    {
		if (active->offset)
		{
		    stm32l4_timer_compare(&servo->timer, TIMER_CHANNEL_1, active->offset);
		}
		else
		{
		    if (servo->events & SERVO_EVENT_SYNC)
		    {
			(*servo->callback)(servo->context, SERVO_EVENT_SYNC);
		    }
		}
	    }

	    servo->index = index;
	}
	else
	{
	    if (servo->events & SERVO_EVENT_SYNC)
	    {
		(*servo->callback)(servo->context, SERVO_EVENT_SYNC);
	    }
	}
    }
}

bool stm32l4_servo_create(stm32l4_servo_t *servo, unsigned int instance, unsigned int priority)
{
    if (!stm32l4_timer_create(&servo->timer, instance, priority, 0))
    {
	return false;
    }

    servo->state = SERVO_STATE_INIT;
    servo->callback = NULL;
    servo->context = NULL;
    servo->events = 0;

    servo->index = 0;
    servo->prescaler = 0;
    servo->period = 0;
    servo->active = NULL;
    servo->pending = NULL;

    return true;
}

bool stm32l4_servo_destroy(stm32l4_servo_t *servo)
{
    if (servo->state != SERVO_STATE_INIT)
    {
	return false;
    }

    servo->state = SERVO_STATE_NONE;

    return true;
}

bool stm32l4_servo_enable(stm32l4_servo_t *servo, const stm32l4_servo_table_t *table, stm32l4_servo_callback_t callback, void *context, uint32_t events)
{
    if (servo->state != SERVO_STATE_INIT)
    {
	return false;
    }

    servo->state = SERVO_STATE_BUSY;

    if (!stm32l4_timer_enable(&servo->timer, 0, 0, 0, stm32l4_servo_event_callback, servo, (TIMER_EVENT_PERIOD | TIMER_EVENT_CHANNEL_1)))
    {
	servo->state = SERVO_STATE_INIT;

	return false;
    }

    if (!stm32l4_servo_configure(servo, table))
    {
	stm32l4_timer_disable(&servo->timer);

	servo->state = SERVO_STATE_INIT;

	return false;
    }

    stm32l4_servo_notify(servo, callback, context, events);

    return true;
}

bool stm32l4_servo_disable(stm32l4_servo_t *servo)
{
    if (servo->state != SERVO_STATE_READY)
    {
	return false;
    }

    stm32l4_timer_disable(&servo->timer);

    servo->events = 0;
    servo->callback = NULL;
    servo->context = NULL;

    servo->state = SERVO_STATE_INIT;

    return true;
}

bool stm32l4_servo_configure(stm32l4_servo_t *servo, const stm32l4_servo_table_t *table)
{
    stm32l4_servo_schedule_t *pending;
    unsigned int entry, index, offset;

    if (servo->state < SERVO_STATE_BUSY)
    {
	return false;
    }

    servo->pending = NULL;

    pending = ((servo->active == &servo->schedule[1]) ? &servo->schedule[0] : &servo->schedule[1]);

    for (offset = 0, entry = 0, index = 0; entry < table->entries; entry++)
    {
	if ((table->slot[entry].pin != GPIO_PIN_NONE) && (table->slot[index].width >= SERVO_PULSE_THRESHOLD))
	{
	    offset += table->slot[entry].width;

	    pending->slot[index].GPIO = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * ((table->slot[entry].pin & GPIO_PIN_GROUP_MASK) >> GPIO_PIN_GROUP_SHIFT));
	    pending->slot[index].mask = (1ul << ((table->slot[entry].pin & GPIO_PIN_INDEX_MASK) >> GPIO_PIN_INDEX_SHIFT));
	    pending->slot[index].offset = offset;

	    index++;
	}
    }

    if (offset == 0)
    {
	pending->period = 0;
	pending->offset = 0;
	pending->entries = 0;
    }
    else
    {
	pending->entries = index;

	servo->prescaler = stm32l4_timer_clock(&servo->timer) / 1000000;

	if ((offset + SERVO_SYNC_MARGIN + SERVO_SYNC_WIDTH) >= SERVO_FRAME_WIDTH)
	{
	    pending->period = (offset + SERVO_SYNC_MARGIN + SERVO_SYNC_WIDTH);
	    pending->offset = SERVO_SYNC_MARGIN;
	}
	else
	{
	    pending->period = SERVO_FRAME_WIDTH;
	    pending->offset = SERVO_FRAME_WIDTH - SERVO_SYNC_WIDTH;
	}
    }

    if (servo->active)
    {
	servo->pending = pending;
    }
    else
    {
	if (pending->period)
	{
	    servo->active = pending;

	    servo->period = pending->period;

	    stm32l4_timer_stop(&servo->timer);
	    stm32l4_timer_configure(&servo->timer, servo->prescaler -1, servo->period -1, 0);
	    stm32l4_timer_channel(&servo->timer, TIMER_CHANNEL_1, pending->slot[0].offset, TIMER_CONTROL_COMPARE_TIMING);
	    stm32l4_timer_start(&servo->timer, false);
	    
	    pending->slot[0].GPIO->BSRR = pending->slot[0].mask;
		
	    servo->state = SERVO_STATE_ACTIVE;
	}
    }

    return true;
}

bool stm32l4_servo_notify(stm32l4_servo_t *servo, stm32l4_servo_callback_t callback, void *context, uint32_t events)
{
    if (servo->state < SERVO_STATE_BUSY)
    {
	return false;
    }

    servo->events = 0;
    servo->callback = callback;
    servo->context = context;
    servo->events = events;

    return true;
}

bool stm32l4_servo_active(stm32l4_servo_t *servo)
{
    return (servo->state == SERVO_STATE_ACTIVE);
}
