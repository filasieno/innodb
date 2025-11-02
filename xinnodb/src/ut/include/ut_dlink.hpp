#pragma once

/// \defgroup dlink DLink
/// \brief Instrusive double linked list
/// \details Doubly linked list is a data structure that allows for efficient insertion and deletion of elements.
/// \ingroup ut

/// \addtogroup dlink
/// @{

struct ut_dlink { 
    struct ut_dlink* next; 
    struct ut_dlink* prev; 
};

inline static void      ut_dlink_init(ut_dlink* link) noexcept;
inline static bool      ut_dlink_is_detached(const ut_dlink* link) noexcept;
inline static void      ut_dlink_detach(ut_dlink* link) noexcept;
inline static void      ut_dlink_clear(ut_dlink* link) noexcept;
inline static void      ut_dlink_enqueue(ut_dlink* queue, ut_dlink* link) noexcept;
inline static ut_dlink* ut_dlink_dequeue(ut_dlink* queue) noexcept;
inline static void      ut_dlink_insert_prev(ut_dlink* list, ut_dlink* link) noexcept;
inline static void      ut_dlink_insert_next(ut_dlink* list, ut_dlink* link) noexcept;
inline static void      ut_dlink_push(ut_dlink* stack, ut_dlink* link) noexcept;
inline static ut_dlink* ut_dlink_pop(ut_dlink* stack) noexcept;
/// @}



// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#define IB_AD(...)

inline void ut_dlink_init(ut_dlink* link) noexcept {
    IB_AD(link != nullptr);
    link->next = link;
    link->prev = link;
}

inline bool ut_dlink_is_detached(const ut_dlink* link) noexcept {
    IB_AD(link != nullptr);
    IB_AD(link->next != nullptr);
    IB_AD(link->prev != nullptr);
    return link->next == link && link->prev == link;
}

inline void ut_dlink_detach(ut_dlink* link) noexcept {
    IB_AD(link != nullptr);
    IB_AD(link->next != nullptr);
    IB_AD(link->prev != nullptr);
    if (ut_dlink_is_detached(link)) return;
    link->next->prev = link->prev;
    link->prev->next = link->next;
    link->next = link;
    link->prev = link;
}

inline void ut_dlink_clear(ut_dlink* link) noexcept {
    IB_AD(link != nullptr);
    link->next = nullptr;
    link->prev = nullptr;
}

inline void ut_dlink_enqueue(ut_dlink* queue, ut_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue->next;
    link->prev = queue;
    link->next->prev = link;
    queue->next = link;
}

inline ut_dlink* ut_dlink_dequeue(ut_dlink* queue) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    if (ut_dlink_is_detached(queue)) return nullptr;
    ut_dlink* target = queue->prev;
    ut_dlink_detach(target);
    return target;
}

inline void ut_dlink_insert_prev(ut_dlink* queue, ut_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue;
    link->prev = queue->prev;
    link->next->prev = link;
    link->prev->next = link;
}

inline void ut_dlink_insert_next(ut_dlink* queue, ut_dlink* link) noexcept {
    IB_AD(queue != nullptr);
    IB_AD(link != nullptr);
    IB_AD(queue->next != nullptr);
    IB_AD(queue->prev != nullptr);
    link->next = queue->next;
    link->prev = queue;
    link->next->prev = link;
    queue->next = link;
}

inline void ut_dlink_push(ut_dlink* stack, ut_dlink* link) noexcept { 
    ut_dlink_insert_next(stack, link); 
}

inline ut_dlink* ut_dlink_pop(ut_dlink* stack) noexcept {
    IB_AD(stack != nullptr);
    IB_AD(stack->next != nullptr);
    IB_AD(stack->prev != nullptr);
    IB_AD(!ut_dlink_is_detached(stack));
    ut_dlink* target = stack->next;
    ut_dlink_detach(target);
    return target;
}

