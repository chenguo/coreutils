typedef enum ia_queue_pushtype_t
{
    QUEUE_TAP,      // non blocking push
    QUEUE_PUSH,     // blocking push
    QUEUE_SHOVE,    // non blocking push + dynamic resize
    QUEUE_PSORT,    // push items in sorted order
    QUEUE_SSORT     // shove items in sorted order
} ia_queue_pushtype_t;

typedef struct ia_queue_obj_t
{
    uint32_t                i_pos;
    void*                   data;
    int32_t                 life;
    struct ia_queue_obj_t*  next;
    struct ia_queue_obj_t*  last;
} ia_queue_obj_t;

typedef struct ia_queue_t
{
    ia_queue_obj_t* head;
    ia_queue_obj_t* tail;
    uint32_t        count;
    uint32_t        size;
    uint32_t        waiter_pos;
    int32_t         life;
    pthread_mutex_t mutex;
    pthread_cond_t  cond_nonempty;
    pthread_cond_t  cond_nonfull;
} ia_queue_t;

static ia_queue_t* ia_queue_open( size_t size, int life );
static void ia_queue_close( ia_queue_t* q );
static void ia_queue_push( ia_queue_t* q, void* data, uint32_t pos );
static void* ia_queue_pop( ia_queue_t* q );
static int ia_queue_is_full( ia_queue_t* q );

#define ABS_MAX_SIZE 1000

static
ia_queue_t* ia_queue_open( size_t size, int life )
{
    ia_queue_t* q = malloc( sizeof(ia_queue_t) );
    memset( q, 0, sizeof(ia_queue_t) );
    q->size = size;
    q->life = life;
    pthread_mutex_init( &q->mutex, NULL );
    pthread_cond_init( &q->cond_nonempty, NULL );
    pthread_cond_init( &q->cond_nonfull, NULL );
    return q;
}

static
void ia_queue_close( ia_queue_t* q )
{
    int rc = pthread_mutex_lock( &q->mutex );
    
    while( q->count ) {
        rc = pthread_mutex_unlock( &q->mutex );
        
        ia_queue_pop( q );
    }
    pthread_mutex_unlock( &q->mutex );
    pthread_mutex_destroy( &q->mutex );
    pthread_cond_destroy( &q->cond_nonempty );
    pthread_cond_destroy( &q->cond_nonfull );
    free( q );
}

static
ia_queue_obj_t* ia_queue_create_obj( void* data )
{
    ia_queue_obj_t* obj = malloc( sizeof(ia_queue_obj_t) );
    if( !obj ) {
        fprintf( stderr, "Out of memory!\n" );
        pthread_exit( NULL );
    }

    obj->data = data;
    return obj;
}

static
void ia_queue_destroy_obj( ia_queue_obj_t* obj )
{
    free( obj );
}

static
int _ia_queue_push( ia_queue_t* q, void* data, uint32_t pos, ia_queue_pushtype_t pt )
{
    int rc;
    ia_queue_obj_t* obj;
    ia_queue_obj_t* tmp;

    if( (pt == QUEUE_TAP && ia_queue_is_full(q))
        || (pt == QUEUE_TAP && q->size >= ABS_MAX_SIZE) )
        return 1;

    // get lock on queue
    rc = pthread_mutex_lock( &q->mutex );
    
    while( q->count >= q->size ) {
        if( pos == q->waiter_pos
            || pt == QUEUE_SHOVE || pt == QUEUE_SSORT ) {
            break;
        }
        rc = pthread_cond_wait( &q->cond_nonfull, &q->mutex );
        
    }

    obj = ia_queue_create_obj( data );
    if( !obj ) {
        return 1;
    }

    obj->last =
    obj->next = NULL;
    obj->i_pos = pos;
    obj->life = q->life;

    // add image to queue
    if( pt == QUEUE_SSORT || pt == QUEUE_PSORT ) {
        tmp = q->tail;
        while( tmp != NULL ) {
            if( tmp->i_pos == obj->i_pos-1 )
                break;
            tmp = tmp->next;
        }
        // empty list + add first node
        if( tmp == NULL && !q->count ) {
            q->tail =
            q->head = obj;
        // nonempty list + insert at beginning
        } else if( tmp == NULL && q->count ) {
            obj->next = q->tail;
            q->tail->last = obj;
            q->tail = obj;
        // nonempty + insert at end
        } else if( tmp == q->head && q->count ) {
            obj->last = tmp;
            tmp->next = obj;
            q->head = obj;
        // insert into middle
        } else {
            obj->next = tmp->next;
            obj->last = tmp;
            obj->next->last = obj;
            obj->last->next = obj;
        }
    } else {
        if( !q->count ) {
            q->tail =
            q->head = obj;
        } else {
            obj->next = NULL;
            obj->last = q->head;
            q->head->next = obj;
            q->head = obj;
        }
    }
    q->count++;

    // if someone is waiting to pop, send signal
    rc = pthread_cond_signal( &q->cond_nonempty );
    

    // unlock queue
    rc = pthread_mutex_unlock( &q->mutex );
    

    return 0;
}

static
void ia_queue_push( ia_queue_t* q, void* data, uint32_t pos )
{
    _ia_queue_push( q, data, pos, QUEUE_PUSH );
}

/* returns unlocked image from queue */
static
void* ia_queue_pop( ia_queue_t* q )
{
    int rc;
    ia_queue_obj_t* obj;
    void* data;

    // get lock on queue
    rc = pthread_mutex_lock( &q->mutex );
    
    while( q->count == 0 ) {
        rc = pthread_cond_wait( &q->cond_nonempty, &q->mutex );
        
    }
    //assert( q->count > 0 );

    // pop image off queue
    obj = q->tail;
    q->tail = q->tail->next;
    data = obj->data;
    q->count--;

    // if someone is waiting to push, send signal
    rc = pthread_cond_signal( &q->cond_nonfull );
    

    // unlock queue
    rc = pthread_mutex_unlock( &q->mutex );
    

    // clean up queue object
    ia_queue_destroy_obj( obj );

    return data;
}

static
int ia_queue_is_full( ia_queue_t* q )
{
    int rc, status;

    rc = pthread_mutex_lock( &q->mutex );

    status = q->count >= q->size;

    rc = pthread_mutex_unlock( &q->mutex );

    return status;
}

static
int ia_queue_is_empty( ia_queue_t* q )
{
    int rc, status;

    rc = pthread_mutex_lock( &q->mutex );

    status = q->count == 0;

    rc = pthread_mutex_unlock( &q->mutex );

    return status;
}

