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
 * Created by Christian Leithner on 7/1/2024.
 */

#pragma once

#include "events/device-service-device-configuration-event.h"

G_BEGIN_DECLS

#define B_DEVICE_SERVICE_DEVICE_CONFIGURATION_COMPLETED_EVENT_TYPE                                                     \
    (b_device_service_device_configuration_completed_event_get_type())

G_DECLARE_FINAL_TYPE(BDeviceServiceDeviceConfigurationCompletedEvent,
                     b_device_service_device_configuration_completed_event,
                     B_DEVICE_SERVICE,
                     DEVICE_CONFIGURATION_COMPLETED_EVENT,
                     BDeviceServiceDeviceConfigurationEvent)

// No properties

/**
 * b_device_service_device_configuration_completed_event_new
 *
 * @brief
 *
 * Returns: (transfer full): BDeviceServiceDeviceConfigurationCompletedEvent*
 */
BDeviceServiceDeviceConfigurationCompletedEvent *b_device_service_device_configuration_completed_event_new(void);

G_END_DECLS
