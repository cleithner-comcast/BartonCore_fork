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

#define G_LOG_DOMAIN "deviceServiceCommFail"
#define LOG_TAG      G_LOG_DOMAIN
#define logFmt(fmt)  "(%s)" fmt, __func__

#include "deviceServiceCommFail.h"
#include "commonDeviceDefs.h"
#include "device-driver/device-driver.h"
#include "device/icDevice.h"
#include "deviceCommunicationWatchdog.h"
#include "deviceDriverManager.h"
#include "deviceServicePrivate.h"
#include "glib.h"
#include "icLog/logging.h"
#include "inttypes.h"
#include "stdint.h"

static void deviceCommFailCallback(const char *uuid)
{
    scoped_icDevice *device = deviceServiceGetDevice(uuid);

    if (device != NULL)
    {
        const DeviceDriver *driver = deviceDriverManagerGetDeviceDriver(device->managingDeviceDriver);

        if (driver != NULL && driver->communicationFailed != NULL)
        {
            driver->communicationFailed(driver->callbackContext, device);
        }
    }
}

static void deviceCommRestoreCallback(const char *uuid)
{
    scoped_icDevice *device = deviceServiceGetDevice(uuid);

    if (device != NULL)
    {
        const DeviceDriver *driver = deviceDriverManagerGetDeviceDriver(device->managingDeviceDriver);

        if (driver != NULL && driver->communicationRestored != NULL)
        {
            driver->communicationRestored(driver->callbackContext, device);
        }

        updateDeviceDateLastContacted(uuid);
    }
}

void deviceServiceCommFailInit(void)
{
    deviceCommunicationWatchdogInit(deviceCommFailCallback, deviceCommRestoreCallback);
}

void deviceServiceCommFailShutdown(void)
{
    deviceCommunicationWatchdogTerm();
}

static uint32_t getDeviceCommFailTimeoutSecs(const icDevice *device, uint32_t defaulTimeoutSecs)
{
    if (device == NULL)
    {
        return defaulTimeoutSecs;
    }

    const char *tmp = deviceGetMetadata(device, COMMON_DEVICE_METADATA_COMMFAIL_OVERRIDE_SECS);
    uint32_t commFailTimeoutSecs = defaulTimeoutSecs;

    if (tmp != NULL)
    {
        uint64_t commFailOverrideSecs;
        g_autoptr(GError) parseError = NULL;

        g_ascii_string_to_unsigned(tmp, 10, 0, UINT32_MAX, &commFailOverrideSecs, &parseError);

        if (parseError == NULL)
        {
            commFailTimeoutSecs = commFailOverrideSecs;
        }
        else
        {
            icWarn("Unable to parse '%s' metadata for device '%s': %s",
                   COMMON_DEVICE_METADATA_COMMFAIL_OVERRIDE_SECS,
                   device->uuid,
                   parseError->message);
        }
    }

    return commFailTimeoutSecs;
}

void deviceServiceCommFailHintDeviceTimeoutSecs(const icDevice *device, uint32_t defaultTimeoutSecs)
{
    DeviceDriver *driver = deviceDriverManagerGetDeviceDriver(device->managingDeviceDriver);

    if (driver != NULL && driver->commFailTimeoutSecsChanged != NULL)
    {
        driver->commFailTimeoutSecsChanged(driver, device, getDeviceCommFailTimeoutSecs(device, defaultTimeoutSecs));
    }
}

void deviceServiceCommFailSetDeviceTimeoutSecs(const icDevice *device, uint32_t defaultTimeoutSecs)
{
    DeviceDriver *driver = deviceDriverManagerGetDeviceDriver(device->managingDeviceDriver);

    if (driver == NULL)
    {
        return;
    }

    uint32_t commFailTimeoutSecs = getDeviceCommFailTimeoutSecs(device, defaultTimeoutSecs);

    if (driver->commFailTimeoutSecsChanged != NULL)
    {
        driver->commFailTimeoutSecsChanged(driver, device, commFailTimeoutSecs);
    }

    if (driver->customCommFail)
    {
        return;
    }

    if (deviceCommunicationWatchdogIsDeviceMonitored(device->uuid))
    {
        deviceCommunicationWatchdogStopMonitoringDevice(device->uuid);
    }

    if (commFailTimeoutSecs != 0)
    {
        const bool isCommFail =
            g_strcmp0(deviceGetResourceValueById(device, COMMON_DEVICE_RESOURCE_COMM_FAIL), "true") == 0;

        deviceCommunicationWatchdogMonitorDevice(device->uuid, commFailTimeoutSecs, isCommFail);
    }
    else
    {
        icInfo("Device communication watchdog disabled for %s %s", device->deviceClass, device->uuid);
    }
}

static void setDriverCommFailTimeoutSecs(DeviceDriver *driver, uint32_t defaultTimeoutSecs)
{
    scoped_icDeviceList *devices = deviceServiceGetDevicesByDeviceDriver(driver->driverName);
    scoped_icLinkedListIterator *deviceIt = linkedListIteratorCreate(devices);

    while (linkedListIteratorHasNext(deviceIt))
    {
        const icDevice *device = linkedListIteratorGetNext(deviceIt);

        deviceServiceCommFailSetDeviceTimeoutSecs(device, defaultTimeoutSecs);
    }
}

#if UINT_MAX < UINT32_MAX
#error "LP32 not supported"
#endif

/* Atomic: g_atomic_int_get/set() */
static unsigned int defaultCommFailTimeoutSecs;

void deviceServiceCommFailSetTimeoutSecs(uint32_t commFailTimeoutSecs)
{
    scoped_icLinkedListNofree *drivers = deviceDriverManagerGetDeviceDrivers();
    scoped_icLinkedListIterator *driverIt = linkedListIteratorCreate(drivers);
    g_atomic_int_set(&defaultCommFailTimeoutSecs, commFailTimeoutSecs);

    while (linkedListIteratorHasNext(driverIt))
    {
        setDriverCommFailTimeoutSecs(linkedListIteratorGetNext(driverIt), commFailTimeoutSecs);
    }
}

uint32_t deviceServiceCommFailGetTimeoutSecs(void)
{
    return g_atomic_int_get(&defaultCommFailTimeoutSecs);
}

uint32_t deviceServiceCommFailGetDeviceTimeoutSecs(const icDevice *device)
{
    return getDeviceCommFailTimeoutSecs(device, g_atomic_int_get(&defaultCommFailTimeoutSecs));
}
