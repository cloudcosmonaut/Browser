#include "ipfs-daemon.h"

#include <glibmm/shell.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/main.h>
#include <iostream>
#include <unistd.h>

#ifdef LEGACY_CXX
#include <experimental/filesystem>
namespace n_fs = ::std::experimental::filesystem;
#else
#include <filesystem>
namespace n_fs = ::std::filesystem;
#endif
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * \brief Spawn the IPFS daemon in an async manner using Glib. If needed under Linux (under Windows, it tries to start IPFS anyway).
 * \return True if process is started, otherwise false.
 */
void IPFSDaemon::spawn()
{
    // Check for PID under UNIX
    int daemonPID = IPFSDaemon::getRunningDaemonPID();
    // Is IPFS Daemon already running?
    if (daemonPID > 0)
    {
        std::cout << "INFO: IPFS Daemon is already running. Do not start another IPFS process." << std::endl;
    }
    else
    {
        std::string command = IPFSDaemon::locateIPFSBinary();
        if (n_fs::exists(command))
        {
            // IPFS command
            Glib::ArrayHandle<std::string> argv = Glib::shell_parse_argv(command + " daemon --init --migrate");

            // Spawn flags
            // Disable stdout/stderr to default terminal. Don't reaped the child automatically
            // We could add "Glib::SPAWN_SEARCH_PATH", if you want to search the binary in the PATH
            Glib::SpawnFlags flags = Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_DO_NOT_REAP_CHILD;

            // Start IPFS, using spawn_async,
            // optionally we can use spawn_async_with_pipes(), to retrieve stdout & stderr to specified output buffers
            Glib::spawn_async(this->workingDir, argv, flags, Glib::SlotSpawnChildSetup(), &this->pid);
            this->isRunning = true;
            this->childWatchHandler = Glib::signal_child_watch().connect(sigc::mem_fun(*this, &IPFSDaemon::child_watch_exit), this->pid);
        }
        else
        {
            std::cerr << "Error: IPFS Daemon is not found. IPFS will not work!" << std::endl;
        }
    }
}

/**
 * \brief Stop process manually
 */
void IPFSDaemon::stop()
{
    if (!this->isRunning)
        return; // It's already stopped

    Glib::spawn_close_pid(pid);
    this->isRunning = false;
}

/**
 * \brief Exit signal handler for the process.
 * Emits the exited signal with the status code.
 */
void IPFSDaemon::child_watch_exit(Glib::Pid pid, int childStatus)
{
    std::cout << "WARN: IPFS Daemon exited, with status: " << childStatus << std::endl;
    Glib::spawn_close_pid(pid);

    // Disconnect from signal
    if (this->childWatchHandler.connected())
        this->childWatchHandler.disconnect();

    this->isRunning = false;

    // Emit exit signal with status code
    exited.emit(childStatus);
}

/**
 * \brief Get Process ID (PID)
 * \return PID
 */
int IPFSDaemon::getPID() const
{
    if (!this->isRunning)
        return 0;
#ifdef _WIN32
    return GetProcessId(pid);
#else
    return pid;
#endif
}

/**
 * \brief Try to locate the ipfs binary path (IPFS go server)
 * \return full path to the ipfs binary, empty string when not found
 */
std::string IPFSDaemon::locateIPFSBinary()
{
    std::string binaryName = "ipfs";
#ifdef _WIN32
    binaryName += ".exe";
#endif
    // Try absolute path first
    for (std::string data_dir : Glib::get_system_data_dirs())
    {
        std::vector<std::string> path_builder{data_dir, "libreweb-browser", "go-ipfs", binaryName};
        std::string ipfs_binary_path = Glib::build_path(G_DIR_SEPARATOR_S, path_builder);
        if (Glib::file_test(ipfs_binary_path, Glib::FileTest::FILE_TEST_IS_EXECUTABLE))
        {
            return ipfs_binary_path;
        }
    }

    // Try local path if the images are not installed (yet)
    // When working directory is in the build/bin folder (relative path)
    std::string currentPath = n_fs::current_path().string();
    std::string ipfs_binary_path = Glib::build_filename(currentPath, "../..", "go-ipfs", binaryName);
    if (Glib::file_test(ipfs_binary_path, Glib::FileTest::FILE_TEST_IS_EXECUTABLE))
    {
        return ipfs_binary_path;
    }
    else
    {
        return "";
    }
}

/**
 * \brief Retrieve IPFS Daemon PID for **UNIX only** (zero if non-existent)
 * \return Process ID (0 of non-existent)
 */
int IPFSDaemon::getRunningDaemonPID()
{
    int pid = 0;
#ifdef __linux__
    int exitCode = -3;
    std::string stdout;
    Glib::spawn_command_line_sync("pidof -s ipfs", &stdout, nullptr, &exitCode);
    // Process exists
    if (exitCode == 0)
    {
        pid = std::stoi(stdout);
    }
#endif
    return pid;
}

/**
 * \brief Determine if we need to kill any running IPFS process (UNIX only)
 * \return true if it needs to be terminated, otherwise false
 */
bool IPFSDaemon::shouldProcessTerminated()
{
#ifdef __linux__
    char pathbuf[1024];
    memset(pathbuf, 0, sizeof(pathbuf));
    std::string path = "/proc/" + std::to_string(this->pid) + "/exe";
    if (readlink(path.c_str(), pathbuf, sizeof(pathbuf) - 1) > 0)
    {
        char beginPath[] = "/usr/share/libreweb-browser";
        // If the begin path does not match (!= 0), return true,
        // meaning the process will be killed.
        bool shouldKill = (strncmp(pathbuf, beginPath, strlen(beginPath)) != 0);

        // Also check the IPFS version
        std::string expectedString = "version 0.11.0";
        std::string stdout;
        Glib::spawn_command_line_sync(path + " version", &stdout);
        // Current running IPFS version matches our IPFS version, keep process running afterall
        if (stdout.find(expectedString) != std::string::npos)
        {
            shouldKill = false;
        }
        return shouldKill;
    }
#endif
    return false; // fallback; do not kill
}