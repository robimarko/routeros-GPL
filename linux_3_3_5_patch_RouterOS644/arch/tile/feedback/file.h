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
 * Cache-packing feedback collection file API.
 */

#ifndef _FEEDBACK_FILE_H
#define _FEEDBACK_FILE_H


/* FIXME: tile-gcc: feedback: this constant is defined in two places
   (tile-feedback.h)  */
#define FEEDBACK_HEADER_MAGIC   0x4fd5adb1
#define FEEDBACK_HEADER_VERSION 2


/* FIXME: tile-gcc: feedback: counter information derived from
   gcov-io.h.  */
#define GCOV_COUNTERS 8
#define GCOV_COUNTERS_SUMMABLE	1

#ifndef __ASSEMBLER__

/* Turns a struct-relative pointer field into an absolute pointer. */
#define REL_FIELD(f, field, type)			\
	((type)((const char *)(f) + (f)->field))

#define REL_FIELD_FOR_ARRAY(f, field, elt, type)	\
	((type)((const char *)(f) + (f)->field[elt]))

/* Unique integer ID assigned to each function. 0 is an invalid ID. */
typedef unsigned int FunctionID;

/* A counter descriptor (see FB_INSTR_INFO). */
typedef unsigned int FeedbackInstrInfo;

/* Compiler's type name for a counter. */
typedef unsigned long long FeedbackCounter;


/*
 * A function with enough linker information for the cache packer
 * to locate it and place it.
 *
 * WARNING: The compiler generates these statically so it must
 * be updated whenever this definition changes.
 *
 * WARNING: If you change this, update the FEEDBACK_ENTRY macro.
 */
typedef struct FeedbackFunction {
	/*
	 * Unique ID number assigned to each function when containing
	 * binary is loaded.
	 */
	FunctionID id;

	/*
	 * Estimated size in bytes of this function when optimized after
	 * feedback (e.g. not instrumented).
	 */
	const unsigned int num_bytes;

	/*
	 * Relative byte offset to function name string (const char*).
	 * The address is relative to the start of this struct.
	 */
	const int relative_function_name;

	/*
	 * Relative byte offset to section name string (const char*).
	 * The address is relative to the start of this struct.
	 */
	const int relative_section_name;

	/*
	 * Relative byte offset to source pathname string (const char*).
	 * The address is relative to the start of this struct.
	 */
	const int relative_source_file_name;

	/* GCC checksum. */
	const unsigned int checksum;

	/* GCC function id, which is only unique per source file. */
	const unsigned int gcc_id;

	/* Length of relative_counters array. */
	const unsigned int num_counters[GCOV_COUNTERS];

	/*
	 * Relative byte offset to the array of counters (const
	 * FeedbackCounter*).
	 */
	const int relative_counters[GCOV_COUNTERS];

#ifdef __cplusplus
#define GET_REL_FIELD(name, type)					\
	type get_##name() const						\
		{							\
			return REL_FIELD(this, relative_##name, type);	\
		}

#define GET_REL_FIELD_FOR_ARRAY(name, type)				\
	type get_##name(size_t elt) const				\
	  {								\
	    return REL_FIELD_FOR_ARRAY(this, relative_##name,		\
				       elt, type);			\
	  }

	GET_REL_FIELD(function_name, const char*)
		GET_REL_FIELD(section_name, const char*)
		GET_REL_FIELD(source_file_name, const char*)
		GET_REL_FIELD_FOR_ARRAY(counters, const FeedbackCounter*)

		private:
	/*
	 * Hidden, nonexistent copy operator. Copying makes no sense because
	 * of the relative fields.
	 */
	FeedbackFunction(const FeedbackFunction&);
	/* Hidden, nonexistent assignment operator. */
	FeedbackFunction& operator=(const FeedbackFunction&);
#endif  /* __cplusplus */

} FeedbackFunction;


/* An Edge in a ConflictMissGraph. */
typedef struct Edge {
	FunctionID f1;
	FunctionID f2;
	unsigned long long weight;

#ifdef __cplusplus
	bool is_valid() const { return f1 != 0 && f2 != 0 && f1 != f2; }
#endif
} Edge;


typedef struct _FeedbackHeader FeedbackHeader;

/* The file header. Instantiated only in crtfbi.S. */
struct _FeedbackHeader {
	/* Magic number (FEEDBACK_HEADER_MAGIC). */
	unsigned int magic;

	/* Version number (FEEDBACK_HEADER_VERSION). */
	unsigned int version;

	union {
		/*
		 * When shared libraries are loaded, we may have several
		 * separate regions of feedback data. This is the file offset
		 * to the next region in the list, relative to the file
		 * offset of this header, or 0 for "end of list".
		 */
		unsigned int relative_file_offset_to_next_header;

		/*
		 * Next in linked list of deferred headers, or NULL if none.
		 * Only used while collecting data, never visible in a file.
		 */
		FeedbackHeader *next_deferred;

                /* Make the size of this union the same regardless of the size
                   of a pointer.  */
                long long dummy;
	} u;

	/*
	 * Total size of the feedback data for this binary (counting
	 * alignment padding at the end of the feedback sections).
	 */
	unsigned int section_size;

	/*
	 * Relative offset from the start of this struct to the start of all
	 * feedback data for this binary.
	 */
	unsigned int relative_functions_start;

	/*
	 * Relative offset from the start of this struct to the end of all
	 * feedback data for this binary.
	 */
	unsigned int relative_functions_end;

	/*
	 * WARNING: the following two fields must be contiguous and in
	 * this order because of how we write them out in cachepack.c.
	 */

	/* The size of the conflict miss graph's Edge array (in edges). */
	unsigned int cmg_edge_array_size;

	/*
	 * The file offset to the conflict miss graph's Edge array.
	 * Any Edge with a 0 id for either f1 or f2 should be ignored.
	 */
	unsigned int cmg_edge_array_file_offset;
};

#endif  /* !__ASSEMBLER__ */

#endif /* !_FEEDBACK_FILE_H */
