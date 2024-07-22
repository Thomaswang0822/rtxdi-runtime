/***************************************************************************
 # Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 #
 # NVIDIA CORPORATION and its licensors retain all intellectual property
 # and proprietary rights in and to this software, related documentation
 # and any modifications thereto.  Any use, reproduction, disclosure or
 # distribution of this software and related documentation without an express
 # license agreement from NVIDIA CORPORATION is strictly prohibited.
 **************************************************************************/

#ifndef PT_RESERVOIR_HLSLI
#define PT_RESERVOIR_HLSLI

#include "ReSTIRPTParameters.h"
#include "RtxdiHelpers.hlsli"

// Define this macro to 0 if your shader needs read-only access to the reservoirs, 
// to avoid compile errors in the RTXDI_StorePTReservoir function
#ifndef RTXDI_ENABLE_STORE_RESERVOIR
#define RTXDI_ENABLE_STORE_RESERVOIR 1
#endif

#ifndef RTXDI_PT_RESERVOIR_BUFFER
#error "RTXDI_PT_RESERVOIR_BUFFER must be defined to point to a RWStructuredBuffer<RTXDI_PackedPTReservoir> type resource"
#endif

// This structure should be different from RTXDI_GIReservoir, e.g. see Note 6.3
// Its fields are described in Note 6.6
struct RTXDI_PTReservoir
{
    // reconnection vertex id; can use 0 to represent INVALID
    uint kReconnectId;
    
    // "random-number generating seed". In the codebase it should be RAB_RandomSamplerState,
    // which would be 2 uint, see shaders/HelperFunctions.hlsli
    RAB_RandomSamplerState rngState;

    // Overloaded: represents RIS weight sum during streaming,
    // then reservoir weight (inverse PDF) after FinalizeResampling
    float weightSum;

    // Number of samples considered for this reservoir
    uint M;

    // Number of frames the chosen sample has survived.
    uint age;
};

// Encoding helper constants for RTXDI_PackedPTReservoir
static const uint RTXDI_PackedPTReservoir_MShift = 0;
static const uint RTXDI_PackedPTReservoir_MaxM = 0x0ff;

static const uint RTXDI_PackedPTReservoir_AgeShift = 8;
static const uint RTXDI_PackedPTReservoir_MaxAge = 0x0ff;

// "misc data" only exists in the packed form of PT reservoir and stored into a gap field of the packed form.
// RTXDI SDK doesn't look into this field at all and when it stores a packed PT reservoir, the field is always filled with zero.
// Application can use this field to store anything.
static const uint RTXDI_PackedPTReservoir_MiscDataMask = 0xffff0000;

// Converts a PTReservoir into its packed form.
// This function should be used only when the application needs to store data with the given argument.
// It can be retrieved when unpacking the PTReservoir, but RTXDI SDK doesn't use the field at all. 
RTXDI_PackedPTReservoir RTXDI_PackPTReservoir(const RTXDI_PTReservoir reservoir, const uint miscData)
{
    RTXDI_PackedPTReservoir data;

    // TODO: how to turn a unresolved RAB_RandomSamplerState{ uint, uint } into uint32_t[2]?
    data.packed_rngState = reservoir.rngState;
    data.kReconnectId = reservoir.kReconnectId;

    data.packed_miscData_age_M =
        (miscData & RTXDI_PackedPTReservoir_MiscDataMask)
        | (min(reservoir.age, RTXDI_PackedPTReservoir_MaxAge) << RTXDI_PackedPTReservoir_AgeShift)
        | (min(reservoir.M, RTXDI_PackedPTReservoir_MaxM) << RTXDI_PackedPTReservoir_MShift);

    data.weight = reservoir.weightSum;
    data.unused = 0;

    return data;
}

// Converts a PackedPTReservoir into its unpacked form.
// This function should be used only when the application wants to retrieve the misc data stored in the gap field of the packed form.
RTXDI_PTReservoir RTXDI_UnpackPTReservoir(RTXDI_PackedPTReservoir data, out uint miscData)
{
    RTXDI_PTReservoir res;

    // TODO: how to turn a unresolved RAB_RandomSamplerState{ uint, uint } into uint32_t[2]?
    res.rngState = data.packed_rngState;
    res.kReconnectId = data.kReconnectId;
    
    res.weightSum = data.weight;

    res.M = (data.packed_miscData_age_M >> RTXDI_PackedPTReservoir_MShift) & RTXDI_PackedPTReservoir_MaxM;

    res.age = (data.packed_miscData_age_M >> RTXDI_PackedPTReservoir_AgeShift) & RTXDI_PackedPTReservoir_MaxAge;

    miscData = data.packed_miscData_age_M & RTXDI_PackedPTReservoir_MiscDataMask;

    return res;
}

// Converts a PackedPTReservoir into its unpacked form.
RTXDI_PTReservoir RTXDI_UnpackPTReservoir(RTXDI_PackedPTReservoir data)
{
    uint miscFlags; // unused;
    return RTXDI_UnpackPTReservoir(data, miscFlags);
}

RTXDI_PTReservoir RTXDI_LoadPTReservoir(
    RTXDI_ReservoirBufferParameters reservoirParams,
    uint2 reservoirPosition,
    uint reservoirArrayIndex)
{
    uint pointer = RTXDI_ReservoirPositionToPointer(reservoirParams, reservoirPosition, reservoirArrayIndex);
    return RTXDI_UnpackPTReservoir(RTXDI_PT_RESERVOIR_BUFFER[pointer]);
}

RTXDI_PTReservoir RTXDI_LoadPTReservoir(
    RTXDI_ReservoirBufferParameters reservoirParams,
    uint2 reservoirPosition,
    uint reservoirArrayIndex,
    out uint miscFlags)
{
    uint pointer = RTXDI_ReservoirPositionToPointer(reservoirParams, reservoirPosition, reservoirArrayIndex);
    return RTXDI_UnpackPTReservoir(RTXDI_PT_RESERVOIR_BUFFER[pointer], miscFlags);
}

#if RTXDI_ENABLE_STORE_RESERVOIR

void RTXDI_StorePackedPTReservoir(
    const RTXDI_PackedPTReservoir packedPTReservoir,
    RTXDI_ReservoirBufferParameters reservoirParams,
    uint2 reservoirPosition,
    uint reservoirArrayIndex)
{
    uint pointer = RTXDI_ReservoirPositionToPointer(reservoirParams, reservoirPosition, reservoirArrayIndex);
    RTXDI_PT_RESERVOIR_BUFFER[pointer] = packedPTReservoir;
}

void RTXDI_StorePTReservoir(
    const RTXDI_PTReservoir reservoir,
    RTXDI_ReservoirBufferParameters reservoirParams,
    uint2 reservoirPosition,
    uint reservoirArrayIndex)
{
    RTXDI_StorePackedPTReservoir(
        RTXDI_PackPTReservoir(reservoir, 0), reservoirParams, reservoirPosition, reservoirArrayIndex);
}

void RTXDI_StorePTReservoir(
    const RTXDI_PTReservoir reservoir,
    const uint miscFlags,
    RTXDI_ReservoirBufferParameters reservoirParams,
    uint2 reservoirPosition,
    uint reservoirArrayIndex)
{
    RTXDI_StorePackedPTReservoir(
        RTXDI_PackPTReservoir(reservoir, miscFlags), reservoirParams, reservoirPosition, reservoirArrayIndex);
}

RTXDI_PTReservoir RTXDI_EmptyPTReservoir()
{
    RTXDI_PTReservoir s;

    s.kReconnectId = 0;
    s.rngState = uint2(0, 0);
    s.weightSum = 0.0;
    s.M = 0;
    s.age = 0;

    return s;
}

bool RTXDI_IsValidPTReservoir(const RTXDI_PTReservoir reservoir)
{
    return reservoir.M != 0;
}

#endif // RTXDI_ENABLE_STORE_RESERVOIR

#endif // PT_RESERVOIR_HLSLI