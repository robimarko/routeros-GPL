/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 *
 * Cache-packing feedback collection entry points.
 */

#include "file.h"
#include <arch/chip.h>
#ifdef __KERNEL__
#define THREADS_SUPPORTED
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/sections.h>
#else
#include "threads.h"
#include "mmap.h"
#include "die.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#endif


/*
 * Our overall threading model is that each thread has a per-thread
 * LRUStack to track which functions it executed recently. It also has
 * a small ConflictMissGraph that tracks which potential conflicts it
 * has seen recently. Once a thread has seen enough conflicts, it
 * grabs a global lock and updates g_graph, the global
 * ConflictMissGraph, with its locally collected data. The idea is to
 * amortize locking overhead by only dumping values into the global
 * pool occasionally.
 */

struct FeedbackThreadData;


#ifdef THREADS_SUPPORTED
static void flush_deferred_graph(struct FeedbackThreadData *t);
#endif


#if !defined(__KERNEL__) && defined(THREADS_SUPPORTED)

#define THREAD_DATA_UNAVAILABLE ((struct FeedbackThreadData *)-1UL)


/*
 * Returns the raw FeedbackThreadData for the current thread.
 * There are three possible return values:
 *
 * NULL:
 *   There is no FeedbackThreadData allocated yet, but it is legal
 *   to create one and record it for later.
 * THREAD_DATA_UNAVAILABLE:
 *   There is no FeedbackThreadData available right now, and it would
 *   be wrong to make one (e.g. it is busy).
 * other:
 *   A valid FeedbackThreadData struct.
 */
static inline struct FeedbackThreadData *get_raw_feedback_thread_data(void)
{
	void **ptr = __feedback_get_raw_thread_data_ptr_ptr();
	return (ptr != NULL) ? (struct FeedbackThreadData *)*ptr :
		THREAD_DATA_UNAVAILABLE;
}


/*
 * Changes the raw FeedbackThreadData recorded on the current thread.
 * Assumes that get_raw_feedback_thread_data did not return
 * THREAD_DATA_UNAVAILABLE.
 */
static inline void set_raw_feedback_thread_data(struct FeedbackThreadData *t)
{
	void **ptr = __feedback_get_raw_thread_data_ptr_ptr();
	*ptr = t;
}


/* Destructor for __feedback_record_thread_data. */
void __feedback_destroy_thread_data(void *t)
{
	/* Dump any pending data into the global graph so we don't lose it. */
	flush_deferred_graph((struct FeedbackThreadData *)t);

	/*
	 * Indicate that this thread has no thread-specific data any more
	 * so that e.g. the following 'free' will not try to update it.
	 */
	set_raw_feedback_thread_data(THREAD_DATA_UNAVAILABLE);

	/* Free the dynamic storage used by this thread. */
	free(t);
}


#endif  /* THREADS_SUPPORTED */



/*
 *
 * LRUStack
 *
 */

/*
 * Track up to this many bytes of recently-executed functions. Anything
 * older than this is assumed to no longer be in the cache so we don't
 * care about any interference with it.
 * The coefficient 4 compensates for the fact that the compiler currently
 * initializes FeedbackFunction's num_bytes field with the size of the
 * instrumented function, not the estimated size of the optimized function.
 */
#define MAX_LRU_STACK_BYTES (4 * CHIP_L2_CACHE_SIZE())

/*
 * A limit on how many functions can appear on the stack.
 * If we have seen more than this many unique functions since the last
 * time we saw some function, it will almost certainly not still be in
 * the cache any more.
 */
#define MAX_LRU_STACK_FUNCTIONS 256


/*
 * An entry in the LRU stack. We store here the two fields of a
 * FeedbackFunction we need rather than the FeedbackFunction* itself
 * because the FeedbackFunctions on the stack could have been
 * unmapped by a dlclose.  There is also a cache efficiency argument
 * -- this keeps the memory contiguous when we scan the array, and
 * accessing FeedbackFunction fields can be much more expensive in
 * multithreaded scenarios because they are not homed on the current
 * thread.
 */
struct LRUStackEntry {
	/* ID of this function. */
	FunctionID function_id;

	/* Number of bytes occupied by this function. */
	unsigned int num_bytes;
};

struct LRUStack {
	/* Total estimated size of all functions on the stack. */
	unsigned int num_bytes;

	/* Total number of functions on the stack. */
	unsigned int num_functions;

	/* A stack, with the most recently accessed function at index 0. */
	struct LRUStackEntry functions[MAX_LRU_STACK_FUNCTIONS];
};


/* Remove the oldest function on the stack (i.e. the bottom of the stack). */
static void lru_stack_remove_oldest(struct LRUStack *stack)
{
	stack->num_functions--;
	stack->num_bytes -= stack->functions[stack->num_functions].num_bytes;
}


/*
 *
 * ConflictMissGraph
 *
 */


struct ConflictMissGraph {
	/* Open-addressed hash table. */
	Edge *edges;

	/* Length of the edges array (a power of two). */
	unsigned int max_edges;

	/* How many valid edges are in the edges array? */
	unsigned int num_edges;

};


/* Global conflict miss graph tracking all function cache interference. */
static struct ConflictMissGraph g_graph = {
	.edges = NULL,
	.max_edges = 0,
	.num_edges = 0
};


#ifndef __KERNEL__
static void cmg_grow_and_rehash(struct ConflictMissGraph *graph);
#endif

static inline bool edge_is_valid(const Edge *edge)
{
	return edge->f1 != 0;
}


static inline bool cmg_is_full(struct ConflictMissGraph *graph)
{
	/* We leave at least half the entries empty to speed searching. */
	return (graph->num_edges * 2 >= graph->max_edges);
}


/*
 * Looks up the requested edge in the graph and returns it, creating a
 * new one if necessary. The returned pointer is only valid until
 * the next call to this function.
 */
static Edge *cmg_find_or_create_edge(struct ConflictMissGraph* graph,
				     FunctionID f1, FunctionID f2)
{
	unsigned int i, mask;
	Edge *edges;

	if (cmg_is_full(graph)) {
#ifdef __KERNEL__
		/*
		 * Note: we might intead elect to just walk the graph
		 * and zero out all edges that contribute very little
		 * to the total edge weight.
		 */
		panic("feedback_max_edges=%d: used all feedback edges\n",
		      graph->max_edges);
#else
		/* Guarantee we have room in case we need to add a new edge. */
		cmg_grow_and_rehash(graph);
#endif
	}

	if (f1 > f2) {
		/* Canonicalize edge order, since it's an undirected graph. */
		FunctionID tmp = f1;
		f1 = f2;
		f2 = tmp;
	}

	mask = graph->max_edges - 1;
	edges = graph->edges;

	/*
	 * Search the hash table for a matching edge.
	 * This particular computation yields very good hash values.
	 */
	i = __insn_crc32_32(__insn_crc32_32(0, f1), f2);
	for ( ; ; i++) {
		Edge *e = &edges[i & mask];
		if (e->f1 == f1 && e->f2 == f2) {
			/* We found an exact match. */
			return e;
		} else if (!edge_is_valid(e)) {
			/*
			 * Fill in the first empty slot we found with the
			 * new edge.
			 */
			e->weight = 0;
			e->f2 = f2;
			e->f1 = f1;
			++graph->num_edges;
			return e;
		}
	}
}


#ifndef __KERNEL__

static void process_deferred_headers(void);


/* Doubles the size of 'graph', rehashing the entries. */
static void cmg_grow_and_rehash(struct ConflictMissGraph *graph)
{
	unsigned int new_size;

	const size_t old_max_edges = graph->max_edges;
	if (old_max_edges == 0) {
		/*
		 * Choose the smallest array size such that we will
		 * get an integral number of pages.
		 */
		new_size = getpagesize() >> __insn_ctz(sizeof(Edge));
	} else {
		/*
		 * Determine the new array size, requesting some
		 * number of pages.
		 */
		new_size = old_max_edges * 2;
	}

	/* Allocate the new array. */
	off_t file_offset;
	__feedback_lock(MUTEX_FILE);
	Edge *new_edges =
		__feedback_reserve_mmap_space(NULL, new_size * sizeof(Edge),
					      &file_offset);
	__feedback_unlock(MUTEX_FILE);

	/* We are required to check this after releasing MUTEX_FILE. */
	process_deferred_headers();

	/* Create a new graph to fill in with the rehashed entries. */
	struct ConflictMissGraph new_graph = {
		.edges = new_edges,
		.max_edges = new_size,
		.num_edges = 0
	};

	/* Insert edges from the old table into the new one. */
	Edge *old_edges = graph->edges;
	for (unsigned int i = 0; i < old_max_edges; i++) {
		Edge *e = &old_edges[i];
		if (edge_is_valid(e)) {
			Edge *new_edge =
				cmg_find_or_create_edge(&new_graph,
							e->f1, e->f2);
			new_edge->weight = e->weight;
		}
	}

	/* Keep the new, larger hash table. */
	*graph = new_graph;

	/*
	 * Update the file header to point to the new graph. Using
	 * pwrite works even if the first instrumented binary has
	 * already been unloaded.
	 */
	FeedbackHeader fake_header;
	fake_header.cmg_edge_array_size = new_size;
	fake_header.cmg_edge_array_file_offset = file_offset;
	__feedback_mmap_pwrite(&fake_header.cmg_edge_array_size,
			       sizeof(fake_header.cmg_edge_array_size) +
			       sizeof(fake_header.cmg_edge_array_file_offset),
			       offsetof(FeedbackHeader, cmg_edge_array_size));

	if (old_max_edges != 0)	{
		/*
		 * Free the old table array, zeroing it first to make
		 * the mmap file more compressible (why not?)
		 */
		memset(old_edges, 0, old_max_edges * sizeof(Edge));
		munmap(old_edges, old_max_edges * sizeof(Edge));
	}
}

#endif /* !__KERNEL__ */


/*
 * Increments the edge weight between f1 and f2, creating a new edge
 * if necessary.
 */
static inline void increment_edge_weight(struct ConflictMissGraph *graph,
					 FunctionID f1,
					 FunctionID f2)
{
	Edge *e = cmg_find_or_create_edge(graph, f1, f2);
	e->weight++;
}


/*
 * Per-thread cache packing data. This is used to periodically
 * update g_graph (under a lock), which all threads share.
 */
struct FeedbackThreadData {
	/* Is this thread currently already doing something in this library? */
	bool busy;

	/* Stack of recently executed functions. */
	struct LRUStack lru_stack;

#ifdef THREADS_SUPPORTED

#ifndef __KERNEL__
	/*
	 * How many edge increments we are allowed to do before dumping them
	 * into the global graph. The only reason there is any limit at all
	 * to keep the global graph more or less up to date, to handle
	 * various shutdown scenarios.
	 */
#define MAX_DEFERRED_INCREMENTS 16384
#endif

	/*
	 * How many edge increments we have done so far without dumping them
	 * into g_graph.
	 */
	unsigned int num_deferred_increments;

	/*
	 * To amortize locking overhead, we dump edges into this graph
	 * and periodically transfer these to the global graph.
	 */
	struct ConflictMissGraph deferred_graph;

	/*
	 * Size of the deferred_graph_edges array. This limits how many
	 * distinct edges we can see before we have to call
	 * flush_deferred_graph.  We make the kernel arrays bigger since we
	 * know we only have a fixed one-per-cpu model, so the memory used
	 * is limited, and this way we capture more locality.
	 */
#ifdef __KERNEL__
#define DEFERRED_GRAPH_EDGE_ARRAY_SIZE 65536
#else
#define DEFERRED_GRAPH_EDGE_ARRAY_SIZE 1024
#endif

	/*
	 * Raw storage for the above graph, which is not allowed to grow.
	 * Its size must be a power of two.
	 */
	Edge deferred_graph_edges[DEFERRED_GRAPH_EDGE_ARRAY_SIZE]
	__attribute__((aligned(CHIP_L2_LINE_SIZE())));

#endif  /* THREADS_SUPPORTED */
};


#ifdef __KERNEL__

/* Max number of edges; can be reset from the kernel command line. */
static unsigned long max_edges = 1 * 1024 * 1024;
static int __init set_max_edges(char *str)
{
	strict_strtoul(str, 0, &max_edges);
	return 1;
}
__setup("feedback_max_edges=", set_max_edges);

/* Pointer to and size of edges data; used from feedback_read() for /proc. */
static void *__feedback_edges_ptr;
static long __feedback_edges_size;	/* size in bytes */
extern int __feedback_edges_count;	/* size in edges: defined in entry.S */

static DEFINE_PER_CPU(struct FeedbackThreadData, feedback_thread_data)
	__attribute__((aligned(CHIP_L2_LINE_SIZE())));

/* Start collecting cache-pack profiling data during init. */
int __init feedback_init(void)
{
	extern FeedbackFunction __feedback_functions_start[];
	extern FeedbackFunction __feedback_functions_end[];
	int order, i;
	long size;
	FeedbackFunction *f;
	struct page *page;

	if (max_edges > 0) {
		/* Fill in function IDs. */
		FunctionID id = 1;
		for (f = __feedback_functions_start;
		     f < __feedback_functions_end; f++)
			f->id = id++;

		/*
		 * Calculate memory to hold the edge data, rounding up
		 * max_edges.
		 */
		order = get_order(max_edges * sizeof(Edge));
		size = PAGE_SIZE << order;
		max_edges = size / sizeof(Edge);

		/* Allocate memory and initialize g_graph. */
		page = alloc_pages(GFP_KERNEL | __GFP_ZERO | __GFP_COMP, order);
		if (page == NULL)
			panic("feedback_max_edges=%ld: too many edges; not enough memory\n",
			      max_edges);
		__feedback_edges_ptr = (Edge *)page_address(page);
		__feedback_edges_count = max_edges;
		__feedback_edges_size = max_edges * sizeof(Edge);
		g_graph.edges = __feedback_edges_ptr;
		g_graph.max_edges = max_edges;

		/*
		 * Initialize per-cpu data, which is what enables actual
		 * tracking.
		 */
		__insn_mf();
		for (i = 0; i < NR_CPUS; ++i) {
			struct FeedbackThreadData *t =
				&per_cpu(feedback_thread_data, i);
			t->deferred_graph.max_edges =
				DEFERRED_GRAPH_EDGE_ARRAY_SIZE;
			t->deferred_graph.edges = t->deferred_graph_edges;
		}
#ifdef CONFIG_HOTPLUG_CPU
#error Need to call __feedback_destroy_thread_data when cpu is taken offline.
#endif

		pr_info("Enabled cache-packing feedback, max %lu edges\n",
			max_edges);
	} else {
		pr_info("Disabled cache-packing feedback\n");
	}
	return 0;
}
arch_initcall(feedback_init);

/* Inline the kernel thread functions. */

static DEFINE_RAW_SPINLOCK(graph_lock);
static unsigned long graph_lock_flags;

#define MUTEX_GRAPH 0   /* the only mutex type used in kernel code */

static inline int __feedback_using_threads(void)
{
	return 1;
}

static inline void __feedback_lock(int which)
{
	unsigned long flags;
	raw_local_irq_save(flags);
	preempt_disable();
	__raw_spin_lock(&graph_lock);
	graph_lock_flags = flags;
}

static inline void __feedback_unlock(int which)
{
	unsigned long flags = graph_lock_flags;
	__raw_spin_unlock(&graph_lock);
	raw_local_irq_restore(flags);
	preempt_enable();
}

static inline struct FeedbackThreadData *get_feedback_thread_data(void)
{
	struct FeedbackThreadData *t = &__get_cpu_var(feedback_thread_data);
	return t->deferred_graph.edges ? t : NULL;
}

void flush_my_deferred_graph(void *dummy)
{
	struct FeedbackThreadData *t = get_feedback_thread_data();
	if (t)
		flush_deferred_graph(t);
}

#else


/*
 * Returns the struct that holds cache packing data for this thread,
 * or NULL if none is available, in which case the caller should
 * do nothing.
 */
static struct FeedbackThreadData*
get_feedback_thread_data(void)
{
	/*
	 * Default data to use when there are no threads involved.
	 * There is no need to initialize it since we won't actually use
	 * anything but its LRUStack and busy flag, and zeroed fields are a
	 * fine default value for those.
	 */
	static struct FeedbackThreadData thread_data_if_no_threads;

#ifndef THREADS_SUPPORTED

	return &thread_data_if_no_threads;

#else  /* THREADS_SUPPORTED */

	if (!__feedback_using_threads())
		return &thread_data_if_no_threads;

	struct FeedbackThreadData *ret = get_raw_feedback_thread_data();
	if (ret != NULL) {
		if (__builtin_expect(ret == THREAD_DATA_UNAVAILABLE, 0)) {
			/*
			 * We are recursing while still trying to create
			 * the thread data, so don't return anything.
			 */
			return NULL;
		} else {
			return ret;
		}
	}

	/* We need to allocate new thread-local storage. */

	/*
	 * Prevent infinite recursion while we allocate the data by temporarily
	 * using a distinguished non-null pointer.
	 */
	set_raw_feedback_thread_data(THREAD_DATA_UNAVAILABLE);

	/* Allocate and initialize the new struct. */
	ret = malloc(sizeof *ret);
	if (ret == NULL)
		__feedback_die("malloc failed: %s.", strerror(errno));
	memset(ret, 0, sizeof *ret);
	ret->deferred_graph.edges = ret->deferred_graph_edges;
	ret->deferred_graph.max_edges = DEFERRED_GRAPH_EDGE_ARRAY_SIZE;

	int err = __feedback_record_thread_data(ret);
	if (err != 0)
		__feedback_die("Unable to make pthread key: %s.",
			       strerror(err));

	/* Record the final pointer in our fast location. */
	set_raw_feedback_thread_data(ret);

	return ret;

#endif /* THREADS_SUPPORTED */
}

#endif /* __KERNEL__ */


#ifdef THREADS_SUPPORTED

/* Dump any edges from the deferred graph into the global graph. */
static void flush_deferred_graph(struct FeedbackThreadData *t)
{
	Edge *global;
	Edge *local = &t->deferred_graph_edges[0];
	Edge *e;
	Edge *e2;
	unsigned int mask = g_graph.max_edges - 1;
	unsigned int valid_edges, i;

	/*
	 * Squish down the local table so it's more compact.
	 * This reduces the amount of time we hold the lock a bit,
	 * and makes the prefetching more effective as well.
	 */
	valid_edges = 0;
	for (i = 0; i < DEFERRED_GRAPH_EDGE_ARRAY_SIZE; i++) {
		e = &local[i];

		if (edge_is_valid(e)) {
			/* Copy aside the edge. */
			Edge edge = *e;

			/* Invalidate its entry at its old index. */
			e->f1 = 0;

			/* Record this edge in the compact array. */
			local[valid_edges++] = edge;
		}
	}

	__feedback_lock(MUTEX_GRAPH);

	/* Insert edges from the old table into the new one. */
	global = &g_graph.edges[0];
	for (i = 0; i < valid_edges; i++) {
		/* Prefetch the local graph. */
		int prefetch = i + 32;
		if (prefetch < valid_edges)
			__insn_prefetch(&local[prefetch]);

		/*
		 * Prefetch the global graph (which depends on the local
		 * graph entries).
		 */
		prefetch = i + 16;
		if (prefetch < valid_edges) {
			unsigned int hash;
			e = &local[prefetch];
			hash = __insn_crc32_32(__insn_crc32_32(0, e->f1),
					       e->f2);
			__insn_prefetch(&global[hash & mask]);
		}

		/* Add the local edge weight to the appropriate global edge. */
		e = &local[i];
		e2 = cmg_find_or_create_edge(&g_graph, e->f1, e->f2);
		e2->weight += e->weight;

		/* Discard the deferred edge now that we have processed it. */
		e->f1 = 0;
	}

	__feedback_unlock(MUTEX_GRAPH);

	t->deferred_graph.num_edges = 0;
	t->num_deferred_increments = 0;
}

#endif  /*! THREADS_SUPPORTED */


/*
 * Updates cache packing data collection information to note that
 * we are now executing function f.
 */
void __feedback_function_entered(FeedbackFunction *func)
{
	struct FeedbackThreadData *t;
	struct LRUStack *stack;
	FunctionID f;
	unsigned int func_num_bytes, i;
	int j;
	bool using_threads, was_busy;
#ifdef __KERNEL__
	unsigned long flags;
#endif

#ifndef __KERNEL__
	if (__builtin_expect(!__feedback_is_ready(), 0)) {
		/* Very early on startup. Do nothing. */
		return;
	}
#endif

	t = get_feedback_thread_data();
#ifdef THREADS_SUPPORTED
	if (__builtin_expect(t == NULL, 0))
		return;
#endif

	/* Capture the old value of "busy", and set it to true. */
#ifdef __KERNEL__
	raw_local_irq_save(flags);  /* don't skip edges due to "busy" */
#endif
	was_busy = t->busy;
	t->busy = true;

	if (__builtin_expect(was_busy, 0)) {
		/* Recursing on this thread, so do nothing. */
		t->busy = was_busy;
#ifdef __KERNEL__
		raw_local_irq_restore(flags);
#endif
		return;
	}

	stack = &t->lru_stack;
	f = func->id;
	func_num_bytes = func->num_bytes;

	using_threads = __feedback_using_threads();

	/*
	 * Search the stack to see if 'f' is already somewhere on it.
	 * 'f' will almost always be near the top of the stack so this is OK.
	 */
	for (i = 0; i < stack->num_functions; i++) {
		if (stack->functions[i].function_id == f) {
			/*
			 * This is the common case: 'f' is already on the
			 * stack.  Increment edge weights between 'f' and
			 * everything accessed since 'f' was last accessed,
			 * since those are the functions that could have
			 * evicted 'f' since we last executed it. Slide
			 * over the stack to make room for 'f' at the head
			 * while we are at it.
			 */
			for (j = i - 1; j >= 0; j--) {
				FunctionID other_id =
					stack->functions[j].function_id;
				stack->functions[j + 1] = stack->functions[j];

				if (!using_threads) {
					/*
					 * Without threads, just modify the
					 * global graph directly.
					 */
					increment_edge_weight(&g_graph, f,
							      other_id);
				}
#ifdef THREADS_SUPPORTED
				else {
#ifndef __KERNEL__
					if (t->num_deferred_increments >=
					    MAX_DEFERRED_INCREMENTS)
						flush_deferred_graph(t);
					++t->num_deferred_increments;
#endif

					if (cmg_is_full(&t->deferred_graph))
						flush_deferred_graph(t);

					/*
					 * Increment edge weights in the
					 * deferred graph.
					 */
					increment_edge_weight(
						&t->deferred_graph,
						f, other_id);
				}
#endif
			}

			/* Set f as the top of the stack. */
			stack->functions[0].function_id = f;
			stack->functions[0].num_bytes = func_num_bytes;

			t->busy = was_busy;
#ifdef __KERNEL__
			raw_local_irq_restore(flags);
#endif
			return;
		}
	}

	/*
	 * Function is not yet on the stack, so push it on.
	 */

	/*
	 * Age out old entries to make room, to model the limited size of
	 * the cache, but always keep at least three entries to avoid
	 * pathological behavior with large functions.
	 */
	while (stack->num_bytes + func_num_bytes > MAX_LRU_STACK_BYTES &&
	       stack->num_functions > 3) {
		lru_stack_remove_oldest(stack);
	}

	/*
	 * If we have too many distinct functions on the stack, make room for
	 * the new one.
	 */
	if (stack->num_functions == MAX_LRU_STACK_FUNCTIONS)
		lru_stack_remove_oldest(stack);

	/*
	 * Slide over the rest of the stack to make room.
	 * We use this loop instead of memmove in case memmove is instrumented.
	 */
	for (j = stack->num_functions - 2; j >= 0; j--)
		stack->functions[j + 1] = stack->functions[j];

	/* Set the top of the stack to 'f'. */
	stack->functions[0].function_id = f;
	stack->functions[0].num_bytes = func_num_bytes;

	stack->num_functions++;
	stack->num_bytes += func_num_bytes;

	t->busy = was_busy;
#ifdef __KERNEL__
	raw_local_irq_restore(flags);
#endif
}
#ifdef __KERNEL__
EXPORT_SYMBOL(__feedback_function_entered);
#endif

/* For now, we treat resumed and entered exactly the same. */
void __feedback_function_resumed(FeedbackFunction *f)
	__attribute__((alias("__feedback_function_entered")));
#ifdef __KERNEL__
EXPORT_SYMBOL(__feedback_function_resumed);
#endif

#ifndef __KERNEL__

/*
 * A linked list of FeedbackHeaders waiting to be written out to the
 * memory-mapped output file as soon as that file is opened.
 *
 * Protected by MUTEX_DEFERRED.
 */
static FeedbackHeader *g_deferred;

/*
 * The directory into which we will create the memory-mapped file,
 * or NULL if unknown.
 *
 * Protected by MUTEX_DEFERRED.
 */
static const char *g_feedback_dir;


/*
 * Return true if there is some deferred work to process the next
 * time we grab MUTEX_FILE. By the time we grab MUTEX_FILE it might
 * have already been done by another thread, but at least this tells
 * us whether we should check.
 */
static bool unlock_and_check_for_more_work(void)
{
	__feedback_unlock(MUTEX_DEFERRED);
	__feedback_unlock(MUTEX_FILE);

	__feedback_lock(MUTEX_DEFERRED);
	bool ret = (g_feedback_dir != NULL ||
		    (__feedback_is_ready() && g_deferred != NULL));
	__feedback_unlock(MUTEX_DEFERRED);

	return ret;
}


/*
 * This processes any headers in the g_deferred list.
 *
 * On entry, the current thread's 'busy' flag must be true,
 * so we will not attempt to grab MUTEX_GRAPH.
 *
 * This function MUST be called by anyone releasing MUTEX_FILE.
 */
static void process_deferred_headers(void)
{
	while (1) {
		if (__feedback_is_exiting())
			return;

		if (__feedback_trylock(MUTEX_FILE) != 0) {
			/*
			 * Someone already has the file lock (possibly even
			 * this thread) so processing headers is their
			 * problem. Give up.
			 */
			return;
		}

		/* Check the list of deferred headers to see if it is empty. */
		__feedback_lock(MUTEX_DEFERRED);
		FeedbackHeader *h = g_deferred;
		if (h == NULL) {
			if (unlock_and_check_for_more_work())
				continue;
			else
				return;
		}

		if (__feedback_is_ready()) {
			g_deferred = h->u.next_deferred;
			__feedback_unlock(MUTEX_DEFERRED);
		} else {
			/* Open the file. */
			const char *dir = g_feedback_dir;
			if (dir == NULL) {
				if (unlock_and_check_for_more_work())
					continue;
				else
					return;
			}

			/*
			 * NULL out the directory name, since we are taking
			 * care of it.
			 */
			g_feedback_dir = NULL;

			/*
			 * Pop off the header, since we are about to write
			 * it out.
			 */
			g_deferred = h->u.next_deferred;

			__feedback_unlock(MUTEX_DEFERRED);

			/* Actually open the file. */
			__feedback_create_file(dir);

			/*
			 * NOTE: now we are now committed to writing 'h' to
			 * the file no matter what, since it must be the
			 * first thing in the file so the file gets the
			 * right header. We cannot release MUTEX_FILE until
			 * this happens.
			 */
		}

		/*
		 * At this point:
		 *
		 * - we hold MUTEX_FILE
		 * - we do not hold MUTEX_DEFERRED
		 * - h holds one header to emit to the file.
		 */

		h->u.relative_file_offset_to_next_header = 0;

		/* Assign all functions unique IDs. */
		FeedbackFunction *start =
			REL_FIELD(h, relative_functions_start,
				  FeedbackFunction*);
		FeedbackFunction *end =
			REL_FIELD(h, relative_functions_end,
				  FeedbackFunction*);

		/* Assign each function a unique ID. 0 is an invalid ID. */
		static FunctionID next_id = 1;
		FunctionID id = next_id;
		for (FeedbackFunction *f = start; f < end; f++)
			f->id = id++;
		next_id = id;

		/* Map this section into our memory mapped file. */
		off_t offset;
		__feedback_reserve_mmap_space(h, h->section_size, &offset);

		/*
		 * Only once the section has been completely written out do
		 * we set the magic number field, in case this program is
		 * killed partway through writing out the section.
		 */
		static const unsigned int magic = FEEDBACK_HEADER_MAGIC;
		__feedback_mmap_pwrite(&magic, sizeof(magic), offset +
				       offsetof(FeedbackHeader, magic));

		/*
		 * Append this header to the mmap file's linked list
		 * of all headers.  The tricky thing here is that the
		 * previous header could conceivably have been
		 * unmapped by a dlclose. So we need to write it
		 * directly.
		 */
		static off_t previous_header_file_offset = (off_t)-1ULL;
		if (previous_header_file_offset != (off_t)-1ULL) {
			unsigned int rel_offset =
				(unsigned int)(offset -
					       previous_header_file_offset);
			size_t field_offset =
				offsetof(FeedbackHeader,
					 u.relative_file_offset_to_next_header);

			__feedback_mmap_pwrite(&rel_offset, sizeof(rel_offset),
					       previous_header_file_offset +
					       field_offset);
		}
		previous_header_file_offset = offset;

		/* Release the file lock and check for more work to do. */
		__feedback_unlock(MUTEX_FILE);
	}
}


/*
 * This is called each time a new binary (executable
 * or shared library) is loaded, so it can register its instrumented
 * functions.
 */
void __feedback_register(FeedbackHeader *header,
			 const char *feedback_dir_or_null)
{
	/* Disable __feedback_function_entered. */
	struct FeedbackThreadData *const t = get_feedback_thread_data();
	bool was_busy = false;
	if (t != NULL) {
		was_busy = t->busy;
		t->busy = true;
	}

	__feedback_lock(MUTEX_DEFERRED);

	/* Prepend this to the linked list of headers to process. */
	header->u.next_deferred = g_deferred;
	g_deferred = header;

	if (g_feedback_dir == NULL && !__feedback_is_ready())
		g_feedback_dir = feedback_dir_or_null;

	__feedback_unlock(MUTEX_DEFERRED);

	process_deferred_headers();

	/* Reenable __feedback_function_entered. */
	if (t != NULL)
		t->busy = was_busy;
}


void __feedback_unregister(FeedbackHeader *header)
{
#ifdef THREADS_SUPPORTED
	/*
	 * Paranoia: something is being shut down, perhaps the entire
	 * program.  Now is a convenient time to flush whatever this thread
	 * has deferred in case this is the main thread and we are exiting.
	 */
	struct FeedbackThreadData *const t = get_feedback_thread_data();
	if (t != NULL)
		flush_deferred_graph(t);
#endif

	__feedback_lock(MUTEX_DEFERRED);

	/*
	 * Splice this header out of the g_deferred list, in case we somehow
	 * had not yet gotten around to writing it out.
	 */
	FeedbackHeader *h;
	for (FeedbackHeader **hp = &g_deferred;
	     (h = *hp) != NULL;
	     hp = &h->u.next_deferred) {
		if (h == header) {
			*hp = h->u.next_deferred;
			break;
		}
		hp = &h->u.next_deferred;
	}

	__feedback_unlock(MUTEX_DEFERRED);
}

#else

static ssize_t feedback_read(struct file *file, char __user *buf, size_t size,
			     loff_t *ppos)
{
	void *start = __feedback_section_start;
	size_t avail = __feedback_section_end - __feedback_section_start;

	if (*ppos == 0)
		on_each_cpu_mask(cpu_online_mask,
				 flush_my_deferred_graph, NULL, 1);
	if (*ppos < avail) {
		/* Return a short read as we cross into edges data. */
		if (*ppos + size > avail)
			size = avail - *ppos;
	} else {
		/* Bias the start to below the actual edges data. */
		start = __feedback_edges_ptr - avail;
		avail += __feedback_edges_size;
	}

	return simple_read_from_buffer(buf, size, ppos, start, avail);
}
static const struct file_operations proc_tile_feedback_fops = {
	.read		= feedback_read
};

static int proc_tile_feedback_init(void)
{
	struct proc_dir_entry *entry =
		create_proc_entry("tile/feedback", 0444, NULL);
	if (entry)
		entry->proc_fops = &proc_tile_feedback_fops;
        return 0;
}
late_initcall(proc_tile_feedback_init);

#endif /* !__KERNEL__ */
