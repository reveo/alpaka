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

#include <alpaka/core/Vec.hpp>                  // alpaka::Vec
#include <alpaka/core/EnabledAccelerators.hpp>  // alpaka::acc::EnabledAccelerators

#include <algorithm>                            // std::min

//-----------------------------------------------------------------------------
//! The alpaka library.
//-----------------------------------------------------------------------------
namespace alpaka
{
    namespace detail
    {
        struct CorrectMaxBlockKernelExtent
        {
            //-----------------------------------------------------------------------------
            //! \return The maximum block size per dimension supported by all of the given accelerators.
            //-----------------------------------------------------------------------------
            template<
                typename TAcc>
                void operator()(TAcc, alpaka::Vec<3u> & v3uiBlockKernelExtent)
            {
                using DeviceManager = alpaka::device::DeviceManager<TAcc>;

                auto const deviceProperties(DeviceManager::getCurrentDevice().getProperties());
                auto const & v3uiBlockKernelsExtentMax(deviceProperties.m_v3uiBlockKernelsExtentMax);

                v3uiBlockKernelExtent = alpaka::Vec<3u>(
                    std::min(v3uiBlockKernelExtent[0u], v3uiBlockKernelsExtentMax[0u]),
                    std::min(v3uiBlockKernelExtent[1u], v3uiBlockKernelsExtentMax[1u]),
                    std::min(v3uiBlockKernelExtent[2u], v3uiBlockKernelsExtentMax[2u])
                    );
            }
        };

        struct CorrectMaxBlockKernelCount
        {
            //-----------------------------------------------------------------------------
            //! \return The maximum block size supported by all of the given accelerators.
            //-----------------------------------------------------------------------------
            template<
                typename TAcc>
                void operator()(TAcc, std::size_t & uiBlockKernelCount)
            {
                using DeviceManager = alpaka::device::DeviceManager<TAcc>;
                auto const deviceProperties(DeviceManager::getCurrentDevice().getProperties());
                auto const & uiBlockKernelCountMax(deviceProperties.m_uiBlockKernelsCountMax);

                uiBlockKernelCount = std::min(uiBlockKernelCount, uiBlockKernelCountMax);
            }
        };
    }

    //-----------------------------------------------------------------------------
    //! \return The maximum block size per dimension supported by all of the enabled accelerators.
    //-----------------------------------------------------------------------------
    alpaka::Vec<3u> getMaxBlockKernelExtentEnabledAccelerators()
    {
        alpaka::Vec<3u> v3uiMaxBlockKernelExtent(
            std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::max(),
            std::numeric_limits<std::size_t>::max());

        boost::mpl::for_each<acc::EnabledAccelerators>(
            std::bind(detail::CorrectMaxBlockKernelExtent(), std::placeholders::_1, std::ref(v3uiMaxBlockKernelExtent))
            );

        return v3uiMaxBlockKernelExtent;
    }

    //-----------------------------------------------------------------------------
    //! \return The maximum block size supported by all of the enabled accelerators.
    //-----------------------------------------------------------------------------
    std::size_t getMaxBlockKernelCountEnabledAccelerators()
    {
        std::size_t uiMaxBlockKernelCount(
            std::numeric_limits<std::size_t>::max());

        boost::mpl::for_each<acc::EnabledAccelerators>(
            std::bind(detail::CorrectMaxBlockKernelCount(), std::placeholders::_1, std::ref(uiMaxBlockKernelCount))
            );

        return uiMaxBlockKernelCount;
    }

    namespace detail
    {
        //-----------------------------------------------------------------------------
        //! \param uiMaxDivisor The maximum divisor.
        //! \param uiDividend The dividend.
        //! \return A number that satisfies the following conditions:
        //!     1) uiDividend/ret==0
        //!     2) ret<=uiMaxDivisor
        //-----------------------------------------------------------------------------
        std::size_t nextLowerOrEqualFactor(std::size_t const & uiMaxDivisor, std::size_t const & uiDividend)
        {
            std::size_t uiDivisor(uiMaxDivisor);
            // \TODO: This is not very efficient. Replace with a better algorithm.
            while((uiDividend%uiDivisor) != 0)
            {
                --uiDivisor;
            }
            return uiDivisor;
        }
    }

    //-----------------------------------------------------------------------------
    //! \param v3uiGridKernelsExtent        
    //!     The maximum divisor.
    //! \param bAdaptiveBlockKernelsExtent  
    //!     If the block kernels extent should be selected adaptively to the given accelerator
    //!     or the minimum supported by all accelerator.
    //! \return The work extent.
    // \TODO: Make this a template depending on Accelerator and Kernel
    //-----------------------------------------------------------------------------
    template<
        typename TAcc>
        alpaka::WorkExtent getValidWorkExtent(alpaka::Vec<3u> const & v3uiGridKernelsExtent, bool const & bAdaptiveBlockKernelsExtent)
    {
        // \TODO: Print a warning when the grid kernels extent is a prime number and the resulting block kernels extent is 1.

        assert(v3uiGridKernelsExtent[0u]>0);
        assert(v3uiGridKernelsExtent[1u]>0);
        assert(v3uiGridKernelsExtent[2u]>0);

        alpaka::Vec<3u> v3uiMaxBlockKernelsExtent;
        std::size_t uiMaxBlockKernelsCount;

        // Get the maximum block kernels extent depending on the the input.
        if(bAdaptiveBlockKernelsExtent)
        {
            using DeviceManager = alpaka::device::DeviceManager<TAcc>;
            auto const deviceProperties(DeviceManager::getCurrentDevice().getProperties());
            v3uiMaxBlockKernelsExtent = deviceProperties.m_v3uiBlockKernelsExtentMax;
            uiMaxBlockKernelsCount = deviceProperties.m_uiBlockKernelsCountMax;
        }
        else
        {
            v3uiMaxBlockKernelsExtent = alpaka::getMaxBlockKernelExtentEnabledAccelerators();
            uiMaxBlockKernelsCount = alpaka::getMaxBlockKernelCountEnabledAccelerators();
        }

        // Restrict the max block kernels extent with the grid kernels extent.
        // This removes dimensions not required.
        // This has to be done before the uiMaxBlockKernelsCount clipping to get the maximum correctly.
        v3uiMaxBlockKernelsExtent = alpaka::Vec<3u>(
            std::min(v3uiMaxBlockKernelsExtent[0u], v3uiGridKernelsExtent[0u]),
            std::min(v3uiMaxBlockKernelsExtent[1u], v3uiGridKernelsExtent[1u]),
            std::min(v3uiMaxBlockKernelsExtent[2u], v3uiGridKernelsExtent[2u]));

        // If the block kernels extent allows more kernels then available on the accelerator, clip it.
        std::size_t const uiBlockKernelsCount(v3uiMaxBlockKernelsExtent.prod());
        if(uiBlockKernelsCount>uiMaxBlockKernelsCount)
        {
            // Very primitive clipping. Just halve it until it fits.
            // \TODO: Use a better algorithm for clipping.
            while(v3uiMaxBlockKernelsExtent.prod()>uiMaxBlockKernelsCount)
            {
                v3uiMaxBlockKernelsExtent = alpaka::Vec<3u>(
                    std::max(static_cast<std::size_t>(1u), static_cast<std::size_t>(v3uiMaxBlockKernelsExtent[0u] / 2u)),
                    std::max(static_cast<std::size_t>(1u), static_cast<std::size_t>(v3uiMaxBlockKernelsExtent[1u] / 2u)),
                    std::max(static_cast<std::size_t>(1u), static_cast<std::size_t>(v3uiMaxBlockKernelsExtent[2u] / 2u)));
            }
        }

        // Make the block kernels extent divide the grid kernels extent.
        alpaka::Vec<3u> const v3uiBlockKernelsExtent(
            detail::nextLowerOrEqualFactor(v3uiMaxBlockKernelsExtent[0u], v3uiGridKernelsExtent[0u]),
            detail::nextLowerOrEqualFactor(v3uiMaxBlockKernelsExtent[1u], v3uiGridKernelsExtent[1u]),
            detail::nextLowerOrEqualFactor(v3uiMaxBlockKernelsExtent[2u], v3uiGridKernelsExtent[2u]));

        // Set the grid blocks extent.
        alpaka::Vec<3u> const v3uiGridBlocksExtent(
            v3uiGridKernelsExtent[0u] / v3uiBlockKernelsExtent[0u],
            v3uiGridKernelsExtent[1u] / v3uiBlockKernelsExtent[1u],
            v3uiGridKernelsExtent[2u] / v3uiBlockKernelsExtent[2u]);

        return alpaka::WorkExtent(v3uiGridBlocksExtent, v3uiBlockKernelsExtent);
    }
}