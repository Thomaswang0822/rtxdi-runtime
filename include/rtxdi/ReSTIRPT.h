/***************************************************************************
 # Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 #
 # NVIDIA CORPORATION and its licensors retain all intellectual property
 # and proprietary rights in and to this software, related documentation
 # and any modifications thereto.  Any use, reproduction, disclosure or
 # distribution of this software and related documentation without an express
 # license agreement from NVIDIA CORPORATION is strictly prohibited.
 **************************************************************************/

#pragma once

#include <stdint.h>
#include "rtxdi/ReSTIRPTParameters.h"
#include "rtxdi/RtxdiUtils.h"

namespace rtxdi
{
	static constexpr uint32_t c_NumReSTIRPTReservoirBuffers = 2;

	enum class ReSTIRPT_ResamplingMode : uint32_t
	{
		None = 0,
		Temporal = 1,
		Spatial = 2,
		TemporalAndSpatial = 3,
		FusedSpatiotemporal = 4,
	};

	// Parameters used to initialize the ReSTIRPTContext
	// Changing any of these requires recreating the context.
	struct ReSTIRPTStaticParameters
	{
		uint32_t NeighborOffsetCount = 8192;
		uint32_t RenderWidth = 0;
		uint32_t RenderHeight = 0;

		CheckerboardMode CheckerboardSamplingMode = CheckerboardMode::Off;
	};

	constexpr ReSTIRPT_BufferIndices getDefaultReSTIRPTBufferIndices()
	{
		ReSTIRPT_BufferIndices bufferIndices = {};
		bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = 0;
		bufferIndices.temporalResamplingInputBufferIndex = 0;
		bufferIndices.temporalResamplingOutputBufferIndex = 0;
		bufferIndices.spatialResamplingInputBufferIndex = 0;
		bufferIndices.spatialResamplingOutputBufferIndex = 0;
		return bufferIndices;
	}

	constexpr ReSTIRPT_TemporalResamplingParameters getDefaultReSTIRPTTemporalResamplingParams()
	{
		ReSTIRPT_TemporalResamplingParameters params = {};
		params.boilingFilterStrength = 0.2f;
		params.depthThreshold = 0.1f;
		params.enableBoilingFilter = true;
		params.enableFallbackSampling = true;
		params.enablePermutationSampling = false;
		params.maxHistoryLength = 8;
		params.maxReservoirAge = 30;
		params.normalThreshold = 0.6f;
		params.temporalBiasCorrectionMode = ResTIRPT_TemporalBiasCorrectionMode::Basic;
		return params;
	}

	constexpr ReSTIRPT_SpatialResamplingParameters getDefaultReSTIRPTSpatialResamplingParams()
	{
		ReSTIRPT_SpatialResamplingParameters params = {};
		params.numSpatialSamples = 2;
		params.spatialBiasCorrectionMode = ResTIRPT_SpatialBiasCorrectionMode::Basic;
		params.spatialDepthThreshold = 0.1f;
		params.spatialNormalThreshold = 0.6f;
		params.spatialSamplingRadius = 32.0f;
		return params;
	}

	constexpr ReSTIRPT_FinalShadingParameters getDefaultReSTIRPTFinalShadingParams()
	{
		ReSTIRPT_FinalShadingParameters params = {};
		params.enableFinalMIS = true;
		params.enableFinalVisibility = true;
		return params;
	}

	class ReSTIRPTContext
	{
	public:
		ReSTIRPTContext(const ReSTIRPTStaticParameters& params);

		// getters

		ReSTIRPTStaticParameters getStaticParams() const;
		uint32_t getFrameIndex() const;
		RTXDI_ReservoirBufferParameters getReservoirBufferParameters() const;
		ReSTIRPT_ResamplingMode getResamplingMode() const;
		ReSTIRPT_BufferIndices getBufferIndices() const;
		ReSTIRPT_TemporalResamplingParameters getTemporalResamplingParameters() const;
		ReSTIRPT_SpatialResamplingParameters getSpatialResamplingParameters() const;
		ReSTIRPT_FinalShadingParameters getFinalShadingParameters() const;

		// setters

		void setFrameIndex(uint32_t frameIndex);
		void setResamplingMode(ReSTIRPT_ResamplingMode resamplingMode);
		void setTemporalResamplingParameters(const ReSTIRPT_TemporalResamplingParameters& temporalResamplingParams);
		void setSpatialResamplingParameters(const ReSTIRPT_SpatialResamplingParameters& spatialResamplingParams);
		void setFinalShadingParameters(const ReSTIRPT_FinalShadingParameters& finalShadingParams);

		static uint32_t numReservoirBuffers;

	private:
		ReSTIRPTStaticParameters m_staticParams;

		uint32_t m_frameIndex;
		RTXDI_ReservoirBufferParameters m_reservoirBufferParams;
		ReSTIRPT_ResamplingMode m_resamplingMode;
		ReSTIRPT_BufferIndices m_bufferIndices;
		ReSTIRPT_TemporalResamplingParameters m_temporalResamplingParams;
		ReSTIRPT_SpatialResamplingParameters m_spatialResamplingParams;
		ReSTIRPT_FinalShadingParameters m_finalShadingParams;

		void updateBufferIndices();
	};
}
