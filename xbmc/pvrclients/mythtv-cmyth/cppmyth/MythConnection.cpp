#include "MythConnection.h"
#include "MythRecorder.h"
#include "MythFile.h"
#include "MythProgramInfo.h"
#include "MythEventHandler.h"
#include "client.h"

using namespace ADDON;


/*   
*								MythConnection
*/

MythConnection::MythConnection():
m_conn_t(new MythPointerThreadSafe<cmyth_conn_t>()),m_server(""),m_port(0)
{  
}


MythConnection::MythConnection(CStdString server,unsigned short port):
m_conn_t(new MythPointerThreadSafe<cmyth_conn_t>),m_server(server),m_port(port)
{
  char *cserver=strdup(server.c_str());
  cmyth_conn_t connection=CMYTH->ConnConnectCtrl(cserver,port,64*1024, 16*1024);
  free(cserver);
  *m_conn_t=(connection);
  
}

bool MythConnection::IsConnected()
{
  return *m_conn_t!=0;
}

MythRecorder MythConnection::GetFreeRecorder()
{
  Lock();
  MythRecorder retval = MythRecorder(CMYTH->ConnGetFreeRecorder(*m_conn_t),*this);
  Unlock();
  return retval;
}

MythRecorder MythConnection::GetRecorder(int n)
{
  Lock();
  MythRecorder retval = MythRecorder(CMYTH->ConnGetRecorderFromNum(*m_conn_t,n),*this);
  Unlock();
  return retval;
}

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetRecordedPrograms()
  {
    Lock();
    boost::unordered_map<CStdString, MythProgramInfo>  retval;
    cmyth_proglist_t proglist=CMYTH->ProglistGetAllRecorded(*m_conn_t);
    int len=CMYTH->ProglistGetCount(proglist);
    for(int i=0;i<len;i++)
    {
      cmyth_proginfo_t cmprog=CMYTH->ProglistGetItem(proglist,i);
      MythProgramInfo prog=CMYTH->ProginfoGetDetail(*m_conn_t,cmprog);
      CStdString path=prog.Path();
      retval.insert(std::pair<CStdString,MythProgramInfo>(path.c_str(),prog));
    }
    CMYTH->RefRelease(proglist);
    Unlock();
    return retval;
  }

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetPendingPrograms()
   {
    Lock();
    boost::unordered_map<CStdString, MythProgramInfo>  retval;
    cmyth_proglist_t proglist=CMYTH->ProglistGetAllPending(*m_conn_t);
    int len=CMYTH->ProglistGetCount(proglist);
    for(int i=0;i<len;i++)
    {
      MythProgramInfo prog=CMYTH->ProglistGetItem(proglist,i);
      CStdString filename;
      filename.Format("%i_%i",prog.ChannelID(),prog.StartTime());
      retval.insert(std::pair<CStdString,MythProgramInfo>(filename.c_str(),prog));
    }
    CMYTH->RefRelease(proglist);
    Unlock();
    return retval;
  }

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetScheduledPrograms()
   {
    Lock();
    boost::unordered_map<CStdString, MythProgramInfo>  retval;
    cmyth_proglist_t proglist=CMYTH->ProglistGetAllScheduled(*m_conn_t);
    int len=CMYTH->ProglistGetCount(proglist);
    for(int i=0;i<len;i++)
    {
      cmyth_proginfo_t cmprog=CMYTH->ProglistGetItem(proglist,i);
      MythProgramInfo prog=CMYTH->ProginfoGetDetail(*m_conn_t,cmprog);//Release cmprog????
      CStdString path=prog.Path();
      retval.insert(std::pair<CStdString,MythProgramInfo>(path.c_str(),prog));
    }
    CMYTH->RefRelease(proglist);
    Unlock();
    return retval;
  }

  bool  MythConnection::DeleteRecording(MythProgramInfo &recording)
  {
    Lock();
    bool retval = CMYTH->ProginfoDeleteRecording(*m_conn_t,*recording.m_proginfo_t)==0;
    Unlock();
    return retval;
  }


MythEventHandler MythConnection::CreateEventHandler()
{
  return MythEventHandler(m_server,m_port);
}

CStdString MythConnection::GetServer()
{
  return m_server;
}

int MythConnection::GetProtocolVersion()
{
  Lock();
  int retval = CMYTH->ConnGetProtocolVersion(*m_conn_t);
  Unlock();
  return retval;
}

bool MythConnection::GetDriveSpace(long long &total,long long &used)
{
  Lock();
  bool retval = CMYTH->ConnGetFreespace(*m_conn_t,&total,&used)==0;
  Unlock();
  return retval;
}

bool MythConnection::UpdateSchedules(int id)
{
  Lock();
  CStdString cmd;
  cmd.Format("RESCHEDULE_RECORDINGS %i",id);
  bool retval = CMYTH->ScheduleRecording(*m_conn_t,cmd.Buffer())>=0;
  Unlock();
  return retval;  
}

MythFile MythConnection::ConnectFile(MythProgramInfo &recording)
{
  Lock();
  MythFile retval = MythFile(CMYTH->ConnConnectFile(*recording.m_proginfo_t,*m_conn_t,64*1024, 16*1024),*this);
  Unlock();
  return retval;
}

bool MythConnection::IsNull()
{
  if(m_conn_t==NULL)
    return true;
  return *m_conn_t==NULL;
}

void MythConnection::Lock()
{
  if(g_bExtraDebug)
    XBMC->Log(LOG_DEBUG,"Lock %i",m_conn_t.get());
  m_conn_t->Lock();
  
}

void MythConnection::Unlock()
{
  if(g_bExtraDebug)
    XBMC->Log(LOG_DEBUG,"Unlock %i",m_conn_t.get());
  m_conn_t->Unlock();
  
}