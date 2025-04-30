//------------------------------ tabstop = 4 ----------------------------------
//
// If not stated otherwise in this file or this component's LICENSE file the
// following copyright and licenses apply:
//
// Copyright 2024 Comcast Cable Communications Management, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//
//------------------------------ tabstop = 4 ----------------------------------

//
// Created by Christian Leithner on 12/9/20
//

#include "icConcurrent/icDebounce.h"
#include "icConcurrent/delayedTask.h"
#include <stdlib.h>

struct _icDebounce
{
    uint32_t taskHandle;

    debounceExpiredFunc expiredCallback;
    void *context;
    uint64_t timeoutMillis;
};

extern inline void debounceDestroy__auto(icDebounce **debounce);

/**
 * Create an icDebounce type. This type is a reusable timer that invokes a callback on timer expiration.
 * The timer can be reset on debounce.
 *
 * @param callback - The expire callback to invoke once the timer expires
 * @param context - An optional context object to pass to the expire callback
 * @param timeoutMillis - The amount of time, in milliseconds, the icDebounce timer should run for
 *
 * @return the new icDebounce
 */
icDebounce *debounceCreate(debounceExpiredFunc callback, void *context, uint64_t timeoutMillis)
{
    icDebounce *retVal = NULL;
    if (callback != NULL)
    {
        retVal = calloc(1, sizeof(struct _icDebounce));
        if (retVal != NULL)
        {
            retVal->expiredCallback = callback;
            retVal->context = context;
            retVal->timeoutMillis = timeoutMillis;
        }
    }

    return retVal;
}

/**
 * Destroys an icDebounce type.
 *
 * @param debounce - The icDebounce type to destroy
 */
void debounceDestroy(icDebounce *debounce)
{
    if (debounce != NULL)
    {
        cancelDelayTask(debounce->taskHandle);
        free(debounce);
    }
}

/**
 * Determines is an icDebounce timer is running.
 *
 * @param debounce - The icDebounce type
 * @return true if the icDebounce timer is running, false otherwise
 */
bool debounceIsRunning(icDebounce *debounce)
{
    bool retVal = false;
    if (debounce != NULL)
    {
        retVal = isDelayTaskWaiting(debounce->taskHandle);
    }

    return retVal;
}

/**
 * Notify an icDebounce that it should reschedule its timer if it is running, or start it fresh if it is not.
 *
 * @param debounce - The icDebounce type that's timer should be started/reset
 * @return true if the icDebounce timer was started or rescheduled, false if an error was encountered
 */
bool debounce(icDebounce *debounce)
{
    bool retVal = false;

    if (debounce != NULL)
    {
        // Try rescheduling first if the handle exists
        if ((retVal = rescheduleDelayTask(debounce->taskHandle, debounce->timeoutMillis, DELAY_MILLIS)) == false)
        {
            // Handle doesn't exist, so schedule a new one
            debounce->taskHandle =
                scheduleDelayTask(debounce->timeoutMillis, DELAY_MILLIS, debounce->expiredCallback, debounce->context);
            retVal = (debounce->taskHandle != 0);
        }
    }

    return retVal;
}
