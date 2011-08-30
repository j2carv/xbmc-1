#include "cppmyth.h"
#include "xbmc_pvr_dll.h"

class PVRClientMythTV
{
public:
  PVRClientMythTV();
  ~PVRClientMythTV();

  /* Server handling */
  bool Connect();
  const char * GetBackendName();
  const char * GetBackendVersion();
  const char * GetConnectionString();
  bool GetDriveSpace(long long *iTotal, long long *iUsed);
  PVR_ERROR GetEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd);
  int GetNumChannels();
private:
  MythConnection m_con;
  MythEventHandler m_eventHandler;
  MythDatabase m_db;
  CStdString m_protocolVersion;
  CStdString m_connectionString;
  std::vector<MythChannel> m_channels;
};