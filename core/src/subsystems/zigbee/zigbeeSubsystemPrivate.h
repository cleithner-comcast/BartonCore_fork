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
 * These are called internally by the event handler.
 *
 * Created by Thomas Lea on 3/29/16.
 */

#ifndef FLEXCORE_ZIGBEESUBSYSTEMPRIVATE_H_H
#define FLEXCORE_ZIGBEESUBSYSTEMPRIVATE_H_H

#include <stdbool.h>
#include <stdint.h>

void zigbeeSubsystemHandleZhalStartupEvent(void);

void zigbeeSubsystemDeviceDiscovered(IcDiscoveredDeviceDetails *details);

void zigbeeSubsystemAttributeReportReceived(ReceivedAttributeReport *report);

void zigbeeSubsystemClusterCommandReceived(ReceivedClusterCommand *command);

void zigbeeSubsystemDeviceLeft(uint64_t eui64);

void zigbeeSubsystemDeviceRejoined(uint64_t eui64, bool isSecure);

void zigbeeSubsystemLinkKeyUpdated(uint64_t eui64, bool isUsingHashBasedKey);

void zigbeeSubsystemApsAckFailure(uint64_t eui64);

void zigbeeSubsystemDeviceBeaconReceived(uint64_t eui64,
                                         uint16_t panId,
                                         bool isOpen,
                                         bool hasEndDeviceCapacity,
                                         bool hasRouterCapability,
                                         uint8_t depth);

void zigbeeSubsystemDeviceOtaUpgradeMessageSent(OtaUpgradeEvent *otaEvent);
void zigbeeSubsystemDeviceOtaUpgradeMessageReceived(OtaUpgradeEvent *otaEvent);

void zigbeeSubsystemRequestUnclaimedDevicesLeave(void);

/**
 * Set network configuration and bump counters. Call this prior to network initialization.
 * @param eui64 Network coordinator address
 * @param networkBlob base64 encoded configuration from/for ZigbeeCore (opaque)
 * @return whether all properties were set.
 */
bool zigbeeSubsystemSetNetworkConfig(uint64_t eui64, const char *networkBlob);

bool zigbeeSubsystemClaimDiscoveredDevice(IcDiscoveredDeviceDetails *details, DeviceMigrator *deviceMigrator);

/**
 * Add a premature cluster command
 * @param command the command to add
 */
void zigbeeSubsystemAddPrematureClusterCommand(const ReceivedClusterCommand *command);

#endif // FLEXCORE_ZIGBEESUBSYSTEMPRIVATE_H_H
