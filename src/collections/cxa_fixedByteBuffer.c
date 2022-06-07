/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 *
 * @author Christopher Armenio
 */
#include "cxa_fixedByteBuffer.h"


// ******** includes ********
#include <cxa_assert.h>
#include <string.h>
#include <sys/types.h>


// ******** local macro definitions ********


// ******** local type definitions ********


// ******** local function prototypes ********


// ********  local variable declarations *********


// ******** global function implementations ********
void cxa_fixedByteBuffer_init(cxa_fixedByteBuffer_t *const fbbIn, void *const bufferLocIn, const size_t bufferMaxSize_bytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(bufferLocIn);

	// setup our internal state
	cxa_array_init(&fbbIn->bytes, 1, bufferLocIn, bufferMaxSize_bytesIn);
}


void cxa_fixedByteBuffer_init_inPlace(cxa_fixedByteBuffer_t *const fbbIn, const size_t currNumElemsIn, void *const bufferLocIn, const size_t bufferMaxSize_bytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(bufferLocIn);

	// setup our internal state
	cxa_array_init_inPlace(&fbbIn->bytes, 1, currNumElemsIn, bufferLocIn, bufferMaxSize_bytesIn);
}


void cxa_fixedByteBuffer_init_subBufferFixedSize(cxa_fixedByteBuffer_t *const subFbbIn, cxa_fixedByteBuffer_t *const parentFbbIn, const size_t startIndexIn, size_t numBytesIn)
{
	cxa_assert(subFbbIn);
	cxa_assert(parentFbbIn);
	cxa_assert( (startIndexIn + numBytesIn) <= cxa_fixedByteBuffer_getSize_bytes(parentFbbIn) );

	// setup our internal state
	cxa_array_init_inPlace(&subFbbIn->bytes, 1, numBytesIn, cxa_array_get(&parentFbbIn->bytes, startIndexIn), numBytesIn);
}


void cxa_fixedByteBuffer_init_subBufferRemainingElems(cxa_fixedByteBuffer_t *const subFbbIn, cxa_fixedByteBuffer_t *const parentFbbIn, const size_t startIndexIn)
{
	cxa_assert(subFbbIn);
	cxa_assert(parentFbbIn);
	cxa_assert(startIndexIn <= cxa_fixedByteBuffer_getSize_bytes(parentFbbIn));

	// setup our internal state
	size_t numElems = cxa_fixedByteBuffer_getSize_bytes(parentFbbIn) - startIndexIn;
	cxa_array_init_inPlace(&subFbbIn->bytes, 1, numElems, cxa_array_get_noBoundsCheck(&parentFbbIn->bytes, startIndexIn), numElems);
}


void cxa_fixedByteBuffer_init_subBufferParentMaxSize(cxa_fixedByteBuffer_t *const subFbbIn, cxa_fixedByteBuffer_t *const parentFbbIn, const size_t startIndexIn)
{
	cxa_assert(subFbbIn);
	cxa_assert(parentFbbIn);
	cxa_assert(startIndexIn <= cxa_fixedByteBuffer_getMaxSize_bytes(parentFbbIn));

	// setup our internal state
	size_t maxSize_bytes = cxa_fixedByteBuffer_getMaxSize_bytes(parentFbbIn) - startIndexIn;
	cxa_array_init_inPlace(&subFbbIn->bytes, 1, 0, cxa_array_get_noBoundsCheck(&parentFbbIn->bytes, startIndexIn), maxSize_bytes);
}


bool cxa_fixedByteBuffer_append(cxa_fixedByteBuffer_t *const fbbIn, uint8_t *const ptrIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(ptrIn);

	// make sure we have room for the operation
	if( cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < numBytesIn ) return false;

	for( size_t i = 0; i < numBytesIn; i++ )
	{
		if( !cxa_array_append(&fbbIn->bytes, &(ptrIn[i])) ) return false;
	}

	return true;
}


bool cxa_fixedByteBuffer_append_transposed(cxa_fixedByteBuffer_t *const fbbIn, uint8_t *const ptrIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(ptrIn);

	// make sure we have room for the operation
	if( cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < numBytesIn ) return false;

	for( size_t i = 0; i < numBytesIn; i++ )
	{
		if( !cxa_array_append(&fbbIn->bytes, &(ptrIn[numBytesIn-i-1])) ) return false;
	}

	return true;
}


bool cxa_fixedByteBuffer_append_lengthPrefixedField_uint16BE(cxa_fixedByteBuffer_t *const fbbIn, uint8_t *const ptrIn, const uint16_t numBytesIn)
{
	cxa_assert(fbbIn);
	if( numBytesIn > 0 ) cxa_assert(ptrIn);

	// make sure we have room for the operation
	if( cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < (sizeof(numBytesIn) + numBytesIn) ) return false;

	// first the size
	if( !cxa_fixedByteBuffer_append_uint16BE(fbbIn, numBytesIn) ) return false;

	// now the actual data
	return cxa_fixedByteBuffer_append(fbbIn, ptrIn, numBytesIn);
}


void* cxa_fixedByteBuffer_append_emptyBytes(cxa_fixedByteBuffer_t *const fbbIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);

	// make sure we have room for the operation
	if( cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < numBytesIn ) return NULL;

	void* retVal = cxa_fixedByteBuffer_get_pointerToIndex(fbbIn, cxa_fixedByteBuffer_getSize_bytes(fbbIn));
	for( size_t i = 0; i < numBytesIn; i++ )
	{
		// shouldn't happen, but we should test
		if( !cxa_array_append_empty(&fbbIn->bytes) ) return NULL;
	}
	return retVal;
}


bool cxa_fixedByteBuffer_append_fbb(cxa_fixedByteBuffer_t *const fbbIn, cxa_fixedByteBuffer_t *const sourceFbbIn)
{
	cxa_assert(fbbIn);

	if( sourceFbbIn == NULL ) return true;

	cxa_array_iterate(&sourceFbbIn->bytes, currByte, uint8_t)
	{
		if( !cxa_fixedByteBuffer_append_uint8(fbbIn, *currByte) ) return false;
	}

	return true;
}


bool cxa_fixedByteBuffer_remove(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);

	// make sure we have room for the operation
	if( (indexIn + numBytesIn) > cxa_fixedByteBuffer_getSize_bytes(fbbIn) ) return false;

	for( size_t i = 0; i < numBytesIn; i++ )
	{
		if( !cxa_array_remove_atIndex(&fbbIn->bytes, indexIn) ) return false;
	}

	return true;
}


bool cxa_fixedByteBuffer_remove_cString(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn)
{
	cxa_assert(fbbIn);

	uint8_t* targetString = cxa_fixedByteBuffer_get_pointerToIndex(fbbIn, indexIn);
	if( targetString == NULL ) return false;

	// figure out how long the string is...
	size_t strLen_bytes = strlen((const char*)targetString) + 1;

	return cxa_fixedByteBuffer_remove(fbbIn, indexIn, strLen_bytes);
}


uint8_t* cxa_fixedByteBuffer_get_pointerToIndex(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn)
{
	cxa_assert(fbbIn);

	return (uint8_t*)cxa_array_get(&fbbIn->bytes, indexIn);
}


bool cxa_fixedByteBuffer_get(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, bool transposeIn, uint8_t *const valOut, const size_t numBytesIn)
{
	cxa_assert(fbbIn);

	// make sure we have enough bytes in the buffer for this operation
	if( (indexIn + numBytesIn) > cxa_fixedByteBuffer_getSize_bytes(fbbIn)) return false;

	// if we don't have anyplace to copy the data, we're done!
	if( !valOut ) return true;

	// we need to copy the data out
	for( size_t i = 0; i < numBytesIn; i++ )
	{
		uint8_t *currByte = cxa_array_get(&fbbIn->bytes, indexIn+i);
		if( currByte == NULL ) return false;

		if( !transposeIn ) valOut[i] = *currByte;
		else valOut[numBytesIn-i-1] = *currByte;
	}

	return true;
}


bool cxa_fixedByteBuffer_get_cString(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, char *const stringOut, size_t maxOutputSize_bytes)
{
	cxa_assert(fbbIn);
	cxa_assert(stringOut);

	// make sure we have enough bytes in the buffer for this operation
	char* targetString = (char*)cxa_array_get(&fbbIn->bytes, indexIn);
	if( targetString == NULL) return false;

	// size+1 for terminator
	size_t targetStringSize_bytes = strlen(targetString)+1;

	if( targetStringSize_bytes > maxOutputSize_bytes ) return false;

	// if we don't have anyplace to copy the data, we're done!
	if( !stringOut ) return true;

	memcpy(stringOut, targetString, targetStringSize_bytes);

	return true;
}


bool cxa_fixedByteBuffer_get_cString_inPlace(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, char** stringOut, size_t *strLen_bytesOut)
{
	cxa_assert(fbbIn);

	// make sure we have enough bytes in the buffer for this operation
	char* targetString = (char*)cxa_array_get(&fbbIn->bytes, indexIn);
	if( targetString == NULL) return false;

	// set our output parameters
	if( stringOut != NULL ) *stringOut = targetString;
	if( strLen_bytesOut != NULL ) *strLen_bytesOut = strlen(targetString);

	return true;
}


bool cxa_fixedByteBuffer_get_lengthPrefixedField_uint16BE(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, uint8_t** ptrOut, uint16_t* numBytesOut)
{
	cxa_assert(fbbIn);

	// get our length bytes
	uint16_t numBytes;
	if( !cxa_fixedByteBuffer_get_uint16BE(fbbIn, indexIn, numBytes) ) return false;

	// make sure we have enough bytes in the buffer for this operation
	if( (indexIn + 2 + numBytes) > cxa_fixedByteBuffer_getSize_bytes(fbbIn) ) return false;

	if( ptrOut != NULL ) *ptrOut = cxa_fixedByteBuffer_get_pointerToIndex(fbbIn, indexIn+2);
	if( numBytesOut != NULL ) *numBytesOut = numBytes;

	return true;
}


bool cxa_fixedByteBuffer_get_lengthPrefixedCString_uint16BE(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, char** ptrOut, uint16_t* numBytesOut, bool *isNullTerminatedOut)
{
	cxa_assert(fbbIn);

	char* lclPtrOut;
	uint16_t lclNumBytesOut;
	if( !cxa_fixedByteBuffer_get_lengthPrefixedField_uint16BE(fbbIn, indexIn, (uint8_t**)&lclPtrOut, &lclNumBytesOut) ) return false;

	if( ptrOut != NULL ) *ptrOut = lclPtrOut;
	if( numBytesOut != NULL ) *numBytesOut = lclNumBytesOut;
	if( isNullTerminatedOut ) *isNullTerminatedOut = (lclNumBytesOut > 0) ? (lclPtrOut[lclNumBytesOut-1] == 0) : false;

	return true;
}


bool cxa_fixedByteBuffer_replace(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, uint8_t *const ptrIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(ptrIn);

	// make sure we have enough bytes in the buffer for this operation
	if( (indexIn + numBytesIn) > cxa_fixedByteBuffer_getSize_bytes(fbbIn)) return false;

	for( size_t i = 0; i < numBytesIn; i++ )
	{
		if( !cxa_array_overwrite(&fbbIn->bytes, indexIn+i, &(ptrIn[i])) ) return false;
	}

	return true;
}



bool cxa_fixedByteBuffer_replace_cString(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, char *const stringIn)
{
	cxa_assert(fbbIn);
	cxa_assert(stringIn);

	// calculate our sizes
	size_t replacementStringSize_bytes = strlen(stringIn)+1;
	char* targetString = (char*)cxa_fixedByteBuffer_get_pointerToIndex(fbbIn, indexIn);
	if( targetString == NULL ) return false;

	size_t targetStringSize_bytes = strlen(targetString)+1;
	ssize_t discrepantSize_bytes = replacementStringSize_bytes - targetStringSize_bytes;

	// make sure we have room for this operation at the specified index
	if( (indexIn+replacementStringSize_bytes) > cxa_fixedByteBuffer_getMaxSize_bytes(fbbIn) ) return false;

	// now, make sure that we have enough free space in the buffer if the replacement string is larger
	if( (discrepantSize_bytes > 0) && (cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < ((size_t)discrepantSize_bytes)) ) return false;

	// if we made it here, we should be good to perform the operation...start by removing the current string
	if( !cxa_fixedByteBuffer_remove_cString(fbbIn, indexIn) ) return false;

	// now insert the new string
	return cxa_fixedByteBuffer_insert_cString(fbbIn, indexIn, stringIn);
}


bool cxa_fixedByteBuffer_insert(cxa_fixedByteBuffer_t *const fbbIn, const size_t indexIn, uint8_t *const ptrIn, const size_t numBytesIn)
{
	cxa_assert(fbbIn);
	cxa_assert(ptrIn);

	// make sure we have room for the operation
	if( cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) < numBytesIn ) return false;

	// make sure the index is in bounds
	if( indexIn > cxa_fixedByteBuffer_getSize_bytes(fbbIn) ) return false;

	for( size_t i = 0; i < numBytesIn; i++ )
	{
		if( !cxa_array_insert(&fbbIn->bytes, indexIn+i, (void*)&(ptrIn[i])) ) return false;
	}

	return true;
}


uint8_t* cxa_fixedByteBuffer_get_pointerToStartOfData(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);

	return cxa_array_get_noBoundsCheck(&fbbIn->bytes, 0);
}


size_t cxa_fixedByteBuffer_getSize_bytes(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);

	return cxa_array_getSize_elems(&fbbIn->bytes);
}


size_t cxa_fixedByteBuffer_getMaxSize_bytes(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);

	return cxa_array_getMaxSize_elems(&fbbIn->bytes);
}


size_t cxa_fixedByteBuffer_getFreeSize_bytes(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);

	return cxa_array_getFreeSize_elems(&fbbIn->bytes);
}


bool cxa_fixedByteBuffer_isFull(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);
	return cxa_fixedByteBuffer_getFreeSize_bytes(fbbIn) == 0;
}


bool cxa_fixedByteBuffer_isEmpty(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);
	return cxa_fixedByteBuffer_getSize_bytes(fbbIn) == 0;
}


void cxa_fixedByteBuffer_clear(cxa_fixedByteBuffer_t *const fbbIn)
{
	cxa_assert(fbbIn);

	cxa_array_clear(&fbbIn->bytes);
}


// ******** local function implementations ********
