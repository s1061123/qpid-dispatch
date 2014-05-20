#ifndef __dispatch_router_h__
#define __dispatch_router_h__ 1
/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** @defgroup router
 * 
 * Register addresses, send messages.
 */
#include <qpid/dispatch/dispatch.h>
#include <qpid/dispatch/message.h>
#include <qpid/dispatch/iterator.h>
#include <stdbool.h>

typedef struct qd_address_t qd_address_t;
typedef uint8_t             qd_address_semantics_t;

/**
 * @name Address fanout semantics
 * @{
 */
#define QD_FANOUTMASK      0x03
#define QD_FANOUT_SINGLE   0x00 ///< Message will be delivered to a single consumer.
#define QD_FANOUT_MULTIPLE 0x01 ///< Message will be delivered to multiple consumers.
#define QD_FANOUT_GROUP    0x02 ///< Message will be delivered to one consumer per group.
#define QD_FANOUT(d) (d & QD_FANOUTMASK) ///< Get fanout bits.
///@}

/**
 * @name Address bias semantics for SINGLE/GROUP fanout
 * @{
 */

#define QD_BIASMASK     0x0c
#define QD_BIAS_NONE    0x00 ///< Apply no bias (also used for multiple fanout).
#define QD_BIAS_CLOSEST 0x04 ///< Message will be delivered to the closest (lowest cost) consumer.
#define QD_BIAS_SPREAD  0x08 ///< Messages will be spread arbitrarily across all consumers.
#define QD_BIAS_LATENCY 0x0c ///< Messages will be spread to minimize latency in light of each consumer's rate of consumption.
#define QD_BIAS(d) (d & QD_BIASMASK)
///@}


/**
 * @name Address congestion semantics.
 *
 * This controls that the router will do with
 * received messages that are destined for congested destinations.
 * @{
 */
#define QD_CONGESTIONMASK          0x30
/** Drop/Release the message.*/
#define QD_CONGESTION_DROP         0x00
 /**
  * Stop issuing replacement credits to slow the producer.  This puts a cap on
  * the total number of messages addressed to this address from a particular
  * producer that can be buffered in the router.
  */
#define QD_CONGESTION_BACKPRESSURE 0x10
 /** Redirect messages to an alternate address. */
#define QD_CONGESTION_REDIRECT     0x20
#define QD_CONGESTION(d) (d & QD_CONGESTIONMASK)
/// @}

/** @name Other semantics
 * @{
 */
#define QD_DROP_FOR_SLOW_CONSUMERS 0x40
#define QD_BYPASS_VALID_ORIGINS    0x80
///@}

/**
 * @name Sematics groups
 * @{
 */
#define QD_SEMANTICS_ROUTER_CONTROL (QD_FANOUT_MULTIPLE | QD_BIAS_NONE | QD_CONGESTION_DROP | QD_DROP_FOR_SLOW_CONSUMERS | QD_BYPASS_VALID_ORIGINS)
#define QD_SEMANTICS_DEFAULT        (QD_FANOUT_MULTIPLE | QD_BIAS_NONE | QD_CONGESTION_DROP | QD_DROP_FOR_SLOW_CONSUMERS)
///@}

typedef void (*qd_router_message_cb_t)(void *context, qd_message_t *msg, int link_id);

const char *qd_router_id(const qd_dispatch_t *qd);

/** Register an address in the router's hash table.
 * @param address String form of address
 * @param handler Callback to be called when a message is received for the address.
 * @param semantics Semantics for the address.
 * @param context Context to be passed to the handler.
 */
qd_address_t *qd_router_register_address(qd_dispatch_t          *qd,
                                         const char             *address,
                                         qd_router_message_cb_t  handler,
                                         qd_address_semantics_t  semantics,
                                         bool                    global,
                                         void                   *context);

void qd_router_unregister_address(qd_address_t *address);

void qd_address_set_redirect(qd_address_t *address, qd_address_t *redirect);

void qd_address_set_static_cc(qd_address_t *address, qd_address_t *cc);

void qd_address_set_dynamic_cc(qd_address_t *address, qd_address_t *cc);

/** Send msg to local links and next-hops for address */
void qd_router_send(qd_dispatch_t       *qd,
                    qd_field_iterator_t *address,
                    qd_message_t        *msg);

/** Send msg to local links and next-hops for address */
void qd_router_send2(qd_dispatch_t *qd,
                     const char    *address,
                     qd_message_t  *msg);

void qd_router_build_node_list(qd_dispatch_t *qd, qd_composed_field_t *field);

/** String form of address for logging */
const char* qd_address_logstr(qd_address_t* address);
#endif
