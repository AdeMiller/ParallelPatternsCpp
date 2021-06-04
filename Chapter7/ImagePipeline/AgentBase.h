//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <sstream>
#include <exception>
#include <stdexcept>
#include <agents.h>
#include <ppl.h>
#include <tuple>

#include "ImageInfo.h"

#define WM_REPORTERROR (WM_USER + 1)
#define WM_UPDATEWINDOW (WM_USER + 2)

namespace ImagePipeline
{
    using namespace ::Concurrency;

    typedef std::tuple<Phases, /* image filename */ wstring, /* exception message */ wstring> ErrorInfo;

    class IImagePipelineDialog
    {
    public:
        virtual SIZE GetImageSize() const = 0;
        virtual HWND GetWindow() const = 0;
    };

    class AgentBase : public agent
    {
    private:
        HWND m_dialogWindow;
        ISource<bool>& m_cancellationSource;
        ITarget<ErrorInfo>& m_errorTarget;
        // Marked mutable to allow the agent work functions to be declared as const. This is true 
        // except in the error case where ShutdownOnError is called and m_shutdownPending is set to true.
        mutable overwrite_buffer<bool> m_shutdownPending;
        // Marked mutable as this is completely internal not public state.
        mutable critical_section m_latestImageLock; 
        ImageInfoPtr m_pLatestImage;
    
    protected:
        int GetPipelineCapacity() const { return 20; }

    public:
        AgentBase(HWND dialog, ISource<bool>& cancellationSource, ITarget<ErrorInfo>& errorTarget) : 
            m_dialogWindow(dialog), 
            m_cancellationSource(cancellationSource),
            m_errorTarget(errorTarget)
        {
            send(m_shutdownPending, false);
        }

        void SetCurrentImage(const ImageInfoPtr pInfo)
        {
            {
                critical_section::scoped_lock lock(m_latestImageLock);
                m_pLatestImage = pInfo;                                         // Side effect; sets the latest image.
            }
            PostMessageW(m_dialogWindow, WM_UPDATEWINDOW, 0, 0);
        }

        ImageInfoPtr GetCurrentImage() const
        {
            ImageInfoPtr pInfo;
            {
                critical_section::scoped_lock lock(m_latestImageLock);
                pInfo = m_pLatestImage;
            }
            return pInfo;
        }

        virtual int GetQueueSize(int queue) const = 0;

        bool IsCancellationPending() const { return receive(m_shutdownPending) || receive(m_cancellationSource); }

        // Pipeline stage work functions.

        // Each pipeline work function is stateless except for DisplayImage. They take an incoming ImageInfoPtr and apply an operation to it returning
        // a pointer to the updated image. DisplayImage updates the UI dialog with the latest complete image. They also trap all exceptions and process them.

        ImageInfoPtr LoadImage(int sequence, const wstring& filePath, const LARGE_INTEGER& offset) const
        {
            ImageInfoPtr pInfo = nullptr;
            try
            {
                if (!IsCancellationPending())
                    pInfo = ImageInfoPtr(ImagePipeline::CreateImage(filePath, sequence, offset));
            }
            catch (CException* e)
            {
                ShutdownOnError(kLoad, filePath, e);
                e->Delete();
            }
            catch (exception& e)
            {
                ShutdownOnError(kFilter, pInfo, e);
            }
            TRACE2("Load image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), IsCancellationPending() ? L"(skipped)" : L"");
            return pInfo;
        }

        void ScaleImage(ImageInfoPtr pInfo, const SIZE& size) const
        {
            try
            {
                if (!IsCancellationPending() && (nullptr != pInfo))
                    ImagePipeline::ScaleImage(pInfo, size);
            }
            catch (CException* e)
            {
                ShutdownOnError(kScale, pInfo, e);
                e->Delete();
            }
            catch (exception& e)
            {
                ShutdownOnError(kFilter, pInfo, e);
            }
            TRACE2("Scale image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), IsCancellationPending() ? L"(skipped)" : L"");
        }

        void FilterImage(ImageInfoPtr pInfo, double noiseLevel) const
        {
            try 
            {
                if (!IsCancellationPending() && (nullptr != pInfo))
                    ImagePipeline::FilterImage(pInfo, noiseLevel);
            }
            catch (CException* e)
            {
                ShutdownOnError(kFilter, pInfo, e);
                e->Delete();
            }
            catch (exception& e)
            {
                ShutdownOnError(kFilter, pInfo, e);
            }
            TRACE2("Filter image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), IsCancellationPending() ? L"(skipped)" : L"");
        }

        void DisplayImage(ImageInfoPtr pInfo)
        {
            try
            {
                if (!IsCancellationPending() && (nullptr != pInfo))
                    ImagePipeline::DisplayImage(pInfo, this);                   // Side effect; sets the current image.
            }
            catch (CException* e)
            {
                ShutdownOnError(kDisplay, pInfo, e);
                e->Delete();
            }
            catch (exception& e)
            {
                ShutdownOnError(kFilter, pInfo, e);
            }
            TRACE2("Display image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), IsCancellationPending() ? L"(skipped)" : L"");
        }

    private:
        // Shutdown on error: Signal shutdown pending to pipeline, send message to error buffer and notify dialog that an error has been reported.

        void ShutdownOnError(Phases phase, const ImageInfoPtr pInfo, const CException* const e) const
        {
            ShutdownOnError(phase, pInfo->GetName(), e);
        }

        void ShutdownOnError(Phases phase, const wstring& filePath, const CException* const e) const
        {
            wstring message = GetExceptionMessage(e);
            SendError(phase, filePath, message);
        }

        void ShutdownOnError(Phases phase, const ImageInfoPtr pInfo, const exception& e) const
        {
            ShutdownOnError(phase, pInfo->GetName(), e);
        }

        void ShutdownOnError(Phases phase, const wstring& filePath, const exception& e) const
        {
            wostringstream message;
            message << e.what();
            SendError(phase, filePath, message.str());
        }
        
        void SendError(Phases phase, const wstring& filePath, wstring message) const
        {
            TRACE1("Exception thrown for image %d\n", filePath.c_str());
            send(m_shutdownPending, true);
            send(m_errorTarget, ErrorInfo(phase, filePath, message));
            PostMessageW(m_dialogWindow, WM_REPORTERROR, 0, 0);
        }

        wstring GetExceptionMessage(const CException* const e) const
        {
            const size_t maxLength = 255;
            TCHAR szCause[maxLength];
            e->GetErrorMessage(szCause, maxLength);
            wstring message(szCause);
            return message;
        }

        // Code for testing exception handling behavior within agents.
        //
        // To see error handling behavior add a call to AgentBase::ThrowCException() pr ThrowStdException()  
        // inside on or more of the try {...} block of; LoadImage, ScaleImage, FilterImage or DisplayImage
    public:
        static void ThrowCException(int throwSequence, const ImageInfoPtr pInfo)
        {
            if (nullptr == pInfo)
                return;
            if (pInfo->GetSequence() == throwSequence)
                AfxThrowNotSupportedException();
        }

        static void ThrowStdException(int throwSequence, const ImageInfoPtr pInfo)
        {
            if (nullptr == pInfo)
                return;
            if (pInfo->GetSequence() == throwSequence)
                throw std::logic_error("Pipeline error.");
        }
    };
};
