/**
* \file
* Copyright 2014-2015 Benjamin Worpitz
*
* This file is part of alpaka.
*
* alpaka is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* alpaka is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with alpaka.
* If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <alpaka/core/Vec.hpp>          // Vec
#include <alpaka/core/Positioning.hpp>  // origin::Grid/Blocks

#include <boost/mpl/placeholders.hpp>   // boost::mpl::_1

//-----------------------------------------------------------------------------
//! The name space for the accelerator library.
//-----------------------------------------------------------------------------
namespace alpaka
{
    //#############################################################################
    //! The accelerator interface.
    //
    // All the methods of this interface are declared ALPAKA_FCT_ACC. 
    // Because the kernel is always compiled with ALPAKA_FCT_ACC for all accelerators (even for AccSerial), equivalently there has to be an implementation of all methods for host and device for all accelerators. 
    // These device functions are not implemented and will not call the underlying implementation for the device code, because this will never be executed and would not compile.
    //#############################################################################
    template<
        typename TAcc = boost::mpl::_1>
    class IAcc :
        protected TAcc
    {
    public:
        //-----------------------------------------------------------------------------
        //! Constructor.
        //-----------------------------------------------------------------------------
        ALPAKA_FCT_ACC IAcc() = default;
        //-----------------------------------------------------------------------------
        //! Copy constructor.
        //-----------------------------------------------------------------------------
        ALPAKA_FCT_ACC IAcc(IAcc const &) = delete;
#if (!BOOST_COMP_MSVC) || (BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(14, 0, 0))
        //-----------------------------------------------------------------------------
        //! Move constructor.
        //-----------------------------------------------------------------------------
        ALPAKA_FCT_ACC IAcc(IAcc &&) = delete;
#endif
        //-----------------------------------------------------------------------------
        //! Copy assignment.
        //-----------------------------------------------------------------------------
        ALPAKA_FCT_ACC IAcc & operator=(IAcc const &) = delete;

        //-----------------------------------------------------------------------------
        //! \return The requested extents.
        //-----------------------------------------------------------------------------
        template<
            typename TOrigin, 
            typename TUnit, 
            typename TDim = dim::Dim3>
        ALPAKA_FCT_ACC DimToVecT<TDim> getWorkDiv() const
        {
#ifndef __CUDA_ARCH__
            return TAcc::template getWorkDiv<TOrigin, TUnit, TDim>();
#else
            return {};
#endif
        }

        //-----------------------------------------------------------------------------
        //! \return The requested indices.
        //-----------------------------------------------------------------------------
        template<
            typename TOrigin,
            typename TUnit, 
            typename TDim = dim::Dim3>
        ALPAKA_FCT_ACC DimToVecT<TDim> getIdx() const
        {
#ifndef __CUDA_ARCH__
            return TAcc::template getIdx<TOrigin, TUnit, TDim>();
#else
            return {};
#endif
        }

        //-----------------------------------------------------------------------------
        //! Execute the atomic operation on the given address with the given value.
        //! \return The old value before executing the atomic operation.
        //-----------------------------------------------------------------------------
        template<
            typename TOp, 
            typename T>
        ALPAKA_FCT_ACC T atomicOp(
            T * const addr, 
            T const & value) const
        {
#ifndef __CUDA_ARCH__
            return TAcc::template atomicOp<TOp, T>(addr, value);
#else
            return {};
#endif
        }

        //-----------------------------------------------------------------------------
        //! Syncs all threads in the current block.
        //-----------------------------------------------------------------------------
        ALPAKA_FCT_ACC void syncBlockThreads() const
        {
#ifndef __CUDA_ARCH__
            return TAcc::syncBlockThreads();
#endif
        }

        //-----------------------------------------------------------------------------
        //! \return Allocates block shared memory.
        //-----------------------------------------------------------------------------
        template<
            typename T, 
            UInt TuiNumElements>
        ALPAKA_FCT_ACC T * allocBlockSharedMem() const
        {
            static_assert(TuiNumElements > 0, "The number of elements to allocate in block shared memory must not be zero!");
#ifndef __CUDA_ARCH__
            return TAcc::template allocBlockSharedMem<T, TuiNumElements>();
#else
            return nullptr;
#endif
        }

        //-----------------------------------------------------------------------------
        //! \return The pointer to the externally allocated block shared memory.
        //-----------------------------------------------------------------------------
        template<
            typename T>
        ALPAKA_FCT_ACC T * getBlockSharedExternMem() const
        {
#ifndef __CUDA_ARCH__
            return TAcc::template getBlockSharedExternMem<T>();
#else
            return nullptr;
#endif
        }
    };

    namespace traits
    {
        namespace acc
        {
            //#############################################################################
            //! The accelerator interface accelerator type trait specialization.
            //#############################################################################
            template<
                typename TAcc>
            struct AccType<
                IAcc<TAcc>>
            {
                using type = TAcc;
            };
        }
    }
}
