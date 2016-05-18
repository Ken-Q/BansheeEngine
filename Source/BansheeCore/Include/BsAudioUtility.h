//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Provides various utility functionality relating to audio. */
	class BS_CORE_EXPORT AudioUtility
	{
	public:
		/** 
		 * Converts a set of audio samples using multiple channels into a set of mono samples. 
		 *
		 * @param[in]	input		A set of input samples. Per-channels samples should be interleaved. Size of each sample
		 *							is determined by @p bitDepth. Total size of the buffer should be @p numSamples *
		 *							@p numChannels * @p bitDepth / 8.
		 * @param[out]	output		Pre-allocated buffer to store the mono samples. Should be of @p numSamples * 
		 *							@p bitDepth / 8 size.
		 * @param[in]	bitDepth	Size of a single sample in bits.
		 * @param[in]	numSamples	Number of samples per a single channel.
		 * @param[in]	numChannels	Number of channels in the input data.
		 */
		static void convertToMono(UINT8* input, UINT8* output, UINT32 bitDepth, UINT32 numSamples, UINT32 numChannels);

		/**
		 * Converts a set of audio samples of a certain bit depth to a new bit depth.
		 *
		 * @param[in]	input		A set of input samples. Total size of the buffer should be @p numSamples *
		 *							@p inBitDepth / 8.
		 * @param[in]	inBitDepth	Size of a single sample in the @p input array, in bits.
		 * @param[out]	output		Pre-allocated buffer to store the output samples in. Total size of the buffer should be
		 *							@p numSamples * @p outBitDepth / 8.
		 * @param[in]	inBitDepth	Size of a single sample in the @p output array, in bits.
		 * @param[in]	numSamples	Total number of samples to process.
		 */
		static void convertBitDepth(UINT8* input, UINT32 inBitDepth, UINT8* output, UINT32 outBitDepth, UINT32 numSamples);
	};

	/** @} */
}