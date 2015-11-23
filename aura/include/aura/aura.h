#ifndef AURA_H
#define AURA_H

#define _GNU_SOURCE

#include <search.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <aura/slog.h>
#include <search.h>
#include <errno.h> 
#include <stdbool.h>

#include "list.h"
#include "format.h"
#include "endian.h"


/**  Node status */
enum aura_node_status {
	AURA_STATUS_OFFLINE,//!< AURA_STATUS_OFFLINE
	AURA_STATUS_ONLINE, //!< AURA_STATUS_ONLINE
};

/** Remote method call status */
enum aura_call_status { 
	AURA_CALL_COMPLETED,     //!< AURA_CALL_COMPLETED
	AURA_CALL_TIMEOUT,       //!< AURA_CALL_TIMEOUT
	AURA_CALL_TRANSPORT_FAIL,//!< AURA_CALL_TRANSPORT_FAIL
};

/** File descriptor action */
enum aura_fd_action { 
	AURA_FD_ADDED, //!< Descriptor added
	AURA_FD_REMOVED//!< Descriptor removed
};

struct aura_pollfds { 
	int magic;
	struct aura_node *node; 
	int fd;
	uint32_t events;
};

struct aura_object;
struct aura_node { 
	const struct aura_transport    *tr;
	struct aura_export_table *tbl;
	void                     *transport_data;
	void                     *user_data;
	enum aura_node_status     status;
	struct list_head          outbound_buffers;
	struct list_head          inbound_buffers;
	/* Synchronos calls put their stuff here */
	bool sync_call_running;
	bool need_endian_swap;
	struct aura_buffer *sync_ret_buf; 
	int sync_call_result;

	/* Synchronous event storage */
	struct list_head          event_buffers;
	int sync_event_max;
	int sync_event_count;

	/* General callbacks */
	void *status_changed_arg;
	void (*status_changed_cb)(struct aura_node *node, int newstatus, void *arg);

	void *etable_changed_arg;
	void (*etable_changed_cb)(struct aura_node *node, 
				  struct aura_export_table *old, 
				  struct aura_export_table *new, 
				  void *arg);

	void *object_migration_failed_arg;
	void (*object_migration_failed_cb)(struct aura_node *node, 
					struct aura_object *failed, 
					void *arg);

	void (*unhandled_evt_cb)(struct aura_node *node, 
				 struct aura_buffer *ret, 
				 void *arg);
	void *unhandled_evt_arg;
	void *fd_changed_arg;
	void (*fd_changed_cb)(const struct aura_pollfds *fd, 
			      enum aura_fd_action act, void *arg);

		
	/* Event system and polling */
	int numfds; /* Currently available space for descriptors */
	int nextfd; /* Next descriptor to add */
	struct aura_pollfds *fds; /* descriptor and event array */

	void *eventsys_data; /* eventloop structure */
	unsigned int poll_timeout; 
	uint64_t last_checked;
	struct list_head eventloop_node_list;
	const struct aura_object *current_object;
};


struct aura_object { 
	int id;
	char *name;
	char *arg_fmt;
	char *ret_fmt;

	int valid; 
	char* arg_pprinted;
	char* ret_pprinted;
	int num_args; 
	int num_rets; 

	int arglen;
	int retlen;

	/* Store callbacks here. 
	   TODO: Can there be several calls of the same method pending? */
	int pending;
	void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg);
	void *arg;
};

struct aura_eventloop { 
	int autocreated;
	int keep_running;
	int poll_timeout;
	struct list_head nodelist;
	void *eventsysdata;
};


#define object_is_event(o)  (o->arg_fmt==NULL)
#define object_is_method(o) (o->arg_fmt!=NULL)

struct aura_export_table {
	int size;
	int next;
	struct aura_node *owner;
	struct hsearch_data index; 
	struct aura_object objects[];
};


#define __BIT(n) (1<< n)

/**
 * \addtogroup trapi
 * @{
 */

/** Represents an aura transport module */
struct aura_transport
{
	/** \brief Required
	 *
	 * String name identifying the transport e.g. "usb"
	 */
	const char *name;
	/** \brief Optional
	 *
	 * Flags. TODO: Is it still needed?
	 */
	uint32_t flags; 
	
	/**
	 * \brief Optional.
	 *
	 * Additional bytes to allocate for each buffer.
	 *
	 *  If your transport layer requires any additional bytes to encapsulate the
	 *  actual serialized message - just specify how many. This is node to avoid
	 *  unneeded copying while formatting the message for transmission in the transport
	 *  layer.
	 */
	int      buffer_overhead;

	/** \brief Optional.
	 *
	 * Offset in the buffer at which core puts serialized data.
	 *
	 * NOTE: Your buffer_overhead should more or equal buffer_offset bytes. Otherwise
	 * core will complain and refuse to register your transport.
	 *
	 */
	int      buffer_offset;

	/** \brief Required.
	 *
	 *  Open function.
	 *
	 *  Open function should perform sanity checking on supplied (in ap) arguments
	 *  and allocate internal data structures and set required periodic timeout in
	 *  the respective node. See transport-dummy.c for a boilerplate.
	 *
	 *  Avoid doing any blocking stuff in open(), do in in loop instead in non-blocking fashion.
	 *  @param node current node
	 *  @param opts string containing transport-specific options
	 *  @return 0 if everything went fine, anything other will be considered an error.
	 */
	int    (*open)(struct aura_node *node, const char *opts);
	/**
	 * \brief Required.
	 *
	 * Close function.
	 *
	 * The reverse of open. Free any allocated memory, etc.
	 * This function may block if required. But avoid it if you can.
	 *
	 * @param node current node
	 */
	void   (*close)(struct aura_node *node);
	/**
	 * \brief Required
	 *
	 * The workhorse of your transport plugin.
	 *
	 * This function should check the node->outbound_buffers queue for any new messages to
	 * deliver and place any incoming messages into node->inbound_buffers queue.
	 *
	 * This function is called by the core when:
	 * - Descriptors associated with this node report being ready for I/O (fd will be set to
	 *   the descriptor that reports being ready for I/O)
	 * - Node's periodic timeout expires (fd will be NULL)
	 * - A new message has been put into outbound queue that has previously been empty
	 *   (e.g. Wake up! We've got work to do) (fd will NULL)
	 *
	 * @param node current node
	 * @param fd pointer to struct aura_pollfds that generated an event.
	 */
	void   (*loop)(struct aura_node *node, const struct aura_pollfds *fd);
	
	/**
	 * \brief Optional.
	 *
	 * Your transport may implement passing aura_buffers as arguments. 
	 * This may be extremely useful for DSP applications, where you also 
	 * implement your own buffer_request and buffer_release to take care of
	 * allocating memory on the DSP side. This function should serialize buffer buf
	 * into buffer dst. Buffer pointer/handle must be cast to uint64_t.
	 *
	 * @param buf
	 */
	void               (*buffer_put)(struct aura_buffer *dst, struct aura_buffer *buf);
	/**
	 * \brief Optional
	 *
	 * Your transport may implement passing aura_buffers as arguments. 
	 * This may be extremely useful for DSP applications, where you also 
	 * implement your own buffer_request and buffer_release to take care of
	 * allocating memory on the DSP side. 
	 * This function should deserialize and return aura_buffer from buffer buf
	 * @param buf
	 * @param ptr
	 */
	struct aura_buffer *(*buffer_get)(struct aura_buffer *buf);
	/**
	 * \brief Optional
	 *
	 * Override Buffer allocation. This may be called if you need
	 * any special consideration when allocating buffers (e.g. ION).
	 * The size includes any transport-required overhead, but doesn't include
	 * the actual struct aura_buffer.
	 *
	 * The simplest implementation would be:
	 * \code{.c}
	 *  struct aura_buffer *aura_buffer_internal_request(struct aura_node *node, int size) {
	 *		int act_size = sizeof(struct aura_buffer) + size;
	 *  	struct aura_buffer *ret = malloc(act_size);
	 *  	ret->size = size;
	 *  	ret->pos = 0;
	 *  	return ret;
	 *  }
	 *  \endcode
	 *
	 *
	 * @param node current node
	 * @param size requested buffer size (not including struct aura_buffer)
	 * @return
	 */
	struct aura_buffer *(*buffer_request)(struct aura_node *node, int size);
	/**
	 * \brief Optional.
	 *
	 * Override Buffer deallocation. This may be called if you need
	 * any special consideration when allocating and deallocating buffers (e.g. ION)
	 *
	 *
	 * @param node
	 * @param size
	 * @return
	 */
	void                (*buffer_release)(struct aura_node *node, struct aura_buffer *buf);

	/** \brief Private.
	 *
	 * Transport usage count */
	int usage;

	/** \brief Private.
	 *
	 * List entry for global transport list */
	struct list_head registry;
};

/**
 * @}
 */


/**
 * \addtogroup bufapi
 * @{
 */

/** Represents a buffer (surprize!) that is used for serialization/deserialization
 *
 *  * The data is stored in NODE endianness.
 *  * The buffer has an internal data pointer
 *  * The buffer has an associated object
 */
struct aura_buffer {
	/** Size of the data in this buffer */
	int                 size;
	/** Pointer to the next element */
	int                 pos;
	/** object assosiated with this buffer */
	struct aura_object *object;
	/** Transport-specific data associated with this buffer */
	void               *transportdata;
	/** The node that owns the buffer */
	struct aura_node   *owner;
	/** list_entry. Used to queue/deueue buffers */
	struct list_head    qentry;
	/** The actual data in this buffer */
	char                data[];
};

/**
 * @}
 */


struct aura_buffer *aura_serialize(struct aura_node *node, const char *fmt, va_list ap);
int  aura_fmt_len(struct aura_node *node, const char *fmt);
char* aura_fmt_pretty_print(const char* fmt, int *valid, int *num_args);


const struct aura_transport *aura_transport_lookup(const char *name);
void aura_set_status(struct aura_node *node, int status);


#define AURA_TRANSPORT(s)					   \
	void __attribute__((constructor (101))) do_reg_##s(void) { \
		aura_transport_register(&s);			   \
	}

enum aura_endianness aura_get_host_endianness();
void aura_hexdump (char *desc, void *addr, int len);
void aura_set_node_endian(struct aura_node *node, enum aura_endianness en);

void aura_queue_buffer(struct list_head *list, struct aura_buffer *buf);
struct aura_buffer *aura_dequeue_buffer(struct list_head *head);
void aura_requeue_buffer(struct list_head *head, struct aura_buffer *buf);
struct aura_buffer *aura_peek_buffer(struct list_head *head);

struct aura_export_table *aura_etable_create(struct aura_node *owner, int n);
void aura_etable_add(struct aura_export_table *tbl, 
		    const char *name, 
		    const char *argfmt, 
		     const char *retfmt);
void aura_etable_activate(struct aura_export_table *tbl);
struct aura_object *aura_etable_find(struct aura_export_table *tbl, 
				     const char *name);
struct aura_object *aura_etable_find_id(struct aura_export_table *tbl, 
					int id);
void aura_etable_destroy(struct aura_export_table *tbl);

struct aura_node *aura_open(const char* name, const char *opts); 
void aura_close(struct aura_node *dev); 


int aura_queue_call(struct aura_node *node, 
		    int id,
		    void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg),
		    void *arg,
		    struct aura_buffer *buf);

int aura_start_call_raw(
	struct aura_node *dev, 
	int id,
	void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg),
	void *arg,
	...);

int aura_start_call(
	struct aura_node *dev, 
	const char *name,
	void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg),
	void *arg,
	...);

int aura_call_raw(
	struct aura_node *dev, 
	int id,
	struct aura_buffer **ret,
	...);

int aura_call(
	struct aura_node *dev, 
	const char *name,
	struct aura_buffer **ret,
	...);

int aura_set_event_callback_raw(
	struct aura_node *node,
	int id,
	void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg),
	void *arg);

int aura_set_event_callback(
	struct aura_node *node,
	const char *event,
	void (*calldonecb)(struct aura_node *dev, int status, struct aura_buffer *ret, void *arg),
	void *arg);

void aura_enable_sync_events(struct aura_node *node, int count);
int aura_get_pending_events(struct aura_node *node);
int aura_get_next_event(struct aura_node *node, const struct aura_object ** obj, struct aura_buffer **retbuf);


void __attribute__((noreturn)) aura_panic(struct aura_node *node);

/** \addtogroup retparse
 *  @{
 */

/**
 * \brief Get an unsigned 8 bit integer from aura buffer
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
uint8_t  aura_buffer_get_u8 (struct aura_buffer *buf);

/**
 * \brief Get an unsigned 16 bit integer from aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
uint16_t aura_buffer_get_u16(struct aura_buffer *buf);

/**
 * \brief Get an unsigned 32 bit integer from aura buffer.
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
uint32_t aura_buffer_get_u32(struct aura_buffer *buf);

/**
 * \brief Get an unsigned 64 bit integer from aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
uint64_t aura_buffer_get_u64(struct aura_buffer *buf);

/**
 * \brief Get a signed 8 bit integer from aura buffer.
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 * @param buf
 * @return
 */
int8_t  aura_buffer_get_s8 (struct aura_buffer *buf);

/**
 * \brief Get a signed 16 bit integer from aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
int16_t aura_buffer_get_s16(struct aura_buffer *buf);

/**
 * \brief Get a signed 32 bit integer from aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyound
 * the buffer boundary.
 *
 * @param buf
 * @return
 */
int32_t aura_buffer_get_s32(struct aura_buffer *buf);

/**
 * \brief Get a signed 64 bit integer from aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 *
 * @param buf
 * @return
 */
int64_t aura_buffer_get_s64(struct aura_buffer *buf);

/**
 * \brief Put an unsigned 8 bit integer to aura buffer
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void  aura_buffer_put_u8 (struct aura_buffer *buf, uint8_t value);

/**
 * \brief Put an unsigned 16 bit integer to aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_u16(struct aura_buffer *buf, uint16_t value);

/**
 * \brief Put an unsigned 32 bit integer to aura buffer.
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_u32(struct aura_buffer *buf, uint32_t value);

/**
 * \brief Put an unsigned 64 bit integer to aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_u64(struct aura_buffer *buf, uint64_t value);

/**
 * \brief Put a signed 8 bit integer to aura buffer.
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 * @param buf
 * @param value
 * @return
 */
void  aura_buffer_put_s8 (struct aura_buffer *buf, int8_t value);

/**
 * \brief Put a signed 16 bit integer to aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_s16(struct aura_buffer *buf, int16_t value);

/**
 * \brief Put a signed 32 bit integer to aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyound
 * the buffer boundary.
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_s32(struct aura_buffer *buf, int32_t value);

/**
 * \brief Put a signed 64 bit integer to aura buffer
 *
 * This function will swap endianness if needed.
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 *
 * @param buf
 * @param value
 * @return
 */
void aura_buffer_put_s64(struct aura_buffer *buf, int64_t value);


/**
 * \brief Get a pointer to the binary data block within buffer and advance
 * internal pointer by len bytes. The data in the buffer is managed internally and should
 * not be freed by the caller.
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf aura buffer
 * @param len data length
 */
const void *aura_buffer_get_bin(struct aura_buffer *buf, int len);


/**
 * \brief Copy data of len bytes to aura buffer from a buffer pointed by data
 *
 * This function will advance internal aura_buffer pointer by len bytes.
 *
 * This function will cause a panic if attempted to read beyond
 * the buffer boundary.
 *
 * @param buf aura buffer
 * @param data data buffer
 * @param len data length
 */
void aura_buffer_put_bin(struct aura_buffer *buf, const void *data, int len);


/**
 * Retrieve transport-specific pointer from aura buffer.
 * Handling of this data-type is transport specific.
 *
 * @param buf
 */
const void *aura_buffer_get_ptr(struct aura_buffer *buf);

/**
 * @}
 */

void aura_etable_changed_cb(struct aura_node *node, 
			    void (*cb)(struct aura_node *node, 
				       struct aura_export_table *old, 
				       struct aura_export_table *new, 
				       void *arg),
			    void *arg);

void aura_status_changed_cb(struct aura_node *node, 
			    void (*cb)(struct aura_node *node, int newstatus, void *arg),
			    void *arg);

void aura_fd_changed_cb(struct aura_node *node, 
			 void (*cb)(const struct aura_pollfds *fd, enum aura_fd_action act, void *arg),
			 void *arg);

void aura_unhandled_evt_cb(struct aura_node *node, 
			   void (*cb)(struct aura_node *node, 
				      struct aura_buffer *buf, 
				      void *arg),
			   void *arg);

void aura_object_migration_failed_cb(struct aura_node *node, 
				     void (*cb)(struct aura_node *node, 
						struct aura_object *failed, 
						void *arg),
				     void *arg);

const struct aura_object *aura_get_current_object(struct aura_node *node);

void aura_add_pollfds(struct aura_node *node, int fd, uint32_t events);
void aura_del_pollfds(struct aura_node *node, int fd);
int aura_get_pollfds(struct aura_node *node, const struct aura_pollfds **fds);


/** \addtogroup loop
 *  @{
 */

/**
 * Create an eventloop from one or more nodes.
 * e.g. aura_eventloop_create(node1, node2, node3);
 * @param ... one or more struct aura_node*
 */
#define aura_eventloop_create(...) \
	aura_eventloop_create__(0, ##__VA_ARGS__, NULL)


/**
 *
 * @}
 */


void aura_eventloop_destroy(struct aura_eventloop *loop);
void *aura_eventloop_vcreate(va_list ap);
void *aura_eventloop_create__(int dummy, ...);
void aura_eventloop_add(struct aura_eventloop *loop, struct aura_node *node);
void aura_eventloop_del(struct aura_node *node);
void aura_eventloop_break(struct aura_eventloop *loop);


void aura_handle_events(struct aura_eventloop *loop);
void aura_handle_events_timeout(struct aura_eventloop *loop, int timeout_ms);
void aura_handle_events_forever(struct aura_eventloop *loop);
void aura_wait_status(struct aura_node *node, int status);

struct aura_buffer *aura_buffer_request(struct aura_node *nd, int size);
void aura_buffer_release(struct aura_node *nd, struct aura_buffer *buf);

/* event system data access functions */
struct aura_eventloop *aura_eventloop_get_data(struct aura_node *node);


#include <aura/inlines.h>

#endif
