#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE 2

typedef struct IpPortPair {
    char *ip;
    int port;
    int port_nm;
    int idx;
} IpPortPair;

typedef struct {
    char *keys[CACHE_SIZE]; // File name
    IpPortPair values[CACHE_SIZE]; // IP and port pair
    // char *ip[CACHE_SIZE];
    // int port[CACHE_SIZE];
    int lru_order[CACHE_SIZE]; // Maintains order for least recently used
    int count;
} LRUCache;

// Initialize cache
void init_cache(LRUCache *cache) {
    cache->count = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache->keys[i] = NULL;
        cache->values[i].ip = NULL;
        cache->values[i].port = -1;
        cache->values[i].port_nm = -1;
        cache->values[i].idx = -1;
        // cache->port[i] = -1;
        // cache->ip[i] = NULL;
        cache->lru_order[i] = -1;
    }
}

// Update LRU information
void update_lru(LRUCache *cache, int index) {
    int pos = -1;
    for (int i = 0; i < cache->count; i++) {
        if (cache->lru_order[i] == index) {
            pos = i;
            break;
        }
    }
    if (pos == -1) return;

    for (int i = pos; i < cache->count - 1; i++) {
        cache->lru_order[i] = cache->lru_order[i + 1];
    }
    cache->lru_order[cache->count - 1] = index;
}

// Update cache usage
void use_cache(LRUCache *cache, int index) {
    if (cache->count < CACHE_SIZE) {
        cache->lru_order[cache->count++] = index;
    } else {
        int lru_index = cache->lru_order[0];
        for (int i = 1; i < CACHE_SIZE; i++) {
            cache->lru_order[i - 1] = cache->lru_order[i];
        }
        cache->lru_order[CACHE_SIZE - 1] = index;
        free(cache->keys[lru_index]);
        cache->keys[lru_index] = NULL;
        free(cache->values[lru_index].ip);
        cache->values[lru_index].ip = NULL;
        cache->values[lru_index].port = -1;
        cache->values[lru_index].port_nm = -1;
        cache->values[lru_index].idx = -1;
    }
}

// Fetch IP and port if it's a hit
IpPortPair get(LRUCache *cache, const char *key) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->keys[i] && strcmp(cache->keys[i], key) == 0) {
            update_lru(cache, i);
            return cache->values[i];
        }
    }
    return (IpPortPair){NULL, -1, -1, -1}; // Cache miss
}

// Insert key, IP, and port into cache
void put(LRUCache *cache, const char *key, const char *ip, int port, int port_nm, int idx) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->keys[i] && strcmp(cache->keys[i], key) == 0) {
            free(cache->values[i].ip);
            cache->values[i].ip = strdup(ip);
            cache->values[i].port = port;
            cache->values[i].port_nm = port_nm;
            cache->values[i].idx = idx;
            update_lru(cache, i);
            return;
        }
    }

    int index = -1;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->keys[i] == NULL) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        index = cache->lru_order[0];
        update_lru(cache, index);
    } else {
        use_cache(cache, index);
    }

    cache->keys[index] = strdup(key);
    cache->values[index].ip = strdup(ip);
    cache->values[index].port = port;
    cache->values[index].port_nm = port_nm;
    cache->values[index].idx = idx;
}

void print_cache(LRUCache *cache) {
    printf("Cache contents (most recently used to least recently used):\n");
    for (int i = cache->count - 1; i >= 0; i--) {
        int index = cache->lru_order[i];
        if (cache->keys[index] != NULL) {
            printf("Key: %s, IP: %s, Port: %d, Port_nm: %d, Index: %d\n", cache->keys[index], cache->values[index].ip, cache->values[index].port, cache->values[index].port_nm, cache->values[index].idx);
        }
    }
}

void delete_from_cache(LRUCache *cache, const char *key) {
    int index = -1;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->keys[i] && strcmp(cache->keys[i], key) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Key not found in cache.\n");
        return; // Key not found in cache
    }

    // Free memory allocated for the key and value
    free(cache->keys[index]);
    free(cache->values[index].ip);

    // Set key and value to NULL
    cache->keys[index] = NULL;
    cache->values[index].ip = NULL;
    cache->values[index].port = -1;
    cache->values[index].port_nm = -1;
    cache->values[index].idx = -1;

    // Update the LRU order
    int pos = -1;
    for (int i = 0; i < cache->count; i++) {
        if (cache->lru_order[i] == index) {
            pos = i;
            break;
        }
    }

    if (pos != -1) {
        for (int i = pos; i < cache->count - 1; i++) {
            cache->lru_order[i] = cache->lru_order[i + 1];
        }
        cache->lru_order[cache->count - 1] = -1;
        cache->count--;
    }
}