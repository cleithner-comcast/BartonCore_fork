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

/**
 * Created by Raiyan Chowdhury on 12/05/23.
 */

#include "SubscribeInteraction.h"
#include <lib/support/FibonacciUtils.h>

using namespace barton;

void SubscribeInteraction::OnSubscriptionEstablished(chip::SubscriptionId aSubscriptionId)
{
    mOnResubscriptionsAttempted = 0;
    eventHandler->OnSubscriptionEstablished(aSubscriptionId, subscriptionPromise, clusterStateCache);
}

CHIP_ERROR SubscribeInteraction::OnResubscriptionNeeded(chip::app::ReadClient *apReadClient,
                                                        CHIP_ERROR aTerminationCause)
{
    mOnResubscriptionsAttempted++;
    auto timeTillNextResubscription = CustomComputeTimeTillNextSubscription();

    ChipLogProgress(DataManagement,
                    "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
                    "ms due to error %" CHIP_ERROR_FORMAT,
                    apReadClient->GetFabricIndex(),
                    ChipLogValueX64(apReadClient->GetPeerNodeId()),
                    mOnResubscriptionsAttempted,
                    timeTillNextResubscription,
                    aTerminationCause.Format());

    return apReadClient->ScheduleResubscription(
        timeTillNextResubscription, chip::NullOptional, aTerminationCause == CHIP_ERROR_TIMEOUT);
}

uint32_t SubscribeInteraction::CustomComputeTimeTillNextSubscription()
{
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (mOnResubscriptionsAttempted <= CUSTOM_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX)
    {
        maxWaitTimeInMsec =
            chip::GetFibonacciForIndex(mOnResubscriptionsAttempted) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS;
    }
    else
    {
        maxWaitTimeInMsec = CUSTOM_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS;
    }

    if (maxWaitTimeInMsec != 0)
    {
        minWaitTimeInMsec = (CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP * maxWaitTimeInMsec) / 100;
        waitTimeInMsec = minWaitTimeInMsec + (chip::Crypto::GetRandU32() % (maxWaitTimeInMsec - minWaitTimeInMsec));
    }

    return waitTimeInMsec;
}

void SubscribeInteraction::AbandonSubscription()
{
    eventHandler->AbandonSubscription(subscriptionPromise);
}

bool SubscribeInteraction::SetLivenessCheckMillis(uint32_t livenessMillis)
{
    bool ok = false;

    if (readClient->GetSubscriptionId().HasValue())
    {
        readClient->OverrideLivenessTimeout(chip::System::Clock::Milliseconds32(livenessMillis));
        ok = true;
    }

    return ok;
}

void SubscribeInteraction::OnAttributeChanged(chip::app::ClusterStateCache *cache,
                                              const chip::app::ConcreteAttributePath &path)
{
    eventHandler->OnAttributeChanged(cache, path, deviceUuid);
}

void SubscribeInteraction::OnEventData(const chip::app::EventHeader &aEventHeader,
                                       chip::TLV::TLVReader *apData,
                                       const chip::app::StatusIB *apStatus)
{
    eventHandler->OnEventData(aEventHeader, apData, apStatus, deviceUuid, *this);
}

void SubscribeInteraction::OnDeallocatePaths(chip::app::ReadPrepareParams &&params)
{
    delete params.mpEventPathParamsList;
    delete params.mpAttributePathParamsList;
}
