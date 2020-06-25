/**
 * Implementation of the timing demo
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
 */

#ifndef _EXAMPLE_TIMING_ELEMENT_SENDER_H_
#define _EXAMPLE_TIMING_ELEMENT_SENDER_H_

#include <fep_participant_sdk.h>

 /**
 * Car model
 * Base class for all car models
 */
class cCarModel
{
public:
    /**
     * CTOR
     * @param[in] strSignalName  Name of fep signal
     * @param[in] strStepName  Name of step listener
     */
    cCarModel(const char* strSignalName, const char* strStepName);

    /**
    * DTOR
    */
    ~cCarModel();

public:
    /**
    * Initialize
    * Put model into initialized state
    * @param[in] pSignalRegistry pointer to the signal registry interface
    * @param[in] pUserDataAccess pointer to the user data access interface
    * @param[in] pTiming pointer to the timing interface
    * @returns  Standard result code.
    */
    fep::Result Initialize(fep::ISignalRegistry* pSignalRegistry,
        fep::IUserDataAccess* pUserDataAccess, fep::ITiming* pTiming);

    /**
    * Finalize
    * Undo initialize step. Model is back in initalized state.
    * @returns  Standard result code.
    */
    fep::Result Finalize();

public:
    /**
    * Configure
    * Put model into configured state
    * @returns  Standard result code.
    */
    virtual fep::Result Configure();
    
    /**
    * Reset
    * Undo configure state. Model is back in initalized state.
    * @returns  Standard result code.
    */
    virtual fep::Result Reset();

public:
    /**
    * Set Position
    *
    * @param [in] pos_x position in x direction
    * @param [in] pos_y position in y direction
    */
    void SetPosition(const double pos_x, const double pos_y)
    {
        m_pos_x = pos_x;
        m_pos_y = pos_y;
    }

    /**
    * Set Velocity
    *
    * @param [in] vel_x velocity in x direction
    * @param [in] vel_y velocity in y direction
    */
    void SetVelocity(const double vel_x, const double vel_y)
    {
        m_vel_x = vel_x;
        m_vel_y = vel_y;
    }

    /**
    * Set Acceleration
    *
    * @param [in] acc_x acceleration in x direction
    * @param [in] acc_y acceleration in y direction
    */
    void SetAcceleration(const double acc_x, const double acc_y)
    {
        m_acc_x = acc_x;
        m_acc_y = acc_y;
    }

protected:
    /** 
     * Step Listener
     * Execute next simulation step
     * 
     * @param [in] tmSimulation simualtion timestamp
     * @param [in] pStepDataAccess pointer to step data access interface
     */
    virtual void Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);

private:
    ///@cond nodoc
    static void Simulate_caller(void* _this, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cCarModel*>(_this)->Simulate(tmSimulation, pStepDataAccess);
    }
    ///@endcond nodoc

protected:
    /// Signal Name
    std::string m_strSignalName;
    /// Step Name
    std::string m_strStepName;
    /// Pointer to the signal registry interface
    fep::ISignalRegistry* m_pSignalRegistry;
    /// Pointer to the user data access interface
    fep::IUserDataAccess* m_pUserDataAccess;
    /// Pointer to the timing interface
    fep::ITiming* m_pTiming;
    /// Stores the handle for the signal
    handle_t m_hSignalOut;
    /// Data Sample
    fep::IUserDataSample* m_pDataSample;
    /// Last sim time for step A
    timestamp_t m_tmLastStep;
    /// Current car data: position in x direction
    double m_pos_x;
    /// Current car data: position in y direction
    double m_pos_y;
    /// Current car data: velocity in x direction
    double m_vel_x;
    /// Current car data: velocity in y direction
    double m_vel_y;
    /// Current car data: acceleration in x direction
    double m_acc_x;
    /// Current car data: acceleration in y direction
    double m_acc_y;
};

/**
* Car model
* Own/Controlled cars
*/
class cOwnCar : public cCarModel
{
public:
    /**
    * CTOR
    * @param[in] strSignalName  Name of fep signal
    * @param[in] strStepName  Name of step listener
    */
    cOwnCar(const char* strSignalName, const char* strStepName);

    /**
    * DTOR
    */
    ~cOwnCar();

protected:
    void Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess) override;

public:
    fep::Result Configure() override;
    fep::Result Reset() override;

private:
    /// Pointer to input user data sample
    fep::IUserDataSample* m_pInDataSample;
    /// Input signal handle
    handle_t m_hSignalIn;
};

/**
* Car model
* Simulated cars
*/
class cCarSin : public cCarModel
{
public:
    /**
    * CTOR
    * @param[in] strSignalName  Name of fep signal
    * @param[in] strStepName  Name of step listener
    */
    cCarSin(const char* strSignalName, const char* strStepName);

    /**
    * DTOR
    */
    ~cCarSin();

protected:
    void Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess) override;
};

/**
* Element Environment 
* Simulated environment
*/
class cElementEnvironment : public fep::cModule
{
public:
    cElementEnvironment(const std::string& strTimingConfiguration);
    ~cElementEnvironment();

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override;
    fep::Result ProcessIdleEntry(const fep::tState eOldState) override;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override;
    fep::Result ProcessShutdownEntry(const fep::tState eOldState) override;

private:
    /// The simulated car A
    cCarSin oCarA;
    /// The simulated car B
    cCarSin oCarB;
    /// The own car
    cOwnCar oOwn;
    /// Timing configuration file
    std::string m_strTimingConfiguration;
};

#endif
