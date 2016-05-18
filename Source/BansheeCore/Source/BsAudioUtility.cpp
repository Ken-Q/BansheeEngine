//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudioUtility.h"

namespace BansheeEngine
{
	void convertToMono8(UINT8* input, UINT8* output, UINT32 numSamples, UINT32 numChannels)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			UINT16 sum = 0;
			for (UINT32 j = 0; j < numChannels; j++)
			{
				sum += *input;
				++input;
			}

			*output = sum / numChannels;
			++output;
		}
	}

	void convertToMono16(INT16* input, INT16* output, UINT32 numSamples, UINT32 numChannels)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			INT32 sum = 0;
			for (UINT32 j = 0; j < numChannels; j++)
			{
				sum += *input;
				++input;
			}

			*output = sum / numChannels;
			++output;
		}
	}

	INT32 convert24To32Bits(const UINT8* input)
	{
		bool isNegative = (input[2] & 0x80) != 0;
		if (isNegative) // Sign extend if negative
			return (0xFF << 24) | (input[2] << 16) | (input[1] << 8) | input[0];
		else
			return (input[2] << 16) | (input[1] << 8) | input[0];
	}

	void convert32To24Bits(INT32 input, UINT8* output)
	{
		UINT32 valToEncode = *(UINT32*)&input;
		output[0] = valToEncode & 0x000000FF;
		output[1] = (valToEncode >> 8) & 0x000000FF;
		output[2] = (valToEncode >> 16) & 0x000000FF;
	}

	void convertToMono24(UINT8* input, UINT8* output, UINT32 numSamples, UINT32 numChannels)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			INT32 sum = 0;
			for (UINT32 j = 0; j < numChannels; j++)
			{
				sum += convert24To32Bits(input);
				input += 3;
			}

			INT32 avg = sum / numChannels;
			convert32To24Bits(avg, output);
			output += 3;
		}
	}

	void convertToMono32(INT32* input, INT32* output, UINT32 numSamples, UINT32 numChannels)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			INT64 sum = 0;
			for (UINT32 j = 0; j < numChannels; j++)
			{
				sum += *input;
				++input;
			}

			*output = (INT32)(sum / numChannels);
			++output;
		}
	}

	void convert8To32Bits(UINT8* input, INT32* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			INT8 val = (INT8)(input[i] - 128);
			output[i] = val << 24;
		}
	}

	void convert16To32Bits(INT16* input, INT32* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
			output[i] = input[i] << 16;
	}

	void convert24To32Bits(UINT8* input, INT32* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			output[i] = convert24To32Bits(input);
			input += 3;
		}
	}

	void convert32To8Bits(INT32* input, UINT8* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			INT8 val = (INT8)(input[i] >> 24);
			output[i] = (UINT8)(val + 128);
		}
	}

	void convert32To16Bits(INT32* input, INT16* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
			output[i] = (INT16)(input[i] >> 16);
	}

	void convert32To24Bits(INT32* input, UINT8* output, UINT32 numSamples)
	{
		for (UINT32 i = 0; i < numSamples; i++)
		{
			convert32To24Bits(input[i], output);
			output += 3;
		}
	}

	void AudioUtility::convertToMono(UINT8* input, UINT8* output, UINT32 bitDepth, UINT32 numSamples, UINT32 numChannels)
	{
		switch (bitDepth)
		{
		case 8:
			convertToMono8(input, output, numSamples, numChannels);
			break;
		case 16:
			convertToMono16((INT16*)input, (INT16*)output, numSamples, numChannels);
			break;
		case 24:
			convertToMono24(input, output, numSamples, numChannels);
			break;
		case 32:
			convertToMono32((INT32*)input, (INT32*)output, numSamples, numChannels);
			break;
		default:
			assert(false);
			break;
		}
	}

	void AudioUtility::convertBitDepth(UINT8* input, UINT32 inBitDepth, UINT8* output, UINT32 outBitDepth, UINT32 numSamples)
	{
		INT32* srcBuffer = nullptr;

		bool needTempBuffer = inBitDepth != 32;
		if (needTempBuffer)
			srcBuffer = (INT32*)bs_stack_alloc(numSamples * sizeof(INT32));
		else
			srcBuffer = (INT32*)input;

		// Note: I convert to a temporary 32-bit buffer and then use that to convert to actual requested bit depth. 
		//       It would be more efficient to convert directly from source to requested depth without a temporary buffer,
		//       at the cost of additional complexity. If this method ever becomes a performance issue consider that.
		switch (inBitDepth)
		{
		case 8:
			convert8To32Bits(input, srcBuffer, numSamples);
			break;
		case 16:
			convert16To32Bits((INT16*)input, srcBuffer, numSamples);
			break;
		case 24:
			convert8To32Bits(input, srcBuffer, numSamples);
			break;
		case 32:
			// Do nothing
			break;
		default:
			assert(false);
			break;
		}

		switch (outBitDepth)
		{
		case 8:
			convert32To8Bits(srcBuffer, output, numSamples);
			break;
		case 16:
			convert32To16Bits(srcBuffer, (INT16*)output, numSamples);
			break;
		case 24:
			convert32To24Bits(srcBuffer, output, numSamples);
			break;
		case 32:
			memcpy(output, srcBuffer, numSamples * sizeof(INT32));
			break;
		default:
			assert(false);
			break;
		}

		if (needTempBuffer)
		{
			bs_stack_free(srcBuffer);
			srcBuffer = nullptr;
		}
	}
}