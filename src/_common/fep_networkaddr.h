/**
 * Class to get information about network address and interfaces.
 * 
 * @file
 *

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
 *
 */

#ifndef _NETWORK_ADDRESS_H_
#define _NETWORK_ADDRESS_H_

#include <cstdint>
#include <iosfwd>
#include <string>

#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{
    namespace networkaddr
    {
        // forward declaration
        class cAddressPrivate;

        /**
        * Network address class
        */
        class FEP_PARTICIPANT_EXPORT cAddress
        {
        public:
            /// Default Ctor
            cAddress();

        public:
            /**
            * Copy constructor
            *
            * @param a [in] reference to cAddress to copy from.
            */
            cAddress(const cAddress& a);

        public:
            /**
            * Constructor
            * Accepts host name (e.g. localhost), ip.address (e.g. "127.0.0.1")
            * or interface name (e.g. "eth0").
            *
            * @param host_or_interface_name  [in] host or interface name
            */
            cAddress(const char* host_or_interface_name);

        public:
            /// Dtor
            ~cAddress();

        public:
            /// Assignment operator
            cAddress& operator=(const cAddress& a);

        public:
            /**
            * Check if a valid address is available
            *
            * @return True if network is valid.
            */
            bool isValid() const;

        public:
            /**
            * Get error code returned by last operation.
            *
            * @return Result code.
            */
            fep::Result GetLastResult() const;

        public:
            /**
            * Get host address as IPv4 address.
            * E.g. 127.0.0.1 is returned as 0x0100007f.
            *
            * @return The host address.
            */
            uint32_t GetHostAddr() const;

            /**
            * Get host address formatted as string
             *
            * @return The host address as string.
            */
            std::string GetHostAddrString() const;

            /**
            * Get broadcast address. 
            * E.g. for a host address 192.168.178.103 with a netmask 255.255.255.0
            * the broadcast address is 192.168.178.255.
            *
            * @return The broadcast address.
            */
            uint32_t GetBroadcastAddr() const;

            /**
            * Get netmask address.
            *
            * @return The netmask address.
            */
            uint32_t GetNetmaskAddr() const;

            /**
            * Get network address.
            * E.g. for a host address 192.168.178.103 with a netmask 255.255.255.0
            * the network address is 192.168.178.0.
            *
            * @return The network address.
            */
            uint32_t GetNetworkAddr() const;

            /**
            * Get interface name.
            * The interface name used by the operating system.
            *
            * @return The interface name.
            */
            std::string GetInterfaceName() const;

            /**
            * Get friendly name of interface.
            * If a more descriptive/friendly interface name is available.
            * Fall back to getInterfaceName.
            *
            * @return The friendly interface name.
            */
            std::string GetInterfaceFriendlyName() const;

            /**
            * Get interface description.
            *
            * @return The interface description.
            */
            std::string GetInterfaceDescription() const;

            /** 
            * Get best available interface name.
            *
            * @return The interface description, friendly name or name.
            */
            std::string GetInterfaceBestName() const;

        private:
            /**
            * Set internal interface.
            *
            * @param interface_name     [in] interface name to set.
            * @param with_special_names [in] allow usage of internal names, like "Default"
            *                       or "Ethernet-0" (matches first ethernet interface found).
            *
            * @return Result code.
            */
            fep::Result SetInterfaceInternal(const char* interface_name, bool with_special_names);

            /**
            * Set interface.
            *
            * @param interface_name     [in] interface name to set.
            *
            * @return Result code.
            */
            fep::Result SetInterface(const char* interface_name);
            
            /**
            * Set address.
            *
            * @param inaddr     [in] struct of address to set.
            *
            * @return Result code.
            */
            fep::Result SetAddress(struct in_addr *inaddr);
            
            /**
            * Set host name.
            *
            * @param host_name  [in] host name to set.
            *
            * @return Result code.
            */
            fep::Result SetHostname(const char* host_name);
            
         public:
            /**
            * Create address from interface name.
            *
            * @param interface_name  [in] the interface name.
            *
            * @return Instance of address class for the named interface 
            */
            static cAddress FromInterface(const char* interface_name);
            
             /**
            * Create address from host name.
            *
            * @param host_name  [in] the host name.
            *
            * @return Instance of address class for the named host 
            */
            static cAddress FromHostname(const char* host_name);
            
 
        private:
            /**
            * Print information of this address to an output stream
            *
            * @param oss the stream to which the information should be printed
            */
            void PrintToStream(std::ostream& oss) const;

        public:
            /**
             * Provides output to standard output stream.
             */
            friend std::ostream& operator<< (std::ostream& oss, const cAddress& address)
            {
                address.PrintToStream(oss);
                return oss;
            }

        private:
            /// pointer to internal implementation of cAddress
            cAddressPrivate* p;
        };

        // forward declaration
        class cAddressListPrivate;

        /**
        * List class for cAddress
        */
        class FEP_PARTICIPANT_EXPORT cAddressList
        {
        public:
            /**
            * Default constructor
            *
            * @param skip_loopback_and_unknown [in] flag to get only useful addresses (No loopback and unknown)
             */
            cAddressList(const bool skip_loopback_and_unknown= true);

            /**
            * Copy constructor
            *
            * @param other [in] reference to cAddressList to copy from.
            */
            cAddressList(const cAddressList& other);

            /// Dtor
            ~cAddressList();

        public:
            /**
            * Get size of cAddressList
            *
            * @return The number of elements inside list.
            */
            uint32_t Count();

            /**
            * Get interface at index \a no.
            * 
            * @param no [in] Index
            *
            * @return The interface name.
            */
            std::string GetInterface(uint32_t no);

            /**
            * Get address at index \a no.
            *
            * @param no [in] Index
            *
            * @return The address class instance.
            */
            cAddress GetAddress(uint32_t no);

        public:
            /// Assignment operator
            cAddressList& operator=(const cAddressList& other);

        private:
            /**
            * Print information of this address list to an output stream
            *
            * @param oss the stream to which the information should be printed
            */
            void PrintToStream(std::ostream& oss) const;

        public:
            /**
             * Provides output to standard output stream.
             */
            friend std::ostream& operator<< (std::ostream& oss, const cAddressList& oAddressList)
            {
                oAddressList.PrintToStream(oss);
                return oss;
            }

        private:
            /// pointer to internal implementation of cAddressList
            cAddressListPrivate* p;
        };
    }
}

#endif // _NETWORK_ADDRESS_H_
