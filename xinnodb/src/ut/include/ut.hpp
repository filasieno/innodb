#pragma once

/// \defgroup ut ut
/// \brief Utilities
/// \ingroup components

// Common macros

#ifndef IB_OFFSET_OF

/// \brief Computes the byte offset of a field within its type
/// \ingroup ut
#define IB_OFFSET_OF(type, field) ((static_cast<unsigned long>(reinterpret_cast<uintptr_t>(&((type*)(nullptr)->field)))))

#endif
