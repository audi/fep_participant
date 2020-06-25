::
:: Copyright @ 2019 Audi AG. All rights reserved.
:: 
::     This Source Code Form is subject to the terms of the Mozilla
::     Public License, v. 2.0. If a copy of the MPL was not distributed
::     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
:: 
:: If it is not possible or desirable to put the notice in a particular file, then
:: You may include the notice in a location (such as a LICENSE file in a
:: relevant directory) where a recipient would be likely to look for such a notice.
:: 
:: You may add additional accurate notices of copyright ownership.
::
start /B ..\..\..\bin\fep_launcher --name timing_example --system timing_30_example.fep_system --element timing_master_participant.fep_element --element observer_participant.fep_element --element environment_participant.fep_element --element driver_participant.fep_element --element sensor_back_participant.fep_element --element sensor_front_participant.fep_element --configuration timing_30_example.fep2_launch_config --accept_launched_participants --properties properties/simulation_time.fep_properties