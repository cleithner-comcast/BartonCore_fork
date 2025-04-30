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

/*
 * Created by Thomas Lea on 11/15/21.
 */
#define LOG_TAG     "MatterLightDD"
#define logFmt(fmt) "(%s): " fmt, __func__
#include "subsystems/matter/MatterCommon.h"

#include "MatterDriverFactory.h"
#include "MatterLightDeviceDriver.h"
#include "clusters/OnOff.h"

extern "C" {
#include <commonDeviceDefs.h>
#include <device/deviceModelHelper.h>
#include <deviceServicePrivate.h>
#include <icLog/logging.h>
#include <resourceTypes.h>
#include <subsystems/zigbee/zigbeeCommonIds.h>
}

#include <chrono>
#include <subsystems/matter/DiscoveredDeviceDetailsStore.h>
#include <subsystems/matter/Matter.h>

using namespace barton;
using chip::Callback::Callback;
using namespace std::chrono_literals;

#define MATTER_LIGHT_DEVICE_DRIVER_NAME "matterLight"

// this is our endpoint, not the device's
#define LIGHT_ENDPOINT                  "1"

struct ClusterReadContext
{
    void *driverContext;                            // the context provided to the driver for the operation
    icInitialResourceValues *initialResourceValues; // non-null if this read is the initial resource fetch
    char **value;                                   // non-null if this read is a regular resource read
};

// auto register with the factory
bool MatterLightDeviceDriver::registeredWithFactory =
    MatterDriverFactory::Instance().RegisterDriver(new MatterLightDeviceDriver());

MatterLightDeviceDriver::MatterLightDeviceDriver() : MatterDeviceDriver(MATTER_LIGHT_DEVICE_DRIVER_NAME, LIGHT_DC, 0) {}

bool MatterLightDeviceDriver::ClaimDevice(DiscoveredDeviceDetails *details)
{
    icDebug();

    /*
     * Matter lighting example:
2022-02-24 05:58:33.313 : [Matter 709760] - INFO: MatterDriverFactory (GetDriver):
    {"vendor":"TEST_VENDOR","product":"TEST_PRODUCT","hwVer":"TEST_VERSION","swVer":"prerelease",
     "endpoints":[{"endpointId":0,"deviceTypes":[{"type":22,"revision":1}],
     "servers":[4,29,31,40,42,43,44,48,49,50,51,52,53,54,55,59,60,62,63,64,65],"clients":[41],"parts":[1,2]},{"endpointId":1,"deviceTypes":[{"type":257,"revision":1}],"servers":[3,4,6,8,29,768,1030],"clients":[],"parts":[]},{"endpointId":2,"deviceTypes":[],"servers":[],"clients":[],"parts":[]}]}
     */

    // see if any endpoint (not the special 0 entry) has our device id
    for (auto &entry : details->endpointDescriptorData)
    {
        if (entry.first > 0)
        {
            for (auto &deviceTypeEntry : *entry.second->deviceTypes)
            {
                switch (deviceTypeEntry)
                {
                    case ON_OFF_LIGHT_DEVICE_ID:
                    case ON_OFF_PLUGIN_UNIT_DEVICE_ID:
                    case DIMMABLE_LIGHT_DEVICE_ID:
                    case DIMMABLE_PLUGIN_UNIT_DEVICE_ID:
                    case COLOR_DIMMABLE_LIGHT_DEVICE_ID:
                    case COLOR_DIMMABLE2_LIGHT_DEVICE_ID:
                    case EXTENDED_COLOR_LIGHT_DEVICE_ID:
                    case EXTENDED_COLOR2_LIGHT_DEVICE_ID:
                    case COLOR_TEMPERATURE_LIGHT_DEVICE_ID:
                    case COLOR_TEMPERATURE2_LIGHT_DEVICE_ID:
                    case ON_OFF_LIGHT_SWITCH_DEVICE_ID:
                    case DIMMABLE_LIGHT_SWITCH_DEVICE_ID:
                    case COLOR_DIMMABLE_LIGHT_SWITCH_DEVICE_ID:
                        return true;

                    default:
                        break;
                }
            }
        }
    }

    return false;
}

void MatterLightDeviceDriver::OnOffChanged(std::string &deviceUuid, bool on, void *asyncContext)
{
    icDebug();

    auto server = static_cast<OnOff *>(GetAnyServerById(deviceUuid, ON_OFF_CLUSTER_ID));

    if (server == nullptr)
    {
        icError("OnOff cluster not present on device %s!", deviceUuid.c_str());
    }
    else
    {
        updateResource(
            deviceUuid.c_str(), LIGHT_ENDPOINT, LIGHT_PROFILE_RESOURCE_IS_ON, stringValueOfBool(on), nullptr);
    }
}

void MatterLightDeviceDriver::OnOffReadComplete(std::string &deviceUuid, bool isOn, void *asyncContext)
{
    icDebug();

    auto readContext = static_cast<ClusterReadContext *>(asyncContext);

    if (readContext->initialResourceValues)
    {
        initialResourceValuesPutEndpointValue(
            readContext->initialResourceValues, LIGHT_ENDPOINT, LIGHT_PROFILE_RESOURCE_IS_ON, isOn ? "true" : "false");
    }
    else
    {
        *readContext->value = strdup(stringValueOfBool(isOn));
    }

    OnDeviceWorkCompleted(readContext->driverContext, true);

    delete readContext;
}

std::vector<MatterCluster *> MatterLightDeviceDriver::GetClustersToSubscribeTo(const std::string &deviceId)
{
    icDebug();

    auto onOffServer = static_cast<OnOff *>(GetAnyServerById(deviceId, ON_OFF_CLUSTER_ID));
    if (onOffServer == nullptr)
    {
        icError("OnOff cluster not present on device %s!", deviceId.c_str());
        return {};
    }

    std::vector<MatterCluster *> clusters;
    clusters.push_back(onOffServer);
    return clusters;
}

void MatterLightDeviceDriver::SynchronizeDevice(std::forward_list<std::promise<bool>> &promises,
                                                const std::string &deviceId,
                                                chip::Messaging::ExchangeManager &exchangeMgr,
                                                const chip::SessionHandle &sessionHandle)
{
    icDebug();

    // currently we dont do anything during configuration except set up reporting, which also triggers an immediate
    //  report and that takes care of synchronizing state/resources as well.
    ConfigureDevice(promises, deviceId, nullptr, exchangeMgr, sessionHandle);
}

void MatterLightDeviceDriver::FetchInitialResourceValues(std::forward_list<std::promise<bool>> &promises,
                                                         const std::string &deviceId,
                                                         icInitialResourceValues *initialResourceValues,
                                                         chip::Messaging::ExchangeManager &exchangeMgr,
                                                         const chip::SessionHandle &sessionHandle)
{
    bool result = false;

    icDebug();

    auto server = static_cast<OnOff *>(GetAnyServerById(deviceId, ON_OFF_CLUSTER_ID));

    if (server == nullptr)
    {
        icError("OnOff cluster not present on device %s!", deviceId.c_str());
        FailOperation(promises);
        return;
    }

    promises.emplace_front();
    auto &readPromise = promises.front();
    auto readContext = new ClusterReadContext {};
    readContext->driverContext = &readPromise;
    readContext->initialResourceValues = initialResourceValues;
    AssociateStoredContext(&readPromise);

    if (!server->GetOnOff(readContext, exchangeMgr, sessionHandle))
    {
        AbandonDeviceWork(readPromise);
        delete readContext;
    }
}

bool MatterLightDeviceDriver::RegisterResources(icDevice *device, icInitialResourceValues *initialResourceValues)
{
    bool result = true;

    icDebug();

    icDeviceEndpoint *endpoint = createEndpoint(device, LIGHT_ENDPOINT, LIGHT_PROFILE, true);

    result &= createEndpointResource(endpoint,
                                     COMMON_ENDPOINT_RESOURCE_LABEL,
                                     "Matter Light",
                                     RESOURCE_TYPE_LABEL,
                                     RESOURCE_MODE_READWRITEABLE | RESOURCE_MODE_EMIT_EVENTS | RESOURCE_MODE_DYNAMIC,
                                     CACHING_POLICY_ALWAYS) != nullptr;

    auto *resource = static_cast<icDeviceResource *>(calloc(1, sizeof(icDeviceResource)));
    resource->id = strdup(LIGHT_PROFILE_RESOURCE_IS_ON);
    resource->endpointId = strdup(LIGHT_ENDPOINT);
    resource->deviceUuid = strdup(device->uuid);

    const char *initialValue =
        initialResourceValuesGetEndpointValue(initialResourceValues, LIGHT_ENDPOINT, LIGHT_PROFILE_RESOURCE_IS_ON);
    if (initialValue)
    {
        resource->value = strdup(
            initialResourceValuesGetEndpointValue(initialResourceValues, LIGHT_ENDPOINT, LIGHT_PROFILE_RESOURCE_IS_ON));
    }
    else
    {
        result = false;
    }
    resource->type = strdup(RESOURCE_TYPE_BOOLEAN);
    resource->mode = RESOURCE_MODE_READWRITEABLE | RESOURCE_MODE_DYNAMIC | RESOURCE_MODE_DYNAMIC_CAPABLE |
                     RESOURCE_MODE_EMIT_EVENTS | RESOURCE_MODE_LAZY_SAVE_NEXT;
    resource->cachingPolicy = CACHING_POLICY_ALWAYS;
    linkedListAppend(endpoint->resources, resource);

    return result;
}

void MatterLightDeviceDriver::ReadResource(std::forward_list<std::promise<bool>> &promises,
                                           const std::string &deviceId,
                                           icDeviceResource *resource,
                                           char **value,
                                           chip::Messaging::ExchangeManager &exchangeMgr,
                                           const chip::SessionHandle &sessionHandle)
{
    bool asyncCleanup = false;

    icDebug("%s", resource->id);

    if (resource->endpointId == nullptr || stringCompare(resource->id, LIGHT_PROFILE_RESOURCE_IS_ON, false) != 0)
    {
        FailOperation(promises);
        return;
    }

    auto server = static_cast<OnOff *>(GetAnyServerById(deviceId, ON_OFF_CLUSTER_ID));

    if (server == nullptr)
    {
        icError("OnOff cluster not present on device %s!", deviceId.c_str());
        FailOperation(promises);
        return;
    }

    promises.emplace_front();
    auto &readPromise = promises.front();
    auto readContext = new ClusterReadContext {};
    readContext->driverContext = &readPromise;
    readContext->value = value;

    AssociateStoredContext(&readPromise);

    if (!server->GetOnOff(readContext, exchangeMgr, sessionHandle))
    {
        AbandonDeviceWork(readPromise);
        delete readContext;
    }
}

bool MatterLightDeviceDriver::WriteResource(std::forward_list<std::promise<bool>> &promises,
                                            const std::string &deviceId,
                                            icDeviceResource *resource,
                                            const char *previousValue,
                                            const char *newValue,
                                            chip::Messaging::ExchangeManager &exchangeMgr,
                                            const chip::SessionHandle &sessionHandle)
{
    bool asyncCleanup = false;

    icDebug("%s = %s", resource->id, newValue);

    bool shouldUpdateResource = true;

    if (resource->endpointId == nullptr || stringCompare(resource->id, LIGHT_PROFILE_RESOURCE_IS_ON, false) != 0)
    {
        FailOperation(promises);
        return false;
    }

    auto server = static_cast<OnOff *>(GetAnyServerById(deviceId, ON_OFF_CLUSTER_ID));

    if (server == nullptr)
    {
        icError("OnOff cluster not present on device %s!", deviceId.c_str());
        FailOperation(promises);
        return false;
    }

    bool on;

    if (!stringToBoolStrict(newValue, &on))
    {
        icError("Invalid arguments");
        FailOperation(promises);
        return false;
    }

    promises.emplace_front();
    auto &setPromise = promises.front();
    AssociateStoredContext(&setPromise);

    if (!server->SetOnOff(&setPromise, on, exchangeMgr, sessionHandle))
    {
        AbandonDeviceWork(setPromise);
    }

    return shouldUpdateResource;
}

std::unique_ptr<MatterCluster> MatterLightDeviceDriver::MakeCluster(std::string const &deviceUuid,
                                                                    chip::EndpointId endpointId,
                                                                    chip::ClusterId clusterId)
{
    switch (clusterId)
    {
        case ON_OFF_CLUSTER_ID:
            return std::make_unique<OnOff>((OnOff::EventHandler *) this, deviceUuid, endpointId);

        default:
            return nullptr;
    }
}
