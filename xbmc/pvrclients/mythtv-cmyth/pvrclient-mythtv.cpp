#include "pvrclient-mythtv.h"
#include "client.h"


PVRClientMythTV::PVRClientMythTV()
  :m_con(),m_eventHandler(),m_db(),m_protocolVersion(""),m_connectionString("")
{
}


bool PVRClientMythTV::Connect()
{
  m_con=MythConnection(g_szHostname,g_iMythPort);
  if(!m_con.IsConnected())
    return false;
  m_eventHandler=m_con.CreateEventHandler();
  m_protocolVersion.Format("%i",m_con.GetProtocolVersion());
  m_connectionString.Format("%s:%i",g_szHostname,g_iMythPort);
  m_db=MythDatabase(g_szHostname,g_szMythDBname,g_szMythDBuser,g_szMythDBpassword);
  m_channels=m_db.ChannelList();
  return true;
}

const char* PVRClientMythTV::GetBackendName()
{
  return m_con.GetServer();
}


const char * PVRClientMythTV::GetBackendVersion()
{
  return m_protocolVersion;
}

const char * PVRClientMythTV::GetConnectionString()
{
  return m_connectionString;
}

bool PVRClientMythTV::GetDriveSpace(long long *iTotal, long long *iUsed)
{
  return m_con.GetDriveSpace(*iTotal,*iUsed);
}

PVR_ERROR PVRClientMythTV::GetEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
  std::vector<MythProgram> programs=m_db.GetGuide(iStart,iEnd);
  for(std::vector<MythProgram>::iterator it=programs.begin();it!=programs.end();it++)
  {
    if(it->chanid==channel.iUniqueId)
    {
      EPG_TAG tag;
      tag.endTime=it->endtime;
      tag.iChannelNumber=it->channum;
      tag.startTime=it->starttime;
      tag.strTitle=it->title;
      tag.strPlot=it->description;
      tag.iUniqueBroadcastId=atoi(it->seriesid);
      PVR->TransferEpgEntry(handle,&tag);
    }
   }
   return PVR_ERROR_NO_ERROR;
  }


int PVRClientMythTV::GetNumChannels()
{
  return m_channels.size();
}
