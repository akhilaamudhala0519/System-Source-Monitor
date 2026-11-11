#include "monitor.h"
#include <string.h>

BOOL ThreadSync_Initialize(MonitorContext *context)
{
    if (context == NULL) {
        return FALSE;
    }

    InitializeCriticalSection(&context->lock);
    context->initialized = TRUE;
    return TRUE;
}

void ThreadSync_Destroy(MonitorContext *context)
{
    if (context != NULL && context->initialized) {
        DeleteCriticalSection(&context->lock);
        context->initialized = FALSE;
    }
}

void ThreadSync_Publish(MonitorContext *context, const SystemMetrics *metrics)
{
    if (context == NULL || metrics == NULL || !context->initialized) {
        return;
    }

    EnterCriticalSection(&context->lock);
    context->metrics = *metrics;
    LeaveCriticalSection(&context->lock);
}

void ThreadSync_Copy(MonitorContext *context, SystemMetrics *metrics)
{
    if (context == NULL || metrics == NULL || !context->initialized) {
        return;
    }

    EnterCriticalSection(&context->lock);
    *metrics = context->metrics;
    LeaveCriticalSection(&context->lock);
}
