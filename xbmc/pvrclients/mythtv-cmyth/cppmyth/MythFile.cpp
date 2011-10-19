#include "MythFile.h"
#include "client.h"

using namespace ADDON;

/*
 *        MythFile
 */


 MythFile::MythFile()
   :m_file_t()
 {

 }

  MythFile::MythFile(cmyth_file_t myth_file)
    : m_file_t(new MythPointer<cmyth_file_t>())
 {
   *m_file_t=myth_file;
 }

  bool  MythFile::IsNull()
  {
    if(m_file_t==NULL)
      return true;
    return *m_file_t==NULL;
  }

  int MythFile::Read(void* buffer,long long length)
  {
   int bytesRead=CMYTH->FileRead(*m_file_t,static_cast<char*>(buffer),length);
   return bytesRead;
  }

  long long MythFile::Seek(long long offset, int whence)
  {
    return CMYTH->FileSeek(*m_file_t,offset,whence);
  }
  
  long long MythFile::Duration()
  {
    return CMYTH->FileLength(*m_file_t);
  }