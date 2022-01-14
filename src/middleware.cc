#include "middleware.h"

#include "file.h"
#include "mainwindow.h"
#include "md-parser.h"
#include <cmark-gfm.h>
#include <glibmm/main.h>

/**
 * Middleware constructor
 */
Middleware::Middleware(MainWindow& mainWindow, const std::string& timeout)
    : mainWindow(mainWindow),
      // Threading:
      requestThread_(nullptr),
      statusThread_(nullptr),
      is_request_thread_done_(false),
      keep_request_thread_running_(true),
      is_status_thread_done_(false),
      // IPFS:
      ipfsHost_("localhost"),
      ipfsPort_(5001),
      ipfsTimeout_(timeout),
      ipfs_fetch_(ipfsHost_, ipfsPort_, ipfsTimeout_),
      ipfs_status_(ipfsHost_, ipfsPort_, ipfsTimeout_),
      ipfsNumberOfPeers_(0),
      ipfsRepoSize_(0),
      ipfsIncomingRate_("0.0"),
      ipfsOutcomingRate_("0.0"),
      // Request & Response:
      waitPageVisible_(false)
{
  doIPFSStatusUpdate();

  // Hook up signals to Main Window methods
  requestStarted_.connect(sigc::mem_fun(mainWindow, &MainWindow::startedRequest));
  requestFinished_.connect(sigc::mem_fun(mainWindow, &MainWindow::finishedRequest));

  // Create a timer, triggers every 4 seconds
  statusTimerHandler_ = Glib::signal_timeout().connect_seconds(sigc::mem_fun(this, &Middleware::doIPFSStatusUpdate), 4);
}

/**
 * Destructor
 */
Middleware::~Middleware()
{
  statusTimerHandler_.disconnect();
  abortRequest();
  abortStatus();
}

/**
 * Fetch document from disk or IPFS, using threading
 * \param path File path that needs to be opened (either from disk or IPFS network)
 * \param isSetAddressBar If true change update the address bar with the file path
 * \param isHistoryRequest Set to true if this is an history request call: back/forward
 * \param isDisableEditor If true the editor will be disabled if needed
 * \param isParseContent If true the content received will be parsed and displayed as markdown syntax (default: true),
 * set to false if you want to editor the content
 */
void Middleware::doRequest(
    const std::string& path, bool isSetAddressBar, bool isHistoryRequest, bool isDisableEditor, bool isParseContent)
{
  // Stop any on-going request first, if applicable
  abortRequest();

  if (requestThread_ == nullptr)
  {
    // Update main window widgets
    mainWindow.preRequest(path, isSetAddressBar, isHistoryRequest, isDisableEditor);
    // Start thread
    requestThread_ = new std::thread(&Middleware::processRequest, this, path, isParseContent);
  }
  else
  {
    // TODO: throw a runtime error, so we can abort everything in the mainwindow
    std::cerr << "ERROR: Could not start request thread. Something went wrong." << std::endl;
  }
}

/**
 * \brief Add current content to IPFS
 * \param path file path in IPFS
 * \return Content identifier (CID)
 */
std::string Middleware::doAdd(const std::string& path)
{
  // TODO: We should run this within a seperate thread, to avoid blocking the main thread.
  // See also the other status calls we are making, but maybe we should use ipfs_fetch_ anyway.
  return ipfs_status_.add(path, getContent());
}

/**
 * \brief Write file to disk
 * \param path file path to disk
 */
void Middleware::doWrite(const std::string& path)
{
  File::write(path, getContent());
}

/**
 * \brief Get filename from path
 * \param path file path
 * \return filename
 */
std::string Middleware::getFilename(const std::string& path)
{
  return File::getFilename(path);
}

/**
 * \brief Set current plain-text content (not parsed)
 */
void Middleware::setContent(const std::string& content)
{
  currentContent_ = content;
}

/**
 * \brief Get current plain content (not parsed)
 * \return content as string
 */
std::string const Middleware::getContent()
{
  return currentContent_;
}

/**
 * \brief Current content parser middleware
 * \return AST structure (of type cmark_node)
 */
cmark_node* Middleware::parseContent()
{
  return Parser::parseContent(currentContent_);
}

/**
 * \brief Reset state
 */
void Middleware::resetContentAndPath()
{
  currentContent_ = "";
  requestPath_ = "";
  finalRequestPath_ = "";
}

/**
 * \brief Get IPFS number of peers
 * \return number of peers (size_t)
 */
std::size_t Middleware::getIPFSNumberOfPeers()
{
  return ipfsNumberOfPeers_;
}

/**
 * \brief Get IPFS repository size
 * \return repo size (int)
 */
int Middleware::getIPFSRepoSize()
{
  return ipfsRepoSize_;
}

/**
 * \brief Get IPFS repository path
 * \return repo path (string)
 */
std::string const Middleware::getIPFSRepoPath()
{
  return ipfsRepoPath_;
}

/**
 * \brief Get IPFS Incoming rate
 * \return incoming rate (string)
 */
std::string const Middleware::getIPFSIncomingRate()
{
  return ipfsIncomingRate_;
}

/**
 * \brief Get IPFS Outgoing rate
 * \return outgoing rate (string)
 */
std::string const Middleware::getIPFSOutcomingRate()
{
  return ipfsOutcomingRate_;
}

/**
 * \brief Get IPFS version
 * \return version (string)
 */
std::string const Middleware::getIPFSVersion()
{
  return ipfsVersion_;
}

/**
 * \brief Get IPFS Client ID
 * \return client ID (string)
 */
std::string const Middleware::getIPFSClientId()
{
  return ipfsClientID_;
}

/**
 * \brief Get IPFS Client Public key
 * \return public key (string)
 */
std::string const Middleware::getIPFSClientPublicKey()
{
  return ipfsClientPublicKey_;
}

/************************************************
 * Private methods
 ************************************************/

/**
 * \brief Get the file from disk or IPFS network, from the provided path,
 * parse the content, and display the document
 * \param path File path that needs to be fetched (from disk or IPFS network)
 * \param isParseContent Set to true if you want to parse and display the content as markdown syntax (from disk or IPFS
 * network), set to false if you want to edit the content
 */
void Middleware::processRequest(const std::string& path, bool isParseContent)
{
  requestStarted_.emit(); // Emit started for Main Window
  // Reset private variables
  currentContent_ = "";
  waitPageVisible_ = false;

  // Do not update the requestPath_ when path is empty,
  // this is used for refreshing the page
  if (!path.empty())
  {
    requestPath_ = path;
  }

  if (requestPath_.empty())
  {
    std::cerr << "Info: Empty request path." << std::endl;
  }
  // Handle homepage
  else if (requestPath_.compare("about:home") == 0)
  {
    mainWindow.showStartpage();
  }
  // Handle disk or IPFS file paths
  else
  {
    // Check if CID
    if (requestPath_.rfind("ipfs://", 0) == 0)
    {
      finalRequestPath_ = requestPath_;
      finalRequestPath_.erase(0, 7);
      fetchFromIPFS(isParseContent);
    }
    else if ((requestPath_.length() == 46) && (requestPath_.rfind("Qm", 0) == 0))
    {
      // CIDv0
      finalRequestPath_ = requestPath_;
      fetchFromIPFS(isParseContent);
    }
    else if (requestPath_.rfind("file://", 0) == 0)
    {
      finalRequestPath_ = requestPath_;
      finalRequestPath_.erase(0, 7);
      openFromDisk(isParseContent);
    }
    else
    {
      // IPFS as fallback / CIDv1
      finalRequestPath_ = requestPath_;
      fetchFromIPFS(isParseContent);
    }
  }

  requestFinished_.emit();        // Emit finished for Main Window
  is_request_thread_done_ = true; // mark thread as done
}

/**
 * \brief Helper method for processRequest(), display markdown file from IPFS network.
 * Runs in a seperate thread.
 * \param isParseContent Set to true if you want to parse and display the content as markdown syntax (from disk or IPFS
 * network), set to false if you want to edit the content
 */
void Middleware::fetchFromIPFS(bool isParseContent)
{
  try
  {
    // TODO: Check file contents (first bytes?), to guess the file type.
    // only proceed further file is UTF-8 unicode text (avoid images etc.).
    std::stringstream contents;
    ipfs_fetch_.fetch(finalRequestPath_, &contents);
    // Set content
    setContent(contents.str());
    if (isParseContent)
    {
      // TODO: Maybe we want to abort the parser when keep_request_thread_running_ = false,
      // depending time the parser is taking?
      cmark_node* doc = parseContent();
      mainWindow.setDocument(doc);
      cmark_node_free(doc);
    }
    else
    {
      // Directly display the plain markdown content
      mainWindow.setText(getContent());
    }
  }
  catch (const std::runtime_error& error)
  {
    std::string errorMessage = std::string(error.what());
    // Ignore error reporting when the request was aborted
    if (errorMessage != "Request was aborted")
    {
      std::cerr << "ERROR: IPFS request failed, with message: " << errorMessage << std::endl;
      if (errorMessage.starts_with("HTTP request failed with status code"))
      {
        std::string message;
        // Remove text until ':\n'
        errorMessage.erase(0, errorMessage.find(':') + 2);
        if (!errorMessage.empty() && errorMessage != "")
        {
          try
          {
            auto content = nlohmann::json::parse(errorMessage);
            message = "Message: " + content.value("Message", "");
            if (message.starts_with("context deadline exceeded"))
            {
              message += ". Time-out is set to: " + ipfsTimeout_;
            }
            message += ".\n\n";
          }
          catch (const nlohmann::json::parse_error& parseError)
          {
            std::cerr << "ERROR: Could not parse at byte: " << parseError.byte << std::endl;
          }
        }
        mainWindow.setMessage("🎂 We're having trouble finding this site.",
                              message + "You could try to reload the page or try increase the time-out (see --help).");
      }
      else if (errorMessage.starts_with("Couldn't connect to server: Failed to connect to localhost"))
      {
        mainWindow.setMessage("⌛ Please wait...",
                              "IPFS daemon is still spinnng-up, page will automatically refresh...");
        waitPageVisible_ = true; // Please wait page is shown (auto-refresh when network is up)
      }
      else
      {
        mainWindow.setMessage("❌ Something went wrong", "Error message: " + std::string(error.what()));
      }
    }
  }
}

/**
 * \brief Helper method for processRequest(), display markdown file from disk.
 * Runs in a seperate thread.
 * \param isParseContent Set to true if you want to parse and display the content as markdown syntax (from disk or IPFS
 * network), set to false if you want to edit the content
 */
void Middleware::openFromDisk(bool isParseContent)
{
  try
  {
    // Abort file read if keep_request_thread_running_ = false and throw runtime error, to stop futher execution
    // eg. when you are reading a very big file from disk.
    setContent(File::read(finalRequestPath_));
    // If the thread stops, don't brother to parse the file/update the GTK window
    if (keep_request_thread_running_)
    {
      if (isParseContent)
      {
        cmark_node* doc = parseContent();
        mainWindow.setDocument(doc);
        cmark_node_free(doc);
      }
      else
      {
        // directly set the plain content
        mainWindow.setText(getContent());
      }
    }
  }
  catch (const std::ios_base::failure& error)
  {
    std::cerr << "ERROR: Could not read file: " << finalRequestPath_ << ". Message: " << error.what()
              << ".\nError code: " << error.code() << std::endl;
    mainWindow.setMessage("🎂 Could not read file", "Message: " + std::string(error.what()));
  }
  catch (const std::runtime_error& error)
  {
    std::cerr << "ERROR: File request failed, file: " << finalRequestPath_ << ". Message: " << error.what()
              << std::endl;
    mainWindow.setMessage("🎂 File not found", "Message: " + std::string(error.what()));
  }
}

/**
 * \brief Timeout slot: Update the IPFS connection status every x seconds.
 * Process requests inside a seperate thread, to avoid blocking the GUI thread.
 * \return always true, when running as a GTK timeout handler
 */
bool Middleware::doIPFSStatusUpdate()
{
  // Stop any on-going status calls first, if applicable
  abortStatus();

  if (statusThread_ == nullptr)
  {
    statusThread_ = new std::thread(&Middleware::processIPFSStatus, this);
  }
  // Keep going (never disconnect the timer)
  return true;
}

/**
 * Process the IPFS status calls.
 * Runs inside a thread.
 */
void Middleware::processIPFSStatus()
{
  std::lock_guard<std::mutex> guard(status_mutex_);
  try
  {
    ipfsNumberOfPeers_ = ipfs_status_.getNrPeers();
    if (ipfsNumberOfPeers_ > 0)
    {
      // Auto-refresh page if needed (when 'Please wait' page was shown)
      if (waitPageVisible_)
        Glib::signal_idle().connect_once(sigc::mem_fun(mainWindow, &MainWindow::refreshRequest));

      std::map<std::string, std::variant<int, std::string>> repoStats = ipfs_status_.getRepoStats();
      ipfsRepoSize_ = std::get<int>(repoStats.at("repo-size"));
      ipfsRepoPath_ = std::get<std::string>(repoStats.at("path"));

      std::map<std::string, float> rates = ipfs_status_.getBandwidthRates();
      char buf[32];
      ipfsIncomingRate_ = std::string(buf, std::snprintf(buf, sizeof buf, "%.1f", rates.at("in") / 1000.0));
      ipfsOutcomingRate_ = std::string(buf, std::snprintf(buf, sizeof buf, "%.1f", rates.at("out") / 1000.0));
    }
    else
    {
      ipfsRepoSize_ = 0;
      ipfsRepoPath_ = "";
      ipfsIncomingRate_ = "0.0";
      ipfsOutcomingRate_ = "0.0";
    }

    if (ipfsClientID_.empty())
      ipfsClientID_ = ipfs_status_.getClientID();
    if (ipfsClientPublicKey_.empty())
      ipfsClientPublicKey_ = ipfs_status_.getClientPublicKey();
    if (ipfsVersion_.empty())
      ipfsVersion_ = ipfs_status_.getVersion();

    // Trigger update of all status fields, in a thread-safe manner
    Glib::signal_idle().connect_once(sigc::mem_fun(mainWindow, &MainWindow::updateStatusPopoverAndIcon));
  }
  catch (const std::runtime_error& error)
  {
    std::string errorMessage = std::string(error.what());
    if (errorMessage != "Request was aborted")
    {
      // Assume no connection or connection lost; display disconnected
      ipfsNumberOfPeers_ = 0;
      ipfsRepoSize_ = 0;
      ipfsRepoPath_ = "";
      ipfsIncomingRate_ = "0.0";
      ipfsOutcomingRate_ = "0.0";
      Glib::signal_idle().connect_once(sigc::mem_fun(mainWindow, &MainWindow::updateStatusPopoverAndIcon));
    }
  }
}

/**
 * Abort request call and stop the thread, if applicable.
 */
void Middleware::abortRequest()
{
  if (requestThread_ && requestThread_->joinable())
  {
    if (is_request_thread_done_)
    {
      requestThread_->join();
    }
    else
    {
      // Trigger the thread to stop now.
      // We call the abort method of the IPFS client.
      ipfs_fetch_.abort();
      keep_request_thread_running_ = false;
      requestThread_->join();
      // Reset states, allowing new threads with new API requests/calls
      ipfs_fetch_.reset();
      keep_request_thread_running_ = true;
    }
    delete requestThread_;
    requestThread_ = nullptr;
    is_request_thread_done_ = false; // reset
  }
}

/**
 * Abort status calls and stop the thread, if applicable.
 */
void Middleware::abortStatus()
{
  if (statusThread_ && statusThread_->joinable())
  {
    if (is_status_thread_done_)
    {
      statusThread_->join();
    }
    else
    {
      // Trigger the thread to stop now.
      // We call the abort method of the IPFS client.
      ipfs_status_.abort();
      statusThread_->join();
      // Reset states, allowing new threads with new API status calls
      ipfs_status_.reset();
    }
    delete statusThread_;
    statusThread_ = nullptr;
    is_status_thread_done_ = false; // reset
  }
}