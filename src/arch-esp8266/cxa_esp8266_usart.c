/**
 * Original Source:
 * https://github.com/esp8266/Arduino.git
 *
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
 *
 * @author Christopher Armenio
 */
#include "cxa_esp8266_usart.h"


// ******** includes ********
#include <stdbool.h>
#include <ets_sys.h>
#include <esp8266_peri.h>
#include <cxa_assert.h>


// ******** local macro definitions ********
#define USART_HW_BUFFER_LEN_BYTES		4			// 0-127


// ******** local type definitions ********


// ******** local function prototypes ********
static cxa_ioStream_readStatus_t ioStream_cb_readByte(uint8_t *const byteOut, void *const userVarIn);
static bool ioStream_cb_writeBytes(void* buffIn, size_t bufferSize_bytesIn, void *const userVarIn);


// ********  local variable declarations *********


// ******** global function implementations ********
void cxa_esp8266_usart_init_noHH(cxa_esp8266_usart_t *const usartIn, cxa_esp8266_usartId_t idIn, const uint32_t baudRate_bpsIn)
{
	cxa_assert(usartIn);
	cxa_assert(idIn == CXA_ESP8266_USART_0);

	// save our references
	usartIn->id = idIn;
	
	// enable RX pin
    GPC(3) = (GPC(3) & (0xF << GPCI)); 	//SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
    GPEC = (1 << 3);					//Disable
    GPF(3) = GPFFS(GPFFS_BUS(3));		//Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
    GPF(3) |= (1 << GPFPU);				//enable pullup on RX

    // enable TX pin
    GPC(1) = (GPC(1) & (0xF << GPCI)); 	//SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
	GPEC = (1 << 1);					//Disable
	GPF(1) = GPFFS(GPFFS_BUS(1));		//Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
	GPF(1) |= (1 << GPFPU);				//enable pullup on RX

	// IOSWAP?
	//IOSWAP &= ~(1 << IOSWAPU0);

	// set the baud rate and 8N1
	USD(usartIn->id) = (ESP8266_CLOCK / baudRate_bpsIn);
    USC0(usartIn->id) = 0x1c;

    // flush the usart
    uint32_t tmp = (1 << UCRXRST) | (1 << UCTXRST);
    USC0(usartIn->id) |= (tmp);
    USC0(usartIn->id) &= ~(tmp);

    // don't need interrupts, full/empty thresholds, or timeouts
    USC1(usartIn->id) = 0;

	// setup our ioStream (last once everything is setup)
	cxa_ioStream_init(&usartIn->super.ioStream);
	cxa_ioStream_bind(&usartIn->super.ioStream, ioStream_cb_readByte, ioStream_cb_writeBytes, (void*)usartIn);
}


// ******** local function implementations ********
static cxa_ioStream_readStatus_t ioStream_cb_readByte(uint8_t *const byteOut, void *const userVarIn)
{
	cxa_esp8266_usart_t* usartIn = (cxa_esp8266_usart_t*)userVarIn;
	cxa_assert(usartIn);

	size_t rxFifoLen_bytes = USS(usartIn->id) & 0xFF;
	if( rxFifoLen_bytes == 0 ) return CXA_IOSTREAM_READSTAT_NODATA;

	uint8_t readVal = USF(usartIn->id);
	if( byteOut != NULL ) *byteOut = readVal;
	return CXA_IOSTREAM_READSTAT_GOTDATA;
}


static bool ioStream_cb_writeBytes(void* buffIn, size_t bufferSize_bytesIn, void *const userVarIn)
{
	cxa_esp8266_usart_t* usartIn = (cxa_esp8266_usart_t*)userVarIn;
	cxa_assert(usartIn);
	
	for( size_t i = 0; i < bufferSize_bytesIn; i++ )
	{
		// wait for previous transmission to finish
		while( ((USS(usartIn->id) >> 16) & 0x000000FF) == 0x80);
	
		// now send our data
		USF(usartIn->id) = ((uint8_t*)buffIn)[i];
	}
	
	return true;
}