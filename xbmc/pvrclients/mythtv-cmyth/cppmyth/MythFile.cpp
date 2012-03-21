
#include "MythFile.h"
#include "client.h"


using namespace ADDON;

/*
 *        MythFile
 */


 MythFile::MythFile()
   :m_file_t(new MythPointer<cmyth_file_t>()),m_conn(MythConnection())
 {
   updatedLength=0;
 }

  MythFile::MythFile(cmyth_file_t myth_file,MythConnection conn)
    : m_file_t(new MythPointer<cmyth_file_t>()),m_conn(conn)
 {
   *m_file_t=myth_file;
   updatedLength=0;
 }
 
void MythFile::updateDuration ( long long length )
 {
   updatedLength=length;
   XBMC->Log(LOG_DEBUG,"EVENT: %s, --UPDATING RECORDING LENGTH-- EVENT length: %u",
             __FUNCTION__,length);
  }
  
  bool  MythFile::IsNull()
  {
    if(m_file_t==NULL)
      return true;
    return *m_file_t==NULL;
  }

  int MythFile::Read(void* buffer,long long length)
  {
   m_conn.Lock();
   int bytesRead=CMYTH->FileRead(*m_file_t,static_cast<char*>(buffer),length);
   m_conn.Unlock();
   return bytesRead;
  }

  long long MythFile::Seek(long long offset, int whence)
  {
    m_conn.Lock();
    long long retval = CMYTH->FileSeek(*m_file_t,offset,whence);
    m_conn.Unlock();
    return retval;
  }
  
  long long MythFile::Duration()
  {
    m_conn.Lock();
    long long retval = CMYTH->FileLength(*m_file_t);
    if (updatedLength > retval) {
      XBMC->Log(LOG_DEBUG,"EVENT: %s -- SENDING UPDATED LENGTH -- ",__FUNCTION__);
      retval = updatedLength;
    }
    m_conn.Unlock();
    return retval;
  }