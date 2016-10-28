/**
 * Copyright 2013 opencxa.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cxa_assert.h"


/**
 * @author Christopher Armenio
 */


// ******** includes ********
#include <stdlib.h>
#include <string.h>
#include <cxa_delay.h>
#include <cxa_numberUtils.h>
#include <cxa_stringUtils.h>


// ******** local macro definitions ********
#define EXIT_STATUS					84
#define GPIO_FLASH_DELAY_MS			100
#define ASSERT_TEXT					"**assert**"
#define PREAMBLE_LOCATION			"loc: "
#define PREAMBLE_MESSAGE			"msg: "


// ******** local type definitions ********


// ******** local function prototypes ********


// ********  local variable declarations *********
static cxa_ioStream_t* ioStream = NULL;
static cxa_assert_cb_t cb = NULL;
#ifdef CXA_ASSERT_GPIO_FLASH_ENABLE
	static cxa_gpio_t *gpio;
#endif


// ******** global function implementations ********
void cxa_assert_setIoStream(cxa_ioStream_t *const ioStreamIn)
{
	ioStream = ioStreamIn;
}


void cxa_assert_setAssertCb(cxa_assert_cb_t cbIn)
{
	cb = cbIn;
}


#ifdef CXA_ASSERT_GPIO_FLASH_ENABLE
	void cxa_assert_setAssertGpio(cxa_gpio_t *const gpioIn)
	{
		gpio = gpioIn;
	}
#endif


void cxa_assert_impl(const char *msgIn, const char *fileIn, const long int lineIn)
{
	if( ioStream != NULL )
	{
		cxa_ioStream_writeLine(ioStream, "");
		cxa_ioStream_writeLine(ioStream, ASSERT_TEXT);

		if( fileIn != NULL )
		{
			// shorten our file name
			char *file_sep = strrchr(fileIn, '/');
			if(file_sep) fileIn = file_sep+1;
			else{
				file_sep = strrchr(fileIn, '\\');
				if (file_sep) fileIn = file_sep+1;
			}

			cxa_ioStream_writeString(ioStream, PREAMBLE_LOCATION);
			cxa_ioStream_writeString(ioStream, fileIn);
			cxa_ioStream_writeByte(ioStream, ':');
			cxa_ioStream_writeFormattedLine(ioStream, "%d", lineIn);

			if( msgIn != NULL ) cxa_ioStream_writeLine(ioStream, msgIn);
		}
	}

	if( cb != NULL ) cb();

	#ifdef CXA_ASSERT_GPIO_FLASH_ENABLE
		if( gpio != NULL )
		{
			cxa_gpio_setDirection(gpio, CXA_GPIO_DIR_OUTPUT);
			while(1)
			{
				cxa_gpio_toggle(gpio);
				cxa_delay_ms(GPIO_FLASH_DELAY_MS);
			}
		} else CXA_ASSERT_EXIT_FUNC(EXIT_STATUS);
	#else
		CXA_ASSERT_EXIT_FUNC(EXIT_STATUS);
	#endif
}


// ******** local function implementations ********
