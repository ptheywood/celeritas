//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/io/detail/ReprImpl.hh
//---------------------------------------------------------------------------//
#pragma once

#include <iomanip>
#include <iostream>
#include <type_traits>

#include "corecel/Assert.hh"
#include "corecel/io/ScopedStreamFormat.hh"

namespace celeritas
{
template<class T>
struct ReprTraits;

namespace detail
{
//---------------------------------------------------------------------------//
// STREAMABLE
//---------------------------------------------------------------------------//
/*!
 * Thin temporary wrapper for printing a complex class to a stream.
 */
template<class T>
struct Repr
{
    const T&    obj;
    const char* name = nullptr;
};

//---------------------------------------------------------------------------//
/*!
 * Write a streamable object to a stream.
 */
template<class T>
std::ostream& operator<<(std::ostream& os, const Repr<T>& s)
{
    ScopedStreamFormat save_fmt(&os);
    ReprTraits<T>::init(os);
    if (s.name)
    {
        ReprTraits<T>::print_type(os, s.name);
        os << '{';
    }
    ReprTraits<T>::print_value(os, s.obj);
    if (s.name)
    {
        os << '}';
    }
    return os;
}

//---------------------------------------------------------------------------//
// HELPER FUNCTION DECLARATIONS
//---------------------------------------------------------------------------//
void repr_char(std::ostream& os, char value);

std::string char_to_hex_string(unsigned char value);

void print_simple_type(std::ostream& os, const char* type, const char* name);

template<class T>
inline void
print_container_type(std::ostream& os, const char* type, const char* name)
{
    os << type << '<';
    ReprTraits<T>::print_type(os);
    os << '>';
    if (name)
    {
        os << ' ' << name;
    }
}

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace celeritas
