#pragma once

#if DEBUG
#define ASSERT(cnd) assert((cnd))
#else
#define ASSERT(cnd) 
#endif

#if DEBUG
#    if VERBOSE
#        define CHECK(cnd) if(!(cnd)){std::cerr<<"("<<__LINE__<<"): Check failed: "<<(#cnd)<<std::endl; assert(0);} \
                           else {std::cout<<"("<<__LINE__<<"): Check passed: "<<(#cnd)<<std::endl;}
#    else
#        define CHECK(cnd) if(!(cnd)){std::cerr<<"("<<__LINE__<<"): Check failed: "<<(#cnd)<<std::endl; return false;}
#    endif
#else
#    define CHECK(cnd) 
#endif
