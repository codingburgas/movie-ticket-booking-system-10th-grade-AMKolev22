//
// Copyright (C) 2004-2008 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_H_INCLUDED
#define SOCI_H_INCLUDED

// namespace soci
#include <../../vendor/soci/include/soci/soci-platform.h>
#include <../../vendor/soci/include/soci/backend-loader.h>
#include <../../vendor/soci/include/soci/blob.h>
#include <../../vendor/soci/include/soci/blob-exchange.h>
#include <../../vendor/soci/include/soci/column-info.h>
#include <../../vendor/soci/include/soci/connection-pool.h>
#include <../../vendor/soci/include/soci/error.h>
#include <../../vendor/soci/include/soci/exchange-traits.h>
#include <../../vendor/soci/include/soci/into.h>
#include <../../vendor/soci/include/soci/into-type.h>
#include <../../vendor/soci/include/soci/once-temp-type.h>
#include <../../vendor/soci/include/soci/prepare-temp-type.h>
#include <../../vendor/soci/include/soci/procedure.h>
#include <../../vendor/soci/include/soci/ref-counted-prepare-info.h>
#include <../../vendor/soci/include/soci/ref-counted-statement.h>
#include <../../vendor/soci/include/soci/row.h>
#include <../../vendor/soci/include/soci/row-exchange.h>
#include <../../vendor/soci/include/soci/rowid.h>
#include <../../vendor/soci/include/soci/rowid-exchange.h>
#include <../../vendor/soci/include/soci/rowset.h>
#include <../../vendor/soci/include/soci/session.h>
#include <../../vendor/soci/include/soci/soci-backend.h>
#include <../../vendor/soci/include/soci/statement.h>
#include <../../vendor/soci/include/soci/transaction.h>
#include <../../vendor/soci/include/soci/type-conversion.h>
#include <../../vendor/soci/include/soci/type-conversion-traits.h>
#include <../../vendor/soci/include/soci/type-holder.h>
#include <../../vendor/soci/include/soci/type-ptr.h>
#include <../../vendor/soci/include/soci/type-wrappers.h>
#include <../../vendor/soci/include/soci/unsigned-types.h>
#include <../../vendor/soci/include/soci/use.h>
#include <../../vendor/soci/include/soci/use-type.h>
#include <../../vendor/soci/include/soci/values.h>
#include <../../vendor/soci/include/soci/values-exchange.h>


// namespace boost
#ifdef SOCI_USE_BOOST
#include <boost/version.hpp>
#if defined(BOOST_VERSION) && BOOST_VERSION >= 103500
#include "soci/boost-fusion.h"
#endif // BOOST_VERSION
#include "soci/boost-optional.h"
#include "soci/boost-tuple.h"
#include "soci/boost-gregorian-date.h"
#endif // SOCI_USE_BOOST

#endif // SOCI_H_INCLUDED
