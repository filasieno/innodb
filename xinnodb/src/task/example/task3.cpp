#include <iostream>
#include <vector>

// Coroutine states
enum ProducerState { PRODUCER_START, PRODUCER_LOOP, PRODUCER_END };
enum ConsumerState { CONSUMER_START, CONSUMER_WAIT, CONSUMER_PROCESS, CONSUMER_END };

// Shared data structure
struct SharedData {
    std::vector<int> buffer;
    bool producer_done = false;
    int consumed_count = 0;
};

// Producer coroutine using computed goto
void producer_coroutine(SharedData& data, ProducerState& state) {
    static void* producer_labels[] = {
        &&producer_start,
        &&producer_loop, 
        &&producer_end
    };
    
    goto *producer_labels[state];
    
producer_start:
    std::cout << "Producer: Starting production\n";
    state = PRODUCER_LOOP;
    return;  // Yield to consumer
    
producer_loop:
    for (int i = 1; i <= 5; ++i) {
        data.buffer.push_back(i * 10);
        std::cout << "Producer: Generated " << (i * 10) << "\n";
        
        // Yield after each production
        state = PRODUCER_LOOP;
        return;
    }
    
    data.producer_done = true;
    std::cout << "Producer: Finished production\n";
    state = PRODUCER_END;
    return;
    
producer_end:
    std::cout << "Producer: End of coroutine\n";
    return;
}

// Consumer coroutine using computed goto  
void consumer_coroutine(SharedData& data, ConsumerState& state, ProducerState& producer_state) {
    static void* consumer_labels[] = {
        &&consumer_start,
        &&consumer_wait,
        &&consumer_process,
        &&consumer_end
    };
    
    goto *consumer_labels[state];
    
consumer_start:
    std::cout << "Consumer: Starting consumption\n";
    state = CONSUMER_WAIT;
    return;  // Yield to producer
    
consumer_wait:
    if (!data.buffer.empty()) {
        state = CONSUMER_PROCESS;
        goto *consumer_labels[state];  // Jump to process
    }
    
    if (data.producer_done) {
        state = CONSUMER_END;
        return;
    }
    
    // Wait for producer
    state = CONSUMER_WAIT;
    return;
    
consumer_process:
    if (!data.buffer.empty()) {
        int value = data.buffer.front();
        data.buffer.erase(data.buffer.begin());
        data.consumed_count++;
        
        std::cout << "Consumer: Processed " << value << " (total: " << data.consumed_count << ")\n";
        
        // Yield back to producer for more data
        state = CONSUMER_WAIT;
        return;
    }
    
    // No more data to process
    if (data.producer_done) {
        state = CONSUMER_END;
    } else {
        state = CONSUMER_WAIT;
    }
    return;
    
consumer_end:
    std::cout << "Consumer: Finished processing " << data.consumed_count << " items\n";
    return;
}

int main() {
    SharedData shared_data;
    ProducerState producer_state = PRODUCER_START;
    ConsumerState consumer_state = CONSUMER_START;
    
    std::cout << "=== Producer-Consumer Coroutines Demo ===\n\n";
    
    // Run the coroutines cooperatively
    while (producer_state != PRODUCER_END || consumer_state != CONSUMER_END) {
        // Run producer if not done
        if (producer_state != PRODUCER_END) {
            std::cout << "--- Producer Turn ---\n";
            producer_coroutine(shared_data, producer_state);
        }
        
        // Run consumer
        if (consumer_state != CONSUMER_END) {
            std::cout << "--- Consumer Turn ---\n";
            consumer_coroutine(shared_data, consumer_state, producer_state);
        }
        
        std::cout << "Buffer size: " << shared_data.buffer.size() << "\n\n";
    }
    
    std::cout << "=== Demo Complete ===\n";
    return 0;
}