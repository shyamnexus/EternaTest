/**
 * @file onvif_events.c
 * @brief ONVIF Events Service — Profile S compliant implementation
 *
 * Implements the WS-BaseNotification PullPoint subscription model:
 *   - CreatePullPointSubscription
 *   - PullMessages
 *   - Renew
 *   - Unsubscribe
 *   - SetSynchronizationPoint
 *   - GetEventProperties
 *   - GetServiceCapabilities (Events)
 *
 * The subscription manager maintains a fixed-size pool of subscriptions,
 * each with its own circular event buffer. A background reaper thread
 * periodically removes expired subscriptions.
 */

#include "onvif_main.h"
#include "wsaapi.h"
#include "soapH.h"
#include "soapStub.h"
#include "wsddapi.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* ========================================================================== */
/*  Configuration constants                                                   */
/* ========================================================================== */

#define MAX_SUBSCRIPTIONS       8       /* Max concurrent PullPoint clients    */
#define EVENT_QUEUE_DEPTH       64      /* Circular buffer depth per sub       */
#define DEFAULT_TERM_SECS       60      /* Default subscription lifetime (s)   */
#define MAX_TERM_SECS           3600    /* Maximum subscription lifetime (s)   */
#define REAPER_INTERVAL_SECS    10      /* Reaper thread wakeup interval       */
#define UUID_STR_LEN            37      /* "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0" */

/* ONVIF topic dialect URI */
#define TOPIC_DIALECT   "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"
/* ONVIF message content filter dialect */
#define CONTENT_FILTER_DIALECT "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"

/* ========================================================================== */
/*  Internal data types                                                       */
/* ========================================================================== */

/** A single queued event notification */
typedef struct {
    char     topic[128];            /* e.g. "tns1:Device/Trigger/DigitalInput" */
    char     source_name[64];       /* e.g. "InputToken"                       */
    char     source_value[64];      /* e.g. "DigitalInput_0"                   */
    char     data_name[64];         /* e.g. "LogicalState"                     */
    char     data_value[64];        /* e.g. "true"                             */
    struct timeval timestamp;
    int      is_property;           /* 1 = property event (initial state)      */
} onvif_event_t;

/** A PullPoint subscription */
typedef struct {
    int              active;                        /* 0 = slot free           */
    char             subscription_id[UUID_STR_LEN]; /* UUID                    */
    struct timeval   creation_time;
    struct timeval   termination_time;
    /* Circular event buffer */
    onvif_event_t    events[EVENT_QUEUE_DEPTH];
    int              ev_head;       /* Next write index                        */
    int              ev_tail;       /* Next read index                         */
    int              ev_count;      /* Number of queued events                 */
    pthread_mutex_t  lock;
    pthread_cond_t   cond;          /* Signaled when events arrive             */
} onvif_subscription_t;

/* ========================================================================== */
/*  Module-level state                                                        */
/* ========================================================================== */

static onvif_subscription_t g_subs[MAX_SUBSCRIPTIONS];
static pthread_mutex_t      g_subs_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t            g_reaper_thread;
static int                  g_reaper_running = 0;

/* ========================================================================== */
/*  Utility helpers                                                           */
/* ========================================================================== */

/** Generate a random UUID string using /dev/urandom */
static void generate_uuid(char *buf, size_t len)
{
    unsigned char raw[16];
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        if (fread(raw, 1, 16, f) < 16)
            memset(raw, 0x42, 16); /* fallback */
        fclose(f);
    } else {
        /* Last-resort fallback using time + pid */
        struct timeval tv;
        gettimeofday(&tv, NULL);
        memset(raw, 0, 16);
        memcpy(raw, &tv.tv_sec, sizeof(tv.tv_sec));
        memcpy(raw + 4, &tv.tv_usec, sizeof(tv.tv_usec));
        pid_t pid = getpid();
        memcpy(raw + 8, &pid, sizeof(pid));
    }
    /* Set version 4 and variant bits */
    raw[6] = (raw[6] & 0x0F) | 0x40;
    raw[8] = (raw[8] & 0x3F) | 0x80;
    snprintf(buf, len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             raw[0],raw[1],raw[2],raw[3], raw[4],raw[5], raw[6],raw[7],
             raw[8],raw[9], raw[10],raw[11],raw[12],raw[13],raw[14],raw[15]);
}

/** Get current time as struct timeval */
static struct timeval now_tv(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

/** Parse ISO-8601 duration (subset: PT<n>S or PT<n>M or PT<n>H) -> seconds.
 *  Returns 0 on parse failure. */
static int parse_duration_seconds(const char *dur)
{
    if (!dur || dur[0] != 'P')
        return 0;
    const char *p = dur + 1;
    if (*p == 'T') p++;
    int value = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    switch (*p) {
        case 'S': return value;
        case 'M': return value * 60;
        case 'H': return value * 3600;
        default:  return value > 0 ? value : 0;
    }
}

/** Check if subscription has expired */
static int subscription_expired(const onvif_subscription_t *sub)
{
    struct timeval now = now_tv();
    return (now.tv_sec > sub->termination_time.tv_sec) ||
           (now.tv_sec == sub->termination_time.tv_sec &&
            now.tv_usec >= sub->termination_time.tv_usec);
}

/** Find subscription by UUID (g_subs_lock must be held) */
static onvif_subscription_t* find_subscription(const char *id)
{
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (g_subs[i].active && strcmp(g_subs[i].subscription_id, id) == 0)
            return &g_subs[i];
    }
    return NULL;
}

/** Allocate a free subscription slot (g_subs_lock must be held).
 *  Returns NULL if pool is full. */
static onvif_subscription_t* alloc_subscription(void)
{
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (!g_subs[i].active) {
            memset(&g_subs[i], 0, sizeof(onvif_subscription_t));
            g_subs[i].active = 1;
            pthread_mutex_init(&g_subs[i].lock, NULL);
            pthread_cond_init(&g_subs[i].cond, NULL);
            return &g_subs[i];
        }
    }
    return NULL;
}

/** Destroy a subscription (g_subs_lock must be held) */
static void destroy_subscription(onvif_subscription_t *sub)
{
    if (!sub) return;
    pthread_mutex_destroy(&sub->lock);
    pthread_cond_destroy(&sub->cond);
    sub->active = 0;
}

/** Enqueue an event into a subscription's circular buffer */
static void enqueue_event(onvif_subscription_t *sub, const onvif_event_t *ev)
{
    pthread_mutex_lock(&sub->lock);
    memcpy(&sub->events[sub->ev_head], ev, sizeof(onvif_event_t));
    sub->ev_head = (sub->ev_head + 1) % EVENT_QUEUE_DEPTH;
    if (sub->ev_count < EVENT_QUEUE_DEPTH)
        sub->ev_count++;
    else
        sub->ev_tail = (sub->ev_tail + 1) % EVENT_QUEUE_DEPTH; /* overwrite oldest */
    pthread_cond_signal(&sub->cond);
    pthread_mutex_unlock(&sub->lock);
}

/** Dequeue up to max_msgs events from a subscription.
 *  Returns the count of events dequeued. Caller must hold sub->lock. */
static int dequeue_events(onvif_subscription_t *sub, onvif_event_t *out, int max_msgs)
{
    int count = 0;
    while (sub->ev_count > 0 && count < max_msgs) {
        memcpy(&out[count], &sub->events[sub->ev_tail], sizeof(onvif_event_t));
        sub->ev_tail = (sub->ev_tail + 1) % EVENT_QUEUE_DEPTH;
        sub->ev_count--;
        count++;
    }
    return count;
}

/** Extract subscription UUID from WS-Addressing To header or HTTP path.
 *  Writes cleaned UUID into id_buf (must be UUID_STR_LEN bytes).
 *  Returns 0 on success, -1 if not found. */
static int extract_subscription_id(struct soap *soap, char *id_buf)
{
    const char *sub_id = NULL;
    /* Try WS-Addressing To header first */
    if (soap->header && soap->header->wsa5__To) {
        const char *id_param = strstr(soap->header->wsa5__To, "Id=");
        if (id_param) sub_id = id_param + 3;
    }
    /* Fallback: HTTP query string path */
    if (!sub_id && soap->path) {
        const char *id_param = strstr(soap->path, "Id=");
        if (id_param) sub_id = id_param + 3;
    }
    if (!sub_id)
        return -1;

    memset(id_buf, 0, UUID_STR_LEN);
    strncpy(id_buf, sub_id, UUID_STR_LEN - 1);
    /* Trim at first non-UUID character */
    for (int i = 0; i < UUID_STR_LEN - 1 && id_buf[i]; i++) {
        char c = id_buf[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '-')) {
            id_buf[i] = '\0';
            break;
        }
    }
    return 0;
}

/* ========================================================================== */
/*  Public API: Push events from other modules                                */
/* ========================================================================== */

/**
 * Push an event to all active subscriptions.
 * Called from motion detection, GPIO, etc.
 */
void onvif_events_push(const char *topic, const char *src_name, const char *src_val,
                        const char *data_name, const char *data_val, int is_property)
{
    onvif_event_t ev;
    memset(&ev, 0, sizeof(ev));
    snprintf(ev.topic, sizeof(ev.topic), "%s", topic);
    if (src_name)  snprintf(ev.source_name,  sizeof(ev.source_name),  "%s", src_name);
    if (src_val)   snprintf(ev.source_value, sizeof(ev.source_value), "%s", src_val);
    if (data_name) snprintf(ev.data_name,    sizeof(ev.data_name),    "%s", data_name);
    if (data_val)  snprintf(ev.data_value,   sizeof(ev.data_value),   "%s", data_val);
    ev.timestamp = now_tv();
    ev.is_property = is_property;

    pthread_mutex_lock(&g_subs_lock);
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (g_subs[i].active && !subscription_expired(&g_subs[i])) {
            enqueue_event(&g_subs[i], &ev);
        }
    }
    pthread_mutex_unlock(&g_subs_lock);
}

/* ========================================================================== */
/*  Reaper thread — cleans up expired subscriptions                           */
/* ========================================================================== */

static void *reaper_thread_func(void *arg)
{
    (void)arg;
    while (g_reaper_running) {
        sleep(REAPER_INTERVAL_SECS);
        pthread_mutex_lock(&g_subs_lock);
        for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
            if (g_subs[i].active && subscription_expired(&g_subs[i])) {
                printf("[Events] Reaping expired subscription %s\n", g_subs[i].subscription_id);
                /* Wake any blocked PullMessages before destroying */
                pthread_cond_broadcast(&g_subs[i].cond);
                destroy_subscription(&g_subs[i]);
            }
        }
        pthread_mutex_unlock(&g_subs_lock);
    }
    return NULL;
}

/** Start the reaper thread (called once at ONVIF init) */
void onvif_events_init(void)
{
    memset(g_subs, 0, sizeof(g_subs));
    g_reaper_running = 1;
    pthread_create(&g_reaper_thread, NULL, reaper_thread_func, NULL);
    printf("[Events] Subscription manager initialized (max=%d, queue=%d)\n",
           MAX_SUBSCRIPTIONS, EVENT_QUEUE_DEPTH);
}

/** Stop the reaper thread (called at shutdown) */
void onvif_events_cleanup(void)
{
    g_reaper_running = 0;
    pthread_join(g_reaper_thread, NULL);
    pthread_mutex_lock(&g_subs_lock);
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (g_subs[i].active)
            destroy_subscription(&g_subs[i]);
    }
    pthread_mutex_unlock(&g_subs_lock);
    printf("[Events] Subscription manager shut down\n");
}

/* ========================================================================== */
/*  ONVIF Service Implementations                                            */
/* ========================================================================== */

/**
 * GetServiceCapabilities (Events)
 * Returns WSPullPointSupport=true, MaxPullPoints, MaxNotificationProducers
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__GetServiceCapabilities(struct soap *soap,
    struct _tev__GetServiceCapabilities *req,
    struct _tev__GetServiceCapabilitiesResponse *resp)
{
    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    resp->Capabilities = (struct tev__Capabilities *)soap_malloc(soap, sizeof(struct tev__Capabilities));
    memset(resp->Capabilities, 0, sizeof(struct tev__Capabilities));

    /* WSPullPointSupport = true */
    resp->Capabilities->WSPullPointSupport = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *resp->Capabilities->WSPullPointSupport = xsd__boolean__true_;

    /* WSSubscriptionPolicySupport = false */
    resp->Capabilities->WSSubscriptionPolicySupport = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *resp->Capabilities->WSSubscriptionPolicySupport = xsd__boolean__false_;

    /* WSPausableSubscriptionManagerInterfaceSupport = false */
    resp->Capabilities->WSPausableSubscriptionManagerInterfaceSupport = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *resp->Capabilities->WSPausableSubscriptionManagerInterfaceSupport = xsd__boolean__false_;

    /* MaxNotificationProducers */
    resp->Capabilities->MaxNotificationProducers = (int *)soap_malloc(soap, sizeof(int));
    *resp->Capabilities->MaxNotificationProducers = MAX_SUBSCRIPTIONS;

    /* MaxPullPoints */
    resp->Capabilities->MaxPullPoints = (int *)soap_malloc(soap, sizeof(int));
    *resp->Capabilities->MaxPullPoints = MAX_SUBSCRIPTIONS;

    /* PersistentNotificationStorage = false */
    resp->Capabilities->PersistentNotificationStorage = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *resp->Capabilities->PersistentNotificationStorage = xsd__boolean__false_;

    return SOAP_OK;
}

/**
 * GetEventProperties
 * Returns the TopicSet, supported filter dialects, and topic namespace.
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__GetEventProperties(struct soap *soap,
    struct _tev__GetEventProperties *req,
    struct _tev__GetEventPropertiesResponse *resp)
{
    printf("[Events] GetEventProperties called\n");

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    /* TopicNamespaceLocation (at least 1 required) */
    resp->__sizeTopicNamespaceLocation = 1;
    resp->TopicNamespaceLocation = (char **)soap_malloc(soap, sizeof(char *));
    resp->TopicNamespaceLocation[0] = soap_strdup(soap,
        "http://www.onvif.org/onvif/ver10/topics/topicns.xml");

    /* FixedTopicSet = true */
    resp->wsnt__FixedTopicSet = xsd__boolean__true_;

    /* TopicSet — build using DOM elements */
    resp->wstop__TopicSet = (struct wstop__TopicSetType *)
        soap_malloc(soap, sizeof(struct wstop__TopicSetType));
    memset(resp->wstop__TopicSet, 0, sizeof(struct wstop__TopicSetType));

    /*
     * Build the TopicSet as DOM elements.
     * Profile S requires at minimum topics for device state and optionally
     * motion/analytics. We define:
     *   tns1:Device/Trigger/DigitalInput   (boolean property)
     *   tns1:RuleEngine/CellMotionDetector/Motion  (boolean property)
     *   tns1:VideoSource/MotionAlarm       (boolean property)
     */
    int num_topics = 3;
    resp->wstop__TopicSet->__size = num_topics;
    resp->wstop__TopicSet->__any = (struct soap_dom_element *)
        soap_malloc(soap, sizeof(struct soap_dom_element) * num_topics);
    memset(resp->wstop__TopicSet->__any, 0, sizeof(struct soap_dom_element) * num_topics);

    /* Topic 1: tns1:Device/Trigger/DigitalInput */
    struct soap_dom_element *t1 = &resp->wstop__TopicSet->__any[0];
    t1->soap = soap;
    t1->name = soap_strdup(soap, "tns1:Device");
    t1->nstr = soap_strdup(soap, "http://www.onvif.org/ver10/topics");

    /* Topic 2: tns1:RuleEngine/CellMotionDetector/Motion */
    struct soap_dom_element *t2 = &resp->wstop__TopicSet->__any[1];
    t2->soap = soap;
    t2->name = soap_strdup(soap, "tns1:RuleEngine");
    t2->nstr = soap_strdup(soap, "http://www.onvif.org/ver10/topics");

    /* Topic 3: tns1:VideoSource/MotionAlarm */
    struct soap_dom_element *t3 = &resp->wstop__TopicSet->__any[2];
    t3->soap = soap;
    t3->name = soap_strdup(soap, "tns1:VideoSource");
    t3->nstr = soap_strdup(soap, "http://www.onvif.org/ver10/topics");

    /* TopicExpressionDialect (at least 1 required) */
    resp->__sizeTopicExpressionDialect = 1;
    resp->wsnt__TopicExpressionDialect = (char **)soap_malloc(soap, sizeof(char *));
    resp->wsnt__TopicExpressionDialect[0] = soap_strdup(soap, TOPIC_DIALECT);

    /* MessageContentFilterDialect (at least 1 required) */
    resp->__sizeMessageContentFilterDialect = 1;
    resp->MessageContentFilterDialect = (char **)soap_malloc(soap, sizeof(char *));
    resp->MessageContentFilterDialect[0] = soap_strdup(soap, CONTENT_FILTER_DIALECT);

    /* ProducerPropertiesFilterDialect — none */
    resp->__sizeProducerPropertiesFilterDialect = 0;
    resp->ProducerPropertiesFilterDialect = NULL;

    /* MessageContentSchemaLocation (at least 1 required) */
    resp->__sizeMessageContentSchemaLocation = 1;
    resp->MessageContentSchemaLocation = (char **)soap_malloc(soap, sizeof(char *));
    resp->MessageContentSchemaLocation[0] = soap_strdup(soap,
        "http://www.onvif.org/onvif/ver10/schema/onvif.xsd");

    return SOAP_OK;
}

/**
 * CreatePullPointSubscription
 * Allocates a subscription slot and returns the EPR with a unique UUID.
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__CreatePullPointSubscription(struct soap *soap,
    struct _tev__CreatePullPointSubscription *req,
    struct _tev__CreatePullPointSubscriptionResponse *resp)
{
    printf("[Events] CreatePullPointSubscription called\n");

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    /* Parse termination time */
    int term_secs = DEFAULT_TERM_SECS;
    if (req->InitialTerminationTime) {
        int parsed = parse_duration_seconds(req->InitialTerminationTime);
        if (parsed > 0 && parsed <= MAX_TERM_SECS)
            term_secs = parsed;
        else if (parsed > MAX_TERM_SECS)
            term_secs = MAX_TERM_SECS;
    }

    /* Allocate subscription slot */
    pthread_mutex_lock(&g_subs_lock);
    onvif_subscription_t *sub = alloc_subscription();
    if (!sub) {
        pthread_mutex_unlock(&g_subs_lock);
        onvif_fault(soap, ONVIF_ENV_RECEIVER, "ter:Action", "ter:CapabilityViolated",
                    "Maximum number of PullPoint subscriptions reached");
        return SOAP_FAULT;
    }

    generate_uuid(sub->subscription_id, UUID_STR_LEN);
    sub->creation_time = now_tv();
    sub->termination_time.tv_sec = sub->creation_time.tv_sec + term_secs;
    sub->termination_time.tv_usec = sub->creation_time.tv_usec;
    pthread_mutex_unlock(&g_subs_lock);

    printf("[Events] Created subscription %s (TTL=%ds)\n", sub->subscription_id, term_secs);

    /* Build response */
    /* SubscriptionReference (EPR) */
    char addr_buf[256];
    snprintf(addr_buf, sizeof(addr_buf),
             "http://%s:%d/onvif/Events/PullPointSubscription?Id=%s",
             ip_address ? ip_address : "0.0.0.0",
             ONVIF_LISTEN_PORT,
             sub->subscription_id);

    resp->SubscriptionReference.Address = soap_strdup(soap, addr_buf);
    resp->SubscriptionReference.ReferenceParameters = NULL;
    resp->SubscriptionReference.Metadata = NULL;
    resp->SubscriptionReference.__size = 0;
    resp->SubscriptionReference.__any = NULL;
    resp->SubscriptionReference.__anyAttribute = NULL;

    /* CurrentTime, TerminationTime */
    resp->wsnt__CurrentTime = now_tv();
    resp->wsnt__TerminationTime = sub->termination_time;
    resp->__size = 0;
    resp->__any = NULL;

    return SOAP_OK;
}

/**
 * PullMessages
 * Blocks up to the requested timeout waiting for events, then returns
 * whatever is queued (up to MessageLimit).
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__PullMessages(struct soap *soap,
    struct _tev__PullMessages *req,
    struct _tev__PullMessagesResponse *resp)
{
    printf("[Events] PullMessages called (timeout=%s, limit=%d)\n",
           req->Timeout ? req->Timeout : "null", req->MessageLimit);

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    /* Extract subscription ID */
    char id_buf[UUID_STR_LEN];
    if (extract_subscription_id(soap, id_buf) != 0) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "SubscriptionId not found in request");
        return SOAP_FAULT;
    }

    pthread_mutex_lock(&g_subs_lock);
    onvif_subscription_t *sub = find_subscription(id_buf);
    if (!sub || subscription_expired(sub)) {
        pthread_mutex_unlock(&g_subs_lock);
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "Subscription not found or expired");
        return SOAP_FAULT;
    }
    pthread_mutex_unlock(&g_subs_lock);

    /* Parse timeout */
    int timeout_secs = 10; /* default 10s */
    if (req->Timeout) {
        int parsed = parse_duration_seconds(req->Timeout);
        if (parsed > 0) timeout_secs = parsed;
        if (timeout_secs > 60) timeout_secs = 60; /* Cap at 60s */
    }
    int msg_limit = req->MessageLimit;
    if (msg_limit <= 0) msg_limit = 1;
    if (msg_limit > EVENT_QUEUE_DEPTH) msg_limit = EVENT_QUEUE_DEPTH;

    /* Wait for events or timeout */
    pthread_mutex_lock(&sub->lock);
    if (sub->ev_count == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_secs;
        pthread_cond_timedwait(&sub->cond, &sub->lock, &ts);
    }

    /* Dequeue available events */
    onvif_event_t *ev_buf = (onvif_event_t *)calloc(msg_limit, sizeof(onvif_event_t));
    int count = 0;
    if (ev_buf)
        count = dequeue_events(sub, ev_buf, msg_limit);
    pthread_mutex_unlock(&sub->lock);

    /* Build response */
    resp->CurrentTime = now_tv();
    resp->TerminationTime = sub->termination_time;

    if (count > 0 && ev_buf) {
        resp->__sizeNotificationMessage = count;
        resp->wsnt__NotificationMessage = (struct wsnt__NotificationMessageHolderType *)
            soap_malloc(soap, sizeof(struct wsnt__NotificationMessageHolderType) * count);
        memset(resp->wsnt__NotificationMessage, 0,
               sizeof(struct wsnt__NotificationMessageHolderType) * count);

        for (int i = 0; i < count; i++) {
            struct wsnt__NotificationMessageHolderType *msg = &resp->wsnt__NotificationMessage[i];

            /* Topic */
            msg->Topic = (struct wsnt__TopicExpressionType *)
                soap_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
            memset(msg->Topic, 0, sizeof(struct wsnt__TopicExpressionType));
            msg->Topic->Dialect = soap_strdup(soap, TOPIC_DIALECT);
            msg->Topic->__mixed.soap = soap;
            msg->Topic->__mixed.name = NULL;
            msg->Topic->__mixed.text = soap_strdup(soap, ev_buf[i].topic);

            /* Message body as DOM — tt:Message with Source/Data items */
            msg->Message.__any.soap = soap;
            msg->Message.__any.name = soap_strdup(soap, "tt:Message");
            msg->Message.__any.nstr = soap_strdup(soap, "http://www.onvif.org/ver10/schema");

            char msg_data[512];
            snprintf(msg_data, sizeof(msg_data),
                "<tt:Source><tt:SimpleItem Name=\"%s\" Value=\"%s\"/></tt:Source>"
                "<tt:Data><tt:SimpleItem Name=\"%s\" Value=\"%s\"/></tt:Data>",
                ev_buf[i].source_name, ev_buf[i].source_value,
                ev_buf[i].data_name, ev_buf[i].data_value);
            msg->Message.__any.text = soap_strdup(soap, msg_data);

            msg->ProducerReference = NULL;
            msg->SubscriptionReference = NULL;
        }
    } else {
        resp->__sizeNotificationMessage = 0;
        resp->wsnt__NotificationMessage = NULL;
    }

    free(ev_buf);
    return SOAP_OK;
}

/**
 * Renew — extends subscription termination time.
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__Renew(struct soap *soap,
    struct _wsnt__Renew *req,
    struct _wsnt__RenewResponse *resp)
{
    printf("[Events] Renew called (TerminationTime=%s)\n",
           req->TerminationTime ? req->TerminationTime : "null");

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    char id_buf[UUID_STR_LEN];
    if (extract_subscription_id(soap, id_buf) != 0) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "SubscriptionId not found in request");
        return SOAP_FAULT;
    }

    /* Parse new termination time */
    int term_secs = DEFAULT_TERM_SECS;
    if (req->TerminationTime) {
        int parsed = parse_duration_seconds(req->TerminationTime);
        if (parsed > 0 && parsed <= MAX_TERM_SECS)
            term_secs = parsed;
        else if (parsed > MAX_TERM_SECS)
            term_secs = MAX_TERM_SECS;
    }

    pthread_mutex_lock(&g_subs_lock);
    onvif_subscription_t *sub = find_subscription(id_buf);
    if (!sub) {
        pthread_mutex_unlock(&g_subs_lock);
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "Subscription not found");
        return SOAP_FAULT;
    }

    /* Extend termination time from now */
    struct timeval now = now_tv();
    sub->termination_time.tv_sec = now.tv_sec + term_secs;
    sub->termination_time.tv_usec = now.tv_usec;
    pthread_mutex_unlock(&g_subs_lock);

    printf("[Events] Renewed subscription %s (+%ds)\n", id_buf, term_secs);

    resp->TerminationTime = sub->termination_time;
    resp->CurrentTime = (struct timeval *)soap_malloc(soap, sizeof(struct timeval));
    *resp->CurrentTime = now;

    return SOAP_OK;
}

/**
 * Unsubscribe — destroys a subscription.
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__Unsubscribe(struct soap *soap,
    struct _wsnt__Unsubscribe *req,
    struct _wsnt__UnsubscribeResponse *resp)
{
    printf("[Events] Unsubscribe called\n");

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    char id_buf[UUID_STR_LEN];
    if (extract_subscription_id(soap, id_buf) != 0) {
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "SubscriptionId not found in request");
        return SOAP_FAULT;
    }

    pthread_mutex_lock(&g_subs_lock);
    onvif_subscription_t *sub = find_subscription(id_buf);
    if (!sub) {
        pthread_mutex_unlock(&g_subs_lock);
        onvif_fault(soap, ONVIF_ENV_SENDER, "ter:InvalidArgVal", "ter:InvalidSubscriptionId",
                    "Subscription not found");
        return SOAP_FAULT;
    }

    printf("[Events] Unsubscribed %s\n", id_buf);
    destroy_subscription(sub);
    pthread_mutex_unlock(&g_subs_lock);

    return SOAP_OK;
}

/**
 * SetSynchronizationPoint
 * Injects current-state property events for all known topics into the
 * subscription so PullMessages gets an initial snapshot.
 */
SOAP_FMAC5 int32_t SOAP_FMAC6 __tev__SetSynchronizationPoint(struct soap *soap,
    struct _tev__SetSynchronizationPoint *req,
    struct _tev__SetSynchronizationPointResponse *resp)
{
    printf("[Events] SetSynchronizationPoint called\n");

    int32_t ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (ret != SOAP_OK)
        return ret;

    /* Extract subscription ID (optional — may target all subs) */
    char id_buf[UUID_STR_LEN];
    int has_id = (extract_subscription_id(soap, id_buf) == 0 && id_buf[0] != '\0');

    /* Inject current-state property events */
    pthread_mutex_lock(&g_subs_lock);

    /* DigitalInput initial state: false */
    onvif_event_t ev;
    memset(&ev, 0, sizeof(ev));
    snprintf(ev.topic, sizeof(ev.topic), "tns1:Device/Trigger/DigitalInput");
    snprintf(ev.source_name, sizeof(ev.source_name), "InputToken");
    snprintf(ev.source_value, sizeof(ev.source_value), "DigitalInput_0");
    snprintf(ev.data_name, sizeof(ev.data_name), "LogicalState");
    snprintf(ev.data_value, sizeof(ev.data_value), "false");
    ev.timestamp = now_tv();
    ev.is_property = 1;

    /* MotionAlarm initial state: false */
    onvif_event_t ev_motion;
    memset(&ev_motion, 0, sizeof(ev_motion));
    snprintf(ev_motion.topic, sizeof(ev_motion.topic), "tns1:VideoSource/MotionAlarm");
    snprintf(ev_motion.source_name, sizeof(ev_motion.source_name), "Source");
    snprintf(ev_motion.source_value, sizeof(ev_motion.source_value), "VideoSource_1");
    snprintf(ev_motion.data_name, sizeof(ev_motion.data_name), "State");
    snprintf(ev_motion.data_value, sizeof(ev_motion.data_value), "false");
    ev_motion.timestamp = now_tv();
    ev_motion.is_property = 1;

    if (has_id) {
        onvif_subscription_t *sub = find_subscription(id_buf);
        if (sub && !subscription_expired(sub)) {
            enqueue_event(sub, &ev);
            enqueue_event(sub, &ev_motion);
        }
    } else {
        for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
            if (g_subs[i].active && !subscription_expired(&g_subs[i])) {
                enqueue_event(&g_subs[i], &ev);
                enqueue_event(&g_subs[i], &ev_motion);
            }
        }
    }

    pthread_mutex_unlock(&g_subs_lock);
    return SOAP_OK;
}
