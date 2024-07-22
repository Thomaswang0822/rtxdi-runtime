/***************************************************************************
 # Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 #
 # NVIDIA CORPORATION and its licensors retain all intellectual property
 # and proprietary rights in and to this software, related documentation
 # and any modifications thereto.  Any use, reproduction, disclosure or
 # distribution of this software and related documentation without an express
 # license agreement from NVIDIA CORPORATION is strictly prohibited.
 **************************************************************************/

#ifndef RTXDI_RESTIRPT_PARAMETERS_H
#define RTXDI_RESTIRPT_PARAMETERS_H

#include "RtxdiTypes.h"
#include "RtxdiParameters.h"

struct RTXDI_PackedPTReservoir
{
#ifdef __cplusplus
    using uint2 = uint32_t[2];
#endif
    uint2       packed_rngState;    // There isn't RAB_RandomSamplerState (2 uint) in host code.
    uint32_t    kReconnectId;   // reconnection vertex id

    uint32_t    packed_miscData_age_M; // See Reservoir.hlsli about the detail of the bit field.

    float       weight;
    float       unused;
};

#ifdef __cplusplus
enum class ResTIRPT_TemporalBiasCorrectionMode : uint32_t
{
    Off = RTXDI_BIAS_CORRECTION_OFF,
    Basic = RTXDI_BIAS_CORRECTION_BASIC,
    // Pairwise is not supported
    Raytraced = RTXDI_BIAS_CORRECTION_RAY_TRACED
};

enum class ResTIRPT_SpatialBiasCorrectionMode : uint32_t
{
    Off = RTXDI_BIAS_CORRECTION_OFF,
    Basic = RTXDI_BIAS_CORRECTION_BASIC,
    // Pairwise is not supported
    Raytraced = RTXDI_BIAS_CORRECTION_RAY_TRACED
};
#else
#define ResTIRPT_TemporalBiasCorrectionMode uint32_t
#define ResTIRPT_SpatialBiasCorrectionMode uint32_t
#endif

// Very similar to RTXDI_TemporalResamplingParameters but it has an extra field
// It's also not the same algo, and we don't want the two to be coupled
struct ReSTIRPT_TemporalResamplingParameters
{
    float           depthThreshold;
    float           normalThreshold;
    uint32_t        enablePermutationSampling;
    uint32_t        maxHistoryLength;

    uint32_t        maxReservoirAge;
    uint32_t        enableBoilingFilter;
    float           boilingFilterStrength;
    uint32_t        enableFallbackSampling;

    ResTIRPT_TemporalBiasCorrectionMode temporalBiasCorrectionMode;// = ResTIRPT_TemporalBiasCorrectionMode::Basic;
    uint32_t uniformRandomNumber;
    uint32_t pad2;
    uint32_t pad3;
};

// See note for ReSTIRPT_TemporalResamplingParameters
struct ReSTIRPT_SpatialResamplingParameters
{
    float       spatialDepthThreshold;
    float       spatialNormalThreshold;
    uint32_t    numSpatialSamples;
    float       spatialSamplingRadius;

    ResTIRPT_SpatialBiasCorrectionMode  spatialBiasCorrectionMode;// = ResTIRPT_SpatialBiasCorrectionMode::Basic;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
};

struct ReSTIRPT_FinalShadingParameters
{
    uint32_t enableFinalVisibility;// = true;
    uint32_t enableFinalMIS;// = true;
    uint32_t pad1;
    uint32_t pad2;
};

struct ReSTIRPT_BufferIndices
{
    // likely change to sth like pathTreesOutputBufferIndex
    uint32_t secondarySurfaceReSTIRDIOutputBufferIndex;
    uint32_t temporalResamplingInputBufferIndex;
    uint32_t temporalResamplingOutputBufferIndex;
    uint32_t spatialResamplingInputBufferIndex;

    uint32_t spatialResamplingOutputBufferIndex;
    uint32_t finalShadingInputBufferIndex;
    uint32_t pad1;
    uint32_t pad2;
};

struct ReSTIRPT_Parameters
{
    RTXDI_ReservoirBufferParameters reservoirBufferParams;
    ReSTIRPT_BufferIndices bufferIndices;
    ReSTIRPT_TemporalResamplingParameters temporalResamplingParams;
    ReSTIRPT_SpatialResamplingParameters spatialResamplingParams;
    ReSTIRPT_FinalShadingParameters finalShadingParams;
};

#endif // RTXDI_RESTIRPT_PARAMETERS_H