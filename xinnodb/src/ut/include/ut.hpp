#pragma once

/// \defgroup ut ut
/// \brief Utilities
/// \ingroup components

// Common macros

#ifndef IB_OFFSET_OF
/// \brief Computes the byte offset of a field within its type
/// \ingroup ut
#define IB_OFFSET_OF(type, field) (static_cast<std::size_t>( reinterpret_cast<const char*>(&reinterpret_cast<const type*>(0)->field) - reinterpret_cast<const char*>(0) ))
#endif