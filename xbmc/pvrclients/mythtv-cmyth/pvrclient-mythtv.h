#include "cppmyth.h"
#include "xbmc_pvr_types.h"
#include <map>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

class PVRClientMythTV
{
public:
  PVRClientMythTV();

  /* Server handling */
  bool Connect();
  const char * GetBackendName();
  const char * GetBackendVersion();
  const char * GetConnectionString();
  bool GetDriveSpace(long long *iTotal, long long *iUsed);
  PVR_ERROR GetEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd);
  int GetNumChannels();
  PVR_ERROR GetChannels(PVR_HANDLE handle, bool bRadio);
  int GetRecordingsAmount(void);
  PVR_ERROR GetRecordings(PVR_HANDLE handle);
  PVR_ERROR DeleteRecording(const PVR_RECORDING &recording);
  int GetTimersAmount();
  PVR_ERROR GetTimers(PVR_HANDLE handle);
  PVR_ERROR AddTimer(const PVR_TIMER &timer);
  PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete);
  PVR_ERROR UpdateTimer(const PVR_TIMER &timer);
  bool OpenLiveStream(const PVR_CHANNEL &channel);
  void CloseLiveStream();
  int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize);
  int GetCurrentClientChannel();
  bool SwitchChannel(const PVR_CHANNEL &channelinfo);
  //SET_SIGNAL_MONITORING_RATE ->signal
  long long SeekLiveStream(long long iPosition, int iWhence);
  long long LengthLiveStream();
  PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus);

  bool OpenRecordedStream(const PVR_RECORDING &recinfo);
  void CloseRecordedStream();
  int ReadRecordedStream(unsigned char *pBuffer, unsigned int iBufferSize);
  long long SeekRecordedStream(long long iPosition, int iWhence);
  long long LengthRecordedStream();

  int GetChannelGroupsAmount();
  PVR_ERROR GetChannelGroups(PVR_HANDLE handle, bool bRadio);
  PVR_ERROR GetChannelGroupMembers(PVR_HANDLE handle, const PVR_CHANNEL_GROUP &group);

private:
  struct mythcat{};
  struct pvrcat{};
  typedef boost::bimap<
    boost::bimaps::unordered_set_of< boost::bimaps::tagged< CStdString , mythcat >,boost::hash< CStdString > >,
    boost::bimaps::tagged< int , pvrcat >
    > catbimap;
  int Genre(CStdString g);
  CStdString Genre(int g);

  catbimap m_categoryMap;
  MythConnection m_con;
  MythEventHandler m_eventHandler;
  MythDatabase m_db;
  MythRecorder m_rec;
  MythFile m_file;
  CStdString m_protocolVersion;
  CStdString m_connectionString;
  time_t m_EPGstart;
  time_t m_EPGend;
  std::vector< MythProgram > m_EPG;
  std::map< int , MythChannel > m_channels;
  boost::unordered_map< CStdString, MythProgramInfo > m_recordings;
  boost::unordered_map< CStdString, std::vector< int > > m_channelGroups;
};