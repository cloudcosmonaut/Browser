#ifndef IPFS_DAEMON_H
#define IPFS_DAEMON_H

#include <glibmm/spawn.h>
#include <string>

/**
 * \class IPFS Daemon Process
 * \brief Helper class to start/stop IPFS deamon, all static methods
 */
class IPFSDaemon
{
public:
  void spawn();
  void stop();
  int getPID() const;
  sigc::signal<void, int> exited;

protected:
  // Signals
  void child_watch_exit(Glib::Pid pid, int childStatus);

private:
  std::string workingDir = ""; // cwd
  Glib::Pid pid = 0;
  bool isRunning = false;
  sigc::connection childWatchHandler;

  static std::string locateIPFSBinary();
  static int getExistingPID();
  // bool shouldProcessTerminated();
};
#endif