/////////////////////////////////////////////////////////////
//
//  os.c
//
//  Simple cooperative task scheduler/processor
//

#include <os.h>
#include <assert.h>
#include <stdio.h>
#include <processor.h>

///////////////////////////////////////////////////
// Constants

#define MAX_TASKS 40

///////////////////////////////////////////////////
// Local types and macros

typedef struct TaskContext TaskContext;

struct TaskContext
{
    Task         task;
    void*        reference;
    TaskContext* next;
};

typedef struct
{
    TaskContext* first;
    TaskContext* last;
    unsigned     count;
} TaskQueue;

typedef struct
{
    TaskQueue freeContextBufferQueue;
    TaskQueue pendingHighTaskQueue;
    TaskQueue pendingLowTaskQueue;
    bool      runTaskProcessor;
} OsData;

///////////////////////////////////////////////////
// Local function prototypes

static void         osTaskProcessor();
static TaskContext* dequeueTaskContext(TaskQueue* taskQueue);
static void         enqueueTaskContext(TaskQueue* taskQueue, TaskContext* taskContext);

///////////////////////////////////////////////////
// Local data

static TaskContext staticTaskContexts[MAX_TASKS];

static OsData osData;

///////////////////////////////////////////////////
// Interface functions

void osInit()
{
    osData.freeContextBufferQueue.first = NULL;
    osData.freeContextBufferQueue.last  = NULL;
    osData.freeContextBufferQueue.count = 0;

    osData.pendingHighTaskQueue.first   = NULL;
    osData.pendingHighTaskQueue.last    = NULL;
    osData.pendingHighTaskQueue.count   = 0;
    
    osData.pendingLowTaskQueue.first    = NULL;
    osData.pendingLowTaskQueue.last     = NULL;
    osData.pendingLowTaskQueue.count    = 0;

    osData.runTaskProcessor             = true;
    
    int i;
    
    for(i = 0; i < MAX_TASKS; i++)
    {
        enqueueTaskContext(&osData.freeContextBufferQueue, &staticTaskContexts[i]);
    }
}

void osStart()
{
    osTaskProcessor();
}

bool scheduleTask(Task task, void* reference, bool high)
{
    ASSERT(task);

    printf("\nGot Schedule Task\n");

    TaskContext* taskContext = dequeueTaskContext(&osData.freeContextBufferQueue);
    
    if(taskContext)
    {
        taskContext->task      = task;
        taskContext->reference = reference;
        enqueueTaskContext(high ? &osData.pendingHighTaskQueue :
                                  &osData.pendingLowTaskQueue, taskContext);
        return true;
    }
    else
    {
        puts("Task Sched error");
        //TODO: Handle/report error
        return false;
    }
}

///////////////////////////////////////////////////
// Local functions

static void osTaskProcessor()
{
    // Loop processing scheduled tasks
    // Shouldn't return (outside of unit testing)

    while(osData.runTaskProcessor)
    {
        bool high = true;
        TaskContext* taskContext = dequeueTaskContext(&osData.pendingHighTaskQueue);
        
        if(!taskContext)
        {
            high = false;
            taskContext = dequeueTaskContext(&osData.pendingLowTaskQueue);
        }

        if(taskContext)
        {
            if(taskContext->task(taskContext->reference))
            {
                enqueueTaskContext(high ? &osData.pendingHighTaskQueue :
                                          &osData.pendingLowTaskQueue, taskContext);
            }
            else
            {
                enqueueTaskContext(&osData.freeContextBufferQueue, taskContext);
            }
        }

        // else - TODO: handle idle tasks here
    }
}

static TaskContext* dequeueTaskContext(TaskQueue* taskQueue)
{
    ASSERT(taskQueue);
    
    TaskContext* taskContext;
    
    CRData crdata = enterCriticalRegion();
    {
        taskContext = taskQueue->first;

        if(taskContext)
        {
            taskQueue->count--;
            taskQueue->first  = taskQueue->first->next;
            taskContext->next = NULL;
            
            if(!taskQueue->first)
            {
                taskQueue->last = NULL;
            }
        }
    }
    exitCriticalRegion(crdata);
    
    return taskContext;
}

static void enqueueTaskContext(TaskQueue* taskQueue, TaskContext* taskContext)
{
    ASSERT(taskQueue);
    ASSERT(taskContext);
    
    taskContext->next = NULL;

    CRData crdata = enterCriticalRegion();
    {
        taskQueue->count++;
        
        if(taskQueue->last)
        {
            taskQueue->last->next = taskContext;
        }
        else
        {
            taskQueue->first = taskContext;
        }

        taskQueue->last = taskContext;
    }
    exitCriticalRegion(crdata);
}

// EOF
