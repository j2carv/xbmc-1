#pragma once

#include "libcmyth.h"
#include "utils/StdString.h"
#include <boost/shared_ptr.hpp>
#include "MythPointer.h"


class MythProgramInfo 
{
  friend class MythConnection;
public:
  MythProgramInfo();
  MythProgramInfo(cmyth_proginfo_t cmyth_proginfo);
  CStdString ProgramID();
  CStdString Title();
  CStdString Path();
  CStdString Description();
  CStdString ChannelName();
  int ChannelID();
  time_t RecStart();
  int Duration();
  CStdString Category();
  CStdString RecordingGroup();
  long long uid();
  bool IsNull();
private:
  boost::shared_ptr< MythPointer< cmyth_proginfo_t > > m_proginfo_t;
};