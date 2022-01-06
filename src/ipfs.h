#ifndef IPFS_H
#define IPFS_H

#include "ipfs/client.h"
#include <string>
#include <variant>

/**
 * \class IPFS
 * \brief IPFS Abstraction Layer to the C++ IPFS HTTP Client
 */
class IPFS
{
public:
  explicit IPFS(const std::string& host, int port, const std::string& timeout);
  std::size_t getNrPeers();
  std::string const getClientID();
  std::string const getClientPublicKey();
  std::string const getVersion();
  std::map<std::string, float> getBandwidthRates();
  std::map<std::string, std::variant<int, std::string>> getRepoStats();
  void fetch(const std::string& path, std::iostream* contents);
  std::string const add(const std::string& path, const std::string& content);
  void abort();

private:
  std::string host;    /* IPFS host name */
  int port;            /* IFPS port number */
  std::string timeout; /* IPFS timeout (eg. 6s) */
  ipfs::Client client; /* IPFS Client object */
};
#endif