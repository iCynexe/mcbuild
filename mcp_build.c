#include <stdarg.h>

#include "mcp_ids.h"
#include "mcp_build.h"
#include "mcp_gamestate.h"

////////////////////////////////////////////////////////////////////////////////
// Helpers

static int scan_opt(char **words, const char *fmt, ...) {
    int i;
    
    const char * fmt_opts = index(fmt, '=');
    assert(fmt_opts);
    ssize_t optlen = fmt_opts+1-fmt; // the size of the option name with '='

    for(i=0; words[i]; i++) {
        if (!strncmp(words[i],fmt,optlen)) {
            va_list ap;
            va_start(ap,fmt);
            int res = vsscanf(words[i]+optlen,fmt+optlen,ap);
            va_end(ap);
            return res;
        }
    }

    return 0;
}

#define SQ(x) ((x)*(x))

////////////////////////////////////////////////////////////////////////////////
// Structures

#define DIR_UP      0
#define DIR_DOWN    1
#define DIR_SOUTH   2
#define DIR_NORTH   3
#define DIR_EAST    4
#define DIR_WEST    5

// this structure is used to define an absolute block placement
// in the active building process
typedef struct {
    int32_t     x,y,z;          // coordinates of the block to place
    bid_t       b;              // block type, including the meta

    // state flags
    union {
        int8_t  state;
        struct {
            int8_t placed  : 1; // true if this block is already in place
            int8_t blocked : 1; // true if this block is obstructed by something else
            int8_t inreach : 1; // this block is close enough to place
            int8_t pending : 1; // block was placed but pending confirmation from the server
        };
    };

    // a bitmask of neighbors (6 bits only),
    // set bit means there is a neighbor in that direction
    union {
        int8_t  neigh;
        struct {
            int8_t  n_yp : 1;   // up    (y-pos)
            int8_t  n_yn : 1;   // down  (y-neg)
            int8_t  n_zp : 1;   // south (z-pos)
            int8_t  n_zn : 1;   // north (z-neg)
            int8_t  n_xp : 1;   // east  (x-pos)
            int8_t  n_xn : 1;   // west  (x-neg)
        };
    };

    int32_t dist; // distance to the block center (squared)
} blk;

// this structure defines a relative block placement 
typedef struct {
    int32_t     x,y,z;  // coordinates of the block to place (relative to pivot)
    bid_t       b;      // block type, including the meta
                        // positional meta is north-oriented
} blkr;

// maximum number of blocks in the buildable list
#define MAXBUILDABLE 1024

struct {
    int active;
    lh_arr_declare(blk,task);  // current active building task
    lh_arr_declare(blkr,plan); // currently loaded/created buildplan

    int buildable[MAXBUILDABLE];
} build;

#define BTASK GAR(build.task)
#define BPLAN GAR(build.plan)

////////////////////////////////////////////////////////////////////////////////

// maximum reach distance for building, squared, in fixp units (1/32 block)
#define MAXREACH SQ(5<<5)

void build_update() {
    // player position or look have changed - update our placeable blocks list
    if (!build.active) return;

    //TODO: recalculate placeable blocks list

    int i;

    // 1. Update 'inreach' flag for all blocks and set the block distance
    // inreach=1 does not necessarily mean the block is really reachable -
    // this will be determined later in more detail, but those with
    // inreach=0 are definitely too far away to bother.
    for(i=0; i<C(build.task); i++) {
        blk *b = P(build.task)+i;
        int32_t dx = gs.own.x-(b->x<<5)+16;
        int32_t dy = gs.own.y-(b->y<<5)+16;
        int32_t dz = gs.own.z-(b->z<<5)+16;
        b->dist = SQ(dx)+SQ(dy)+SQ(dz);

        b->inreach = (b->dist<MAXREACH);
    }

    /* Further strategy:
       - determine which neighbors are available for the blocks 'inreach'

       - skip those neighbor faces looking away from you

       - skip those neighbor faces unsuitable for the block orientation
         you want to achieve - for now we can skip that for the plain
         blocks - they can be placed on any neighbor. Later we'll need
         to determine this properly for the stairs, slabs, etc.

       - for each neighbor face, calculate which from 15x15 points can be
         'clicked' to be able to place the block we want the way we want.
         For now, we can just say "all of them" - this will work with plain
         blocks. Later we can introduce support for slabs, stairs etc., e.g.
         in order to place the upper slab we will have to choose only the
         upper 15x7 block from the matrix.

       - for each of the remaining points, calculate their direct visibility
         this is optional for now, because it's obviously very difficult
         to achieve and possibly not even checked properly.

       - for each of the remaining points, calculate their exact distance,
         skip those farther away than 4.0 blocks (this is now the proper
         in-reach calculation)

       - for each of the remaining points, store the one with the largest
         distance in the blk - this will serve as the selector for the
         build-the-most-distant-blocks-first strategy to avoid isolating blocks

       - store the suitable dots (as a bit array in a 16xshorts?) in the
         blk struct

       - when building, select the first suitable block for building,
         and choose a random dot from the stored set

    */
}

void build_progress(MCPacketQueue *sq, MCPacketQueue *cq) {
    // time update - try to build any blocks from the placeable blocks list
    if (!build.active) return;

    //TODO: select one of the blocks from the buildable list and place it
}

////////////////////////////////////////////////////////////////////////////////

static void build_floor(char **words, char *reply) {
    build_clear();

    int xsize,zsize;
    if (scan_opt(words, "size=%d,%d", &xsize, &zsize)!=2) {
        sprintf(reply, "Usage: build floor size=<xsize>,<zsize>");
        return;
    }
    if (xsize<=0 || zsize<=0) return;

    //TODO: material
    bid_t mat = { .bid=0x04, .meta=0 };

    int x,z;
    for(x=0; x<xsize; x++) {
        for(z=0; z<zsize; z++) {
            blkr *b = lh_arr_new(BPLAN);
            b->b = mat;
            b->x = x;
            b->z = -z;
            b->y = 0;
        }
    }

    char buf[256];
    sprintf(reply, "Created floor %dx%d material=%s\n",
            xsize, zsize, get_bid_name(buf, mat));
}

//TODO: orientation and rotate brec accordingly
static void build_place(char **words, char *reply) {
    // check if we have a plan
    if (!C(build.plan)) {
        sprintf(reply, "You have no active buildplan!\n");
        return;
    }

    // parse coords
    int px,py,pz;
    if (scan_opt(words, "coord=%d,%d,%d", &px, &pz, &py)!=3) {
        sprintf(reply, "Usage: build place coord=<x>,<z>,<y>");
        return;
    }
    sprintf(reply, "Place pivot at %d,%d (%d)\n",px,pz,py);

    // abort current buildtask
    build_cancel();

    // create a new buildtask from our buildplan
    int i;
    for(i=0; i<C(build.plan); i++) {
        blkr *bp = P(build.plan)+i;
        blk  *bt = lh_arr_new_c(BTASK); // new element in the buildtask

        bt->x = bp->x+px;
        bt->y = bp->y+py;
        bt->z = bp->z+pz;
        bt->b = bp->b;
    }
    build.active = 1;
    build_update();
}

////////////////////////////////////////////////////////////////////////////////

//TODO: print needed material amounts
void build_dump_plan() {
    int i;
    char buf[256];
    for(i=0; i<C(build.plan); i++) {
        blkr *b = &P(build.plan)[i];
        printf("%3d %+4d,%+4d,%3d %3x/%02x (%s)\n",
               i, b->x, b->z, b->y, b->b.bid, b->b.meta, get_bid_name(buf, b->b));
    }
}

void build_dump_task() {
    int i;
    char buf[256];
    for(i=0; i<C(build.task); i++) {
        blk *b = &P(build.task)[i];
        printf("%3d %+5d,%+5d,%3d %3x/%02x dist=%-5d %c (%s)\n",
               i, b->x, b->z, b->y, b->b.bid, b->b.meta,
               b->dist, b->inreach?'*':' ',
               get_bid_name(buf, b->b));
    }
}

void build_clear() {
    build_cancel();
    lh_arr_free(BPLAN);
}

void build_cancel() {
    build.active = 0;
    lh_arr_free(BTASK);
    build.buildable[0] = -1;
}

void build_cmd(char **words, MCPacketQueue *sq, MCPacketQueue *cq) {
    char reply[32768];
    reply[0]=0;

    if (!words[1]) {
        sprintf(reply, "Usage: build <type> [ parameters ... ] or build cancel");
    }
    else if (!strcmp(words[1], "floor")) {
        build_floor(words+2, reply);
    }
    else if (!strcmp(words[1], "place")) {
        build_place(words+2, reply);
    }
    else if (!strcmp(words[1], "cancel")) {
        build_cancel();
    }
    else if (!strcmp(words[1], "dumpplan")) {
        build_dump_plan();
    }
    else if (!strcmp(words[1], "dumptask")) {
        build_dump_task();
    }

    if (reply[0]) chat_message(reply, cq, "green", 0);
}
