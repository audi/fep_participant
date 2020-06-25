/**
 * Implementation of the tester for the FEP Property Tree extensions (serialization test)
 *
 * @file

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

/*
* Test Case:   TestConfiguration
* Test ID:     1.1
* Test Title:  Test serialization of properties
* Description: Tests if serialization especially the json library work correcly under various environments (locales)
* Strategy:    Evaluate various serialization strings are matching expectations. If possible do deserialization.
* Passed If:   no errors occur
* Ticket:      #30867
* Requirement: FEPSDK-1629 FEPSDK-1628 FEPSDK-1630 FEPSDK-1631
*/


#ifdef WIN32
// Disable min/max macros. They collide with std::numeric_limits. 
#define NOMINMAX
#endif // WIN32

#include <algorithm>
#include <iostream>
#include <limits>
#include <gtest/gtest.h>

#include "fep3/participant/default_participant.h"
#include "fep3/rpc_components/configuration/configuration_rpc_intf_def.h"
#include "fep3/rpc_components/configuration/configuration_service_client.h"
#include "fep3/components/rpc/fep_rpc.h"

#include <a_util/strings.h>

using namespace fep;

struct ConfigTestHelper
{
    ParticipantFEP2 partWhereTheClientIs;
    ParticipantFEP2 partWhereThePropertyTreeIs;
    std::unique_ptr<fep::rpc_object_client<rpc_stubs::RPCConfigurationClient,
                           fep::rpc::IRPCConfigurationDef>> _stub;
    fep::IPropertyTree* _target_tree = nullptr;
    void prepareForTest()
    {
        partWhereThePropertyTreeIs.Create("partWhereThePropertyTreeIs");
        _target_tree = getComponent<fep::IPropertyTree>(partWhereThePropertyTreeIs);

        partWhereTheClientIs.Create("partWhereTheClientIs");
        _stub.reset(new fep::rpc_object_client<rpc_stubs::RPCConfigurationClient,
            fep::rpc::IRPCConfigurationDef>("partWhereThePropertyTreeIs",
                fep::rpc::IRPCConfigurationDef::getRPCDefaultName(),
                *getComponent<fep::IRPC>(partWhereTheClientIs)));
    }
};

bool containsSubNodes(std::string subnodes, std::vector<std::string> checklist)
{
    std::vector<std::string> subnodelist = a_util::strings::split(subnodes, ",");
    for (auto& checksubnodename : checklist)
    {
        bool isinlist = false;
        for (auto& currentsubnode : subnodelist)
        {
            if (currentsubnode == checksubnodename)
            {
                isinlist = true;
            }
        }
        if (!isinlist)
        {
            return false;
        }
    }
    return true;
}

std::string useSlashes(std::string dotted_string)
{
    return a_util::strings::replace(dotted_string, ".", "/");
}

#define TEST_PROP_NAME "test_prop_name"
#define TEST_PROP_NODE "test_prop_node"
#define TEST_PROP_NODE_NAME  TEST_PROP_NODE "." TEST_PROP_NAME

#define TEST_PROP_ROOT_NODE "/"

//setting template
template<typename T>
std::string testGettingOf(ConfigTestHelper& test_helper, const T& val, const std::string& prop_path)
{
    T target_tree_value = val;

    test_helper._target_tree->SetPropertyValue(prop_path.c_str(), target_tree_value);

    std::string property_path_used = "";
    property_path_used += prop_path;

    auto far_value_1 = test_helper._stub->getProperty(useSlashes(property_path_used));
    auto far_config_value_1 = far_value_1["value"].asString();
    return far_config_value_1;
}


/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationGetter)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    { //bool false
        bool target_tree_value = false;

        ASSERT_EQ(a_util::strings::toBool(testGettingOf(test_helper, target_tree_value, TEST_PROP_NAME)), target_tree_value);
        ASSERT_EQ(a_util::strings::toBool(testGettingOf(test_helper, target_tree_value, TEST_PROP_NODE_NAME)), target_tree_value);
    }
    { //bool true
        bool target_tree_value = true;

        ASSERT_EQ(a_util::strings::toBool(testGettingOf(test_helper, target_tree_value, TEST_PROP_NAME)), target_tree_value);
        ASSERT_EQ(a_util::strings::toBool(testGettingOf(test_helper, target_tree_value, TEST_PROP_NODE_NAME)), target_tree_value);
    }
    { //integer
        int32_t target_tree_value = 12345;

        ASSERT_EQ(a_util::strings::toInt32(testGettingOf(test_helper, target_tree_value, TEST_PROP_NAME "int")), target_tree_value);
        ASSERT_EQ(a_util::strings::toInt32(testGettingOf(test_helper, target_tree_value, TEST_PROP_NODE_NAME "int")), target_tree_value);
    }
    { //double
        double target_tree_value = 12345.12345;

        ASSERT_EQ(a_util::strings::toDouble(testGettingOf(test_helper, target_tree_value, TEST_PROP_NAME "DOUBLE")), target_tree_value);
        ASSERT_EQ(a_util::strings::toDouble(testGettingOf(test_helper, target_tree_value, TEST_PROP_NODE_NAME "DOUBLE")), target_tree_value);
    }

    { //string
        std::string target_tree_value = "this_is_a_test_string";

        test_helper._target_tree->SetPropertyValue(TEST_PROP_NAME "STRING", target_tree_value.c_str());
        test_helper._target_tree->SetPropertyValue(TEST_PROP_NODE_NAME "STRING", target_tree_value.c_str());

        auto far_value_1 = test_helper._stub->getProperty(useSlashes("/" TEST_PROP_NAME "STRING"));
        auto far_config_value_1 = far_value_1["value"].asString();
        ASSERT_EQ(target_tree_value, far_config_value_1);

        auto far_value_2 = test_helper._stub->getProperty(useSlashes("/" TEST_PROP_NODE_NAME "STRING"));
        auto far_config_value_2 = far_value_2["value"].asString();
        ASSERT_EQ(target_tree_value, far_config_value_2);
    }
}

//setting template
template<typename T>
void testSettingOf(ConfigTestHelper& test_helper, const T& val, const T& initval, const std::string& prop_path, const std::string& type_name)
{
    T far_value_to_set = val;

    //we need to set it before. you can only set properties from "far" if the property exists
    test_helper._target_tree->SetPropertyValue(prop_path.c_str(), initval);

    std::string property_path_used = "";
    property_path_used += prop_path;

    auto far_error_value_1 = test_helper._stub->setProperty(useSlashes(property_path_used),
        type_name,
        a_util::strings::toString(far_value_to_set));
    ASSERT_EQ(far_error_value_1, 0);

    T target_tree_value_1;
    test_helper._target_tree->GetPropertyValue(prop_path.c_str(), target_tree_value_1);

    ASSERT_EQ(far_value_to_set, target_tree_value_1);
}

//specialisation for string values
void testSettingOf(ConfigTestHelper& test_helper, const std::string& val, const std::string& initval, const std::string& prop_path, const std::string& type_name)
{
    std::string far_value_to_set = val;

    test_helper._target_tree->SetPropertyValue(prop_path.c_str(), initval.c_str());

    std::string property_path_used = "";
    property_path_used += prop_path;

    auto far_error_value_1 = test_helper._stub->setProperty(useSlashes(property_path_used),
        type_name,
        val);
    ASSERT_EQ(far_error_value_1, 0);

    std::string target_tree_value_1;
    const char* target_val;
    test_helper._target_tree->GetPropertyValue(prop_path.c_str(), target_val);
    target_tree_value_1 = target_val;

    ASSERT_EQ(far_value_to_set, target_tree_value_1);
}

/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationSetterFailure)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    std::string property_path_used = "";
    property_path_used += TEST_PROP_NAME "Failed";

    //setting is only allowed fron system side if property does exist!
    auto far_error_value_1 = test_helper._stub->setProperty(useSlashes(property_path_used),
        "string",
        "value");
    //this must be an error result!
    ASSERT_NE(far_error_value_1, 0);
}

/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationSetter)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    { //bool false
        
        testSettingOf<bool>(test_helper, false, true, TEST_PROP_NAME, "bool");
        testSettingOf<bool>(test_helper, false, true, TEST_PROP_NODE_NAME, "bool");

    }
    { //bool true
        testSettingOf<bool>(test_helper, true, false, TEST_PROP_NAME, "bool");
        testSettingOf<bool>(test_helper, true, false, TEST_PROP_NODE_NAME, "bool");
    }
    { //integer

        int32_t target_tree_value = 12345;
        testSettingOf(test_helper, target_tree_value, 1, TEST_PROP_NAME "INT", "int");
        testSettingOf(test_helper, target_tree_value, 2, TEST_PROP_NODE_NAME "INT", "int");
    }
    { //double
        double target_tree_value = 12345.12345;
        testSettingOf<double>(test_helper, target_tree_value, 1, TEST_PROP_NAME "DOUBLE", "double");
        testSettingOf<double>(test_helper, target_tree_value, 2, TEST_PROP_NODE_NAME "DOUBLE", "double");
    }

    { //string
        std::string target_tree_value = "test_string_this_is";
        testSettingOf(test_helper, target_tree_value, "wrong", TEST_PROP_NAME "string", "string");
        testSettingOf(test_helper, target_tree_value, "wrong", TEST_PROP_NODE_NAME "string", "string");
    }
}

//setting template
template<typename T>
void testSettingOfArray(ConfigTestHelper& test_helper,
                   const std::vector<T>& val,
                   const std::vector<T>& init_val,
                   const std::string& prop_path,
                   const std::string& type_name)
{
    std::vector<T> far_value_to_set = val;

    T* unsafe_init_val_pointer = new T[init_val.size()];
    size_t current_idx = 0;
    for (const auto& init_val_idx : init_val)
    {
        unsafe_init_val_pointer[current_idx] = init_val_idx;
        current_idx++;
    }
    //we need to set it before. you can only set properties from "far" if the property exists
    test_helper._target_tree->SetPropertyValues(prop_path.c_str(), unsafe_init_val_pointer, init_val.size());

    delete[] unsafe_init_val_pointer;

    std::string property_path_used = "";
    property_path_used += prop_path;

    std::string string_array_value_to_set;
    bool first = true;
    for (const auto& current_val : val)
    {
        if (!first)
        {
            string_array_value_to_set += ";";
        }
        string_array_value_to_set += a_util::strings::toString(current_val);
        first = false;
    }

    auto far_error_value_1 = test_helper._stub->setProperty(useSlashes(property_path_used),
        type_name,
        string_array_value_to_set);

    ASSERT_EQ(far_error_value_1, 0);

    std::vector<T> array_val_on_target_1;
    auto prop = test_helper._target_tree->GetProperty(prop_path.c_str());
    ASSERT_TRUE(prop->IsArray());
    for (size_t idx = 0; idx < prop->GetArraySize(); idx++)
    {
        T currentval;
        prop->GetValue(currentval, idx);
        array_val_on_target_1.push_back(currentval);
    }


    ASSERT_EQ(far_value_to_set, array_val_on_target_1);
}

/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationSetterArrayType)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    { //bool
        std::vector<bool> array_val = {true, false, true};
        testSettingOfArray<bool>(test_helper, array_val, { true }, TEST_PROP_NAME, "array-bool");
        testSettingOfArray<bool>(test_helper, array_val, { true }, TEST_PROP_NODE_NAME, "array-bool");
    }
    { //integer
        std::vector<int32_t> array_val = { 1, 2, 3 };
        std::vector<int32_t> init_val = {0, 0, 1, 4};
        testSettingOfArray(test_helper, array_val, init_val, TEST_PROP_NAME "int", "array-int");
        testSettingOfArray(test_helper, array_val, init_val, TEST_PROP_NODE_NAME "int", "array-int");
    }
    { //double
        //maybe we need some other comparison here with precision
        //might not work on jenkins 
        std::vector<double> array_val = { 12345.12345, 12345.12345, 12345.12345 };
        std::vector<double> init_val = { 0.0f, 1.0f, 1.1f, 4.1f };
        testSettingOfArray(test_helper, array_val, init_val, TEST_PROP_NAME "double", "array-double");
        testSettingOfArray(test_helper, array_val, init_val, TEST_PROP_NODE_NAME "double", "array-double");
    }
}

//setting template
template<typename T>
void testGettingOfArray(ConfigTestHelper& test_helper,
    const std::vector<T>& init_val,
    const std::string& prop_path,
    const std::string& type_name)
{
    T* unsafe_init_val_pointer = new T[init_val.size()];
    size_t current_idx = 0;
    for (const auto& init_val_idx : init_val)
    {
        unsafe_init_val_pointer[current_idx] = init_val_idx;
        current_idx++;
    }
    
    //we need to set it before. you can only set properties from "far" if the property exists
    test_helper._target_tree->SetPropertyValues(prop_path.c_str(), &unsafe_init_val_pointer[0], init_val.size());

    delete [] unsafe_init_val_pointer;

    std::string property_path_used = "";
    property_path_used += prop_path;

    auto far_value = test_helper._stub->getProperty(useSlashes(property_path_used));

    ASSERT_EQ(far_value["type"].asString(), type_name);

    std::string string_array_value_to_expected;
    bool first = true;
    for (const auto& current_val : init_val)
    {
        if (!first)
        {
            string_array_value_to_expected += ";";
        }
        string_array_value_to_expected += a_util::strings::toString(current_val);
        first = false;
    }

    ASSERT_EQ(far_value["value"].asString(), string_array_value_to_expected);

  
}

/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationArrayGetter)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    { //bool
        std::vector<bool> array_val = { true, false, true };
        testGettingOfArray<bool>(test_helper, array_val, TEST_PROP_NAME, "array-bool");
        testGettingOfArray<bool>(test_helper, array_val, TEST_PROP_NODE_NAME, "array-bool");
    }

    { //integer
        std::vector<int32_t> array_val = { 1, 1234, 12 };
        testGettingOfArray(test_helper, array_val, TEST_PROP_NAME "int", "array-int");
        testGettingOfArray(test_helper, array_val, TEST_PROP_NODE_NAME "int", "array-int");
    }

    { //double
        std::vector<double> array_val = { 1.0, 1.0, 12.12 };
        testGettingOfArray(test_helper, array_val, TEST_PROP_NAME "double", "array-double");
        testGettingOfArray(test_helper, array_val, TEST_PROP_NODE_NAME "double", "array-double");
    }
}



/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationNodeProperties)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    test_helper._target_tree->SetPropertyValue("mainnode", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1", true);
    test_helper._target_tree->SetPropertyValue("mainnode.2", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1.1", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1.2", true);

    auto subnodes = test_helper._stub->getProperties("/");
    ASSERT_TRUE(containsSubNodes(subnodes, { "Header" }));

    subnodes = test_helper._stub->getProperties("");
    ASSERT_TRUE(containsSubNodes(subnodes, { "Header" }));

    subnodes = test_helper._stub->getProperties("/");
    ASSERT_TRUE(containsSubNodes(subnodes, { "mainnode" }));

    subnodes = test_helper._stub->getProperties("/mainnode");
    ASSERT_TRUE(containsSubNodes(subnodes, { "1", "2" }));

    subnodes = test_helper._stub->getProperties("/mainnode/1");
    ASSERT_TRUE(containsSubNodes(subnodes, { "1", "2" }));

    subnodes = test_helper._stub->getProperties("/mainnode/2");
    ASSERT_TRUE(subnodes.empty());
}

/**
 * @req_id ""
 */
TEST(TesterConfiguration, TestRPCForConfigurationExists)
{
    ConfigTestHelper test_helper;
    test_helper.prepareForTest();

    test_helper._target_tree->SetPropertyValue("mainnode", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1", true);
    test_helper._target_tree->SetPropertyValue("mainnode.2", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1.1", true);
    test_helper._target_tree->SetPropertyValue("mainnode.1.2", true);

    auto exists_val = test_helper._stub->exists("/");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("/");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("/mainnode");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("mainnode");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("/mainnode_not_exists");
    ASSERT_FALSE(exists_val);

    exists_val = test_helper._stub->exists("/mainnode/1");
    ASSERT_TRUE(exists_val);

    exists_val = test_helper._stub->exists("/mainnode/3");
    ASSERT_FALSE(exists_val);
}
