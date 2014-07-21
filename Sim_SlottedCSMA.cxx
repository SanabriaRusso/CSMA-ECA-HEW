
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
	int accessCategory; 
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
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <array>

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
	int accessCategory; 
};

struct SLOT_notification
{
	int status; 
};


#endif 


#line 8 "STA.h"


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

#line 9 "STA.h"


#line 1 "includes/computeBackoff.hh"
using namespace std;

void computeBackoff(int &backlog, double &qSize, int &AC, int &stickiness, int &backoffStage, double &counter){

	int CWmin = 0;
	
	switch (AC){
		case 0:
			if(qSize > 0) CWmin = 16;
			break;
		case 1:
			if(qSize > 0) CWmin = 16;
			break;
		case 2:
			if(qSize > 0) CWmin = 8;
			break;
		case 3:
			if(qSize > 0) CWmin = 4;
			break;	
	}
	
	if(CWmin > 0)
	{
		if(stickiness != 0){
			counter = (int)(pow(2,backoffStage)*CWmin/2)-1;
		}else{
			counter = rand() % (int)(pow(2,backoffStage)*CWmin);
		}
		backlog = 1;
	}else
	{
		backlog = 0;
	}
}
#line 10 "STA.h"


#line 1 "includes/selectMACProtocol.hh"
using namespace std;



void selectMACProtocol(const int &cut, const int &node_id, int &EDCA, int &hysteresis, int &fairShare, int &maxAggregation){

	if(node_id < cut)
	{
		
		EDCA = 1;
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
		
		EDCA = 0;
	}
}
#line 11 "STA.h"




#define MAXSTAGE 5


#define MAX_RET 6


#define AC 4


using namespace std;


#line 90 "STA.h"
;


#line 105 "STA.h"
;


#line 117 "STA.h"
;


#line 168 "STA.h"
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
	int accessCategory; 
};

struct SLOT_notification
{
	int status; 
};


#endif 


#line 5 "BatchPoissonSource.h"


#define MAXSEQ 1024


#line 54 "BatchPoissonSource.h"
;


#line 69 "BatchPoissonSource.h"
;
	

#line 74 "BatchPoissonSource.h"
;


#line 99 "BatchPoissonSource.h"
;


#line 124 "BatchPoissonSource.h"
;


#line 149 "BatchPoissonSource.h"
;


#line 174 "BatchPoissonSource.h"
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


#line 104 "Sim_SlottedCSMA.cc"
;


#line 109 "Sim_SlottedCSMA.cc"
;


#line 127 "Sim_SlottedCSMA.cc"
;

#include "compcxx_Sim_SlottedCSMA.h"
class compcxx_Channel_9;/*template <class T> */
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
public:compcxx_Channel_9* p_compcxx_parent;};

class compcxx_Channel_9;/*template <class T> */
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
public:compcxx_Channel_9* p_compcxx_parent;};

class compcxx_Channel_9;/*template <class T> */
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
public:compcxx_Channel_9* p_compcxx_parent;};


#line 23 "Channel.h"
class compcxx_Channel_9 : public compcxx_component, public TypeII
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

		compcxx_Channel_9 () { 
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

class compcxx_BatchPoissonSource_11;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_6 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_6() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_BatchPoissonSource_11* p_compcxx_parent;};

class compcxx_BatchPoissonSource_11;/*template <class T> */
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
public:compcxx_BatchPoissonSource_11* p_compcxx_parent;};

class compcxx_BatchPoissonSource_11;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_7 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_7() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_BatchPoissonSource_11* p_compcxx_parent;};

class compcxx_BatchPoissonSource_11;/*template <class T> */
#line 269 "./COST/cost.h"
class compcxx_Timer_8 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_8() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
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
public:compcxx_BatchPoissonSource_11* p_compcxx_parent;};


#line 9 "BatchPoissonSource.h"
class compcxx_BatchPoissonSource_11 : public compcxx_component, public TypeII
{
	public:
		
		class my_BatchPoissonSource_out_f_t:public compcxx_functor<BatchPoissonSource_out_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_BatchPoissonSource_out_f_t out_f;/*outport void out(Packet &packet)*/;	

		
		compcxx_Timer_5 /*<trigger_t> */inter_packet_timerBK;
		compcxx_Timer_6 /*<trigger_t> */inter_packet_timerBE;
		compcxx_Timer_7 /*<trigger_t> */inter_packet_timerVI;
		compcxx_Timer_8 /*<trigger_t> */inter_packet_timerVO;
		
		/*inport */inline void new_packetBK(trigger_t& tBK);
		/*inport */inline void new_packetBE(trigger_t& tBE);
		/*inport */inline void new_packetVI	(trigger_t& tVI);
		/*inport */inline void new_packetVO(trigger_t& tVO);

		compcxx_BatchPoissonSource_11 () { 
			inter_packet_timerBK.p_compcxx_parent=this /*connect inter_packet_timerBK.to_component,*/;
			inter_packet_timerBE.p_compcxx_parent=this /*connect inter_packet_timerBE.to_component,*/;
			inter_packet_timerVI.p_compcxx_parent=this /*connect inter_packet_timerVI.to_component,*/;
			inter_packet_timerVO.p_compcxx_parent=this /*connect inter_packet_timerVO.to_component,*/; }

	public:
		int L;
		long int seqBK;
		long int seqBE;
		long int seqVI;
		long int seqVO;
		int MaxBatch;	
		double packet_rateBK;
		double packet_rateBE;
		double packet_rateVI;
		double packet_rateVO;
	
	public:
		void Setup();
		void Start();
		void Stop();
			
};


#line 26 "STA.h"
class compcxx_STA_10 : public compcxx_component, public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
    	
        int node_id;
        int K; 
        int system_stickiness; 
        std::array<int, AC> stationStickiness; 
        int hysteresis;
        int fairShare;
        
        
        int cut;
        int EDCA;
        
        
        int maxAggregation;
        
        
        double incommingPackets;
        std::array<double, AC> blockedPackets;
        std::array<double, AC> queuesSizes;

    private:
    	

        std::array<double, AC> backoffCounters;
        std::array<int, AC> backoffStages;
        
        

        int BE; 
        int BK; 
        int VI; 
        int VO; 
        
        std::array<int,AC> backlogged; 

		Packet packet;
        FIFO <Packet> MACQueueBK;
        FIFO <Packet> MACQueueBE;
        FIFO <Packet> MACQueueVI;
        FIFO <Packet> MACQueueVO;

    public:
        
        /*inport */void inline in_slot(SLOT_notification &slot);
        /*inport */void inline in_packet(Packet &packet);
        class my_STA_out_packet_f_t:public compcxx_functor<STA_out_packet_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_STA_out_packet_f_t out_packet_f;/*outport void out_packet(Packet &packet)*/;
};


#line 19 "Sim_SlottedCSMA.cc"
class compcxx_SlottedCSMA_12 : public compcxx_component, public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int maxAggregation, int simSeed);
		void Stop();
		void Start();		

	public:
		compcxx_Channel_9 channel;

		compcxx_array<compcxx_STA_10  >stas;
		compcxx_array<compcxx_BatchPoissonSource_11  >sources;

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
void compcxx_Channel_9 :: Setup()
{

}
#line 77 "Channel.h"
void compcxx_Channel_9 :: Start()
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
void compcxx_Channel_9 :: Stop()
{
	printf("\n\n");
	printf("---- Channel ----\n");
	printf("Slot Status Probabilities (channel point of view): Empty = %e, Succesful = %e, Collision = %e \n",empty_slots/total_slots,succesful_slots/total_slots,collision_slots/total_slots);
	printf("Total packets sent to the Channel: %d", (int)succesful_slots);
	printf("\n\n");
	
	slotsInTime.close();
}
#line 120 "Channel.h"
void compcxx_Channel_9 :: Sampler(trigger_t &)
{
    
    
	







	
	cpSampler.Set(SimTime() + 1);
}


#line 136 "Channel.h"
void compcxx_Channel_9 :: NewSlot(trigger_t &)
{
	

	SLOT_notification slot;

	slot.status = number_of_transmissions_in_current_slot;

	number_of_transmissions_in_current_slot = 0;
	L_max = 0;

	for(int n = 0; n < Nodes; n++) out_slot[n](slot); 

	rx_time.Set(SimTime());	
}


#line 152 "Channel.h"
void compcxx_Channel_9 :: EndReceptionTime(trigger_t &)
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
void compcxx_Channel_9 :: in_packet(Packet &packet)
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
*/void compcxx_Timer_6/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_6/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_6/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_6/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->new_packetBE(m_event.data));
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
  (p_compcxx_parent->new_packetBK(m_event.data));
}




#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_7/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_7/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_7/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->new_packetVI(m_event.data));
}




#line 290 "./COST/cost.h"

#line 290 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::Set(trigger_t const & data, double time)
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
*/void compcxx_Timer_8/*<trigger_t >*/::Set(double time)
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
*/void compcxx_Timer_8/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 321 "./COST/cost.h"

#line 321 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_8/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->new_packetVO(m_event.data));
}




#line 21 "BatchPoissonSource.h"

#line 22 "BatchPoissonSource.h"

#line 23 "BatchPoissonSource.h"

#line 24 "BatchPoissonSource.h"

#line 51 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: Setup()
{

}
#line 56 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: Start()
{
	inter_packet_timerBK.Set(Exponential(1/packet_rateBK));
	seqBK = 0;
	
	inter_packet_timerBE.Set(Exponential(1/packet_rateBE));
	seqBE = 0;
		
	inter_packet_timerVI.Set(Exponential(1/packet_rateVI));
	seqVI = 0;
	
	inter_packet_timerVO.Set(Exponential(1/packet_rateVO));
	seqVO = 0;	
}
#line 71 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: Stop()
{

}
#line 76 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: new_packetBK(trigger_t &)
{
	Packet packetBK;

	packetBK.L = L;
	packetBK.accessCategory = 1;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetBK.seq = seqBK;
		packetBK.send_time = SimTime();

		(out_f(packetBK));

		seqBK++;
		

	}
	
	inter_packet_timerBK.Set(SimTime()+Exponential(RB/packet_rateBK));	

}
#line 101 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: new_packetBE(trigger_t &)
{
	Packet packetBE;
	packetBE.accessCategory = 0;

	packetBE.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetBE.seq = seqBE;
		packetBE.send_time = SimTime();

		(out_f(packetBE));

		seqBE++;
		

	}
	
	inter_packet_timerBE.Set(SimTime()+Exponential(RB/packet_rateBE));

}
#line 126 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: new_packetVI(trigger_t &)
{
	Packet packetVI;
	packetVI.accessCategory = 2;

	packetVI.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetVI.seq = seqVI;
		packetVI.send_time = SimTime();

		(out_f(packetVI));

		seqVI++;
		

	}
	
	inter_packet_timerVI.Set(SimTime()+Exponential(RB/packet_rateVI));	

}
#line 151 "BatchPoissonSource.h"
void compcxx_BatchPoissonSource_11 :: new_packetVO(trigger_t &)
{
	Packet packetVO;
	packetVO.accessCategory = 3;

	packetVO.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetVO.seq = seqVO;
		packetVO.send_time = SimTime();

		(out_f(packetVO));

		seqVO++;
		

	}
	
	inter_packet_timerVO.Set(SimTime()+Exponential(RB/packet_rateVO));	

}
#line 82 "STA.h"
void compcxx_STA_10 :: Setup()
{

	for(int i = 0; i < AC; i++)
	{
		computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
	}

}
#line 92 "STA.h"
void compcxx_STA_10 :: Start()
{
	selectMACProtocol(cut, node_id, EDCA, hysteresis, fairShare, maxAggregation);
	
	

	incommingPackets = 0;
	
	BE = 0;
    BK = 1;
    VI = 2;
    VO = 3;
	
}
#line 107 "STA.h"
void compcxx_STA_10 :: Stop()
{
    





    
    
}
#line 119 "STA.h"
void compcxx_STA_10 :: in_slot(SLOT_notification &slot)
{	
	switch(slot.status)
	{
		case 0:
			
			for(int i = 0; i < backlogged.size(); i++)
			{
				if(backlogged.at(i) == 0) 
				{
					computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
					
				}else 
				{
					if(backoffCounters.at(i) > 0)
					{
						
						backoffCounters.at(i)--;
					}
				}
			}
			break;
		case 1:
			for (int i = 0; i < backoffCounters.size(); i++)
			{
				if (backoffCounters.at(i) == 0) 
				{
					computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
				}
			}
	}
	
	
	


	
	int iterator = backoffCounters.size()-1;
	for (auto rIterator = backoffCounters.rbegin(); rIterator < backoffCounters.rend(); rIterator++)
	{
		if(*rIterator == 0)
		{
			cout << "Node #" << node_id << ", AC: " << iterator << " expired" <<endl;
			computeBackoff(backlogged.at(iterator), queuesSizes.at(iterator), iterator, stationStickiness.at(iterator), backoffStages.at(iterator), backoffCounters.at(iterator));
			cout << "---New backoff " << backoffCounters.at(iterator) << endl;
			break;
		}
		iterator--;
	}
}
#line 170 "STA.h"
void compcxx_STA_10 :: in_packet(Packet &packet)
{
    incommingPackets++;
    
    switch (packet.accessCategory){
    	case 0:
    		if(MACQueueBE.QueueSize() < K)
    		{
    			MACQueueBE.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueBE.QueueSize();
    		}else
    		{
    			blockedPackets.at(BE)++;
    		}
    		break;
    	case 1:
    		if(MACQueueBK.QueueSize() < K)
    		{
    			MACQueueBK.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueBK.QueueSize();
    		}else
    		{
    			blockedPackets.at(BK)++;
    		}
    		break;
    	case 2:
    		if(MACQueueVI.QueueSize() < K)
    		{
    			MACQueueVI.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueVI.QueueSize();
    		}else
    		{
    			blockedPackets.at(VI)++;
    		}
    		break;
    	case 3:
    		if(MACQueueVO.QueueSize() < K)
    		{
    			MACQueueVO.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueVI.QueueSize();
    		}else
    		{
    			blockedPackets.at(VO)++;
    		}
    		break;
    }
}


#line 44 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_12 :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift, float percentageEDCA, int maxAggregation, int simSeed)
{
	SimId = Sim_Id;
	Nodes = NumNodes;
	drift = slotDrift;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	
	channel.Nodes = NumNodes;
	channel.out_slot.SetSize(NumNodes);
	channel.error = channelErrors;

	
	
	cut = NumNodes * percentageEDCA;
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
		stas[n].hysteresis = hysteresis;
		stas[n].fairShare = fairShare;
		
		stas[n].cut = intCut;     
		stas[n].maxAggregation = maxAggregation;


		
		sources[n].L = PacketLength;
		sources[n].packet_rateBK = Bandwidth/PacketLength;
		sources[n].packet_rateBE = Bandwidth/PacketLength;
		sources[n].packet_rateVI = (Bandwidth/2)/PacketLength;
		sources[n].packet_rateVO = (Bandwidth/4)/PacketLength;
		sources[n].MaxBatch = Batch;
	}
	
	
	for(int n=0;n<NumNodes;n++)
	{
        channel.out_slot[n].Connect(stas[n],(compcxx_component::Channel_out_slot_f_t)&compcxx_STA_10::in_slot) /*connect channel.out_slot[n],stas[n].in_slot*/;
		stas[n].out_packet_f.Connect(channel,(compcxx_component::STA_out_packet_f_t)&compcxx_Channel_9::in_packet) /*connect stas[n].out_packet,channel.in_packet*/;
		sources[n].out_f.Connect(stas[n],(compcxx_component::BatchPoissonSource_out_f_t)&compcxx_STA_10::in_packet) /*connect sources[n].out,stas[n].in_packet*/;
	}


	Bandwidth_ = Bandwidth;
	PacketLength_ = PacketLength;
	Batch_ = Batch;
		

}
#line 106 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_12 :: Start()
{
	printf("--------------- CSMA/ECA ---------------\n");
}
#line 111 "Sim_SlottedCSMA.cc"
void compcxx_SlottedCSMA_12 :: Stop()
{
	
	
	
	
	
	cout << "--- Overall Statistics ---" << endl << endl;
	







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
	float percentageEDCA;
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
				cout << "(0)./XXXX (1)SimTime (2)NumNodes (3)PacketLength (4)Bandwidth (5)Batch (6)Stickiness (7)hysteresis (8)fairShare (9)channelErrors (10)slotDrift (11)percentageOfEDCA (12)maxAggregation (13)simSeed" << endl << endl;;
				cout << "0) ./XXX: Name of executable file" << endl;
				cout << "1) SimTime: simulation time in seconds" << endl;
				cout << "2) NumNodes: number of contenders" << endl;
				cout << "3) PacketLength: length of the packet in bytes" << endl;
				cout << "4) Bandwidth: number of bits per second generated by the source. With 802.11n and EDCA, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) Stickiness: activates CSMA/ECA. Nodes pick a deterministic backoff (0=off, 1=on)" << endl;
				cout << "7) Hysteresis: nodes do not reset their backoff stage after successful transmissions (0=off, 1=on)" << endl;
				cout << "8) FairShare: nodes at backoff stage k, attempt the transmission of 2^k packets (0=off, 1=on)" << endl;
				cout << "9) ChannelErrors: channel errors probability [0,1]" << endl;
				cout << "10) SlotDrift: probability of miscounting passing empty slots [0,1]" << endl;
				cout << "11) PercetageEDCA: percentage of nodes running EDCA. Used to simulate CSMA/ECA and CSMA/CA mixed scenarios [0,1]" << endl;
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
			cout << "./XXXX SimTime [10] NumNodes [10] PacketLength [1024] Bandwidth [65e6] Batch [1] Stickiness [0] hysteresis [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfEDCA [1] maxAggregation [0] simSeed [0]" << endl;
			MaxSimIter = 1;
			SimTime = 0.01;
			NumNodes = 1;
			PacketLength = 1024;
			Bandwidth = 65e6;
			Batch = 1; 
			Stickiness = 0; 
			hysteresis = 0; 
			fairShare = 0; 
			channelErrors = 0; 
			slotDrift = 0; 
			percentageEDCA = 1; 
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
		percentageEDCA = atof(argv[11]); 
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
	
	if(percentageEDCA > 0) cout << "####################### Mixed setup " << percentageEDCA*100 << "% EDCA #######################" << endl;
		
	compcxx_SlottedCSMA_12 test;

	
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, hysteresis, fairShare, channelErrors, slotDrift, percentageEDCA, maxAggregation, simSeed);
	
	test.Run();


	return(0);
};
