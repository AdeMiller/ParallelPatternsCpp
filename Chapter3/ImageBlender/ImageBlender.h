//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <iterator>
#include <memory>

#include <windows.h>
#include <GdiPlus.h>
#include <ppl.h>

#include "BitmapUtilities.h"
#include "SampleUtilities.h"

using namespace ::std;
using namespace ::Gdiplus;
using namespace ::Concurrency;
using namespace ::BitmapUtilities;
using namespace ::SampleUtilities;

class ImageBlender
{
private:
    static void SetToGray(Bitmap* const source, Bitmap* const layer)
    {
        wstring destDir(L"C:\\SrcTFS\\ParallelPatterns\\Source\\Samples\\cpp\\Debug");
        CopyBitmap(source, layer);
        SetGray(layer); 
        SetAlpha(layer, 128);
    }

    static void Rotate(Bitmap* const source, Bitmap* const layer)
    {
        CopyBitmap(source, layer);
        layer->RotateFlip(Rotate90FlipNone);
        SetAlpha(layer, 128);
    }

    static void Blend(Bitmap* const layer1, Bitmap* const layer2, Graphics* const blender)
    {
        blender->DrawImage(layer1, 0, 0);
        blender->DrawImage(layer2, 0, 0);
    }

    static void SequentialImageProcessing(Bitmap* const source1, Bitmap* const source2, 
        Bitmap* const layer1,  Bitmap* const layer2, 
        Graphics* const blender)
    {
        SetToGray(source1, layer1);
        Rotate(source2, layer2);
        Blend(layer1, layer2, blender);
    }

    static void ParallelStructuredTaskGroupImageProcessing(Bitmap* const source1, Bitmap* const source2,
        Bitmap* layer1, Bitmap* layer2, 
        Graphics* blender)
    {
        structured_task_group tasks;
        auto toGray = make_task([&source1, &layer1](){ SetToGray(source1, layer1); });
        tasks.run(toGray);
        tasks.run_and_wait([&source2, &layer2](){ Rotate(source2, layer2); });
        Blend(layer1, layer2, blender);
    }

    static void ParallelTaskGroupImageProcessing(Bitmap* const source1, Bitmap* const source2,
        Bitmap* layer1, Bitmap* layer2, 
        Graphics* blender)
    {
        task_group tasks;
        tasks.run([&source1, &layer1](){ SetToGray(source1, layer1);} );
        tasks.run_and_wait([&source2, &layer2](){ Rotate(source2, layer2); });
        Blend(layer1, layer2, blender);
    }

    static void ParallelInvokeImageProcessing(Bitmap* const source1, Bitmap* const source2, 
        Bitmap* layer1, Bitmap* layer2, 
        Graphics* blender)
    {
        parallel_invoke([&source1, &layer1](){ SetToGray(source1, layer1); },
            [&source2, &layer2](){ Rotate(source2, layer2);} );
        Blend(layer1, layer2, blender);
    }

public:
    static void DoWork(wstring path1, wstring path2, wstring destDir)
    {
        Concurrency::CurrentScheduler::Create(0);

        // Load source images
        auto source1 = unique_ptr<Bitmap>(new Bitmap(path1.c_str()));
        auto source2 = unique_ptr<Bitmap>(new Bitmap(path2.c_str()));

        // Create result images
        {
            auto layer1 = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto layer2 = unique_ptr<Bitmap>(new Bitmap(source2->GetWidth(), source2->GetHeight(), PixelFormat32bppARGB));
            auto result = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto blender = unique_ptr<Graphics>(Graphics::FromImage(result.get()));

            blender->SetCompositingMode(CompositingModeSourceOver);

            TimedRun([&source1, &source2, &layer1, &layer2, &blender]()
            { 
                SequentialImageProcessing(source1.get(), source2.get(), layer1.get(), layer2.get(), blender.get()); 
            },
                "            Sequential");

            SaveBitmap(layer1.get(), destDir, L"test_layer1.jpg");
            SaveBitmap(layer2.get(), destDir, L"test_layer2.jpg");
            SaveBitmap(result.get(), destDir, L"blended_sequential.jpg");
        }
        {
            auto layer1 = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto layer2 = unique_ptr<Bitmap>(new Bitmap(source2->GetWidth(), source2->GetHeight(), PixelFormat32bppARGB));
            auto result = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto blender = unique_ptr<Graphics>(Graphics::FromImage(result.get()));

            blender->SetCompositingMode(CompositingModeSourceOver);

            TimedRun([&source1, &source2, &layer1, &layer2, &blender]()
            { 
                ParallelTaskGroupImageProcessing(source1.get(), source2.get(), layer1.get(), layer2.get(), blender.get()); 
            },
                "            task_group");
            SaveBitmap(result.get(), destDir, L"blended_paralleltasks.jpg");
        }
        {
            auto layer1 = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto layer2 = unique_ptr<Bitmap>(new Bitmap(source2->GetWidth(), source2->GetHeight(), PixelFormat32bppARGB));
            auto result = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto blender = unique_ptr<Graphics>(Graphics::FromImage(result.get()));

            blender->SetCompositingMode(CompositingModeSourceOver);

            TimedRun([&source1, &source2, &layer1, &layer2, &blender]()
            { 
                ParallelStructuredTaskGroupImageProcessing(source1.get(), source2.get(), layer1.get(), layer2.get(), blender.get()); 
            },
                " structured_task_group");
            SaveBitmap(result.get(), destDir, L"blended_structuredtasks.jpg");
        }
        {
            auto layer1 = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto layer2 = unique_ptr<Bitmap>(new Bitmap(source2->GetWidth(), source2->GetHeight(), PixelFormat32bppARGB));
            auto result = unique_ptr<Bitmap>(new Bitmap(source1->GetWidth(), source1->GetHeight(), PixelFormat32bppARGB));
            auto blender = unique_ptr<Graphics>(Graphics::FromImage(result.get()));

            blender->SetCompositingMode(CompositingModeSourceOver);

            TimedRun([&source1, &source2, &layer1, &layer2, &blender]()
            { 
                ParallelInvokeImageProcessing(source1.get(), source2.get(), layer1.get(), layer2.get(), blender.get()); 
            },
                "       parallel_invoke");
            SaveBitmap(result.get(), destDir, L"blended_parallelinvoke.jpg");
        }
    }
};