/*---------------------------------------------------------------------------*
  Project:  How to set up new/delete operators for C++
  File:     cppsetupdemo.cpp

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/osdemo/src/cppsetupdemo.cpp $
    
    4     6/11/01 7:55p Tian
    Integrated SN changes
    
    3     3/16/01 12:04p Tian
    Comments fix
    
    2     9/29/00 5:27p Tian
    OS now supports pre-main new. 
    
    1     6/22/00 7:24p Tian
    Initial checkin.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  This demo shows how to override the standard new/delete operators with the 
  Dolphin OS memory allocator.  Note the following important caveats:

  - the standard Metrowerks new/delete operators are NOT supported,
  and will cause a DSI exception if you attempt to use them.

  - this demo works because our new/delete operators occur earlier in
  the link order than the Metrowerks Standard Libraries.  If the MSL
  libraries occur earlier in the link order than your definition of
  new/delete, then the MSL new/delete will be used throughout your
  code.

  This demo does show how to use static global constructors.  The trick is
  to initialize the heap in the first invocation of new().  The flag
  /IsHeapInitialized/ is used in this example to indicate whether or not
  the heap has been initialized.
 *---------------------------------------------------------------------------*/

#include <dolphin.h>
#include <stddef.h>
/*---------------------------------------------------------------------------*
  Override new and delete
 *---------------------------------------------------------------------------*/

#define HEAP_ID 0

static BOOL IsHeapInitialized = FALSE;


/*---------------------------------------------------------------------------*
  Name:         CPPInit

  Description:  Initializes the Dolphin OS memory allocator and ensures that
                new and delete will work properly.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CPPInit()
{
    void*    arenaLo;
    void*    arenaHi;

    if (IsHeapInitialized)
    {
        return;
    }

    
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();

    // Create a heap
    // OSInitAlloc should only ever be invoked once.
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1); // 1 heap
    OSSetArenaLo(arenaLo);

    // Ensure boundaries are 32B aligned
    arenaLo = (void*)OSRoundUp32B(arenaLo);
    arenaHi = (void*)OSRoundDown32B(arenaHi);

    // The boundaries given to OSCreateHeap should be 32B aligned
    OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
    // From here on out, OSAlloc and OSFree behave like malloc and free
    // respectively
    OSSetArenaLo(arenaLo=arenaHi);
    IsHeapInitialized = TRUE;

}

inline void* operator new       ( size_t blocksize )
{
    if (!IsHeapInitialized)
    {
        CPPInit();
    }
    return OSAllocFromHeap(HEAP_ID, blocksize);
}

inline void* operator new[]     ( size_t blocksize )
{
    if (!IsHeapInitialized)
    {
        CPPInit();
    }
    return OSAllocFromHeap(HEAP_ID, blocksize);
}

inline void operator delete     ( void* block )
{
    OSFreeToHeap(HEAP_ID, block);
}

inline void operator delete[]   ( void* block )
{
    OSFreeToHeap(HEAP_ID, block);
}



/*---------------------------------------------------------------------------*
  Trivial C++ code to test new and delete
 *---------------------------------------------------------------------------*/


class Foo
{
  private:
    u32 a;
    u32 b;
    char* array;

  public:
    Foo();
    ~Foo();

    void printA(void)
    {
        OSReport("a = %d\n", a);
    }
    void printB(void);
};


/*---------------------------------------------------------------------------*
  Name:         Foo::Foo

  Description:  Constructor for class Foo.  Simply initialize private 
                members.

  Arguments:    

  Returns:      
 *---------------------------------------------------------------------------*/
Foo::Foo()
{
    a = 42;
    b = 24;

    array = new char[32];
}


/*---------------------------------------------------------------------------*
  Name:         Foo::~Foo

  Description:  Destructor for class Foo.  Performs an OSReport so we know 
                it got called properly.

  Arguments:    

  Returns:      
 *---------------------------------------------------------------------------*/
Foo::~Foo()
{
    OSReport("Foo destructor\n");
}



/*---------------------------------------------------------------------------*
  Name:         Foo::printB

  Description:  function to print out b member

  Arguments:    

  Returns:      
 *---------------------------------------------------------------------------*/
void Foo::printB()
{
    OSReport("b = %d\n", b);
}
static Foo StaticFoo;


int main()
{
    Foo * foo;

    OSInit();
    CPPInit();

    foo = new Foo;
    
    foo->printA();
    foo->printB();
    StaticFoo.printA();

    delete foo;
	return 0;
}
