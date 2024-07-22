/***************************************************************************
 # Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 #
 # NVIDIA CORPORATION and its licensors retain all intellectual property
 # and proprietary rights in and to this software, related documentation
 # and any modifications thereto.  Any use, reproduction, disclosure or
 # distribution of this software and related documentation without an express
 # license agreement from NVIDIA CORPORATION is strictly prohibited.
 **************************************************************************/

#include "rtxdi/ReSTIRPT.h"

namespace rtxdi
{

    ReSTIRPTContext::ReSTIRPTContext(const ReSTIRPTStaticParameters& staticParams) :
        m_frameIndex(0),
        m_reservoirBufferParams(CalculateReservoirBufferParameters(staticParams.RenderWidth, staticParams.RenderHeight, staticParams.CheckerboardSamplingMode)),
        m_staticParams(staticParams),
        m_resamplingMode(rtxdi::ReSTIRPT_ResamplingMode::None),
        m_bufferIndices(getDefaultReSTIRPTBufferIndices()),
        m_temporalResamplingParams(getDefaultReSTIRPTTemporalResamplingParams()),
        m_spatialResamplingParams(getDefaultReSTIRPTSpatialResamplingParams()),
        m_finalShadingParams(getDefaultReSTIRPTFinalShadingParams())
    {
    }

    ReSTIRPTStaticParameters ReSTIRPTContext::getStaticParams() const
    {
        return m_staticParams;
    }

    uint32_t ReSTIRPTContext::getFrameIndex() const
    {
        return m_frameIndex;
    }

    RTXDI_ReservoirBufferParameters ReSTIRPTContext::getReservoirBufferParameters() const
    {
        return m_reservoirBufferParams;
    }

    ReSTIRPT_ResamplingMode ReSTIRPTContext::getResamplingMode() const
    {
        return m_resamplingMode;
    }

    ReSTIRPT_BufferIndices ReSTIRPTContext::getBufferIndices() const
    {
        return m_bufferIndices;
    }

    ReSTIRPT_TemporalResamplingParameters ReSTIRPTContext::getTemporalResamplingParameters() const
    {
        return m_temporalResamplingParams;
    }

    ReSTIRPT_SpatialResamplingParameters ReSTIRPTContext::getSpatialResamplingParameters() const
    {
        return m_spatialResamplingParams;
    }

    ReSTIRPT_FinalShadingParameters ReSTIRPTContext::getFinalShadingParameters() const
    {
        return m_finalShadingParams;
    }

    void ReSTIRPTContext::setFrameIndex(uint32_t frameIndex)
    {
        m_frameIndex = frameIndex;
        m_temporalResamplingParams.uniformRandomNumber = JenkinsHash(m_frameIndex);
        updateBufferIndices();
    }

    void ReSTIRPTContext::setResamplingMode(ReSTIRPT_ResamplingMode resamplingMode)
    {
        m_resamplingMode = resamplingMode;
        updateBufferIndices();
    }

    void ReSTIRPTContext::setTemporalResamplingParameters(const ReSTIRPT_TemporalResamplingParameters& temporalResamplingParams)
    {
        m_temporalResamplingParams = temporalResamplingParams;
        m_temporalResamplingParams.uniformRandomNumber = JenkinsHash(m_frameIndex);
    }

    void ReSTIRPTContext::setSpatialResamplingParameters(const ReSTIRPT_SpatialResamplingParameters& spatialResamplingParams)
    {
        m_spatialResamplingParams = spatialResamplingParams;
    }

    void ReSTIRPTContext::setFinalShadingParameters(const ReSTIRPT_FinalShadingParameters& finalShadingParams)
    {
        m_finalShadingParams = finalShadingParams;
    }

    void ReSTIRPTContext::updateBufferIndices()
    {
        switch (m_resamplingMode)
        {
        case rtxdi::ReSTIRPT_ResamplingMode::None:
            m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = 0;
            m_bufferIndices.finalShadingInputBufferIndex = 0;
            break;
        case rtxdi::ReSTIRPT_ResamplingMode::Temporal:
            m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = m_frameIndex & 1;
            m_bufferIndices.temporalResamplingInputBufferIndex = !m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex;
            m_bufferIndices.temporalResamplingOutputBufferIndex = m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex;
            m_bufferIndices.finalShadingInputBufferIndex = m_bufferIndices.temporalResamplingOutputBufferIndex;
            break;
        case rtxdi::ReSTIRPT_ResamplingMode::Spatial:
            m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = 0;
            m_bufferIndices.spatialResamplingInputBufferIndex = 0;
            m_bufferIndices.spatialResamplingOutputBufferIndex = 1;
            m_bufferIndices.finalShadingInputBufferIndex = 1;
            break;
        case rtxdi::ReSTIRPT_ResamplingMode::TemporalAndSpatial:
            m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = 0;
            m_bufferIndices.temporalResamplingInputBufferIndex = 1;
            m_bufferIndices.temporalResamplingOutputBufferIndex = 0;
            m_bufferIndices.spatialResamplingInputBufferIndex = 0;
            m_bufferIndices.spatialResamplingOutputBufferIndex = 1;
            m_bufferIndices.finalShadingInputBufferIndex = 1;
            break;
        case rtxdi::ReSTIRPT_ResamplingMode::FusedSpatiotemporal:
            m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex = m_frameIndex & 1;
            m_bufferIndices.temporalResamplingInputBufferIndex = !m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex;
            m_bufferIndices.spatialResamplingOutputBufferIndex = m_bufferIndices.secondarySurfaceReSTIRDIOutputBufferIndex;
            m_bufferIndices.finalShadingInputBufferIndex = m_bufferIndices.spatialResamplingOutputBufferIndex;
            break;
        }
    }
}
