/**
 * The FEP dPointer
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


#ifndef FEP_D_POINTER_H
#define FEP_D_POINTER_H

#include "fep_participant_export.h"

/**
 * Concrete d-pointer definition.
 * This d-Pointer definition is used to create a d-Pointer reference to the private implementation of the class.
 * To do so it will use the @ref cUTILSDPtr template.
 *
 * @param [in] __dclassname_ d pointer class name.
 * @param [in] __pclassname_ parent class name.
 *
 */
#define FEP_UTILS_D_CLASS(__dclassname_, __pclassname_) \
private: \
   class __dclassname_; \
   cUTILSDPtr<__pclassname_, __dclassname_> _d; \
   friend class __dclassname_;

/**
 * Helper macro for d-pattern definitions. this macro should be used by developer
 * It declares the the private class see @ref FEP_UTILS_D_CLASS.
 *
 * @param [in] __pclassname_ parent class name
 *
 *
 */
#define FEP_UTILS_D(__pclassname_)  FEP_UTILS_D_CLASS(__pclassname_ ## Private, __pclassname_)

/**
 * concrete d-pointer instance
 * @param [in] __classname_ parent class
 *
 */
#define FEP_UTILS_D_CREATE_CLASS(__classname_) \
    _d = new __classname_(); \
    _d->Set_p(this)

/**
 * helper macro for d-pattern usage.
 * @param [in] __classname_ parent class name
 *
 */
#define FEP_UTILS_D_CREATE(__classname_)  FEP_UTILS_D_CREATE_CLASS(__classname_ ## Private)

/**
 * helper macro that defines a local variable "_D" that points to the private implementation.
 * @param [in] __classname_ parent class name
  *
 */
#define FEP_UTILS_DECLARE_LOCAL_D(__classname_)  __classname_ ## Private* _D = _d.Get_d()

/**
 * Template to implement the Private class of the global d_pointer definitions.
 *
 * @tparam _PARENT     [in] Parent Class name.
 * @tparam _PRIVATE_D  [in] Private Class name.
 *
 *
 */
template <class _PARENT, class _PRIVATE_D>
class FEP_PARTICIPANT_EXPORT cUTILSD
{
    protected:
        ///the reference to the parent class
        _PARENT* _p;

    public:
        ///constructor
        cUTILSD()
        {
            _p = nullptr;
        }
        ///destructor
        virtual ~cUTILSD()
        {
            Release_d();
            _p = nullptr;
        }
        /**
         * Sets the parent Reference. This must be called within the constructor of Parent Class.
         * Use the helper macro @ref FEP_UTILS_D_CREATE.
         * @param [in] _pInst a parent reference.
         * @return void
         */
        void Set_p(_PARENT* _pInst)
        {
            _p = _pInst;
            Create_d();
        }

        /**
         * Overwrites the pointer operator to return the Private class reference.
         * @retval The private pointer
         */
        inline _PRIVATE_D* operator->() const
        {
            return (_PRIVATE_D*)(this);
        }

        /**
         * Gets the Private class reference.
         * @return Returns the Private class reference.
         */
        inline _PRIVATE_D* Get_d() const
        {
            return (_PRIVATE_D*)(this);
        }

        /**
         * Cast operator to get the private class reference.
         */
        inline operator _PRIVATE_D*() const
        {
            return (_PRIVATE_D*)(this);
        }

        /**
         * Cast operator to get the own pointer
         */
        operator cUTILSD<_PARENT, _PRIVATE_D>*() const
        {
            return (cUTILSD< _PARENT, _PRIVATE_D >*)(&*this);
        }

    protected:
        /**
         * Virtual function call to overwrite it something needs to be done on creation time.
         */
        virtual void Create_d()
        {
            //nothing to do this is to overwrite
        }
        /**
         * Virtual function call to overwrite it something needs to be done before destroying the object.
         */
        virtual void Release_d()
        {
            //nothing to do this is to overwrite
        }
};

/**
 * Template class for the d-pointer Reference class withing the Parent Class.
 * This is declared with the helper macro @ref FEP_UTILS_D.
 *
 * @param  _PARENT    The parent Class Name.
 * @param  _PRIVATE_D The Private Class Name.
 */
template <class _PARENT, class _PRIVATE_D>
class FEP_PARTICIPANT_EXPORT cUTILSDPtr
{
    protected:
        /// The internal pointer to the private class
        cUTILSD<_PARENT, _PRIVATE_D>* _dRef;

    public:
        cUTILSDPtr()
        {
            _dRef = nullptr;
        }

        virtual ~cUTILSDPtr()
        {
            if (_dRef)
            {
                delete _dRef;
                _dRef = nullptr;
            }
        }

        /**
         * Sets the internal pointer
         * @param _pInst [in]   New pointer value
         * @return void
         */
        void Set_p(_PARENT* _pInst)
        {
            if (_dRef)
            {
                _dRef->Set_p(_pInst);
            }
        }

        /**
         * Pointer operator enable access to the internal pointer
         * @retval The private pointer
         */
        _PRIVATE_D* operator->() const
        {
            return Get_d();
        }

        /**
         * Gets the internal pointer
         * @return Internal pointer value
         */
        _PRIVATE_D* Get_d() const
        {
            return (_PRIVATE_D*)(_dRef);
        }

        /**
         * Cast operator enable access to the internal pointer
         */
        operator _PRIVATE_D*() const
        {
            return Get_d();
        }

        /**
         * Assignment operator allows to set the internal pointer value
         * @param _dInst Assigned instance
         * @retval This
         */
        cUTILSDPtr<_PARENT, _PRIVATE_D>* operator=(_PRIVATE_D* _dInst)
        {
            if (_dRef)
            {
                delete _dRef;
            }
            _dRef = (cUTILSD<_PARENT, _PRIVATE_D>*)_dInst;
            return this;
        }
};

/**
 * Helper Macro to declare the class header.
 * @param [in] __d_classname_ The Private D Class
 * @param [in] __p_classname_ The Parent Class
 */
#define FEP_UTILS_P_DECLARE_CLASS(__d_classname_, __p_classname_) class FEP_PARTICIPANT_EXPORT __p_classname_::__d_classname_ : \
    public cUTILSD<__p_classname_, __p_classname_::__d_classname_>

/**
 * Helper Macro to generate the private class name and to declare the class header.
 * @param [in] __p_classname_ The Parent Class
 */
#define FEP_UTILS_P_DECLARE( __p_classname_) FEP_UTILS_P_DECLARE_CLASS(__p_classname_ ## Private, __p_classname_)

/**
 * Helper Macro to generate the private class name.
 * @param [in] __p_classname_ The Parent Class
 */
#define FEP_UTILS_P_CLASS( __p_classname_)  __p_classname_::__p_classname_ ## Private

/**
 * Helper Macro to declare the private class which has no implementation yet.
 * @param [in] __d_classname_ The Private D Class
 * @param [in] __p_classname_ The Parent Class
 */
#define FEP_UTILS_P_DECLARE_CLASS_PRE(__d_classname_, __p_classname_) \
    FEP_UTILS_P_DECLARE_CLASS(__d_classname_, __p_classname_)\
    { \
        friend class __p_classname_; \
        public: \
            __d_classname_(){}; \
            virtual ~__d_classname_(){}; \
    };

/**
 * Helper Macro to declare the private class which has no implementation yet.
 * It will generate the private classname.
 * @param [in] __p_classname_ The Parent Class
 */
#define FEP_UTILS_P_DECLARE_PRE(__p_classname_) FEP_UTILS_P_DECLARE_CLASS_PRE(__p_classname_ ## Private, __p_classname_)

/**
 * \page page_d_pointer FEP Core d-Pointer Implementation
 *
 * \par Common Description
 * A d-Pointer is to declare the public interface of a class only in the submitted header of a library. \n
 * This is to ensure that the implementation and declaration of private functions and member of the class
 * can be changed while a minor release cycle, which does not change the class definition of and so the object
 * sizes of the provided class.
 *
 * See additional information in the internet by looking for "d-pointer Pattern".
 *
 */
#endif /* FEP_D_POINTER_H */
