/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */
#pragma once
#include "fep_participant_export.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "./../components/legacy/property_tree/fep_propertytree_intf.h"

namespace fep
{

    /**
     * participant interface
     */
    class FEP_PARTICIPANT_EXPORT IParticipant
    {
        protected:
            /// DTOR
            virtual ~IParticipant() = default;

        public:
            /**
             * retrieves the reference to the current fep component container
             */
            virtual IComponents& getComponents() const = 0;
            /**
             * blocking call until the participant has been shutdown from the system side
             */
            virtual fep::Result waitForShutdown() const = 0;
    };

    /**
     * Will register a user define participant component to the participant
     */
    template<class T>
    bool registerComponent(IParticipant& participant, IComponent* component)
    {
        return participant.getComponents().registerComponent<T>(component);
    }
    /**
     * Will unregister a user define participant component to the participant
     */
    template<class T>
    bool unregisterComponent(IParticipant& participant)
    {
        return participant.getComponents().unregisterComponent<T>();
    }
    /**
     * retrieves component from the participant
     */
    template<class T>
    T* getComponent(const IParticipant& participant)
    {
        return participant.getComponents().getComponent<T>();
    }

    /**
     * @brief helper function to set the property within the participant
     *
     * @tparam T Type of the property
     * @param participant the the participant where the property tree is part of
     * @param path named path to the porperty
     * @param value the value to set
     * @return fep::Result
     */
    template<typename T>
    fep::Result setProperty(IParticipant& participant, const char* path, const T& value)
    {
        return setProperty<T>(*participant.getComponents().getComponent<fep::IPropertyTree>(), path, value);
    }

    /**
     * @brief helper function to get a property
     *
     * @tparam T Type of the property
     * @param participant the participant
     * @param path named path to the porperty
     * @param default_value defeault value if property is not found
     * @return T
     */
    template<typename T>
    T getProperty(const IParticipant& participant, const char* path, const T& default_value = T())
    {
        return getProperty<T>(*participant.getComponents().getComponent<IPropertyTree>(), path, default_value);
    }
}
