//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2007. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_POSIX_SEMAPHORE_WRAPPER_HPP
#define BOOST_INTERPROCESS_POSIX_SEMAPHORE_WRAPPER_HPP

#include <boost/interprocess/detail/posix_time_types_wrk.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <string>
#include <semaphore.h>

#ifdef BOOST_INTERPROCESS_POSIX_TIMEOUTS
#include <boost/interprocess/sync/posix/ptime_to_timespec.hpp>
#else
#include <boost/interprocess/detail/os_thread_functions.hpp>
#endif

namespace boost {
namespace interprocess {
namespace detail {

inline bool semaphore_open
   (sem_t *&handle, detail::create_enum_t type, const char *origname, mode_t mode,
    unsigned int count)
{
   bool slash_added = origname[0] != '/';
   //First add preceding "/"
   std::string name;

   if(slash_added){
      name = '/';
   }
   name += origname;

   //Create new mapping
   int oflag = 0;
   if(mode == read_only){
      oflag |= O_RDONLY;
   }
   else if(mode == read_write){
      oflag |= O_RDWR;
   }
   else{
      error_info err(mode_error);
      throw interprocess_exception(err);
   }

   switch(type){
      case detail::DoOpen:
         //No addition
      break;
      case detail::DoCreate:
         oflag |= (O_CREAT | O_EXCL);
      break;
      case detail::DoOpenOrCreate:
         oflag |= O_CREAT;
      break;
      default:
         {
            error_info err = other_error;
            throw interprocess_exception(err);
         }
   }

   //Open file using POSIX API
   if(oflag & O_CREAT)
      handle = sem_open(name.c_str(), oflag, S_IRWXO | S_IRWXG | S_IRWXU, count);
   else
      handle = sem_open(name.c_str(), oflag);

   //Check for error
   if(handle == SEM_FAILED){
      throw interprocess_exception(error_info(errno));
   }

   if(slash_added){
      name.erase(name.begin());
   }

   return true;
}

inline void semaphore_close(sem_t *handle)
{
   int ret = sem_close(handle);
   if(ret != 0){  
      assert(0);
   }
}

inline bool semaphore_unlink(const char *name)
{
   try{
      std::string str;
      //First add preceding "/"
      if(name[0] != '/'){
         str = '/';
      }
      str += name;
      return 0 != sem_unlink(str.c_str());
   }
   catch(...){
      return false;
   }
}

inline void semaphore_init(sem_t *handle, int initialCount)
{
   int ret = sem_init(handle, 1, initialCount);
   if(ret != 0){
      throw interprocess_exception(system_error_code());
   }
}

inline void semaphore_destroy(sem_t *handle)
{
   int ret = sem_destroy(handle);
   if(ret != 0){  
      assert(0);
   }
}

inline void semaphore_post(sem_t *handle)
{
   int ret = sem_post(handle);
   if(ret != 0){
      throw interprocess_exception(system_error_code());
   }
}

inline void semaphore_wait(sem_t *handle)
{
   int ret = sem_wait(handle);
   if(ret != 0){
      throw interprocess_exception(system_error_code());
   }
}

inline bool semaphore_try_wait(sem_t *handle)
{
   int res = sem_trywait(handle);
   if(res == 0)
      return true;
   if(system_error_code() == EAGAIN){
      return false;
   }
   throw interprocess_exception(system_error_code());
   return false;
}

inline bool semaphore_timed_wait(sem_t *handle, const boost::posix_time::ptime &abs_time)
{
   #ifdef BOOST_INTERPROCESS_POSIX_TIMEOUTS
   timespec tspec = detail::ptime_to_timespec(abs_time);
   for (;;){
      int res = sem_timedwait(handle, &tspec);
      if(res == 0)
         return true;
      if (res > 0){
         //buggy glibc, copy the returned error code to errno
         errno = res;
      }
      if(system_error_code() == ETIMEDOUT){
         return false;
      }
      throw interprocess_exception(system_error_code());
   }
   return false;
   #else //#ifdef BOOST_INTERPROCESS_POSIX_TIMEOUTS
   boost::posix_time::ptime now;
   while((now = microsec_clock::universal_time()) < abs_time){
      if(semaphore_try_wait(handle))
         return true;
      thread_yield();
   }
   return false;
   #endif   //#ifdef BOOST_INTERPROCESS_POSIX_TIMEOUTS
}
/*
inline int semaphore_get_count(sem_t *handle)
{
   int count;
   sem_getvalue(handle, &ret);
   return count;
}
*/

class named_semaphore_wrapper
{
   named_semaphore_wrapper();
   named_semaphore_wrapper(const named_semaphore_wrapper&);
   named_semaphore_wrapper &operator= (const named_semaphore_wrapper &);

   public:
   named_semaphore_wrapper
      (detail::create_enum_t type, const char *name, mode_t mode, unsigned int count)
   {  semaphore_open(mp_sem, type, name, mode, count);   }

   ~named_semaphore_wrapper()
   {  semaphore_close(mp_sem);  }

   void post()
   {  semaphore_post(mp_sem); }

   void wait()
   {  semaphore_wait(mp_sem); }

   bool try_wait()
   {  return semaphore_try_wait(mp_sem); }

   bool timed_wait(const boost::posix_time::ptime &abs_time)
   {  return semaphore_timed_wait(mp_sem, abs_time); }

   static bool remove(const char *name)
   {  return semaphore_unlink(name);   }

   private:
   sem_t      *mp_sem;
};

class semaphore_wrapper
{
   semaphore_wrapper();
   semaphore_wrapper(const semaphore_wrapper&);
   semaphore_wrapper &operator= (const semaphore_wrapper &);

   public:
   semaphore_wrapper(int initialCount)
   {  semaphore_init(&m_sem, initialCount);  }

   ~semaphore_wrapper()
   {  semaphore_destroy(&m_sem);  }

   void post()
   {  semaphore_post(&m_sem); }

   void wait()
   {  semaphore_wait(&m_sem); }

   bool try_wait()
   {  return semaphore_try_wait(&m_sem); }

   bool timed_wait(const boost::posix_time::ptime &abs_time)
   {  return semaphore_timed_wait(&m_sem, abs_time); }

   private:
   sem_t       m_sem;
};

}  //namespace detail {
}  //namespace interprocess {
}  //namespace boost {

#endif   //#ifndef BOOST_INTERPROCESS_POSIX_SEMAPHORE_WRAPPER_HPP
