# ------------------------------ tabstop = 4 ----------------------------------
#
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2025 Comcast Cable Communications Management, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#
# ------------------------------ tabstop = 4 ----------------------------------

#
# Created by Christian Leithner on 1/30/2025.
#


from dataclasses import dataclass, field

import pytest
from testing.mocks.zhal.events.responses.response import Response


@dataclass
class NetworkInitializeResponse(Response):
    """
    NetworkInitializeResponse represents the network initialize response.
    """

    _response_type: str = field(default="networkInitResponse", init=False)


@pytest.fixture
def network_initialize_response(request):
    network_initialize_response = NetworkInitializeResponse.from_dict(request.param)
    try:
        yield network_initialize_response
    finally:
        pass
