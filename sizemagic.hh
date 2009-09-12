// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet 
/*
 * Mediabox
 * Copyright (C) 2009 Jakob Truelsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __sizemagic_hh__
#define __sizemagic_hh__
#define __STDC_LIMIT_MACROS
#include <stdint.h>

template <class T> struct extremum {
};

#define SPECIAL(t,a,b) template <> struct extremum<t> {static const t min() {return a;}; static const t max() {return b;}};
SPECIAL(uint8_t ,0        ,UINT8_MAX)
SPECIAL(uint16_t,0        ,UINT16_MAX)
SPECIAL(uint32_t,0        ,UINT32_MAX)
SPECIAL(uint64_t,0        ,UINT64_MAX)
SPECIAL(int8_t  ,INT8_MIN ,INT8_MAX)
SPECIAL(int16_t ,INT16_MIN,INT16_MAX)
SPECIAL(int32_t ,INT32_MIN,INT32_MAX)
SPECIAL(int64_t ,INT64_MIN,INT64_MAX)
#undef SPECIAL

#endif //__sizemagic_hh__
