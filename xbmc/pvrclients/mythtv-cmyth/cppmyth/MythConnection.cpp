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
m_conn_t(),m_server(""),m_port(0)
{  
}


MythConnection::MythConnection(CStdString server,unsigned short port):
m_conn_t(new MythPointer<cmyth_conn_t>),m_server(server),m_port(port)
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
  return MythRecorder(CMYTH->ConnGetFreeRecorder(*m_conn_t));
}

MythRecorder MythConnection::GetRecorder(int n)
{
  return MythRecorder(CMYTH->ConnGetRecorderFromNum(*m_conn_t,n));
}

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetRecordedPrograms()
  {
    boost::unordered_map<CStdString, MythProgramInfo>  retval;
    cmyth_proglist_t proglist=CMYTH->ProglistGetAllRecorded(*m_conn_t);
    int len=CMYTH->ProglistGetCount(proglist);
    for(int i=0;i<len;i++)
    {
      cmyth_proginfo_t cmprog=CMYTH->ProglistGetItem(proglist,i);
      MythProgramInfo prog=CMYTH->ProginfoGetDetail(*m_conn_t,cmprog);//Release cmprog????
      CStdString path=prog.Path();
      retval.insert(std::pair<CStdString,MythProgramInfo>(path.c_str(),prog));
    }
    CMYTH->RefRelease(proglist);
    return retval;
  }

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetPendingPrograms()
   {
    boost::unordered_map<CStdString, MythProgramInfo>  retval;
    cmyth_proglist_t proglist=CMYTH->ProglistGetAllPending(*m_conn_t);
    int len=CMYTH->ProglistGetCount(proglist);
    for(int i=0;i<len;i++)
    {
      cmyth_proginfo_t cmprog=CMYTH->ProglistGetItem(proglist,i);
      MythProgramInfo prog=CMYTH->ProginfoGetDetail(*m_conn_t,cmprog);//Release cmprog????
      CStdString path=prog.Path();
      retval.insert(std::pair<CStdString,MythProgramInfo>(path.c_str(),prog));
    }
    CMYTH->RefRelease(proglist);
    return retval;
  }

  boost::unordered_map<CStdString, MythProgramInfo>  MythConnection::GetScheduledPrograms()
   {
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
    return retval;
  }

  bool  MythConnection::DeleteRecording(MythProgramInfo &recording)
  {
    return CMYTH->ProginfoDeleteRecording(*m_conn_t,*recording.m_proginfo_t)==0;
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
  return CMYTH->ConnGetProtocolVersion(*m_conn_t);
}

bool MythConnection::GetDriveSpace(long long &total,long long &used)
{
  return CMYTH->ConnGetFreespace(*m_conn_t,&total,&used)==0;
}

bool MythConnection::UpdateSchedules(int id)
{
  CStdString cmd;
  cmd.Format("RESCHEDULE_RECORDINGS %i",id);
  return CMYTH->ScheduleRecording(*m_conn_t,cmd.Buffer())>=0;
  
}

MythFile MythConnection::ConnectFile(MythProgramInfo &recording)
{
  return CMYTH->ConnConnectFile(*recording.m_proginfo_t,*m_conn_t,64*1024, 16*1024);
}

bool MythConnection::IsNull()
{
  if(m_conn_t==NULL)
    return true;
  return *m_conn_t==NULL;
}