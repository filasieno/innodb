#pragma once

/// \defgroup dlink DLink
/// \brief Instrusive double linked list
/// \details Doubly linked list is a data structure that allows for efficient insertion and deletion of elements.
/// \ingroup ut

/// \addtogroup dlink
/// @{

struct ib_dlink { 
    struct ib_dlink* next; 
    struct ib_dlink* prev; 
};

inline static void      ib_dlink_init(ib_dlink* link) noexcept;
inline static bool      ib_dlink_is_detached(const ib_dlink* link) noexcept;
inline static void      ib_dlink_detach(ib_dlink* link) noexcept;
inline static void      ib_dlink_clear(ib_dlink* link) noexcept;
inline static void      ib_dlink_enqueue(ib_dlink* queue, ib_dlink* link) noexcept;
inline static ib_dlink* ib_dlink_dequeue(ib_dlink* queue) noexcept;
inline static void      ib_dlink_insert_prev(ib_dlink* list, ib_dlink* link) noexcept;
inline static void      ib_dlink_insert_next(ib_dlink* list, ib_dlink* link) noexcept;
inline static void      ib_dlink_push(ib_dlink* stack, ib_dlink* link) noexcept;
inline static ib_dlink* ib_dlink_pop(ib_dlink* stack) noexcept;
/// @}



// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#define IB_AD(...)

inline void ib_dlink_init(ib_dlink* link) noexcept {
    IB_AD(link != nullptr);
    link->next = link;
    link->prev = link;
}

inline bool ib_dlink_is_detached(const ib_dlink* link) noexcept {
    IB_AD(link != nullptr);
    IB_AD(link->next != nullptr);
    IB_AD(link->prev != nullptr);
    return link->next == link && link->prev == link;
}

inline void ib_dlink_detach(ib_dlink* link) noexcept {
    IB_AD(link != nullptr);
    IB_AD(link->next != nullptr);
    IB_AD(link->prev != nullptr);
    if (ib_dlink_is_detached(link)) return;
    link->next->prev = link->prev;
    link->prev->next = link->next;
    link->next = link;
    link->prev = link;
}

inline void ib_dlink_clear(ib_dlink* link) noexcept {
    IB_AD(link != nullptr);
    link->next = nullptr;
    link->prev = nullptr;
}

inline void ib_dlink_enqueue(ib_dlink* queue, ib_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue->next;
    link->prev = queue;
    link->next->prev = link;
    queue->next = link;
}

inline ib_dlink* ib_dlink_dequeue(ib_dlink* queue) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    if (ib_dlink_is_detached(queue)) return nullptr;
    ib_dlink* target = queue->prev;
    ib_dlink_detach(target);
    return target;
}

inline void ib_dlink_insert_prev(ib_dlink* queue, ib_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue;
    link->prev = queue->prev;
    link->next->prev = link;
    link->prev->next = link;
}

inline void ib_dlink_insert_next(ib_dlink* queue, ib_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue->next;
    link->prev = queue;
    link->next->prev = link;
    queue->next = link;
}

inline void ib_dlink_push(ib_dlink* stack, ib_dlink* link) noexcept { 
    ib_dlink_insert_next(stack, link); 
}

inline ib_dlink* ib_dlink_pop(ib_dlink* stack) noexcept {
    IB_AD(stack != nullptr);
    IB_AD(stack->next != nullptr);
    IB_AD(stack->prev != nullptr);
    IB_AD(!ib_dlink_is_detached(stack));
    ib_dlink* target = stack->next;
    ib_dlink_detach(target);
    return target;
}

