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
const std::string s_strDescription = 
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation>07.04.2010</date_creation>"
    "        <date_change>07.04.2010</date_change>"
    "        <description>ADTF Common Description File</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
    "        <datatype name=\"tBool\" size=\"8\" />"
    "        <datatype name=\"tChar\" size=\"8\" />"
    "        <datatype name=\"tUInt8\" size=\"8\" />"
    "        <datatype name=\"tInt8\" size=\"8\" />"
    "        <datatype name=\"tUInt16\" size=\"16\" />"
    "        <datatype name=\"tInt16\" size=\"16\" />"
    "        <datatype name=\"tUInt32\" size=\"32\" />"
    "        <datatype name=\"tInt32\" size=\"32\" />"
    "        <datatype name=\"tUInt64\" size=\"64\" />"
    "        <datatype name=\"tInt64\" size=\"64\" />"
    "        <datatype name=\"tFloat32\" size=\"32\" />"
    "        <datatype name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "        <struct alignment=\"1\" name=\"tTestSignal1\" version=\"2\">"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
    "        </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>";

const std::string s_strDescription2 =
"<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
"<adtf:ddl xmlns:adtf=\"adtf\">"
"    <header>"
"        <language_version>3.00</language_version>"
"        <author>AUDI AG</author>"
"        <date_creation>07.04.2010</date_creation>"
"        <date_change>07.04.2010</date_change>"
"        <description>ADTF Common Description File</description>"
"    </header>"
"    <units>"
"    </units>"
"    <datatypes>"
"        <datatype name=\"tBool\" size=\"8\" />"
"        <datatype name=\"tChar\" size=\"8\" />"
"        <datatype name=\"tUInt8\" size=\"8\" />"
"        <datatype name=\"tInt8\" size=\"8\" />"
"        <datatype name=\"tUInt16\" size=\"16\" />"
"        <datatype name=\"tInt16\" size=\"16\" />"
"        <datatype name=\"tUInt32\" size=\"32\" />"
"        <datatype name=\"tInt32\" size=\"32\" />"
"        <datatype name=\"tUInt64\" size=\"64\" />"
"        <datatype name=\"tInt64\" size=\"64\" />"
"        <datatype name=\"tFloat32\" size=\"32\" />"
"        <datatype name=\"tFloat64\" size=\"64\" />"
"    </datatypes>"
"    <enums>"
"    </enums>"
"    <structs>"
"        <struct alignment=\"1\" name=\"tTestSignal1\" version=\"2\">"
"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
"        </struct>"
"        <struct alignment=\"1\" name=\"tTestSignal2\" version=\"2\">"
"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
"            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
"        </struct>"
"    </structs>"
"    <streams>"
"    </streams>"
"</adtf:ddl>";