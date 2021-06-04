
Code samples for Parallel Programming with Microsoft® Visual C++
 
Use the BookSamples solution.  Here are descriptions of the projects, grouped by solution folder.

Sample timings here are from an Intel i7 Q720 (4 hyperthreaded cores), 1.60 GHz, 32 KB L1 cache, 256 KB L2 cache, 
6 MB L3 cache and 16 GB RAM. Running Windows 7 64bit.

Sample timings are from Release builds.  Each sample timing is from one typical run, can be quite variable.

Appendix A
----------

SchedulerExamples

Small examples from Appendix A in the book.

Appendix B
----------

ProfilerExamples

Small examples from Appendix B in the book. These examples are designed to be used in conjunction 
with the Concurrency Visualizer that ships with Microsoft Visual Studio 2010 Ultimate or Premium editions.

Command line arguments: deadlock, lockcontention, oversubscription, loadimbalance

Chapter2
--------

BasicParallelLoops

Small examples from Chapter 2 in the book.

CreditReview

Credit review example from Chapter 2 in the book.  Builds CreditReview.exe console application.
Creates repository of customers with credit histories, predicts future balances for all customers
by fitting list-squares regression line to credit history, extrapolating trend for next three months.
Generates random credit histories, but always uses the same random seed so runs are reproducible.
Creates repository, then executes and times two versions: sequential for_each, and parallel_for_each.
Prints number of customers, number of months, timings, also histories and predictions for several customers.
Defaults: 2,000,000 customers, each with 36 months credit history.

Sample timings: 

    Sequential : 817.01 ms
      Parallel : 343.04 ms

Optional command line arguments: number of customers, number of months of credit history

Chapter3
--------

BasicParallelTasks

Small examples from Chapter 3 in the book.

ImageBlender

Image blender example from Chapter 3 in the book.  Builds ImageBlender.exe console application.
Reads two images, rotates one, converts the other to grayscale, combines them by alpha blending.
The example executes and times four versions: sequential, task_group, structured_task_group and parallel_invoke.
Prints timings, then opens window and displays blended image.
Defaults: rotates 600 Kb 1024x768 24 bit image (dog.jpg) and converts 320 Kb 1024x681 24 bit image (flowers.jpg)

            Sequential : 444.05 ms
            task_group : 211.56 ms
 structured_task_group : 224.51 ms
       parallel_invoke : 231.73 ms

Optional command line arguments: source directory, image 1 filename, image 2 filename, destination directory.

Chapter4
--------

BasicAggregation

Small examples from Chapter 4 in the book.

SocialNetwork

Social network example from Chapter 4 in the book.  Builds SocialNetwork.exe console application.
Creates repository of subscriber IDs with friends IDs, recommends new friends for one subscriber
by finding all friends-of-friends, ranking them by number of mutual friends, recommending highest ranked.
For each subscriber, generates number and IDs of friends at random, but uses same seed for reproducible runs.
Creates repository, then executes and times three versions: sequential for_each, sequential transform and parallel_transform.
Prints table of first several subscribers: subscriber ID, number of friends, IDs of first several friends.
Prints total number of subscribers and average number of friends per subscriber.
Prints ID of one subscriber and IDs of all that suscriber's (many) friends.
Prints elapsed time and list of top-ranked recommended new friends for each version.
Defaults: 25,000 subscribers with average of 2,000 friends each

Sample timings: 

            Sequential : 2068.51 ms
  Sequential transform : 2009.78 ms
              Parallel :  893.21 ms

Optional command line arguments: number of subscribers, average number of friends.

Chapter5
--------

BasicFutures

Small examples from Chapter 5 in the book.

A-Dash

Financial dashboard example from Chapter 5 in the book.  Builds ADash.exe Windows application.
Demonstrates task graph and futures. Opens window with dashboard GUI. Check the parallel checkbox
to run the parallel version of the calculation. Click Calculate to start tasks.

Status window shows "..calculating" as tasks execute. Click Cancel to stop tasks. 
Buttons become active as tasks complete. Click an active button to view data from that task 
(just a brief message, there is no actual data in this sample). When all tasks are complete, 
status window shows recommendation. Click Quit to close window and exit application.
Defaults: The recommendation is always Buy.

Sample timings: After clicking Calculate, takes about 10 sec to finish all tasks.

Optional command line arguments: None.

Chapter6
--------

BasicDynamicTasks

Small examples from Chapter 6 in the book.

ParallelSort

Parallel QuickSort example from Chapter 6 in book. Builds ParallelSort.exe console application.
Creates array with integer elements, prints first and last several elements. 
Generates random array elements, but always uses the same random seed so runs are reproducible.
Executes sequential and parallel QuickSort, prints timings and first and last several elements of sorted array.
Defaults: Array length: 40,000,000  Threshold array length to use non-recursive insertion sort: 256
Maximum recursion depth where recursive calls are made in new tasks: log2(ProcessorCount) + 4

Sample timings: 

               Sequential : 7574.96 ms
                 Parallel : 2458.15 ms
  Parallel with std::sort : 2075.03 ms

Optional command line arguments: array length, threshold length for insertion sort.

Chapter7 
--------

BasicPipeline

Small examples from Chapter 7 in the book.

ImagePipeline

Image pipeline sample from Chapter 7 in book.  Builds ImagePipeline.exe Windows application.

The program cycles through the jpg images located in a directory and performs a series of steps: 
it resizes each image and adds a black border and then applies a Gaussian noise filter operation to give 
the image a grainy effect. Finally, the program displays the image on the user interface.
Images are processed in sequential order: they appear on the display in exactly the same order
as they appear in the directory listing.

The program opens a window that provides some controls, displays the most recently processed image,
and shows some performance statistics.  Select a processing mode: Sequential, Pipelined, or Load Balanced.  
Click Start to process images. The program will continue processing, cycling through the images in the directory.
Click Stop to stop processing so you can select another mode or study the statistics.  
Click Quit to close the window and exit the program.

Note that the UI will refresh as often as possible but for very high image refresh rates the window will
not refresh for every image.

Defaults: there are 3 24-bit images: dog.jpg (1024x768, 600 KB), flowers.jpg (1024x681 300KB) 
and butterfly.jpg (1024x681, 496 KB)

Sample timings, Time per image: 

       Sequential : 208 ms
       Data flow  : 179 ms
     Control flow : 180 ms
    Load Balanced :  66 ms

Optional command line arguments: None.

Utilities
---------

Types and methods used by other projects.
