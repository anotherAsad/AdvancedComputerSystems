#define SHORT 0
#define FLOAT 1

#define DISABLED 0
#define ENABLED  1

typedef void TilingAgent_ptr(int tile_vlen, int tile_hlen);

void TilingAgent_original(int tile_vlen, int tile_hlen);
void TilingAgent_classic(int tile_vlen, int tile_hlen);

void *ThreadableTilingAgent(void *threadArgs);

void ThreadScheduler(int num_threads, int tile_vlen, int tile_hlen);