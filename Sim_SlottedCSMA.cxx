
#line 1 "Sim_SlottedCSMA.cc"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>


#line 1 "./COST/cost.h"

























#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 "./COST/priority_q.h"























#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <string.h>














template < class ITEM >
class SimpleQueue 
{
 public:
  SimpleQueue() :m_head(NULL) {};
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  ITEM* NextEvent() const { return m_head; };
  const char* GetName();
 protected:
  ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
  static const char* name = "SimpleQueue";
  return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
  if( m_head==NULL || item->time < m_head->time )
  {
    if(m_head!=NULL)m_head->prev=item;
    item->next=m_head;
    m_head=item;
    item->prev=NULL;
    return;
  }
    
  ITEM* i=m_head;
  while( i->next!=NULL && item->time > i->next->time)
    i=i->next;
  item->next=i->next;
  if(i->next!=NULL)i->next->prev=item;
  i->next=item;
  item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
  if(m_head==NULL)return NULL;
  ITEM* item = m_head;
  m_head=m_head->next;
  if(m_head!=NULL)m_head->prev=NULL;
  return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
  if(item==NULL) return;

  if(item==m_head)
  {
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
  }
  else
  {
    item->prev->next=item->next;
    if(item->next!=NULL)
      item->next->prev=item->prev;
  }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
  void Delete(ITEM*);
  void EnQueue(ITEM*);
  bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{

  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=NULL)
  {
    if(i==item)
    {
      pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
    }
    i=i->next;
  }
  SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=item&&i!=NULL)
    i=i->next;
  if(i==NULL)
    pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
  else
    SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
  char out[1000],buff[100];

  ITEM* i=SimpleQueue<ITEM>::m_head;
  bool qerror=false;

  sprintf(out,"queue error %s : ",s);
  while(i!=NULL)
  {
    sprintf(buff,"%f ",i->time);
    strcat(out,buff);
    if(i->next!=NULL)
      if(i->next->prev!=i)
      {
	qerror=true;
	sprintf(buff," {broken} ");
	strcat(out,buff);
      }
    if(i==i->next)
    {
      qerror=true;
      sprintf(buff,"{loop}");
      strcat(out,buff);
      break;
    }
    i=i->next;
  }
  if(qerror)
    printf("%s\n",out);
  return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
  ITEM* DeQueue(double);
  const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
  static const char* name = "ErrorQueue";
  return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
  

  if(drand48()>0.5)
    return SimpleQueue<ITEM>::DeQueue();

  int s=0;
  ITEM* e;
  e=SimpleQueue<ITEM>::m_head;
  while(e!=NULL&&e->time<stoptime)
  {
    s++;
    e=e->next;
  }
  e=SimpleQueue<ITEM>::m_head;
  s=(int)(s*drand48());
  while(s!=0)
  {
    e=e->next;
    s--;
  }
  Delete(e);
  return e;
}

template < class ITEM >
class HeapQueue 
{
 public:
  HeapQueue();
  ~HeapQueue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  const char* GetName();
  ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
 private:
  void SiftDown(int);
  void PercolateUp(int);
  void Validate(const char*);
        
  ITEM** elems;
  int num_of_elems;
  int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
  static const char* name = "HeapQueue";
  return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
  int i,j;
  char out[1000],buff[100];
  for(i=0;i<num_of_elems;i++)
    if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	 ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
    {
      sprintf(out,"queue error %s : ",s);
      for(j=0;j<num_of_elems;j++)
      {
	if(i!=j)
	  sprintf(buff,"%f(%d) ",elems[j]->time,j);
	else
	  sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
	strcat(out,buff);
      }
      printf("%s\n",out);
    }
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
  curr_max=16;
  elems=new ITEM*[curr_max];
  num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
  delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
  if(num_of_elems<=1) return;
  int i=node,k,c1,c2;
  ITEM* temp;
        
  do{
    k=i;
    c1=c2=2*i+1;
    c2++;
    if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
      i=c1;
    if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
      i=c2;
    if(k!=i)
    {
      temp=elems[i];
      elems[i]=elems[k];
      elems[k]=temp;
      elems[k]->pos=k;
      elems[i]->pos=i;
    }
  }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
  int i=node,k,p;
  ITEM* temp;
        
  do{
    k=i;
    if( (p=(i+1)/2) != 0)
    {
      --p;
      if(elems[i]->time < elems[p]->time)
      {
	i=p;
	temp=elems[i];
	elems[i]=elems[k];
	elems[k]=temp;
	elems[k]->pos=k;
	elems[i]->pos=i;
      }
    }
  }while(k!=i);
}

template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
  if(num_of_elems>=curr_max)
  {
    curr_max*=2;
    ITEM** buffer=new ITEM*[curr_max];
    for(int i=0;i<num_of_elems;i++)
      buffer[i]=elems[i];
    delete[] elems;
    elems=buffer;
  }
        
  elems[num_of_elems]=item;
  elems[num_of_elems]->pos=num_of_elems;
  num_of_elems++;
  PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
  if(num_of_elems<=0)return NULL;
        
  ITEM* item=elems[0];
  num_of_elems--;
  elems[0]=elems[num_of_elems];
  elems[0]->pos=0;
  SiftDown(0);
  return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
  int i=item->pos;

  num_of_elems--;
  elems[i]=elems[num_of_elems];
  elems[i]->pos=i;
  SiftDown(i);
  PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
 public:
  CalendarQueue();
  const char* GetName();
  ~CalendarQueue();
  void enqueue(ITEM*);
  ITEM* dequeue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  ITEM* NextEvent() const { return m_head;}
  void Delete(ITEM*);
 private:
  long last_bucket,number_of_buckets;
  double bucket_width;
        
  void ReSize(long);
  double NewWidth();

  ITEM ** buckets;
  long total_number;
  double bucket_top;
  long bottom_threshold;
  long top_threshold;
  double last_priority;
  bool resizable;

  ITEM* m_head;
  char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
  sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	  bucket_width,number_of_buckets);
  return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
  long i;
        
  number_of_buckets=16;
  bucket_width=1.0;
  bucket_top=bucket_width;
  total_number=0;
  last_bucket=0;
  last_priority=0.0;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  resizable=true;
        
  buckets= new ITEM*[number_of_buckets];
  for(i=0;i<number_of_buckets;i++)
    buckets[i]=NULL;
  m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
  delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
  long i;
  ITEM** old_buckets=buckets;
  long old_number=number_of_buckets;
        
  resizable=false;
  bucket_width=NewWidth();
  buckets= new ITEM*[newsize];
  number_of_buckets=newsize;
  for(i=0;i<newsize;i++)
    buckets[i]=NULL;
  last_bucket=0;
  total_number=0;

  
        
  ITEM *item;
  for(i=0;i<old_number;i++)
  {
    while(old_buckets[i]!=NULL)
    {
      item=old_buckets[i];
      old_buckets[i]=item->next;
      enqueue(item);
    }
  }
  resizable=true;
  delete[] old_buckets;
  number_of_buckets=newsize;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
  ITEM* head=m_head;
  m_head=dequeue();
  return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
  long i;
  for(i=last_bucket;;)
  {
    if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
    {
      ITEM * item=buckets[i];
      buckets[i]=buckets[i]->next;
      total_number--;
      last_bucket=i;
      last_priority=item->time;
                        
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      item->next=NULL;
      return item;
    }
    else
    {
      i++;
      if(i==number_of_buckets)i=0;
      bucket_top+=bucket_width;
      if(i==last_bucket)
	break;
    }
  }
        
  
  long smallest;
  for(smallest=0;smallest<number_of_buckets;smallest++)
    if(buckets[smallest]!=NULL)break;

  if(smallest >= number_of_buckets)
  {
    last_priority=bucket_top;
    return NULL;
  }

  for(i=smallest+1;i<number_of_buckets;i++)
  {
    if(buckets[i]==NULL)
      continue;
    else
      if(buckets[i]->time<buckets[smallest]->time)
	smallest=i;
  }
  ITEM * item=buckets[smallest];
  buckets[smallest]=buckets[smallest]->next;
  total_number--;
  last_bucket=smallest;
  last_priority=item->time;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  item->next=NULL;
  return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
  
  if(m_head==NULL)
  {
    m_head=item;
    return;
  }
  if(m_head->time>item->time)
  {
    enqueue(m_head);
    m_head=item;
  }
  else
    enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
  long i;
  if(item->time<last_priority)
  {
    i=(long)(item->time/bucket_width);
    last_priority=item->time;
    bucket_top=bucket_width*(i+1)+bucket_width*0.5;
    i=i%number_of_buckets;
    last_bucket=i;
  }
  else
  {
    i=(long)(item->time/bucket_width);
    i=i%number_of_buckets;
  }

        
  

  if(buckets[i]==NULL||item->time<buckets[i]->time)
  {
    item->next=buckets[i];
    buckets[i]=item;
  }
  else
  {

    ITEM* pos=buckets[i];
    while(pos->next!=NULL&&item->time>pos->next->time)
    {
      pos=pos->next;
    }
    item->next=pos->next;
    pos->next=item;
  }
  total_number++;
  if(resizable&&total_number>top_threshold)
    ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
  if(item==m_head)
  {
    m_head=dequeue();
    return;
  }
  long j;
  j=(long)(item->time/bucket_width);
  j=j%number_of_buckets;
        
  

  
  

  ITEM** p = &buckets[j];
  
  ITEM* i=buckets[j];
    
  while(i!=NULL)
  {
    if(i==item)
    { 
      (*p)=item->next;
      total_number--;
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      return;
    }
    p=&(i->next);
    i=i->next;
  }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
  long i, nsamples;
        
  if(total_number<2) return 1.0;
  if(total_number<=5)
    nsamples=total_number;
  else
    nsamples=5+total_number/10;
  if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
  long _last_bucket=last_bucket;
  double _bucket_top=bucket_top;
  double _last_priority=last_priority;
        
  double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
  ITEM* list,*next,*item;
        
  list=dequeue(); 
  long real_samples=0;
  while(real_samples<nsamples)
  {
    item=dequeue();
    if(item==NULL)
    {
      item=list;
      while(item!=NULL)
      {
	next=item->next;
	enqueue(item);
	item=next;      
      }

      last_bucket=_last_bucket;
      bucket_top=_bucket_top;
      last_priority=_last_priority;

                        
      return 1.0;
    }
    AVG[real_samples]=item->time-list->time;
    avg1+=AVG[real_samples];
    if(AVG[real_samples]!=0.0)
      real_samples++;
    item->next=list;
    list=item;
  }
  item=list;
  while(item!=NULL)
  {
    next=item->next;
    enqueue(item);
    item=next;      
  }
        
  last_bucket=_last_bucket;
  bucket_top=_bucket_top;
  last_priority=_last_priority;
        
  avg1=avg1/(double)(real_samples-1);
  avg1=avg1*2.0;
        
  
  long count=0;
  for(i=0;i<real_samples-1;i++)
  {
    if(AVG[i]<avg1&&AVG[i]!=0)
    {
      avg2+=AVG[i];
      count++;
    }
  }
  if(count==0||avg2==0)   return 1.0;
        
  avg2 /= (double) count;
  avg2 *= 3.0;
        
  return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 38 "./COST/cost.h"


#line 1 "./COST/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 39 "./COST/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  
  };
  TimerBase* object;
  int index;
  unsigned char active;
};



class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	
};

class TypeII;



class CostSimEng
{
 public:

  class seed_t
      {
       public:
	void operator = (long seed) { srand48(seed); };
      };
  seed_t		Seed;
  CostSimEng()
      : stopTime( 0), clearStatsTime( 0), m_clock( 0.0)
      {
        if( m_instance == NULL)
	  m_instance = this;
        else
	  printf("Error: only one simulation engine can be created\n");
      }
  virtual		~CostSimEng()	{ }
  static CostSimEng	*Instance()
      {
        if(m_instance==NULL)
        {
	  printf("Error: a simulation engine has not been initialized\n");
	  m_instance = new CostSimEng;
        }
        return m_instance;
      }
  CorsaAllocator	*GetAllocator(unsigned int datasize)
      {
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
	  if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
      }
  void		AddComponent(TypeII*c)
      {
        m_components.push_back(c);
      }
  void		ScheduleEvent(CostEvent*e)
      {
	if( e->time < m_clock)
	{
	  printf("scheduled event-> time: %f, object: %p, m_clock: %f\n",e->time,e->object,m_clock);
	  assert(e->time>=m_clock);
	}
	m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        
        m_queue.Delete(e);
      }
  double	Random( double v=1.0)	{ return v*drand48();}
  int		Random( int v)		{ return (int)(v*drand48()); }
  double	Exponential(double mean)	{ return -mean*log(Random());}
  virtual void	Start()		{}
  virtual void	Stop()		{}
  void		Run();
  double	SimTime()	{ return m_clock; } 
  void		StopTime( double t)	{ stopTime = t; }
  double	StopTime() const	{ return stopTime; }
  void		ClearStatsTime( double t)	{ clearStatsTime = t; }
  double	ClearStatsTime() const	{ return clearStatsTime; }
  virtual void	ClearStats()	{}
 private:
  double	stopTime;
  double	clearStatsTime;	
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};




class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		
  TypeII()
      {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
      }

#ifdef COST_DEBUG
  void Print(const bool, const char*, ...);
#endif
    
  double Random(double v=1.0) { return v*drand48();}
  int Random(int v) { return (int)(v*drand48());}
  double Exponential(double mean) { return -mean*log(Random());}
  inline double SimTime() const { return m_simeng->SimTime(); }
  inline double StopTime() const { return m_simeng->StopTime(); }
 private:
  CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
  if(flag==false) return;
  va_list ap;
  va_start(ap, format);
  printf("[%.10f] ",SimTime());
  vprintf(format,ap);
  va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
  double	nextTime = (clearStatsTime != 0.0 && clearStatsTime < stopTime) ? clearStatsTime : stopTime;

  m_clock = 0.0;
  eventsProcessed = 0l;
  std::vector<TypeII*>::iterator iter;
      
  struct timeval start_time;    
  gettimeofday( &start_time, NULL);

  Start();

  for( iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Start();

  CostEvent* e=m_queue.DeQueue();
  while( e != NULL)
  {
    if( e->time >= nextTime)
    {
      if( nextTime == stopTime)
	break;
      
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    
    assert( e->time >= m_clock);
    m_clock = e->time;
    e->object->activate( e);
    eventsProcessed++;
    e = m_queue.DeQueue();
  }
  m_clock = stopTime;
  for(iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Stop();
	    
  Stop();

  struct timeval stop_time;    
  gettimeofday(&stop_time,NULL);

  runningTime = stop_time.tv_sec - start_time.tv_sec +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0;
  eventRate = eventsProcessed/runningTime;
  
  
  printf("# -------------------------------------------------------------------------\n");	
  printf("# CostSimEng with %s, stopped at %f\n", m_queue.GetName(), stopTime);	
  printf("# %ld events processed in %.3f seconds, event processing rate: %.0f\n",	
  eventsProcessed, runningTime, eventRate);
  
}







#line 8 "Sim_SlottedCSMA.cc"


#include <deque>


#line 1 "Channel.h"




#define DATARATE 11E6 // Data Transmission Rate
#define PHYRATE 1E6








#define SLOT 16e-06
#define DIFS 34e-06
#define SIFS 9e-06
#define LDBPS 256
#define TSYM 4e-06
			

#line 1 "Aux.h"
#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time; 
	double queuing_time; 
	int aggregation; 
};

struct SLOT_notification
{
	int status; 
};


#endif 


#line 21 "Channel.h"



#line 75 "Channel.h"
;


#line 107 "Channel.h"
;


#line 118 "Channel.h"
;


#line 12 "Sim_SlottedCSMA.cc"


#line 1 "STA.h"
#include <math.h>
#include <iostream>
#include <fstream>

#line 1 "Aux.h"
#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time; 
	double queuing_time; 
	int aggregation; 
};

struct SLOT_notification
{
	int status; 
};


#endif 


#line 4 "STA.h"


#line 1 "FIFO.h"

#ifndef _FIFO_QUEUE_
#define _FIFO_QUEUE_



#include <deque>






template <class DATATYPE> class FIFO 
{	
	private:
		std::deque <DATATYPE> m_queue;
		
	public:
		DATATYPE GetFirstPacket();
		void DelFirstPacket();		
		void PutPacket(DATATYPE &packet);	
		int QueueSize();
};

template <class DATATYPE> DATATYPE FIFO <DATATYPE> :: GetFirstPacket()
{
	return(m_queue.front());	
}; 

template <class DATATYPE> void FIFO <DATATYPE> :: DelFirstPacket()
{
	if(m_queue.size() == 0) printf("Attempting to erase an empty queue\n");
	m_queue.pop_front();
}; 

template <class DATATYPE> void FIFO <DATATYPE> :: PutPacket(DATATYPE &packet)
{	
	m_queue.push_back(packet);
}; 

template <class DATATYPE> int FIFO <DATATYPE> :: QueueSize()
{
	return(m_queue.size());
}; 



#endif

#line 5 "STA.h"


#line 1 "includes/backoff.hh"
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>

#define CWMIN 16 //should be the same as ../STA.h

using namespace std;

int backoff(int backoff_stage, int stickiness, float driftProbability){

	int backoff_counter = 0;
	
	if(stickiness != 0){
		backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
	}else{
		backoff_counter = rand() % (int)(pow(2,backoff_stage)*CWMIN);
	}
	
	
	
	
	
	
	
	if(driftProbability > 0){
	    float slotDrift = rand() % 100 + 1;
	    slotDrift/=100;
	    
	    if((backoff_counter > 0) && (slotDrift > 0) && (slotDrift <= driftProbability/2.))
	    {
	        backoff_counter--; 
	        
	    }else if((slotDrift > driftProbability/2.) && (slotDrift <= driftProbability))
	    {
	        backoff_counter++; 
	        
	    }
	}
	return (backoff_counter);
}
#line 6 "STA.h"



#define MAXSTAGE 5


#define MAX_RET 6


using namespace std;


#line 106 "STA.h"
;


#line 169 "STA.h"
;


#line 228 "STA.h"
;


#line 441 "STA.h"
;


#line 13 "Sim_SlottedCSMA.cc"


#line 1 "BatchPoissonSource.h"



			

#line 1 "Aux.h"
#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time; 
	double queuing_time; 
	int aggregation; 
};

struct SLOT_notification
{
	int status; 
};


#endif 


#line 5 "BatchPoissonSource.h"


#define MAXSEQ 1024


#line 40 "BatchPoissonSource.h"
;


#line 47 "BatchPoissonSource.h"
;
	

#line 52 "BatchPoissonSource.h"
;


#line 76 "BatchPoissonSource.h"
;




#line 14 "Sim_SlottedCSMA.cc"


#line 1 "stats/stats.h"
#include <math.h>
#include <iostream>

using namespace std;

double stats(int successful_slots, int empty_slots, int collision_slots, int payload){
    
    double Ts, Tc;
    double throughput;
    
    
    const double PHY_RATE = 1E6;
    const double RBASIC = 1E6;
    const double RDATA = 11E6;
    const double PCLP_PREAMBLE = 144; 
    const double PCLP_HEADER = 48; 
    
    const double MAC_HEADER = 272; 
    const double L_ACK = 112; 
    
    
    const double SLOT_TIME = 20E-6;
    const double DIFS2 = 50E-6;
    const double SIFS2 = 10E-6;
    const double EIFS2 = SIFS2 + DIFS2 + ((144 + 48 + 112)/RBASIC);

    
    const double L = payload*8;
    
    
    
    Ts = ((PCLP_PREAMBLE + PCLP_HEADER)/PHY_RATE) + ((L + MAC_HEADER)/RDATA) + SIFS2 + ((PCLP_PREAMBLE + PCLP_HEADER)/PHY_RATE) + (L_ACK/RBASIC) + DIFS2;
    Tc = ((PCLP_PREAMBLE + PCLP_HEADER)/RBASIC)+ ((L+MAC_HEADER)/RDATA) + EIFS2;
    
    
    
    
    throughput = successful_slots*L/(empty_slots*SLOT_TIME+successful_slots*Ts+collision_slots*Tc);
    
    return(throughput);
}

#line 15 "Sim_SlottedCSMA.cc"


using namespace std;


#line 108 "Sim_SlottedCSMA.cc"
;


#line 113 "Sim_SlottedCSMA.cc"
;


#line 335 "Sim_SlottedCSMA.cc"
;

#include "compcxx_Sim_SlottedCSMA.h"
class compcxx_Channel_6;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Channel_6* p_compcxx_parent;};

class compcxx_Channel_6;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Channel_6* p_compcxx_parent;};

class compcxx_Channel_6;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Channel_6* p_compcxx_parent;};


#line 23 "Channel.h"
class compcxx_Channel_6 : public compcxx_component, public TypeII
{

	public:
		void Setup();
		void Start();
		void Stop();
			
	public:
		int Nodes;
		float error;

		
		class my_Channel_out_slot_f_t:public compcxx_functor<Channel_out_slot_f_t>{ public:void  operator() (SLOT_notification &slot) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(slot); return (c[0]->*f[0])(slot);};};compcxx_array<my_Channel_out_slot_f_t > out_slot;/*outport void out_slot(SLOT_notification &slot)*/;	
		/*inport */void in_packet(Packet &packet);

		
		compcxx_Timer_2 /*<trigger_t> */slot_time; 
		compcxx_Timer_3 /*<trigger_t> */rx_time; 
		compcxx_Timer_4 /*<trigger_t> */cpSampler; 
		
		/*inport */inline void NewSlot(trigger_t& t1);
		/*inport */inline void EndReceptionTime(trigger_t& t2);
		/*inport */inline void Sampler(trigger_t& t3);

		compcxx_Channel_6 () { 
			slot_time.p_compcxx_parent=this /*connect slot_time.to_component,*/; 
			rx_time.p_compcxx_parent=this /*connect rx_time.to_component,*/;
		    cpSampler.p_compcxx_parent=this /*connect cpSampler.to_component,*/; }

	private:
		double number_of_transmissions_in_current_slot;
		double succ_tx_duration, collision_duration; 
		double empty_slot_duration;
		double L_max;
		double TBack;
		int MAC_H, PCLP_PREAMBLE, PCLP_HEADER;
		int aggregation;
		float errorProbability;
		
		
     	ofstream slotsInTime;

	public: 
		double collision_slots, empty_slots, succesful_slots, total_slots;
		double totalBitsSent;
		long long int test;
};

class compcxx_BatchPoissonSource_8;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_5 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_5() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_BatchPoissonSource_8* p_compcxx_parent;};


#line 9 "BatchPoissonSource.h"
class compcxx_BatchPoissonSource_8 : public compcxx_component, public TypeII
{
	public:
		
		class my_BatchPoissonSource_out_f_t:public compcxx_functor<BatchPoissonSource_out_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_BatchPoissonSource_out_f_t out_f;/*outport void out(Packet &packet)*/;	

		
		compcxx_Timer_5 /*<trigger_t> */inter_packet_timer;
		/*inport */inline void new_packet(trigger_t& t);

		compcxx_BatchPoissonSource_8 () { 
			inter_packet_timer.p_compcxx_parent=this /*connect inter_packet_timer.to_component,*/; }

	public:
		int L;
		int seq;
		double bandwidth; 
		double packet_rate;
		int MaxBatch;
		int aggregation;
	
	public:
		void Setup();
		void Start();
		void Stop();
			
};


#line 17 "STA.h"
class compcxx_STA_7 : public compcxx_component, public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
        int node_id;
        int K; 
        int system_stickiness; 
        int station_stickiness;
        int hysteresis;
        int fairShare;
	
        













        
        double observed_slots;
        double empty_slots;
                                                                         
        
        double collisions;
        double total_transmissions;
        double successful_transmissions;
        double droppedPackets; 
        double packetDisposal;
        double packetDisposalRET; 
        double driftedSlots;

        double incoming_packets;
        double non_blocked_packets;
        double blocked_packets;

        double txDelay;
        double throughput;
        double staDelay; 
        double blockingProbability;
        
        float slotDrift;
        float driftProbability; 
        
        
        int finalBackoffStage;
        int qEmpty;
        int qSize;
        
        
        
        int cut;
        int DCF;
        
        
        int maxAggregation;

    private:
        int backoff_counter;
        int backoff_stage;
        int pickup_backoff_stage;
        int previous_pickup_backoff_stage;
        int backlogged;

        int txAttempt;	

        Packet packet;
        FIFO <Packet> MAC_queue;

    public:
        
        /*inport */void inline in_slot(SLOT_notification &slot);
        /*inport */void inline in_packet(Packet &packet);
        class my_STA_out_packet_f_t:public compcxx_functor<STA_out_packet_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_STA_out_packet_f_t out_packet_f;/*outport void out_packet(Packet &packet)*/;
};


#line 19 "Sim_SlottedCSMA.cc"
class compcxx_SlottedCSMA_9 : public compcxx_component, public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int maxAggregation, int simSeed);
		void Stop();
		void Start();		

	public:
		compcxx_Channel_6 channel;

		compcxx_array<compcxx_STA_7  >stas;
		compcxx_array<compcxx_BatchPoissonSource_8  >sources;

	private:
		int SimId;
		int Nodes;
		double Bandwidth_;
		int PacketLength_;
		int Batch_;
		float drift;
		double intCut, decimalCut, cut; 
		

};


#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 302 "./COST/cost.h"

#line 302 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 313 "./COST/cost.h"

#line 313 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->Sampler(m_event.data));
}




#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 302 "./COST/cost.h"

#line 302 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 313 "./COST/cost.h"

#line 313 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->EndReceptionTime(m_event.data));
}




#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 302 "./COST/cost.h"

#line 302 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 313 "./COST/cost.h"

#line 313 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->NewSlot(m_event.data));
}




#line 44 "Channel.h"

#line 45 "Channel.h"

#line 46 "Channel.h"

#line 72 "Channel.h"
void compcxx_Channel_6 :: Setup()
{

}
#line 77 "Channel.h"
void compcxx_Channel_6 :: Start()
{
	number_of_transmissions_in_current_slot = 0;
	succ_tx_duration = 10E-3;
	collision_duration = 10E-3;
	empty_slot_duration = 9e-06;

	collision_slots = 0;
	empty_slots = 0;
	succesful_slots = 0;
	total_slots = 0;
	test = 0;

	L_max = 0;
	
	MAC_H = 272;
	PCLP_PREAMBLE = 144; 
	PCLP_HEADER = 48;
	
	TBack = 32e-06 + ceil((16 + 256 + 6)/LDBPS) * TSYM;
	totalBitsSent = 0;

	aggregation = 0;
	errorProbability = 0;

	slot_time.Set(SimTime()); 
    cpSampler.Set(SimTime() + 1); 
	
	slotsInTime.open("Results/slotsInTime.txt", ios::app);

}
#line 109 "Channel.h"
void compcxx_Channel_6 :: Stop()
{
	printf("\n\n");
	printf("---- Channel ----\n");
	printf("Slot Status Probabilities (channel point of view): Empty = %e, Succesful = %e, Collision = %e \n",empty_slots/total_slots,succesful_slots/total_slots,collision_slots/total_slots);
	printf("Total packets sent to the Channel: %d", (int)succesful_slots);
	printf("\n\n");
	
	slotsInTime.close();
}
#line 120 "Channel.h"
void compcxx_Channel_6 :: Sampler(trigger_t &)
{
    
    
	







	
	cpSampler.Set(SimTime() + 1);
}


#line 136 "Channel.h"
void compcxx_Channel_6 :: NewSlot(trigger_t &)
{
	

	SLOT_notification slot;

	slot.status = number_of_transmissions_in_current_slot;

	number_of_transmissions_in_current_slot = 0;
	L_max = 0;

	for(int n = 0; n < Nodes; n++) out_slot[n](slot); 

	rx_time.Set(SimTime());	
}


#line 152 "Channel.h"
void compcxx_Channel_6 :: EndReceptionTime(trigger_t &)
{
    
	
	if(number_of_transmissions_in_current_slot==0) 
	{
		slot_time.Set(SimTime()+SLOT);
		empty_slots++;
	}
	if(number_of_transmissions_in_current_slot == 1)
	{
		slot_time.Set(SimTime()+succ_tx_duration);
		succesful_slots++;
		totalBitsSent += aggregation*(L_max*8);
	}
	if(number_of_transmissions_in_current_slot > 1)
	{
		slot_time.Set(SimTime()+collision_duration);
		collision_slots ++;	
	}
		
	total_slots++; 
	
	
	
    if(int(total_slots) % 1000 == 0) 
	{
	        slotsInTime << Nodes << " " << SimTime() << " " <<  total_slots << " " << collision_slots/total_slots << " " << succesful_slots/total_slots << " " << empty_slots/total_slots << endl;
	}
	
}



#line 185 "Channel.h"
void compcxx_Channel_6 :: in_packet(Packet &packet)
{

	if(packet.L > L_max) L_max = packet.L;
	
	aggregation = packet.aggregation;
	
	errorProbability = rand() % 100 + 1;
	
	if((errorProbability > 0) && (errorProbability <= error))
	{
	    
	    
	    number_of_transmissions_in_current_slot+=2;
	}else
	{
	    number_of_transmissions_in_current_slot++;
	}
	
	succ_tx_duration = 32e-06 + ceil((16 + aggregation*(32+(L_max*8)+288) + 6)/LDBPS)*TSYM + SIFS + TBack + DIFS + empty_slot_duration;
	
	collision_duration = succ_tx_duration;
	
	

	
	
}


#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 302 "./COST/cost.h"

#line 302 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 313 "./COST/cost.h"

#line 313 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->new_packet(m_event.data));
}




#line 17 "BatchPoissonSource.h"

#line 37 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_8 :: Setup()
{

}
#line 42 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_8 :: Start()
{
	packet_rate=bandwidth/(L*8);
	inter_packet_timer.Set(Exponential(1/packet_rate));
	seq=0;
}
#line 49 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_8 :: Stop()
{

}
#line 54 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_8 :: new_packet(trigger_t &)
{
	Packet packet;

	packet.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packet.seq = seq;
		packet.send_time = SimTime();

		(out_f(packet));

		seq++;
		if(seq == MAXSEQ) seq = 0;

	}
	
	inter_packet_timer.Set(SimTime()+Exponential(RB/packet_rate));	

}
#line 103 "STA.h"
void compcxx_STA_7 :: Setup()
{

}
#line 108 "STA.h"
void compcxx_STA_7 :: Start()
{
	if(node_id < cut)
	{
		
		DCF = 1;
		system_stickiness = 0;
		station_stickiness = 0;
		hysteresis = 0;
		
		if(maxAggregation > 0)
		{
			fairShare = 1;
			
		}else
		{
			fairShare = 0;
			
		}
	}else
	{
		
		DCF = 0;
	}
	
    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
    backoff_stage = 0;
    pickup_backoff_stage = 0;
    previous_pickup_backoff_stage = 0;
    packet.source = node_id;
    packet.L = 1024;
    packet.send_time = SimTime();
    
    observed_slots = 0;
    empty_slots = 0;
    
    txAttempt = 0;
    collisions = 0;
    successful_transmissions = 0;
    droppedPackets = 0;
    packetDisposal = 0;
    packetDisposalRET = 0;
    total_transmissions = 0;

    incoming_packets = 0;
    non_blocked_packets = 0;
    blocked_packets = 0;
    blockingProbability = 0;

    txDelay = 0;
    
    throughput = 0;
    staDelay = 0;
    
    slotDrift = 0;
    driftedSlots = 0;
    
    
    finalBackoffStage = 0;
    qEmpty = 0;
    
}
#line 171 "STA.h"
void compcxx_STA_7 :: Stop()
{
    
    throughput = packet.L*8*(float)successful_transmissions / SimTime();
    qSize = MAC_queue.QueueSize();
    
    blockingProbability = (float)blocked_packets / (float)incoming_packets;
    
    if(successful_transmissions > 0)
    {
    	staDelay = (float)txDelay / (float)successful_transmissions;
    }else
    {
    	staDelay = 0;
    }
    
    
    finalBackoffStage = backoff_stage;
    
    
    cout << endl;
    cout << "--- Station " << node_id << " stats ---" << endl;
    cout << "Total Transmissions:" << " " <<  total_transmissions << endl;
    if(total_transmissions > 0)
    {
    	cout << "Collisions:" << " " << collisions << endl;
    	cout << "Packets successfully sent:" << " " << successful_transmissions << endl;        
    	cout << "*** DETAILED ***" << endl;
    	cout << "TAU = " << (float)total_transmissions / (float)observed_slots << " |" << " p = " << (float)collisions / (float)total_transmissions << endl;
    	cout << "Throughput of this station = " << throughput << "bps" << endl;
    	cout << "Blocking Probability = " << blockingProbability << endl;
    	cout << "Average Delay (queueing + service) = " << staDelay << endl;
    	cout << endl;
    }
    
    cout <<"-----Debug-----"<<endl;
    if(DCF == 1)
	{
		
		cout << "I am using DCF" << endl;	
	}else
	{
		cout << "I am using something different" << endl;
	}
	cout << "Final backoff stage: " << finalBackoffStage << endl;
    cout << "System stickiness: " << system_stickiness << endl;
    cout << "Station stickiness: " << station_stickiness << endl;
    cout << "Hysteresis: " << hysteresis << endl;
    cout << "Fair Share: " << fairShare << endl;
    if(qEmpty > 1)
    {
    	cout << "The queue emptied " << qEmpty <<" times" << endl;
    }else
    {
    	cout << "Queue was always full" << endl;
    }
    
}
#line 230 "STA.h"
void compcxx_STA_7 :: in_slot(SLOT_notification &slot)
{
    observed_slots++;
    
    

    if (backlogged == 1)
    {
        if (slot.status == 0)
        {
            backoff_counter--;
            empty_slots++;
        }
        if (slot.status == 1)
        {             
            if (backoff_counter == 0) 
            {
                
                if(fairShare > 0)
                {
                	if(maxAggregation > 0)
                	{
                		packetDisposal = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
                		successful_transmissions += packetDisposal;
                		
                		for(int i = 0; i < packetDisposal; i++)
                		{
                			txDelay += SimTime() - packet.queuing_time;
                			MAC_queue.DelFirstPacket();
                			if(i < packetDisposal-1) packet = MAC_queue.GetFirstPacket();
                    	}
                	}else
                	{
                		packetDisposal = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
                		successful_transmissions += packetDisposal;
                		
                		
                		for(int i = 0; i < packetDisposal; i++)
                		{
                			txDelay += SimTime() - packet.queuing_time;
                			MAC_queue.DelFirstPacket();
                			if(i < packetDisposal-1) packet = MAC_queue.GetFirstPacket();
                    	}
                	}
                	packetDisposal = 0;
                }else
                {
                    successful_transmissions++;
                    txDelay += SimTime() - packet.queuing_time;
                    MAC_queue.DelFirstPacket();
                }
                
                txAttempt = 0;
                
                if(hysteresis == 0) backoff_stage = 0;
                
                
                station_stickiness = system_stickiness;
               
                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                    backoff_stage = 0;
                    qEmpty++;
                }else
                {
                    packet = MAC_queue.GetFirstPacket(); 
                    packet.send_time = SimTime();
                    backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                }
                pickup_backoff_stage = backoff_stage;
                previous_pickup_backoff_stage = 0;
            }else
            {
                
                
                backoff_counter--;           
            }
            
        }
        
        
        
        if (slot.status > 1)
        {   
            if (backoff_counter == 0) 
            {
                txAttempt++;
                collisions++;
                
                if(system_stickiness > 0){ 
                    station_stickiness = std::max(0, station_stickiness-1);
                    if(station_stickiness == 0)
                    {
                        backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                    }else 
                    {                       
                        
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                    }
                }else
                {
                    backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                    backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                }
                    
                                                                  
                
                
                if ((MAX_RET < txAttempt) && (txAttempt < (MAX_RET + MAXSTAGE + 1)))
                {   
                    
                    if(hysteresis == 0) backoff_stage = 0;
                    
                    if(fairShare > 0){
                    	if(maxAggregation > 0)
                    	{
                    		
                    		packetDisposalRET = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
                    		droppedPackets+=packetDisposalRET;
                    		txAttempt = 0;
                    		for(int i = 0; i < packetDisposalRET; i++)
                    		{
                    			MAC_queue.DelFirstPacket();
                    		}
                    	}else
                    	{
                    		if(previous_pickup_backoff_stage == 0) 
                    		{
                    			packetDisposalRET = std::min((int)pow(2,pickup_backoff_stage),MAC_queue.QueueSize());
                    		}else
                    		{         		
                    			packetDisposalRET = std::min((int)(pow(2,pickup_backoff_stage)-pow(2,previous_pickup_backoff_stage)),MAC_queue.QueueSize());         		
                    		}
                    		droppedPackets+=packetDisposalRET;
                    		for(int i = 0; i < packetDisposalRET; i++)
                    		{
                    			MAC_queue.DelFirstPacket();
                    		}
                    		previous_pickup_backoff_stage = pickup_backoff_stage;
                 			pickup_backoff_stage = std::min(pickup_backoff_stage+1, MAXSTAGE);
                    	}
                    }else
                    {
                    	txAttempt = 0;
                    	droppedPackets++;
                    	MAC_queue.DelFirstPacket();
                 	}
                 	
                 	if(MAC_queue.QueueSize() > 0)
                 	{
                 		
                 		
                 		
                 		if((txAttempt == MAX_RET + MAXSTAGE) || (pickup_backoff_stage == MAXSTAGE))
                 		{
                 			MAC_queue.DelFirstPacket();
                 			droppedPackets++;
                 			txAttempt = 0;
                 			packetDisposalRET = 0;
                 		}
                 		packet = MAC_queue.GetFirstPacket();
                 		packet.send_time = SimTime();
                 	}else
                 	{
                 		backlogged = 0;
                 	}
                }
            }
            else
            {
                
                backoff_counter--;
            }
        }

    }
    
    if (backlogged == 0)
    {
        if (MAC_queue.QueueSize() > 0)
        {
            backlogged = 1;
            packet = MAC_queue.GetFirstPacket();
            packet.send_time = SimTime();
        }
        
    }
    
    
    if ((backoff_counter == 0) && (backlogged == 1))
    {
        total_transmissions++;
        if(fairShare > 0)
        {
        	if(maxAggregation > 0)
        	{
        		packet.aggregation = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
        	}else
        	{
        		packet.aggregation = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
        	}
        }else
        {
            packet.aggregation = 1;
        }
        
        (out_packet_f(packet));
    }
    
}
#line 443 "STA.h"
void compcxx_STA_7 :: in_packet(Packet &packet)
{
    incoming_packets++;

    if (MAC_queue.QueueSize() < K)
    {
        non_blocked_packets++;
        packet.queuing_time = SimTime();
        MAC_queue.PutPacket(packet);
    }else
    {
        blocked_packets++;
    }
}


#line 44 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_9 :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift, float percentageDCF, int maxAggregation, int simSeed)
{
	SimId = Sim_Id;
	Nodes = NumNodes;
	drift = slotDrift;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	
	channel.Nodes = NumNodes;
	channel.out_slot.SetSize(NumNodes);
	channel.error = channelErrors;

	
	
	cut = NumNodes * percentageDCF;
	decimalCut = modf(cut, &intCut);
	
	if(decimalCut > 0.5)
	{
		intCut++;	
	}
		
	


	
	
	for(int n=0;n<NumNodes;n++)
	{
		
		stas[n].node_id = n;
		stas[n].K = 1000;
		stas[n].system_stickiness = Stickiness;
		stas[n].station_stickiness = 0;
		stas[n].hysteresis = hysteresis;
		stas[n].fairShare = fairShare;
		stas[n].driftProbability = slotDrift;
		stas[n].cut = intCut;     
		stas[n].maxAggregation = maxAggregation;


		
		sources[n].bandwidth = Bandwidth;
		sources[n].L = PacketLength;
		sources[n].MaxBatch = Batch;
	}
	
	
	for(int n=0;n<NumNodes;n++)
	{
        channel.out_slot[n].Connect(stas[n],(compcxx_component::Channel_out_slot_f_t)&compcxx_STA_7::in_slot) /*connect channel.out_slot[n],stas[n].in_slot*/;
		stas[n].out_packet_f.Connect(channel,(compcxx_component::STA_out_packet_f_t)&compcxx_Channel_6::in_packet) /*connect stas[n].out_packet,channel.in_packet*/;
		sources[n].out_f.Connect(stas[n],(compcxx_component::BatchPoissonSource_out_f_t)&compcxx_STA_7::in_packet) /*connect sources[n].out,stas[n].in_packet*/;

	}


	Bandwidth_ = Bandwidth;
	PacketLength_ = PacketLength;
	Batch_ = Batch;
		

}
#line 110 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_9 :: Start()
{
	printf("--------------- SlottedCSMA ---------------\n");
}
#line 115 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_9 :: Stop()
{
	double p_res = 0;
	double delay_res = 0;
	
	double overall_successful_tx = 0;
	double overall_collisions = 0;
	double overall_empty = 0;
	double total_slots = 0;
	double overall_successful_tx_slots = 0;
	double driftedSlots = 0;
	double tx_slots = 0;
	double overall_throughput = 0;
	
	

	double avg_tau = 0;
	double std_tau = 0;
	
	double stas_throughput [Nodes];
	double stas_throughputDCF [Nodes];
	double stas_throughputECA [Nodes];
	double accumThroughputDCF = 0;
	double accumThroughputECA = 0;
	
	int DCFStas = 0;
	int ECAStas = 0;
	
	double fairness_index = 0;
	double systemTXDelay = 0.0;
	double systemAvgBlockingProbability = 0;
	
	double avgBackoffStage = 0;
	double accumaltedDroppedPackets = 0;
	double avgFinalQSize = 0;
	double QEmpties = 0; 

	
	for(int n=0;n<Nodes;n++)
	{
	    avg_tau += ((float)stas[n].total_transmissions / (float)stas[n].observed_slots);
	    driftedSlots += stas[n].driftedSlots;
	    tx_slots += stas[n].total_transmissions;
	    overall_successful_tx+=stas[n].successful_transmissions;
	    avgBackoffStage += stas[n].finalBackoffStage;
	}
	overall_collisions = channel.collision_slots;
	overall_empty = channel.empty_slots;
	total_slots = channel.total_slots;
	overall_successful_tx_slots = channel.succesful_slots;
	driftedSlots /= tx_slots;

	p_res /= Nodes;
	delay_res /= Nodes;
	
	avg_tau /= Nodes;
	
	
	avgBackoffStage /= Nodes;
	cout << endl;
	
	
	
	
	
	
	ofstream staStatistics;
	staStatistics.open("Results/multiStation.txt", ios::app);
	
	for(int i=0; i<Nodes; i++)
	{
	    std_tau += pow(avg_tau - ((float)stas[i].total_transmissions / (float)stas[i].observed_slots),2);
	    stas_throughput[i] = stas[i].throughput;
	    systemTXDelay += stas[i].staDelay;
	    
	    
	    
	    
	    QEmpties += stas[i].qEmpty;
	    
	    
	    if(stas[i].DCF > 0)
	    {
	    	stas_throughputDCF[i] = stas[i].throughput;
	    	stas_throughputECA[i] = 0;
	    	DCFStas++;
	    }else
	    {
	    	stas_throughputECA[i] = stas[i].throughput;
	    	stas_throughputDCF[i] = 0;
	    	ECAStas++;
	    }
	    accumThroughputDCF += stas_throughputDCF[i];
	    accumThroughputECA += stas_throughputECA[i];
	    accumaltedDroppedPackets += stas[i].droppedPackets;
	    avgFinalQSize += stas[i].qSize;
	    
	    
	    if(stas[i].incoming_packets == stas[i].successful_transmissions + stas[i].blocked_packets + stas[i].qSize + stas[i].droppedPackets)
	    {	
	    	cout << "Station " << i << ": is alright" << endl;
	    }else
	    {
	    	cout << "Station " << i << ": differs in " << stas[i].incoming_packets - (stas[i].successful_transmissions + stas[i].blocked_packets + stas[i].qSize + stas[i].droppedPackets) << endl;
	    }
	    
	    
	    systemAvgBlockingProbability += stas[i].blockingProbability;
	    
	    
	    
	    
	    
		staStatistics << i << " " << stas[i].throughput << " " << stas[i].collisions / stas[i].total_transmissions << " " << stas[i].total_transmissions / stas[i].observed_slots << " " << stas[i].staDelay << " " << stas[i].qEmpty << " " << stas[i].qSize << " " << stas[i].finalBackoffStage << " " << stas[i].droppedPackets << endl;
	}
	
	std_tau = pow((1.0/Nodes) * (float)std_tau, 0.5);
	
	systemTXDelay /= Nodes;
	
	systemAvgBlockingProbability /= Nodes;
	avgFinalQSize /= Nodes;
	
	
	double fair_numerator, fair_denominator;
	
	fair_numerator = 0;
	fair_denominator = 0;
	
	
	for(int k = 0; k < Nodes; k++)
	{
	    fair_numerator += stas_throughput[k];
	    fair_denominator += pow(stas_throughput[k],2);        
	}
	
	fairness_index = (pow(fair_numerator,2)) / (Nodes*fair_denominator);
	
	
	overall_throughput = (channel.totalBitsSent)/SimTime();

	ofstream statistics;
	statistics.open("Results/multiSim.txt", ios::app);
	statistics << Nodes << " " << overall_throughput << " " << overall_collisions / total_slots  << " " << fairness_index  << " " << Bandwidth_ << " " << systemTXDelay << " " << avgBackoffStage << " "; 
	if(DCFStas > 0){ 
	    statistics << accumThroughputDCF/DCFStas;
	}else
	{
	    statistics << accumThroughputDCF;
	}statistics << " ";
	if(ECAStas > 0){ 
	    statistics << accumThroughputECA/ECAStas;
	}else
	{
	    statistics << accumThroughputECA;
	}statistics << " " << fair_numerator << " ";
	
	statistics << systemAvgBlockingProbability << " " << accumaltedDroppedPackets/Nodes << " " << overall_successful_tx_slots << " " << overall_collisions << " " << overall_empty << " " << total_slots << " " << avg_tau  << " " << avgFinalQSize << " " << QEmpties << endl;
	
	cout << endl << endl;
	
	
	
	
	
	cout << "--- Overall Statistics ---" << endl;
	cout << "Average TAU = " << avg_tau << endl;
	cout << "Standard Deviation = " << (double)std_tau << endl;
	cout << "Overall Throughput = " << overall_throughput << endl;
	
	
	if((fair_numerator != (accumThroughputDCF + accumThroughputECA)) && (fair_numerator - (accumThroughputDCF+accumThroughputECA) > 1))
	{
		cout << "Error gathering the throughput of each station" << endl;
		cout << "Total: " << fair_numerator << " DCF: " << accumThroughputDCF << ", ECA: " << accumThroughputECA << ", diferring in: " << fair_numerator - (accumThroughputDCF+accumThroughputECA) << endl;
	}
	
	cout << "Jain's Fairness Index = " << fairness_index << endl;
	cout << "Overall average system TX delay = " << systemTXDelay << endl;
	cout << "Percentage of drifted slots = " << driftedSlots*100 << "%" << endl << endl;
	
	
	cout << "***Debugg***" << endl;
	cout << "Average backoff stage [0-5]: " << avgBackoffStage << endl;
	cout << "Average number of dropped packets: " << accumaltedDroppedPackets/Nodes << endl;
	cout << "Average blocking proability: " << systemAvgBlockingProbability << endl;
	cout << "Number of times each MAC queue emptied: " << QEmpties << endl;
	cout << "Slot drift probability: " << drift*100 << "%" << endl;
	cout << "Sx Slots: " << overall_successful_tx_slots << endl;
	cout << "Collision Slots: " << overall_collisions << endl;
	cout << "Empty Slots: " << overall_empty << endl;
	cout << "Total Slots: " << total_slots << endl;
	if(total_slots != (overall_successful_tx_slots+overall_collisions+overall_empty))
	{
	    cout << "They differ by: " << fabs(total_slots - (overall_successful_tx_slots+overall_collisions+overall_empty)) << endl;    
	}else
	{
	    cout << "Total Slots = Sucessful + Collision + Empty" << endl;
	}
	
	cout << "Total bits sent: " << channel.totalBitsSent << " if divided by " << SimTime() << "seconds of simulation, equals = " << (channel.totalBitsSent)/SimTime() << endl << endl;

	cout << "---Debugging the mixed scenario---" << endl;
	cout << "There are: " << DCFStas << " stations with DCF and: " << ECAStas << " with CSMA/ECA." << endl;
	if(Nodes != (DCFStas + ECAStas)) cout << "Miscount of stations" << endl;
	
	
	if(ECAStas == 0) ECAStas = 1;
	if(DCFStas == 0) DCFStas = 1;
	cout << "The average throughput of DCF stations is: " << accumThroughputDCF/DCFStas << "bps" << endl;
	cout << "The average throughput of Full CSMA/ECA staions is: " << accumThroughputECA/ECAStas << "bps" << endl;
	if((accumThroughputECA == 0) || (accumThroughputDCF == 0))
	{
		cout << "Some stations received no throughput, so the CSMA/ECA / CSMA/CA cannot be computed" << endl;
	}else
	{
		cout << "CSMA/ECA / CSMA/CA ratio: " << (accumThroughputECA/ECAStas)/(accumThroughputDCF/DCFStas) << endl;
	}
	

}int main(int argc, char *argv[])
{
	int MaxSimIter;
	double SimTime;
	int NumNodes;
	int PacketLength;
	double Bandwidth;
	int Batch;
	int Stickiness;
	int hysteresis;
	int fairShare;
	float channelErrors;
	float slotDrift;
	float percentageDCF;
	int maxAggregation;
	int simSeed;
	
	if(argc < 12) 
	{
		if(argv[1])
		{
			string word = argv[1];
			string help ("--help");
			string helpShort ("-h");
			if((word.compare(help) == 0) || (word.compare(helpShort) == 0)){
				cout << endl;
				cout << "------------" << endl;
				cout << "Cheatsheet:" << endl;
				cout << "------------" << endl;
				cout << "(0)./XXXX (1)SimTime (2)NumNodes (3)PacketLength (4)Bandwidth (5)Batch (6)Stickiness (7)hysteresis (8)fairShare (9)channelErrors (10)slotDrift (11)percentageOfDCF (12)maxAggregation (13)simSeed" << endl << endl;;
				cout << "0) ./XXX: Name of executable file" << endl;
				cout << "1) SimTime: simulation time in seconds" << endl;
				cout << "2) NumNodes: number of contenders" << endl;
				cout << "3) PacketLength: length of the packet in bytes" << endl;
				cout << "4) Bandwidth: number of bits per second generated by the source. With 802.11n and DCF, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) Stickiness: activates CSMA/ECA. Nodes pick a deterministic backoff (0=off, 1=on)" << endl;
				cout << "7) Hysteresis: nodes do not reset their backoff stage after successful transmissions (0=off, 1=on)" << endl;
				cout << "8) FairShare: nodes at backoff stage k, attempt the transmission of 2^k packets (0=off, 1=on)" << endl;
				cout << "9) ChannelErrors: channel errors probability [0,1]" << endl;
				cout << "10) SlotDrift: probability of miscounting passing empty slots [0,1]" << endl;
				cout << "11) PercetageDCF: percentage of nodes running DCF. Used to simulate CSMA/ECA and CSMA/CA mixed scenarios [0,1]" << endl;
				cout << "12) MaxAggregation: nodes use maximum aggregation when attempting transmission (0=off, 1=on)" << endl;
				cout << "13) SimSeed: simulation seed used to generate random numbers. If testing results, repeat simulations with different seeds everytime" << endl << endl;
				return(0);
			}else
			{
				cout << endl;
				cout << "Alert: Unintelligible command" << endl;
				cout << "Use the parameter --help or -h to display the available settings" << endl << endl;
				return(0);
			}
		}else
		{
			cout << "Executed with default values shown below" << endl;
			cout << "./XXXX SimTime [10] NumNodes [10] PacketLength [1024] Bandwidth [65e6] Batch [1] Stickiness [0] hysteresis [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfDCF [1] maxAggregation [0] simSeed [0]" << endl;
			MaxSimIter = 1;
			SimTime = 10;
			NumNodes = 10;
			PacketLength = 1024;
			Bandwidth = 65e6;
			Batch = 1; 
			Stickiness = 0; 
			hysteresis = 0; 
			fairShare = 0; 
			channelErrors = 0; 
			slotDrift = 0; 
			percentageDCF = 1; 
			maxAggregation = 0;
			simSeed = 0; 
		}
	}else
	{
		MaxSimIter = 1;
		SimTime = atof(argv[1]);
		NumNodes = atoi(argv[2]);
		PacketLength = atoi(argv[3]);
		Bandwidth = atof(argv[4]);
		Batch = atoi(argv[5]); 
		Stickiness = atoi(argv[6]); 
		hysteresis = atoi(argv[7]); 
		fairShare = atoi(argv[8]); 
		channelErrors = atof(argv[9]); 
		slotDrift = atof(argv[10]); 
		percentageDCF = atof(argv[11]); 
		maxAggregation = atoi(argv[12]); 
		simSeed = atof(argv[13]); 
	}

	printf("####################### Simulation (Seed: %d) #######################\n",simSeed);
	if(Stickiness > 0)
	{
		if(hysteresis > 0)
		{
			if(fairShare > 0)
			{
				cout << "####################### Full ECA #######################" << endl;
			}else
			{
				cout << "################### ECA + hysteresis ###################" << endl;
			}
		}else
		{
			cout << "###################### Basic ECA ######################" << endl;
		}
	}else
	{
		cout << "####################### CSMA/CA #######################" << endl;
	}
	
	if(percentageDCF > 0) cout << "####################### Mixed setup " << percentageDCF*100 << "% DCF #######################" << endl;
		
	compcxx_SlottedCSMA_9 test;

	
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, hysteresis, fairShare, channelErrors, slotDrift, percentageDCF, maxAggregation, simSeed);
	
	test.Run();


	return(0);
};
